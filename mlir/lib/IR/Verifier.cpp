//===- Verifier.cpp - MLIR Verifier Implementation ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the verify() methods on the various IR types, performing
// (potentially expensive) checks on the holistic structure of the code.  This
// can be used for detecting bugs in compiler transformations and hand written
// .mlir files.
//
// The checks in this file are only for things that can occur as part of IR
// transformations: e.g. violation of dominance information, malformed operation
// attributes, etc.  MLIR supports transformations moving IR through locally
// invalid states (e.g. unlinking an operation from a block before re-inserting
// it in a new place), but each transformation must complete with the IR in a
// valid form.
//
// This should not check for things that are always wrong by construction (e.g.
// attributes or other immutable structures that are incorrect), because those
// are not mutable and can be checked at time of construction.
//
//===----------------------------------------------------------------------===//

#include "mlir/IR/Verifier.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/Dominance.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/RegionKindInterface.h"
#include "mlir/IR/Threading.h"
#include "llvm/ADT/PointerIntPair.h"
#include <optional>

using namespace mlir;

namespace {
/// This class encapsulates all the state used to verify an operation region.
class OperationVerifier {
public:
  /// If `verifyRecursively` is true, then this will also recursively verify
  /// nested operations.
  explicit OperationVerifier(bool verifyRecursively)
      : verifyRecursively(verifyRecursively) {}

  /// Verify the given operation.
  LogicalResult verifyOpAndDominance(Operation &op);

private:
  using WorkItem = llvm::PointerUnion<Operation *, Block *>;
  using WorkItemEntry = llvm::PointerIntPair<WorkItem, 1, bool>;

  /// This verifier uses a DFS of the tree of operations/blocks. The method
  /// verifyOnEntrance is invoked when we visit a node for the first time, i.e.
  /// before visiting its children. The method verifyOnExit is invoked
  /// upon exit from the subtree, i.e. when we visit a node for the second time.
  LogicalResult verifyOnEntrance(Block &block);
  LogicalResult verifyOnEntrance(Operation &op);

  LogicalResult verifyOnExit(Block &block);
  LogicalResult verifyOnExit(Operation &op);

  /// Verify the properties and dominance relationships of this operation.
  LogicalResult verifyOperation(Operation &op);

  /// Verify the dominance property of regions contained within the given
  /// Operation.
  LogicalResult verifyDominanceOfContainedRegions(Operation &op,
                                                  DominanceInfo &domInfo);

  /// A flag indicating if this verifier should recursively verify nested
  /// operations.
  bool verifyRecursively;
};
} // namespace

LogicalResult OperationVerifier::verifyOpAndDominance(Operation &op) {
  // Verify the operation first, collecting any IsolatedFromAbove operations.
  if (failed(verifyOperation(op)))
    return failure();

  // Since everything looks structurally ok to this point, we do a dominance
  // check for any nested regions. We do this as a second pass since malformed
  // CFG's can cause dominator analysis construction to crash and we want the
  // verifier to be resilient to malformed code.
  if (op.getNumRegions() != 0) {
    DominanceInfo domInfo;
    if (failed(verifyDominanceOfContainedRegions(op, domInfo)))
      return failure();
  }

  return success();
}

/// Returns true if this block may be valid without terminator. That is if:
/// - it does not have a parent region.
/// - Or the parent region have a single block and:
///    - This region does not have a parent op.
///    - Or the parent op is unregistered.
///    - Or the parent op has the NoTerminator trait.
static bool mayBeValidWithoutTerminator(Block *block) {
  if (!block->getParent())
    return true;
  if (!llvm::hasSingleElement(*block->getParent()))
    return false;
  Operation *op = block->getParentOp();
  return !op || op->mightHaveTrait<OpTrait::NoTerminator>();
}

LogicalResult OperationVerifier::verifyOnEntrance(Block &block) {
  for (auto arg : block.getArguments())
    if (arg.getOwner() != &block)
      return emitError(arg.getLoc(), "block argument not owned by block");

  // Verify that this block has a terminator.
  if (block.empty()) {
    if (mayBeValidWithoutTerminator(&block))
      return success();
    return emitError(block.getParent()->getLoc(),
                     "empty block: expect at least a terminator");
  }

  // Check each operation, and make sure there are no branches out of the
  // middle of this block.
  for (Operation &op : block) {
    // Only the last instructions is allowed to have successors.
    if (op.getNumSuccessors() != 0 && &op != &block.back())
      return op.emitError(
          "operation with block successors must terminate its parent block");
  }

  return success();
}

LogicalResult OperationVerifier::verifyOnExit(Block &block) {
  // Verify that this block is not branching to a block of a different
  // region.
  for (Block *successor : block.getSuccessors())
    if (successor->getParent() != block.getParent())
      return block.back().emitOpError(
          "branching to block of a different region");

  // If this block doesn't have to have a terminator, don't require it.
  if (mayBeValidWithoutTerminator(&block))
    return success();

  Operation &terminator = block.back();
  if (!terminator.mightHaveTrait<OpTrait::IsTerminator>())
    return block.back().emitError("block with no terminator, has ")
           << terminator;

  return success();
}

