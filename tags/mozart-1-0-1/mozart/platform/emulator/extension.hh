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
  OZ_E_LAST
};

class OZ_Extension {
public:
  virtual ~OZ_Extension() { OZ_error("invoking destructor ~OZ_Extension()"); }

  OZ_Extension() {}

  void* operator        new(size_t n) { return _OZ_new_OZ_Extension(n); }
  void operator         delete(void*,size_t) 
    { OZ_error("invoking OZ_Extension::operator delete(void*,size_t)"); }
  virtual int           getIdV() = 0;
  virtual OZ_Extension* gcV() = 0;
  virtual void          gcRecurseV() {}
  virtual OZ_Term       printV(int = 10) { return typeV(); }
  virtual OZ_Term       printLongV(int depth = 10, int offset = 0)
    { return OZ_pair2(printV(depth),OZ_atom("\n")); }
  virtual OZ_Term       typeV() { return OZ_atom("extension"); }
  virtual OZ_Term       inspectV() { return typeV(); }
  virtual OZ_Boolean    isChunkV() { return OZ_TRUE; }
  virtual OZ_Term       getFeatureV(OZ_Term)       { return 0; }
  virtual OZ_Return     eqV(OZ_Term)               { return OZ_FAILED; }
  virtual OZ_Boolean    marshalV(void *)           { return OZ_FALSE; }
  virtual void *        __getSpaceInternal()       { return 0; }
  virtual void          __setSpaceInternal(void *) {}
  OZ_Boolean isLocal()  { return _OZ_isLocal_OZ_Extension(__getSpaceInternal()); }
};


class OZ_SituatedExtension: public OZ_Extension {
private:
  void *space;
public:
  OZ_SituatedExtension(void): OZ_Extension() { space = _OZ_currentBoard(); }
  OZ_SituatedExtension(void *sp) : OZ_Extension(), space(sp) {}

  virtual OZ_Term typeV() { return OZ_atom("situatedExtension"); }

  virtual void * __getSpaceInternal()         { return space; }
  virtual void   __setSpaceInternal(void *sp) { space = sp; }
};


_FUNDECL(unsigned int, oz_newUniqueId,()); // starts with OZ_E_LAST

_FUNDECL(OZ_Boolean,OZ_isExtension,(OZ_Term));
_FUNDECL(OZ_Extension*,OZ_getExtension,(OZ_Term));
_FUNDECL(OZ_Term,OZ_extension,(OZ_Extension *));

_FUNDECL(OZ_Term,oz_extension_unmarshal,(int, void*));
_FUNDECL(void,oz_registerExtension,(int, oz_unmarshalProcType));

#endif
