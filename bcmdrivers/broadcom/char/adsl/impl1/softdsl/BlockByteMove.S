/*
<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
#include "MipsAsm.h"
	
/*
Public	void
BlockByteMove (int size, Byte* srcPtr, Byte* dstPtr)
	*****************************************************************************
	*	a0 -- size																*
	*	a1 -- srcPtr
	*	a2 -- dstPtr	
	*****************************************************************************
*/
#if defined(bcm47xx) && defined(USE_FAST_TEXT)
	.section .fast_text,"ax",@progbits
#else
	.text
#endif
	.align	2
	.globl	BlockByteMove
	.ent	BlockByteMove	2
	.livereg	0x3000FF0F 0
	
BlockByteMove:
	blez 	a0, ByteMoveInvalidSize
	ASM_PROLOG
	move	v0, a0
	andi 	t0, a0, 0x1
/*	bne		t0, zero, ByteMoveInvalidSize */
	beq		t0, zero, ByteMoveEvenSize
ByteMoveOddByte:
	lb 		t0, 0(a1)
	sb		t0, 0(a2)
	addu	a1, 1
	addu 	a2, 1 
	addu	a0, -1
ByteMoveEvenSize:
	addu	a0, a1, a0
ByteMoveLoop:
		lb		t0, 0(a1)
		lb		t1, 1(a1)
		sb		t0,	0(a2)
		sb		t1,	1(a2)
		addu	a1,	2
		addu	a2,	2
		bne		a1, a0, ByteMoveLoop
EndByteMoveLoop:
	ASM_EPILOG
	j	ra
	
ByteMoveInvalidSize:
	nop
	la t0, ASSERT_SIZE
	jr t0
	nop
	.end	BlockByteMove
