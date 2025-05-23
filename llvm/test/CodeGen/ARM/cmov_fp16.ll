; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=thumbv8.1m.main-none-none-eabi -mattr=+fullfp16 %s -o - | FileCheck %s --check-prefixes CHECK,CHECK-THUMB
; RUN: llc -mtriple=armv8.2a-arm-none-eabi -mattr=+fullfp16 %s -o - | FileCheck %s --check-prefixes CHECK,CHECK-ARM

define i32 @test_ne(i32 %x, i32 %y, i32 %a, i32 %b) {
; CHECK-LABEL: test_ne:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov s0, r0
; CHECK-NEXT:    cmp r2, r3
; CHECK-NEXT:    vmov s2, r1
; CHECK-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-NEXT:    vseleq.f16 s0, s2, s0
; CHECK-NEXT:    vmov.f16 r0, s0
; CHECK-NEXT:    bx lr
entry:
  %x.half = uitofp i32 %x to half
  %y.half = uitofp i32 %y to half
  %cmp = icmp ne i32 %a, %b
  %cond = select i1 %cmp, half %x.half, half %y.half
  %0 = bitcast half %cond to i16
  %1 = zext i16 %0 to i32
  ret i32 %1
}

define i32 @test_eq(i32 %x, i32 %y, i32 %a, i32 %b) {
; CHECK-LABEL: test_eq:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov s0, r1
; CHECK-NEXT:    cmp r2, r3
; CHECK-NEXT:    vmov s2, r0
; CHECK-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-NEXT:    vseleq.f16 s0, s2, s0
; CHECK-NEXT:    vmov.f16 r0, s0
; CHECK-NEXT:    bx lr
entry:
  %x.half = uitofp i32 %x to half
  %y.half = uitofp i32 %y to half
  %cmp = icmp eq i32 %a, %b
  %cond = select i1 %cmp, half %x.half, half %y.half
  %0 = bitcast half %cond to i16
  %1 = zext i16 %0 to i32
  ret i32 %1
}

define i32 @test_gt(i32 %x, i32 %y, i32 %a, i32 %b) {
; CHECK-LABEL: test_gt:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov s0, r1
; CHECK-NEXT:    cmp r2, r3
; CHECK-NEXT:    vmov s2, r0
; CHECK-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-NEXT:    vselgt.f16 s0, s2, s0
; CHECK-NEXT:    vmov.f16 r0, s0
; CHECK-NEXT:    bx lr
entry:
  %x.half = uitofp i32 %x to half
  %y.half = uitofp i32 %y to half
  %cmp = icmp sgt i32 %a, %b
  %cond = select i1 %cmp, half %x.half, half %y.half
  %0 = bitcast half %cond to i16
  %1 = zext i16 %0 to i32
  ret i32 %1
}

define i32 @test_ge(i32 %x, i32 %y, i32 %a, i32 %b) {
; CHECK-LABEL: test_ge:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov s0, r1
; CHECK-NEXT:    cmp r2, r3
; CHECK-NEXT:    vmov s2, r0
; CHECK-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-NEXT:    vselge.f16 s0, s2, s0
; CHECK-NEXT:    vmov.f16 r0, s0
; CHECK-NEXT:    bx lr
entry:
  %x.half = uitofp i32 %x to half
  %y.half = uitofp i32 %y to half
  %cmp = icmp sge i32 %a, %b
  %cond = select i1 %cmp, half %x.half, half %y.half
  %0 = bitcast half %cond to i16
  %1 = zext i16 %0 to i32
  ret i32 %1
}

define i32 @test_lt(i32 %x, i32 %y, i32 %a, i32 %b) {
; CHECK-LABEL: test_lt:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov s0, r0
; CHECK-NEXT:    cmp r2, r3
; CHECK-NEXT:    vmov s2, r1
; CHECK-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-NEXT:    vselge.f16 s0, s2, s0
; CHECK-NEXT:    vmov.f16 r0, s0
; CHECK-NEXT:    bx lr
entry:
  %x.half = uitofp i32 %x to half
  %y.half = uitofp i32 %y to half
  %cmp = icmp slt i32 %a, %b
  %cond = select i1 %cmp, half %x.half, half %y.half
  %0 = bitcast half %cond to i16
  %1 = zext i16 %0 to i32
  ret i32 %1
}

define i32 @test_le(i32 %x, i32 %y, i32 %a, i32 %b) {
; CHECK-LABEL: test_le:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov s0, r0
; CHECK-NEXT:    cmp r2, r3
; CHECK-NEXT:    vmov s2, r1
; CHECK-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-NEXT:    vselgt.f16 s0, s2, s0
; CHECK-NEXT:    vmov.f16 r0, s0
; CHECK-NEXT:    bx lr
entry:
  %x.half = uitofp i32 %x to half
  %y.half = uitofp i32 %y to half
  %cmp = icmp sle i32 %a, %b
  %cond = select i1 %cmp, half %x.half, half %y.half
  %0 = bitcast half %cond to i16
  %1 = zext i16 %0 to i32
  ret i32 %1
}

