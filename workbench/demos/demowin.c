/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.5  1996/08/23 17:05:41  digulla
    The demo crashes if kprintf() is called, so don't do it.
    New feature: Open console and use RawKeyConvert() to wait for ESC to quit the
    		demo.
    New feature: Added two gadgets: One with GFLG_GADGHCOMP, the other with
    		GFLG_GADGHIMAGE
    New feature: The user can select the gadgets and gets messages for them.
    New feature: More verbose and better error codes.

    Revision 1.4  1996/08/16 14:03:41  digulla
    More demos

    Revision 1.3  1996/08/15 13:17:32  digulla
    More types of IntuiMessages are checked
    Problem with empty window was due to unhandled REFRESH
    Commented some annoying debug output out

    Revision 1.2  1996/08/13 15:35:44  digulla
    Removed some comments
    Replied IntuiMessage

    Revision 1.1  1996/08/13 13:48:27  digulla
    Small Demo: Open a window, render some gfx and wait for a keypress

    Revision 1.5  1996/08/01 17:40:44  digulla
    Added standard header for all files

    Desc:
    Lang:
*/
#define ENABLE_RT	1
#define ENABLE_PURIFY	1

#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/exall.h>
#include <dos/datetime.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/aros_protos.h>
#include <clib/utility_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/console_protos.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <graphics/rastport.h>
#include <stdlib.h>
#include <ctype.h>
#include <aros/rt.h>

#if 0
#   define D(x)    x
#else
#   define D(x)     /* eps */
#endif
#define bug	kprintf

/* Don't define symbols before the entry point. */
extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *ConsoleDevice;
extern const char dosname[];
static LONG tinymain(void);

__AROS_LH0(LONG,entry,struct ExecBase *,sysbase,,)
{
    __AROS_FUNC_INIT
    LONG error=RETURN_FAIL;

    SysBase=sysbase;
    DOSBase=(struct DosLibrary *)OpenLibrary((STRPTR)dosname,39);
    GfxBase=(struct GfxBase *)OpenLibrary(GRAPHICSNAME,39);
    IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",39);

    if (DOSBase && GfxBase && IntuitionBase)
    {
	error=tinymain ();
    }
    else
    {
	D(bug("Counldn't open library\n"));
    }

    if (DOSBase)
	CloseLibrary ((struct Library *)DOSBase);

    if (GfxBase)
	CloseLibrary ((struct Library *)GfxBase);

    if (IntuitionBase)
	CloseLibrary ((struct Library *)IntuitionBase);

    D(bug("return %d\n", error));

    return error;
    __AROS_FUNC_EXIT
}

struct ExecBase *SysBase;
struct DosLibrary *DOSBase;
struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;
struct Library *ConsoleDevice;
const char dosname[]="dos.library";

void Refresh (struct RastPort * rp)
{
    int len;

    SetAPen (rp, 1);
    SetDrMd (rp, JAM2);

    Move (rp, 0, 0);
    Draw (rp, 320, 225);

    Move (rp, 640, 0);
    Draw (rp, 0, 450);

    Move (rp, 300, 40);
    Text (rp, "Hello World.", 12);

    SetAPen (rp, 3);
    RectFill (rp, 90, 0, 120, 30);
    SetAPen (rp, 0);
    RectFill (rp, 100, 10, 110, 20);

    SetAPen (rp, 1);
    RectFill (rp, 150, 10, 160, 20);

    SetAPen (rp, 2);
    RectFill (rp, 200, 10, 210, 20);

    SetAPen (rp, 3);
    RectFill (rp, 250, 10, 260, 20);

    len = TextLength (rp, "Hello World.", 12);

    SetAPen (rp, 2);
    RectFill (rp, 299, 59, 300+len, 60+rp->Font->tf_YSize);

    SetAPen (rp, 1);
    Move (rp, 300, 60 + rp->Font->tf_Baseline);
    Text (rp, "Hello World.", 12);

    SetDrMd (rp, JAM1);
    SetAPen (rp, 1);
    Move (rp, 301, 101);
    Text (rp, "Hello World.", 12);
    SetAPen (rp, 2);
    Move (rp, 300, 100);
    Text (rp, "Hello World.", 12);
}

#define GAD_WID     100
#define GAD_HEI     30

WORD BorderData[6*2*2] =
{
    0, GAD_HEI-1, /* Top (lighter) edge */
    1, -1,
    0, -(GAD_HEI-3),
    (GAD_WID-3), 0,
    1, -1,
    -(GAD_WID-1), 0,

    0, -(GAD_HEI-2), /* Bottom (darker) edge */
    -1, 1,
    0, (GAD_HEI-4),
    -(GAD_WID-4), 0,
    -1, 1,
    (GAD_WID-2), 0,
};
struct Border DemoBottomBorder =
{
    GAD_WID-1, GAD_HEI-1,
    1, 2,
    JAM1,
    6,
    &BorderData[6*2],
    NULL
},
DemoTopBorder =
{
    0, 0,
    2, 1,
    JAM1,
    6,
    &BorderData[0],
    &DemoBottomBorder
};
struct Border DemoIBottomBorder =
{
    GAD_WID-1, GAD_HEI-1,
    2, 1,
    JAM1,
    6,
    &BorderData[6*2],
    NULL
},
DemoITopBorder =
{
    0, 0,
    1, 2,
    JAM1,
    6,
    &BorderData[0],
    &DemoIBottomBorder
};

struct Gadget DemoGadget2 =
{
    NULL, /* NextGadget */
    220, 512-GAD_HEI-10, GAD_WID, GAD_HEI, /* hit box */
    GFLG_GADGHIMAGE | GFLG_LABELSTRING, /* Flags */
    GACT_IMMEDIATE | GACT_RELVERIFY, /* Activation */
    GTYP_BOOLGADGET, /* Type */
    &DemoTopBorder, &DemoITopBorder, /* Render */
    (struct IntuiText *)"Exit", /* Text */
    0L, NULL, /* MutualExcl, SpecialInfo */
    2, /* GadgetID */
    NULL /* UserData */
},
DemoGadget1 =
{
    &DemoGadget2, /* NextGadget */
    20, 512-GAD_HEI-10, GAD_WID, GAD_HEI, /* hit box */
    GFLG_GADGHCOMP | GFLG_LABELSTRING, /* Flags */
    GACT_IMMEDIATE | GACT_RELVERIFY, /* Activation */
    GTYP_BOOLGADGET, /* Type */
    &DemoTopBorder, NULL, /* Render */
    (struct IntuiText *)"Exit", /* Text */
    0L, NULL, /* MutualExcl, SpecialInfo */
    1, /* GadgetID */
    NULL /* UserData */
};

