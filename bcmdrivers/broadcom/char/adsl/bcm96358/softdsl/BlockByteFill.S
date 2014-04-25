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
BlockByteFill (int size, uchar pattern, uchar* dstPtr)
	*****************************************************************************
	*	This function fills uchar type array with the pattern					*
	*	a0 -- size																*
	*	a1 -- pattern
	*	a2 -- dstPtr
	*****************************************************************************
Public	void
BlockByteClear (int size, short* dstPtr)
	*****************************************************************************
	*	This function fills uchar type array with the 0							*
	*	a0 -- size																*
	*	a1 -- pattern
	*	a2 -- dstPtr
	*****************************************************************************
*/
#if defined(bcm47xx) && defined(USE_FAST_TEXT)
	.section .fast_text,"ax",@progbits
#else
	.text
#endif
	.align	2
	.globl  BlockByteClear
	.globl	BlockByteFill
	.ent	BlockByteFill	2
	.livereg	0x3000FF0F 0
	
BlockByteClear:
	move	a2, a1
	move	a1, zero
BlockByteFill:
	blez	a0, ByteFillInvalidSize
	addu	a0, a2, a0
ByteFillLoop:
		sb		a1, 0(a2)
		addu	a2,	1
		bne		a2, a0, ByteFillLoop
EndByteFillLoop:
	j	ra

ByteFillInvalidSize:
	nop
	la t0, ASSERT_SIZE
	jr t0
	nop
	
	.end	BlockByteFill
