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
BlockLongMove (int size, long* srcPtr, long* dstPtr)
	*****************************************************************************
	*	a0 -- size																*
	*	a1 -- srcPtr
	*	a2 -- dstPtr	
	*****************************************************************************
*/
	.text
	.align	2
	.globl	BlockLongMove
	.ent	BlockLongMove	2
	.livereg	0x3000FF0F 0
	
BlockLongMove:
	blez	a0, LongMoveInvalidSize
	andi 	t0, a0, 0x3
	bne		t0, zero, LongMoveInvalidSize
	sll		a0, 2
	addu	a0, a1, a0
LongMoveLoop:
		lw		t0, 0(a1)
		lw		t1, 4(a1)
		lw		t2, 8(a1)
		lw		t3, 12(a1)
		sw		t0,	0(a2)
		sw		t1,	4(a2)
		sw		t2,	8(a2)
		sw		t3,	12(a2)

		addu	a1,	16
		addu	a2,	16
		bne		a1, a0, LongMoveLoop
EndLongMoveLoop:
	j	ra
	
LongMoveInvalidSize:
	nop
	la t0, ASSERT_SIZE
	jr t0
	nop
	.end	BlockLongMove