static LONG tinymain(void)
{
    struct NewWindow nw;
    struct Window * win;
    struct RastPort * rp;
    struct IntuiMessage * im;
    struct IOStdReq cioreq;
    struct InputEvent ievent =
    {
	NULL,
	IECLASS_RAWKEY,
	/* ... */
    };
    int cont, draw;
    ULONG args[4];

    VPrintf ("Welcome to the window demo of AROS\n", NULL);
    Flush (Output ());

    args[0] = (ULONG) tinymain;
    args[1] = (ULONG) Refresh;
    args[2] = (ULONG) _entry;
    VPrintf ("main=%08lx\nRefresh=%08lx\nentry=%08lx\n", args);
    Flush (Output ());

    nw.LeftEdge = 100;
    nw.TopEdge = 100;
    nw.Width = 640;
    nw.Height = 512;
    nw.DetailPen = nw.BlockPen = (UBYTE)-1;
    nw.IDCMPFlags = IDCMP_RAWKEY
		  | IDCMP_REFRESHWINDOW
		  | IDCMP_MOUSEBUTTONS
		  | IDCMP_MOUSEMOVE
		  | IDCMP_GADGETDOWN
		  | IDCMP_GADGETUP
		  ;
    nw.Flags = 0L;
    nw.FirstGadget = &DemoGadget1;
    nw.CheckMark = NULL;
    nw.Title = "Open a window demo";
    nw.Type = WBENCHSCREEN;

    OpenDevice ("console.device", -1, (struct IORequest *)&cioreq, 0);
    ConsoleDevice = (struct Library *)cioreq.io_Device;
    args[0] = (ULONG) ConsoleDevice;
    VPrintf ("Opening console.device=%08lx\n", args);
    Flush (Output ());

    if (!ConsoleDevice)
    {
	D(bug("Couldn't open console\n"));
	return 10;
    }

    win = OpenWindow (&nw);
    D(bug("OpenWindow win=%p\n", win));

    if (!win)
    {
	D(bug("Couldn't open window\n"));
	return 10;
    }

    rp = win->RPort;

    cont = 1;
    draw = 0;

    while (cont)
    {
	if ((im = (struct IntuiMessage *)GetMsg (win->UserPort)))
	{
	    /* D("Got msg\n"); */
	    switch (im->Class)
	    {
	    case IDCMP_RAWKEY: {
		UBYTE buf[10];
		int   len;
		int   t;

		ievent.ie_Code	    = im->Code;
		ievent.ie_Qualifier = im->Qualifier;

		len = RawKeyConvert (&ievent, buf, sizeof (buf), NULL);

		args[0] = im->Code;
		args[1] = im->Qualifier;
		VPrintf ("Key %ld + Qual %lx=\"", args);

		if (len < 0)
		{
		    VPrintf ("\" (buffer too short)", NULL);
		    break;
		}

		for (t=0; t<len; t++)
		{
		    if (buf[t] < 32 || (buf[t] >= 127 && buf[t] < 160))
		    {
			switch (buf[t])
			{
			case '\n':
			    VPrintf ("\\n", NULL);
			    break;

			case '\t':
			    VPrintf ("\\t", NULL);
			    break;

			case '\b':
			    VPrintf ("\\b", NULL);
			    break;

			case '\r':
			    VPrintf ("\\r", NULL);
			    break;

			case 0x1B:
			    VPrintf ("^[", NULL);
			    break;

			default:
			    args[0] = buf[t];
			    VPrintf ("\\x%02x", args);
			    break;
			} /* switch */
		    }
		    else
			FPutC (Output(), buf[t]);
		}
		VPrintf ("\"\n", NULL);

		if (*buf == '\x1b' && len == 1)
		{
		    if (len == 1)
			cont = 0;
		}

		break; }

	    case IDCMP_MOUSEBUTTONS:
		switch (im->Code)
		{
		case SELECTDOWN:
		    SetAPen (rp, 2);
		    Move (rp, im->MouseX, im->MouseY);
		    draw = 1;
		    break;

		case SELECTUP:
		    draw = 0;
		    break;

		case MIDDLEDOWN:
		    SetAPen (rp, 1);
		    Move (rp, im->MouseX, im->MouseY);
		    draw = 1;
		    break;

		case MIDDLEUP:
		    draw = 0;
		    break;

		case MENUDOWN:
		    SetAPen (rp, 3);
		    Move (rp, im->MouseX, im->MouseY);
		    draw = 1;
		    break;

		case MENUUP:
		    draw = 0;
		    break;

		}

		break;

	    case IDCMP_MOUSEMOVE:
		if (draw)
		    Draw (rp, im->MouseX, im->MouseY);

		break;

	    case IDCMP_REFRESHWINDOW:
		Refresh (rp);
		break;

	    case IDCMP_GADGETDOWN: {
		struct Gadget * gadget;

		gadget = (struct Gadget *)im->IAddress;

		args[0] = gadget->GadgetID;
		VPrintf ("User pressed gadget %ld\n", args);

		break; }

	    case IDCMP_GADGETUP: {
		struct Gadget * gadget;

		gadget = (struct Gadget *)im->IAddress;

		args[0] = gadget->GadgetID;
		VPrintf ("User released gadget %ld\n", args);

		break; }

	    } /* switch */

	    Flush (Output ());

	    ReplyMsg ((struct Message *)im);
	}
	else
	{
	    /* D("Waiting\n"); */
	    Wait (1L << win->UserPort->mp_SigBit);
	}
    }

    D(bug("CloseWindow (%p)\n", win));
    CloseWindow (win);

    return 0;
}
