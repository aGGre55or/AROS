/*
    (C) 1995 AROS - The Amiga Replacement OS
    $Id$    $Log
    Desc:
    Lang: english
*/
#include "graphics_intern.h"
#include <graphics/text.h>

extern struct TextFont * driver_OpenFont (struct TextAttr *);

/*****************************************************************************

    NAME */
	#include <graphics/text.h>
	#include <clib/graphics_protos.h>

	__AROS_LH1(struct TextFont *, OpenFont,

/*  SYNOPSIS */
	__AROS_LA(struct TextAttr *, textAttr, A0),

/*  LOCATION */
	struct GfxBase *, GfxBase, 12, Graphics)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    graphics_lib.fd and clib/graphics_protos.h

*****************************************************************************/
{
    __AROS_FUNC_INIT
    __AROS_BASE_EXT_DECL(struct GfxBase *,GfxBase)

    return driver_OpenFont (textAttr);

    __AROS_FUNC_EXIT
} /* OpenFont */
