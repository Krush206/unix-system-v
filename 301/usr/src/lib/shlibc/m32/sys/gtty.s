#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- gtty
.ident	"@(#)libc-m32:sys/gtty.s	1.3"

	.set	_gtty,32*8

	.globl	gtty
	.globl	_cerror

gtty:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_gtty,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET
