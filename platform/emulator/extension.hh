/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Copyright:
 *    Michael Mehl (1998)
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

#ifndef __EXTENSIONHH
#define __EXTENSIONHH

#ifdef INTERFACE
#pragma interface
#endif

#include <stdlib.h>

/*===================================================================
 * Extension
 *=================================================================== */

enum OZ_Registered_Extension_Id {
  OZ_E_UNDEFINED,
  OZ_E_BITARRAY,
  OZ_E_BITSTRING,
  OZ_E_BYTESTRING,
  OZ_E_THREAD,
  OZ_E_HEAPCHUNK,
  OZ_E_CHUNK,
  OZ_E_WEAKDICTIONARY,
  OZ_E_WORD,
  OZ_E_LAST
};

//
class MarshalerBuffer;

//
class OZ_Extension {
private:
  void *space;
public:
  OZ_Extension(void) { space = _OZ_currentBoard(); }
  OZ_Extension(void *sp) : space(sp) {}

  void *  __getSpaceInternal(void)      { return space; }
  void ** __getSpaceRefInternal(void)   { return &space; }
  void    __setSpaceInternal(void * sp) { space = sp;   }

public:
  virtual ~OZ_Extension();

  void* operator        new(size_t n) { return _OZ_new_OZ_Extension(n); }
  void operator         delete(void*,size_t);

  virtual int           getIdV(void) = 0;

  virtual OZ_Extension* gCollectV(void) = 0;
  virtual void          gCollectRecurseV(void) = 0;
  virtual OZ_Extension* sCloneV(void) = 0;
  virtual void          sCloneRecurseV(void) = 0;

  virtual OZ_Term       printV(int = 10) { return typeV(); }
  virtual OZ_Term       printLongV(int depth = 10, int offset = 0);
  virtual OZ_Term       typeV(void);
  virtual OZ_Boolean    isChunkV(void) { return OZ_TRUE; }
  virtual OZ_Term       getFeatureV(OZ_Term);
  virtual OZ_Return     getFeatureV(OZ_Term,OZ_Term&) { return OZ_FAILED; }
  virtual OZ_Return     putFeatureV(OZ_Term,OZ_Term ) { return OZ_FAILED; }
  virtual OZ_Return     eqV(OZ_Term)               { return OZ_FAILED; }
  // 'toBePickledV' and 'pickleV' return 'TRUE' if pickling is defined
  // for them;
  virtual OZ_Boolean    toBePickledV() { return (OZ_FALSE); }
  virtual OZ_Boolean    pickleV(MarshalerBuffer *mb) { return (OZ_FALSE); }
  virtual int           minNeededSpace() { return (0); }

  OZ_Boolean isLocal(void) {
    return _OZ_isLocal_OZ_Extension(__getSpaceInternal());
  }

};

_FUNDECL(unsigned int, oz_newUniqueId,()); // starts with OZ_E_LAST

_FUNDECL(OZ_Boolean,OZ_isExtension,(OZ_Term));
_FUNDECL(OZ_Extension*,OZ_getExtension,(OZ_Term));
_FUNDECL(OZ_Term,OZ_extension,(OZ_Extension *));

_FUNDECL(OZ_Term,oz_extension_unmarshal,(int, void*));
_FUNDECL(void,oz_registerExtension,(int, oz_unmarshalProcType));

#endif
