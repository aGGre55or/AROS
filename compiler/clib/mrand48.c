/*
 * Copyright (c) 1993 Martin Birgmeier
 * All rights reserved.
 *
 * You may redistribute unmodified or modified versions of this source
 * code provided that the above copyright notice and this and the
 * following conditions are retained.
 *
 * This software is provided ``as is'', and comes with no warranties
 * of any kind. I shall in no event be liable for anything that happens
 * to anyone/anything when using this software.
 */

#include "rand48.h"

extern unsigned short _rand48_seed[3];

/*****************************************************************************

    NAME */

	long mrand48(

/*  SYNOPSIS */
	void)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

******************************************************************************/
{
	_dorand48(_rand48_seed);
	return ((long) _rand48_seed[2] << 16) + (long) _rand48_seed[1];
}

