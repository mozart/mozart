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
// kost@ : (Un)marshaling with continuations is tricky: a partially
//         unmarshaled result must be GC"able, so it is wrapped up in
//         a 'GTAbstractEntity' object which can be GC"ed by the
//         Builder's GC routine;
class ByteBuffer;
class GenTraverser;
class GTAbstractEntity;
//
typedef
_FUNTYPEDECL(OZ_Term,
	     oz_unmarshalProcType, (MarshalerBuffer *));
typedef
_FUNTYPEDECL(OZ_Term, 
	     oz_suspUnmarshalProcType, (ByteBuffer*, GTAbstractEntity* &));
typedef 
_FUNTYPEDECL(OZ_Term,
	     oz_unmarshalContProcType, (ByteBuffer*, GTAbstractEntity*));

//
class MarshalerBuffer;

// kost@: Extensions bodies are allocated using the
//        'oz_heapMalloc(..)' (through '_OZ_new_OZ_Extension(..)').
//        Descending data (arrays, etc.) are supposed to be allocated
//        the same way (no explicit/implicit malloc"s!!);

class OZ_Extension
{
private:
  void *space;
public:
  OZ_Extension(void) {
    reinterpret_cast<OZ_Container*>(((void**)((void*)this))-1)->initAsExtension();
    space = _OZ_currentBoard();
    
  }
  OZ_Extension(void *sp) : space(sp) {
    reinterpret_cast<OZ_Container*>(((void**)((void*)this))-1)->initAsExtension();
  }
  
  void *  __getSpaceInternal(void)      { return space; }
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
  virtual OZ_Return	getFeatureV(OZ_Term,OZ_Term&) { return OZ_FAILED; }
  virtual OZ_Return	putFeatureV(OZ_Term,OZ_Term ) { return OZ_FAILED; }
  virtual OZ_Return     eqV(OZ_Term)               { return OZ_FAILED; }
  // Both 'toBePickledV' and 'pickleV' must return 'TRUE'
  // if pickling is defined for the extension;
  virtual OZ_Boolean    toBePickledV() { return (OZ_FALSE); }
  virtual OZ_Boolean    pickleV(MarshalerBuffer *mb) { return (OZ_FALSE); }
  // Both 'toBeMarshaledV' and 'marshalSuspV' must return 'TRUE'
  // if marshaling for distribution is defined for the extension;
  virtual OZ_Boolean    toBeMarshaledV() { return (OZ_FALSE); }
  virtual OZ_Boolean    marshalSuspV(OZ_Term te,
				     ByteBuffer *bs, GenTraverser *gt)
  { return (OZ_FALSE); }
  virtual int           minNeededSpace() { return (0); }

  OZ_Boolean isLocal(void) { 
    return _OZ_isLocal_OZ_Extension(__getSpaceInternal()); 
  }

};

_FUNDECL(unsigned int, oz_newUniqueId,()); // starts with OZ_E_LAST

_FUNDECL(OZ_Boolean,OZ_isExtension,(OZ_Term));
_FUNDECL(OZ_Extension*,OZ_getExtension,(OZ_Term));
_FUNDECL(OZ_Term,OZ_extension,(OZ_Extension *));

_FUNDECL(OZ_Term,oz_extension_unmarshal,(int, MarshalerBuffer*));
_FUNDECL(OZ_Term,oz_extension_unmarshal,(int, ByteBuffer*,
					 GTAbstractEntity* &));
_FUNDECL(OZ_Term,oz_extension_unmarshalCont,(int, ByteBuffer*,
					     GTAbstractEntity*));

_FUNDECL(void, oz_registerExtension, (int,
				      oz_unmarshalProcType,
				      oz_suspUnmarshalProcType,
				      oz_unmarshalContProcType));

#endif
