/*
 *  Authors:
 *    Christian Schulte (schulte@dfki.de)
 *
 *  Copyright:
 *    Christian Schulte, 1998
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */


#ifndef __POINTERMARKSHH
#define __POINTERMARKSHH

#define _MarkPointer(p)     (_ToPointer((_ToInt32(p) | 0x1)))
#define _UnMarkPointer(p)   (_ToPointer((_ToInt32(p) & ~(_ToInt32(0x1)))))
#define _IsMarkedPointer(p) (_ToInt32(p) & 0x1)

#ifdef DEBUG_CHECK
inline Bool IsMarkedPointer(void * p) {
  return _IsMarkedPointer(p);
}

inline void * MarkPointer(void * p) {
  Assert(!IsMarkedPointer(p));
  return _MarkPointer(p);
}

inline void * UnMarkPointer(void * p) {
  Assert(IsMarkedPointer(p));
  return _UnMarkPointer(p);
}

#else

#define MarkPointer(p) _MarkPointer(p)
#define UnMarkPointer(p) _UnMarkPointer(p)
#define IsMarkedPointer(p) _IsMarkedPointer(p)

#endif

#endif
