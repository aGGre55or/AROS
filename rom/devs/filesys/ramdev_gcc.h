/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.2  1996/08/01 17:41:24  digulla
    Added standard header for all files

    Desc:
    Lang:
*/
#ifndef RAMDEV_GCC_H
#define RAMDEV_GCC_H
#include <aros/libcall.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <exec/devices.h>
#include <dos/dos.h>
#include <dos/filesystem.h>

struct rambase
{
    struct Device device;
    struct ExecBase *sysbase;
    struct DosLibrary *dosbase;
    struct UtilityBase *utilitybase;
    BPTR seglist;
    ULONG unitcount;
    struct MsgPort *port;		/* Port to put IORequests to */
    struct SignalSemaphore *sigsem;	/* Semaphore for iofs */
    struct IOFileSys *iofs;		/* IORequest to be aborted or NULL */
    struct MinList waitdoslist;
};

#define init(rambase, segList) \
__AROS_LC2(struct rambase *, init, __AROS_LA(struct rambase *, rambase, D0), __AROS_LA(BPTR, segList, A0), struct ExecBase *, SysBase, 0, ram)

#define open(iob, unitnum, flags) \
__AROS_LC3(void, open, __AROS_LA(struct ramrequest *, iob, A1), __AROS_LA(ULONG, unitnum, D0), __AROS_LA(ULONG, flags, D0), struct rambase *, rambase, 1, ram)

#define close(iob) \
__AROS_LC1(BPTR, close, __AROS_LA(struct ramrequest *, iob, A1), struct rambase *, rambase, 2, ram)

#define expunge() \
__AROS_LC0(BPTR, expunge, struct rambase *, rambase, 3, ram)

#define null() \
__AROS_LC0(int, null, struct rambase *, rambase, 4, ram)

#define beginio(iob) \
__AROS_LC1(void, beginio, __AROS_LA(struct ramrequest *, iob, A1), struct rambase *, rambase, 5, ram)

#define abortio(iob) \
__AROS_LC1(LONG, abortio, __AROS_LA(struct ramrequest *, iob, A1), struct rambase *, rambase, 6, ram)

#endif

