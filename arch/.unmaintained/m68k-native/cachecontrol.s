#    (C) 1995-96 AROS - The Amiga Replacement OS
#    $Id$
#    $Log$
#    Revision 1.3  1996/10/21 21:08:57  aros
#    Changed __AROS_LA to __AROS_LHA
#
#    Revision 1.2  1996/08/01 17:41:33  digulla
#    Added standard header for all files
#
#    Desc:
#    Lang:

#*****************************************************************************
#
#   NAME
#
#	__AROS_LH2(ULONG, CacheControl,
#
#   SYNOPSIS
#	__AROS_LHA(ULONG, cacheBits, D0),
#	__AROS_LHA(ULONG, cacheMask, D1),
#
#   LOCATION
#	struct ExecBase *, SysBase, 107, Exec)
#
#   FUNCTION
#	Change/read the values in the 68030 cacr register. Only the bits set
#	in the mask parameter are affected.
#
#   INPUTS
#	cacheBits - new bit values.
#	cacheMask - Bits to change.
#
#   RESULT
#	Old contents of cacr register.
#
#   NOTES
#
#   EXAMPLE
#
#   BUGS
#
#   SEE ALSO
#
#   INTERNALS
#
#   HISTORY
#
#******************************************************************************

	# Simple 68000s have no chaches
	.globl	_Exec_CacheControl
_Exec_CacheControl:
	rts

