/*
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Konstantin Popov 2001
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __CMEM_HH
#define __CMEM_HH

#if defined(INTERFACE)
#pragma interface
#endif

//
//   Management of C heap
// 
// Right now it just provides for centralized management of free ists.

// First, a common pool of free blocks is (supposedly) more efficient
// than a number of separate blocks. Secondly, maybe even more
// important, all free blocks can be flushed in the centralized
// fashion. Thirdly, this is a convenient place to gather statistics.
//
// I (kost@) am not so sure at all whether we should attempt any "free
// list" management altogether: the malloc library may attempt to do
// that as well. Alternatively, maybe, this code can be used to
// complement a simple "allocate from the top" malloc.

// The real "free list" (can be shared between CppObjMemory and, say,
// in-house implementation of 'malloc()');
extern int32* freelist[];
//
void init_cmem();

//
class CppObjMemory {
public:
  //
  void *operator new(size_t size) {
    int index = size / sizeof(int32);
    int32 *ptr;
    Assert(index);		// must contain at least one word;
    Assert(index * sizeof(int32) == size);
    Assert(index < CMEM_FLENTRIES);

    //
    ptr = freelist[index];
    if (ptr) {
      freelist[index] = (int32 *) *ptr;
      return (ptr);
    } else {
      return (malloc(size));
    }
  }

  //
  void operator delete(void *obj, size_t size) {
    int index = size / sizeof(int32);
    Assert(index);		// must contain at least one word;
    Assert(index * sizeof(int32) == size);
    Assert(index < CMEM_FLENTRIES);
    //
    *((int32 **) obj) = freelist[index];
    freelist[index] = (int32 *) obj;
  }

  // must be empty;
  CppObjMemory() {}
  ~CppObjMemory() {}
};

#endif // __CMEM_HH
