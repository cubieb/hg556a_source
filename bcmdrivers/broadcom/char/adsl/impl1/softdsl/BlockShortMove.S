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
Public	int
BlockShortMove (int size, short* srcPtr, short* dstPtr)
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
	.globl	BlockShortMove
	.ent	BlockShortMove	2
	.livereg	0x3000FF0F 0
	
BlockShortMove:
	blez 	a0, ShortMoveInvalidSize
	move	v0, a0
	andi 	t0, a0, 0x1
	bne		t0, zero, ShortMoveInvalidSize
	andi	t0, a0, 0x7
	sll		a0, 1
	addu	a0, a1, a0
	bne		t0, zero, ShortMoveSlowLoop
	
	andi	t1, v0, 0xF
	bne		t1, zero, ShortMoveFastLoop

	or		t2, a1, a2
	andi	t2, 0x3
	bne		t2, zero, ShortMoveFastLoop

ShortMoveVeryFastLoop:
		lw		t0, 0(a1)
		lw		t1, 4(a1)
		lw		t2, 8(a1)
		lw		t3, 12(a1)
		sw		t0,	0(a2)
		sw		t1,	4(a2)
		sw		t2,	8(a2)
		sw		t3,	12(a2)

		lw		t0, 16(a1)
		lw		t1, 20(a1)
		lw		t2, 24(a1)
		lw		t3, 28(a1)
		sw		t0,	16(a2)
		sw		t1,	20(a2)
		sw		t2,	24(a2)
		sw		t3,	28(a2)

		addu	a1,	32
		addu	a2,	32
		bne		a1, a0, ShortMoveVeryFastLoop
EndShortMoveVeryFastLoop:
	j	ra
	

ShortMoveFastLoop:
		lh		t0, 0(a1)
		lh		t1, 2(a1)
		lh		t2, 4(a1)
		lh		t3, 6(a1)
		lh		t4, 8(a1)
		lh		t5, 10(a1)
		lh		t6, 12(a1)
		lh		t7, 14(a1)
		sh		t0,	0(a2)
		sh		t1,	2(a2)
		sh		t2,	4(a2)
		sh		t3,	6(a2)
		sh		t4,	8(a2)
		sh		t5,	10(a2)
		sh		t6,	12(a2)
		sh		t7,	14(a2)
		addu	a1,	16
		addu	a2,	16
		bne		a1, a0, ShortMoveFastLoop
EndShortMoveFastLoop:
	j	ra

ShortMoveSlowLoop:
		lh		t0, 0(a1)
		lh		t1, 2(a1)
		sh		t0,	0(a2)
		sh		t1,	2(a2)
		addu	a1,	4
		addu	a2,	4
		bne		a1, a0, ShortMoveSlowLoop
EndShortMoveSlowLoop:
	j	ra
	
ShortMoveInvalidSize:
	nop
	la t0, ASSERT_SIZE
	jr t0
	nop
	.end	BlockShortMove