define i32 @test_hi(i32 %x, i32 %y, i32 %a, i32 %b) {
; CHECK-THUMB-LABEL: test_hi:
; CHECK-THUMB:       @ %bb.0: @ %entry
; CHECK-THUMB-NEXT:    vmov s2, r1
; CHECK-THUMB-NEXT:    cmp r2, r3
; CHECK-THUMB-NEXT:    vmov s0, r0
; CHECK-THUMB-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-THUMB-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-THUMB-NEXT:    it hi
; CHECK-THUMB-NEXT:    vmovhi.f32 s2, s0
; CHECK-THUMB-NEXT:    vmov.f16 r0, s2
; CHECK-THUMB-NEXT:    bx lr
;
; CHECK-ARM-LABEL: test_hi:
; CHECK-ARM:       @ %bb.0: @ %entry
; CHECK-ARM-NEXT:    vmov s0, r0
; CHECK-ARM-NEXT:    cmp r2, r3
; CHECK-ARM-NEXT:    vmov s2, r1
; CHECK-ARM-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-ARM-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-ARM-NEXT:    vmovhi.f32 s2, s0
; CHECK-ARM-NEXT:    vmov.f16 r0, s2
; CHECK-ARM-NEXT:    bx lr
entry:
  %x.half = uitofp i32 %x to half
  %y.half = uitofp i32 %y to half
  %cmp = icmp ugt i32 %a, %b
  %cond = select i1 %cmp, half %x.half, half %y.half
  %0 = bitcast half %cond to i16
  %1 = zext i16 %0 to i32
  ret i32 %1
}

define i32 @test_hs(i32 %x, i32 %y, i32 %a, i32 %b) {
; CHECK-THUMB-LABEL: test_hs:
; CHECK-THUMB:       @ %bb.0: @ %entry
; CHECK-THUMB-NEXT:    vmov s2, r1
; CHECK-THUMB-NEXT:    cmp r2, r3
; CHECK-THUMB-NEXT:    vmov s0, r0
; CHECK-THUMB-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-THUMB-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-THUMB-NEXT:    it hs
; CHECK-THUMB-NEXT:    vmovhs.f32 s2, s0
; CHECK-THUMB-NEXT:    vmov.f16 r0, s2
; CHECK-THUMB-NEXT:    bx lr
;
; CHECK-ARM-LABEL: test_hs:
; CHECK-ARM:       @ %bb.0: @ %entry
; CHECK-ARM-NEXT:    vmov s0, r0
; CHECK-ARM-NEXT:    cmp r2, r3
; CHECK-ARM-NEXT:    vmov s2, r1
; CHECK-ARM-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-ARM-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-ARM-NEXT:    vmovhs.f32 s2, s0
; CHECK-ARM-NEXT:    vmov.f16 r0, s2
; CHECK-ARM-NEXT:    bx lr
entry:
  %x.half = uitofp i32 %x to half
  %y.half = uitofp i32 %y to half
  %cmp = icmp uge i32 %a, %b
  %cond = select i1 %cmp, half %x.half, half %y.half
  %0 = bitcast half %cond to i16
  %1 = zext i16 %0 to i32
  ret i32 %1
}

define i32 @test_lo(i32 %x, i32 %y, i32 %a, i32 %b) {
; CHECK-THUMB-LABEL: test_lo:
; CHECK-THUMB:       @ %bb.0: @ %entry
; CHECK-THUMB-NEXT:    vmov s2, r1
; CHECK-THUMB-NEXT:    cmp r2, r3
; CHECK-THUMB-NEXT:    vmov s0, r0
; CHECK-THUMB-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-THUMB-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-THUMB-NEXT:    it lo
; CHECK-THUMB-NEXT:    vmovlo.f32 s2, s0
; CHECK-THUMB-NEXT:    vmov.f16 r0, s2
; CHECK-THUMB-NEXT:    bx lr
;
; CHECK-ARM-LABEL: test_lo:
; CHECK-ARM:       @ %bb.0: @ %entry
; CHECK-ARM-NEXT:    vmov s0, r0
; CHECK-ARM-NEXT:    cmp r2, r3
; CHECK-ARM-NEXT:    vmov s2, r1
; CHECK-ARM-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-ARM-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-ARM-NEXT:    vmovlo.f32 s2, s0
; CHECK-ARM-NEXT:    vmov.f16 r0, s2
; CHECK-ARM-NEXT:    bx lr
entry:
  %x.half = uitofp i32 %x to half
  %y.half = uitofp i32 %y to half
  %cmp = icmp ult i32 %a, %b
  %cond = select i1 %cmp, half %x.half, half %y.half
  %0 = bitcast half %cond to i16
  %1 = zext i16 %0 to i32
  ret i32 %1
}

define i32 @test_ls(i32 %x, i32 %y, i32 %a, i32 %b) {
; CHECK-THUMB-LABEL: test_ls:
; CHECK-THUMB:       @ %bb.0: @ %entry
; CHECK-THUMB-NEXT:    vmov s2, r1
; CHECK-THUMB-NEXT:    cmp r2, r3
; CHECK-THUMB-NEXT:    vmov s0, r0
; CHECK-THUMB-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-THUMB-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-THUMB-NEXT:    it ls
; CHECK-THUMB-NEXT:    vmovls.f32 s2, s0
; CHECK-THUMB-NEXT:    vmov.f16 r0, s2
; CHECK-THUMB-NEXT:    bx lr
;
; CHECK-ARM-LABEL: test_ls:
; CHECK-ARM:       @ %bb.0: @ %entry
; CHECK-ARM-NEXT:    vmov s0, r0
; CHECK-ARM-NEXT:    cmp r2, r3
; CHECK-ARM-NEXT:    vmov s2, r1
; CHECK-ARM-NEXT:    vcvt.f16.u32 s0, s0
; CHECK-ARM-NEXT:    vcvt.f16.u32 s2, s2
; CHECK-ARM-NEXT:    vmovls.f32 s2, s0
; CHECK-ARM-NEXT:    vmov.f16 r0, s2
; CHECK-ARM-NEXT:    bx lr
entry:
  %x.half = uitofp i32 %x to half
  %y.half = uitofp i32 %y to half
  %cmp = icmp ule i32 %a, %b
  %cond = select i1 %cmp, half %x.half, half %y.half
  %0 = bitcast half %cond to i16
  %1 = zext i16 %0 to i32
  ret i32 %1
}

