/*
    Copyright � 2002, The AROS Development Team. 
    All rights reserved.
    
    $Id$
*/

#include <proto/layers.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#ifdef __AROS__
#include <proto/muimaster.h>
#endif

#include "mui.h"
#include "muimaster_intern.h"

/*****************************************************************************

    NAME */
	AROS_LH2(VOID, MUI_EndRefresh,

/*  SYNOPSIS */
	AROS_LHA(struct MUI_RenderInfo *, mri, A0),
	AROS_LHA(ULONG, flags, D0),

/*  LOCATION */
	struct Library *, MUIMasterBase, 29, MUIMaster)
/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS
	The function itself is a bug ;-) Remove it!

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct MUIMasterBase *,MUIMasterBase)

    struct Window *w = mri->mri_Window;

    if (w == NULL)
        return;

    EndRefresh(w, TRUE);
    UnlockLayerInfo(&w->WScreen->LayerInfo);
    mri->mri_Flags &= ~MUIMRI_REFRESHMODE;
    return;

    AROS_LIBFUNC_EXIT

} /* MUIA_EndRefresh */
