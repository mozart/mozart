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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __EXTENSIONHH
#define __EXTENSIONHH

#include "value.hh"

/*===================================================================
 * Extension
 *=================================================================== */

/*
 * TODO
 *  - browser, debugger support
 *  - copy flag for gc methods
 *  - user defined extension class
 *    (id is a good optimization for system extension, but user should use
 *     a URL to identify its class)
 */

// starts with OZ_E_LAST
int oz_newUniqueId();

enum OZ_Registered_Extension_Id {
  OZ_E_USER,
  OZ_E_BITARRAY,
  OZ_E_LAST,
};

class Extension: public ConstTerm {
public:
  virtual ~Extension() {} // C++ compiler requires this, mm2: why?
  Extension() : ConstTerm(Co_Extension) {}

  virtual int           getIdV() { return 0; }

  virtual Extension *   gcV() = 0;
  virtual void          gcRecurseV() {}

  virtual void          printStreamV(ostream &out,int depth = 10);
  virtual void          printLongStreamV(ostream &out,int depth = 10,
					 int offset = 0);

  virtual OZ_Term       typeV();
  virtual OZ_Term       inspectV() { return typeV(); }

  virtual OZ_Term       getFeatureV(OZ_Term fea) { return 0; }

  virtual OZ_Return     unifyV(OZ_Term t)        { return FAILED; }

  virtual int           marshalV(MsgBuffer *bs)  { return 0; }

  virtual Board *       getBoardInternal() { return 0; }
  virtual void          setBoardInternal(Board *bb) {}
};

inline
int oz_isExtension(OZ_Term t) {
  return oz_isConst(t) && tagged2Const(t)->getType()==Co_Extension;
}

inline
Extension *tagged2Extension(OZ_Term t) {
  return (Extension *) tagged2Const(t);
}

class SituatedExtension: public Extension {
private:
  Board *board;
public:
  SituatedExtension(void);
  SituatedExtension(Board *bb) : Extension(), board(bb) {}

  virtual void          printStreamV(ostream &out,int depth = 10);
  virtual OZ_Term       typeV();

  virtual Board *getBoardInternal()        { return board; }
  virtual void setBoardInternal(Board *bb) { board = bb; }
};

typedef OZ_Term (*oz_unmarshalProcType)(MsgBuffer*);
OZ_Term oz_extension_unmarshal(int type,MsgBuffer*);
void oz_registerConstExtension(int type, oz_unmarshalProcType f);
#endif
