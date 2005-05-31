/* 
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
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

#include "base.hh"
#include "gname.hh"
#include "site.hh"
#include "am.hh"
#include "board.hh"

//
#include "hashtblDefs.cc"
template class GenDistEntryTable<GName>;

//
GNameTable gnameTable;
FatIntBody gnameID;
DebugCode(FatInt noGNameID;)

static inline
Bool checkGNameIsAlive(GName *gn)
{
  if (gn->getGCMark() || 
      (gn->getGNameType() == GNT_NAME &&
       tagged2Literal(gn->getValue())->isNamedName())) {
    gn->resetGCMark();
    gn->site->setGCFlag();
    return (OK);
  } else {    
    return (NO);
  }
}

/* OBSERVE - this must be done at the end of other gc */
void GNameTable::gCollectGNameTable()
{
  for (int i = getSize(); i--; ) {
    GName **gnp = getFirstNodeRef(i);
    GName *gn = *gnp;
    while (gn) {
      if (checkGNameIsAlive(gn) == NO) {
	deleteNode(gn, gnp);
	delete gn;
	// 'gnp' stays in place;
      } else {
	gnp = (GName **) gn->getNextNodeRef();
      }
      gn = *gnp;
    }
  }
  compactify();
}

/**********************************************************************/
/*   SECTION 19 :: Globalizing       stateless BASIC otherwise entity */
/**********************************************************************/

GName *Name::globalize()
{
  if (!hasGName()) {
    Assert(oz_isRootBoard(GETBOARD(this)));
    homeOrGName = ToInt32(newGName(makeTaggedLiteral(this),GNT_NAME));
    setFlag(Lit_hasGName);
  }
  return getGName1();
}

GName *Abstraction::globalize()
{
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_PROC));}
  return getGName1();
}

GName *SChunk::globalize()
{
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CHUNK));}
  return getGName1();
}

GName *ObjectClass::globalize()
{
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CLASS));}
  return getGName1();
}

// to be on the safe side;
void initGNameTable()
{
  gnameID.init();
  DebugCode(noGNameID.cInit(););
}

#if defined(DEBUG_CHECK)
TaggedRef findGNameDEBUG(GName *gn)
{
  return (oz_findGName(gn));
}
#endif
