; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown-unknown | FileCheck %s

define <4 x i32> @foo(<4 x i32> %t, <4 x i32> %u) nounwind {
; CHECK-LABEL: foo:
; CHECK:       # %bb.0:
; CHECK-NEXT:    pshufd {{.*#+}} xmm2 = xmm0[3,3,3,3]
; CHECK-NEXT:    movd %xmm2, %eax
; CHECK-NEXT:    pshufd {{.*#+}} xmm2 = xmm1[3,3,3,3]
; CHECK-NEXT:    movd %xmm2, %ecx
; CHECK-NEXT:    cltd
; CHECK-NEXT:    idivl %ecx
; CHECK-NEXT:    movd %edx, %xmm2
; CHECK-NEXT:    pshufd {{.*#+}} xmm3 = xmm0[2,3,2,3]
; CHECK-NEXT:    movd %xmm3, %eax
; CHECK-NEXT:    pshufd {{.*#+}} xmm3 = xmm1[2,3,2,3]
; CHECK-NEXT:    movd %xmm3, %ecx
; CHECK-NEXT:    cltd
; CHECK-NEXT:    idivl %ecx
; CHECK-NEXT:    movd %edx, %xmm3
; CHECK-NEXT:    punpckldq {{.*#+}} xmm3 = xmm3[0],xmm2[0],xmm3[1],xmm2[1]
; CHECK-NEXT:    movd %xmm0, %eax
; CHECK-NEXT:    movd %xmm1, %ecx
; CHECK-NEXT:    cltd
; CHECK-NEXT:    idivl %ecx
; CHECK-NEXT:    movd %edx, %xmm2
; CHECK-NEXT:    pshufd {{.*#+}} xmm0 = xmm0[1,1,1,1]
; CHECK-NEXT:    movd %xmm0, %eax
; CHECK-NEXT:    pshufd {{.*#+}} xmm0 = xmm1[1,1,1,1]
; CHECK-NEXT:    movd %xmm0, %ecx
; CHECK-NEXT:    cltd
; CHECK-NEXT:    idivl %ecx
; CHECK-NEXT:    movd %edx, %xmm0
; CHECK-NEXT:    punpckldq {{.*#+}} xmm2 = xmm2[0],xmm0[0],xmm2[1],xmm0[1]
; CHECK-NEXT:    punpcklqdq {{.*#+}} xmm2 = xmm2[0],xmm3[0]
; CHECK-NEXT:    movdqa %xmm2, %xmm0
; CHECK-NEXT:    retq
  %m = srem <4 x i32> %t, %u
  ret <4 x i32> %m
}

define <4 x i32> @bar(<4 x i32> %t, <4 x i32> %u) nounwind {
; CHECK-LABEL: bar:
; CHECK:       # %bb.0:
; CHECK-NEXT:    pshufd {{.*#+}} xmm2 = xmm0[3,3,3,3]
; CHECK-NEXT:    movd %xmm2, %eax
; CHECK-NEXT:    pshufd {{.*#+}} xmm2 = xmm1[3,3,3,3]
; CHECK-NEXT:    movd %xmm2, %ecx
; CHECK-NEXT:    xorl %edx, %edx
; CHECK-NEXT:    divl %ecx
; CHECK-NEXT:    movd %edx, %xmm2
; CHECK-NEXT:    pshufd {{.*#+}} xmm3 = xmm0[2,3,2,3]
; CHECK-NEXT:    movd %xmm3, %eax
; CHECK-NEXT:    pshufd {{.*#+}} xmm3 = xmm1[2,3,2,3]
; CHECK-NEXT:    movd %xmm3, %ecx
; CHECK-NEXT:    xorl %edx, %edx
; CHECK-NEXT:    divl %ecx
; CHECK-NEXT:    movd %edx, %xmm3
; CHECK-NEXT:    punpckldq {{.*#+}} xmm3 = xmm3[0],xmm2[0],xmm3[1],xmm2[1]
; CHECK-NEXT:    movd %xmm0, %eax
; CHECK-NEXT:    movd %xmm1, %ecx
; CHECK-NEXT:    xorl %edx, %edx
; CHECK-NEXT:    divl %ecx
; CHECK-NEXT:    movd %edx, %xmm2
; CHECK-NEXT:    pshufd {{.*#+}} xmm0 = xmm0[1,1,1,1]
; CHECK-NEXT:    movd %xmm0, %eax
; CHECK-NEXT:    pshufd {{.*#+}} xmm0 = xmm1[1,1,1,1]
; CHECK-NEXT:    movd %xmm0, %ecx
; CHECK-NEXT:    xorl %edx, %edx
; CHECK-NEXT:    divl %ecx
; CHECK-NEXT:    movd %edx, %xmm0
; CHECK-NEXT:    punpckldq {{.*#+}} xmm2 = xmm2[0],xmm0[0],xmm2[1],xmm0[1]
; CHECK-NEXT:    punpcklqdq {{.*#+}} xmm2 = xmm2[0],xmm3[0]
; CHECK-NEXT:    movdqa %xmm2, %xmm0
; CHECK-NEXT:    retq
  %m = urem <4 x i32> %t, %u
  ret <4 x i32> %m
}

define <4 x float> @qux(<4 x float> %t, <4 x float> %u) nounwind {
; CHECK-LABEL: qux:
; CHECK:       # %bb.0:
; CHECK-NEXT:    subq $72, %rsp
; CHECK-NEXT:    movaps %xmm1, {{[-0-9]+}}(%r{{[sb]}}p) # 16-byte Spill
; CHECK-NEXT:    movaps %xmm0, {{[-0-9]+}}(%r{{[sb]}}p) # 16-byte Spill
; CHECK-NEXT:    shufps {{.*#+}} xmm0 = xmm0[3,3,3,3]
; CHECK-NEXT:    shufps {{.*#+}} xmm1 = xmm1[3,3,3,3]
; CHECK-NEXT:    callq fmodf
; CHECK-NEXT:    movaps %xmm0, (%rsp) # 16-byte Spill
; CHECK-NEXT:    movaps {{[-0-9]+}}(%r{{[sb]}}p), %xmm0 # 16-byte Reload
; CHECK-NEXT:    movhlps {{.*#+}} xmm0 = xmm0[1,1]
; CHECK-NEXT:    movaps {{[-0-9]+}}(%r{{[sb]}}p), %xmm1 # 16-byte Reload
; CHECK-NEXT:    movhlps {{.*#+}} xmm1 = xmm1[1,1]
; CHECK-NEXT:    callq fmodf
; CHECK-NEXT:    unpcklps (%rsp), %xmm0 # 16-byte Folded Reload
; CHECK-NEXT:    # xmm0 = xmm0[0],mem[0],xmm0[1],mem[1]
; CHECK-NEXT:    movaps %xmm0, (%rsp) # 16-byte Spill
; CHECK-NEXT:    movaps {{[-0-9]+}}(%r{{[sb]}}p), %xmm0 # 16-byte Reload
; CHECK-NEXT:    movaps {{[-0-9]+}}(%r{{[sb]}}p), %xmm1 # 16-byte Reload
; CHECK-NEXT:    callq fmodf
; CHECK-NEXT:    movaps %xmm0, {{[-0-9]+}}(%r{{[sb]}}p) # 16-byte Spill
; CHECK-NEXT:    movaps {{[-0-9]+}}(%r{{[sb]}}p), %xmm0 # 16-byte Reload
; CHECK-NEXT:    shufps {{.*#+}} xmm0 = xmm0[1,1,1,1]
; CHECK-NEXT:    movaps {{[-0-9]+}}(%r{{[sb]}}p), %xmm1 # 16-byte Reload
; CHECK-NEXT:    shufps {{.*#+}} xmm1 = xmm1[1,1,1,1]
; CHECK-NEXT:    callq fmodf
; CHECK-NEXT:    movaps {{[-0-9]+}}(%r{{[sb]}}p), %xmm1 # 16-byte Reload
; CHECK-NEXT:    unpcklps {{.*#+}} xmm1 = xmm1[0],xmm0[0],xmm1[1],xmm0[1]
; CHECK-NEXT:    unpcklpd (%rsp), %xmm1 # 16-byte Folded Reload
; CHECK-NEXT:    # xmm1 = xmm1[0],mem[0]
; CHECK-NEXT:    movaps %xmm1, %xmm0
; CHECK-NEXT:    addq $72, %rsp
; CHECK-NEXT:    retq
  %m = frem <4 x float> %t, %u
  ret <4 x float> %m
}