LogicalResult OperationVerifier::verifyOnEntrance(Operation &op) {
  // Check that operands are non-nil and structurally ok.
  for (auto operand : op.getOperands())
    if (!operand)
      return op.emitError("null operand found");

  /// Verify that all of the attributes are okay.
  for (auto attr : op.getDiscardableAttrDictionary()) {
    // Check for any optional dialect specific attributes.
    if (auto *dialect = attr.getNameDialect())
      if (failed(dialect->verifyOperationAttribute(&op, attr)))
        return failure();
  }

  // If we can get operation info for this, check the custom hook.
  OperationName opName = op.getName();
  std::optional<RegisteredOperationName> registeredInfo =
      opName.getRegisteredInfo();
  if (registeredInfo && failed(registeredInfo->verifyInvariants(&op)))
    return failure();

  unsigned numRegions = op.getNumRegions();
  if (!numRegions)
    return success();
  auto kindInterface = dyn_cast<RegionKindInterface>(&op);
  // Verify that all child regions are ok.
  MutableArrayRef<Region> regions = op.getRegions();
  for (unsigned i = 0; i < numRegions; ++i) {
    Region &region = regions[i];
    RegionKind kind =
        kindInterface ? kindInterface.getRegionKind(i) : RegionKind::SSACFG;
    // Check that Graph Regions only have a single basic block. This is
    // similar to the code in SingleBlockImplicitTerminator, but doesn't
    // require the trait to be specified. This arbitrary limitation is
    // designed to limit the number of cases that have to be handled by
    // transforms and conversions.
    if (op.isRegistered() && kind == RegionKind::Graph) {
      // Non-empty regions must contain a single basic block.
      if (!region.empty() && !region.hasOneBlock())
        return op.emitOpError("expects graph region #")
               << i << " to have 0 or 1 blocks";
    }

    if (region.empty())
      continue;

    // Verify the first block has no predecessors.
    Block *firstBB = &region.front();
    if (!firstBB->hasNoPredecessors())
      return emitError(op.getLoc(),
                       "entry block of region may not have predecessors");
  }
  return success();
}

LogicalResult OperationVerifier::verifyOnExit(Operation &op) {
  SmallVector<Operation *> opsWithIsolatedRegions;
  if (verifyRecursively) {
    for (Region &region : op.getRegions())
      for (Block &block : region)
        for (Operation &o : block)
          if (o.getNumRegions() != 0 &&
              o.hasTrait<OpTrait::IsIsolatedFromAbove>())
            opsWithIsolatedRegions.push_back(&o);
  }

  std::atomic<bool> opFailedVerify = false;
  parallelForEach(op.getContext(), opsWithIsolatedRegions, [&](Operation *o) {
    if (failed(verifyOpAndDominance(*o)))
      opFailedVerify.store(true, std::memory_order_relaxed);
  });
  if (opFailedVerify.load(std::memory_order_relaxed))
    return failure();

  OperationName opName = op.getName();
  std::optional<RegisteredOperationName> registeredInfo =
      opName.getRegisteredInfo();
  // After the region ops are verified, run the verifiers that have additional
  // region invariants need to veirfy.
  if (registeredInfo && failed(registeredInfo->verifyRegionInvariants(&op)))
    return failure();

  // If this is a registered operation, there is nothing left to do.
  if (registeredInfo)
    return success();

  // Otherwise, verify that the parent dialect allows un-registered operations.
  Dialect *dialect = opName.getDialect();
  if (!dialect) {
    if (!op.getContext()->allowsUnregisteredDialects()) {
      return op.emitOpError()
             << "created with unregistered dialect. If this is "
                "intended, please call allowUnregisteredDialects() on the "
                "MLIRContext, or use -allow-unregistered-dialect with "
                "the MLIR opt tool used";
    }
    return success();
  }

  if (!dialect->allowsUnknownOperations()) {
    return op.emitError("unregistered operation '")
           << op.getName() << "' found in dialect ('" << dialect->getNamespace()
           << "') that does not allow unknown operations";
  }

  return success();
}

