/*
    (C) 1997 AROS - The Amiga Replacement OS
    $Id$

    Desc: Create a gadtools gadget
    Lang: english
*/
#define AROS_TAGRETURNTYPE struct Gadget *

#include "alib_intern.h"

extern struct Library * GadToolsBase;

/*****************************************************************************

    NAME */
#include <intuition/intuition.h>
#include <utility/tagitem.h>
#include <libraries/gadtools.h>
#define NO_INLINE_STDARG /* turn off inline def */
#include <proto/gadtools.h>

	struct Gadget * CreateGadget (

/*  SYNOPSIS */
	ULONG kind,
	struct Gadget * previous,
	struct NewGadget * ng,
	ULONG		tag1,
	...		)

/*  FUNCTION
        Varargs version of gadtools.library/CreateGadgetA().

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_SLOWSTACKTAGS_PRE(tag1)
    retval = CreateGadgetA (kind, previous, ng, AROS_SLOWSTACKTAGS_ARG(tag1));
    AROS_SLOWSTACKTAGS_POST
} /* CreateGadget */
