/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang:
*/
#include <exec/execbase.h>
#include <exec/interrupts.h>
#include <proto/exec.h>
#include <aros/libcall.h>

/*****************************************************************************

    NAME */

	AROS_LH2(struct Interrupt *, SetIntVector,

/*  SYNOPSIS */
	AROS_LHA(ULONG,              intNumber, D0),
	AROS_LHA(struct Interrupt *, interrupt, A1),

/*  LOCATION */
	struct ExecBase *, SysBase, 27, Exec)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

******************************************************************************/
{
    AROS_LIBFUNC_INIT
    struct Interrupt *oldint;

    Disable ();

    oldint = (struct Interrupt *)SysBase->IntVects[intNumber].iv_Node;
    SysBase->IntVects[intNumber].iv_Node = (struct Node *)interrupt;

    if (interrupt)
    {
	SysBase->IntVects[intNumber].iv_Data = interrupt->is_Data;
	SysBase->IntVects[intNumber].iv_Code = interrupt->is_Code;
    }
    else
    {
	SysBase->IntVects[intNumber].iv_Data = (APTR)~0;
	SysBase->IntVects[intNumber].iv_Code = (void *)~0;
    }

    Enable ();

    return oldint;

    AROS_LIBFUNC_EXIT
} /* SetIntVector */