/// Verify the properties and dominance relationships of this operation,
/// stopping region "recursion" at any "isolated from above operations".
/// Such ops are collected separately and verified inside
/// verifyBlockPostChildren.
LogicalResult OperationVerifier::verifyOperation(Operation &op) {
  SmallVector<WorkItemEntry> worklist{{&op, false}};
  while (!worklist.empty()) {
    WorkItemEntry &top = worklist.back();

    auto visit = [](auto &&visitor, WorkItem w) {
      if (auto *o = dyn_cast<Operation *>(w))
        return visitor(o);
      return visitor(cast<Block *>(w));
    };

    const bool isExit = top.getInt();
    top.setInt(true);
    auto item = top.getPointer();

    // 2nd visit of this work item ("exit").
    if (isExit) {
      if (failed(
              visit([this](auto *workItem) { return verifyOnExit(*workItem); },
                    item)))
        return failure();
      worklist.pop_back();
      continue;
    }

    // 1st visit of this work item ("entrance").
    if (failed(visit(
            [this](auto *workItem) { return verifyOnEntrance(*workItem); },
            item)))
      return failure();

    if (Block *currentBlock = dyn_cast<Block *>(item)) {
      // Skip "isolated from above operations".
      for (Operation &o : llvm::reverse(*currentBlock)) {
        if (o.getNumRegions() == 0 ||
            !o.hasTrait<OpTrait::IsIsolatedFromAbove>())
          worklist.emplace_back(&o);
      }
      continue;
    }

    Operation &currentOp = *cast<Operation *>(item);
    if (verifyRecursively)
      for (Region &region : llvm::reverse(currentOp.getRegions()))
        for (Block &block : llvm::reverse(region))
          worklist.emplace_back(&block);
  }
  return success();
}

//===----------------------------------------------------------------------===//
// Dominance Checking
//===----------------------------------------------------------------------===//

/// Emit an error when the specified operand of the specified operation is an
/// invalid use because of dominance properties.
static void diagnoseInvalidOperandDominance(Operation &op, unsigned operandNo) {
  InFlightDiagnostic diag = op.emitError("operand #")
                            << operandNo << " does not dominate this use";

  Value operand = op.getOperand(operandNo);

  /// Attach a note to an in-flight diagnostic that provide more information
  /// about where an op operand is defined.
  if (auto *useOp = operand.getDefiningOp()) {
    Diagnostic &note = diag.attachNote(useOp->getLoc());
    note << "operand defined here";
    Block *block1 = op.getBlock();
    Block *block2 = useOp->getBlock();
    Region *region1 = block1->getParent();
    Region *region2 = block2->getParent();
    if (block1 == block2)
      note << " (op in the same block)";
    else if (region1 == region2)
      note << " (op in the same region)";
    else if (region2->isProperAncestor(region1))
      note << " (op in a parent region)";
    else if (region1->isProperAncestor(region2))
      note << " (op in a child region)";
    else
      note << " (op is neither in a parent nor in a child region)";
    return;
  }
  // Block argument case.
  Block *block1 = op.getBlock();
  Block *block2 = llvm::cast<BlockArgument>(operand).getOwner();
  Region *region1 = block1->getParent();
  Region *region2 = block2->getParent();
  Location loc = UnknownLoc::get(op.getContext());
  if (block2->getParentOp())
    loc = block2->getParentOp()->getLoc();
  Diagnostic &note = diag.attachNote(loc);
  if (!region2) {
    note << " (block without parent)";
    return;
  }
  if (block1 == block2)
    llvm::report_fatal_error("Internal error in dominance verification");
  int index = std::distance(region2->begin(), block2->getIterator());
  note << "operand defined as a block argument (block #" << index;
  if (region1 == region2)
    note << " in the same region)";
  else if (region2->isProperAncestor(region1))
    note << " in a parent region)";
  else if (region1->isProperAncestor(region2))
    note << " in a child region)";
  else
    note << " neither in a parent nor in a child region)";
}

/// Verify the dominance of each of the nested blocks within the given operation
LogicalResult
OperationVerifier::verifyDominanceOfContainedRegions(Operation &op,
                                                     DominanceInfo &domInfo) {
  llvm::SmallVector<Operation *, 8> worklist{&op};
  while (!worklist.empty()) {
    auto *op = worklist.pop_back_val();
    for (auto &region : op->getRegions())
      for (auto &block : region.getBlocks()) {
        // Dominance is only meaningful inside reachable blocks.
        bool isReachable = domInfo.isReachableFromEntry(&block);
        for (auto &op : block) {
          if (isReachable) {
            // Check that operands properly dominate this use.
            for (const auto &operand : llvm::enumerate(op.getOperands())) {
              if (domInfo.properlyDominates(operand.value(), &op))
                continue;

              diagnoseInvalidOperandDominance(op, operand.index());
              return failure();
            }
          }

          // Recursively verify dominance within each operation in the block,
          // even if the block itself is not reachable, or we are in a region
          // which doesn't respect dominance.
          if (verifyRecursively && op.getNumRegions() != 0) {
            // If this operation is IsolatedFromAbove, then we'll handle it in
            // the outer verification loop.
            if (op.hasTrait<OpTrait::IsIsolatedFromAbove>())
              continue;
            worklist.push_back(&op);
          }
        }
      }
  }

  return success();
}

//===----------------------------------------------------------------------===//
// Entrypoint
//===----------------------------------------------------------------------===//

LogicalResult mlir::verify(Operation *op, bool verifyRecursively) {
  OperationVerifier verifier(verifyRecursively);
  return verifier.verifyOpAndDominance(*op);
}
