/* No user fns here.  Pesch 15apr92. */

/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "%W% (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "local.h"

static int
__fwalk (ptr, function)
     struct _reent *ptr;
     register int (*function) ();
{
  register FILE *fp;
  register int n, ret = 0;
  register struct _glue *g;

  for (g = &ptr->__sglue; g != NULL; g = g->_next)
    for (fp = g->_iobs, n = g->_niobs; --n >= 0; fp++)
      if (fp->_flags != 0)
        {
          _flockfile (fp);
          if (fp->_flags != 0 && fp->_file != -1)
            ret |= (*function) (fp);
          _funlockfile (fp);
        }

  return ret;
}

/* Special version of __fwalk where the function pointer is a reentrant
   I/O function (e.g. _fclose_r).  */
static int
__fwalk_reent (ptr, reent_function)
     struct _reent *ptr;
     register int (*reent_function) ();
{
  register FILE *fp;
  register int n, ret = 0;
  register struct _glue *g;

  for (g = &ptr->__sglue; g != NULL; g = g->_next)
    for (fp = g->_iobs, n = g->_niobs; --n >= 0; fp++)
      if (fp->_flags != 0)
        {
          _flockfile (fp);
          if (fp->_flags != 0 && fp->_file != -1)
            ret |= (*reent_function) (ptr, fp);
          _funlockfile (fp);
        }

  return ret;
}

int
_fwalk (ptr, function)
     struct _reent *ptr;
     register int (*function) ();
{
  register int ret = 0;

  __sfp_lock_acquire ();

  /* Must traverse given list for std streams.  */
  if (ptr != _GLOBAL_REENT)
    ret |= __fwalk (ptr, function);

  /* Must traverse global list for all other streams.  */
  ret |= __fwalk (_GLOBAL_REENT, function);

  __sfp_lock_release ();

  return ret;
}

/* Special version of _fwalk which handles a function pointer to a
   reentrant I/O function (e.g. _fclose_r).  */
int
_fwalk_reent (ptr, reent_function)
     struct _reent *ptr;
     register int (*reent_function) ();
{
  register int ret = 0;

  __sfp_lock_acquire ();

  /* Must traverse given list for std streams.  */
  if (ptr != _GLOBAL_REENT)
    ret |= __fwalk_reent (ptr, reent_function);

  /* Must traverse global list for all other streams.  */
  ret |= __fwalk_reent (_GLOBAL_REENT, reent_function);

  __sfp_lock_release ();

  return ret;
}
