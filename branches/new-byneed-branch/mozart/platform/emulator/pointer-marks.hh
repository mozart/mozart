/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */


#ifndef __POINTERMARKSHH
#define __POINTERMARKSHH

#define _MarkPointer(p,m)     (_ToPointer((_ToInt32(p) | (m))))
#define _UnMarkPointer(p,m)   (_ToPointer((_ToInt32(p) & ~(_ToInt32(m)))))
#define _IsMarkedPointer(p,m) (_ToInt32(p) & (m))

#ifdef DEBUG_CHECK
inline Bool IsMarkedPointer(void * p, int m) {
  return _IsMarkedPointer(p,m);
}

inline void * MarkPointer(void * p, int m) {
  return _MarkPointer(p,m);
}

inline void * UnMarkPointer(void * p, int m) {
  return _UnMarkPointer(p,m);
}

#else

#define MarkPointer(p,m) _MarkPointer(p,m)
#define UnMarkPointer(p,m) _UnMarkPointer(p,m)
#define IsMarkedPointer(p,m) _IsMarkedPointer(p,m)

#endif

#endif 

