/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *    Kostja Popov <kost@sics.se>
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

#ifndef __RESOURCE_HH
#define __RESOURCE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "value.hh"

/************************************************************/
/*  Defines                                                 */
/************************************************************/

#define RESOURCE_HASH_TABLE_DEFAULT_SIZE 5
#define RESOURCE_NOT_IN_TABLE 0-1

/************************************************************/
/*  Defines                                                 */
/************************************************************/

class DistResource: public Tertiary{
public:
  NO_DEFAULT_CONSTRUCTORS(DistResource)
  DistResource(OB_TIndex p)
    : Tertiary(OB_TIndex2Ptr(p),Co_Resource,Te_Proxy) {}
  
};

/************************************************************/
/*  ResourceTable                                           */
/************************************************************/

//
// Resource hash table maps OZ_Term"s - either values, or references
// to immediate variables - to OTI"s (internal ones).
//
class RHTNode : public GenDistEntryNode<RHTNode>,
		public CppObjMemory {
private:
  OZ_Term entity;
  OB_TIndex oti;

public:
  RHTNode(OZ_Term entityIn, OB_TIndex otiIn)
    : entity(entityIn), oti(otiIn) {}
  RHTNode(OZ_Term entityIn)
    : entity(entityIn) { DebugCode(oti = (OB_TIndex) -1;); }
  ~RHTNode() {
    DebugCode(entity = (OZ_Term) -1;);
    DebugCode(oti = (OB_TIndex) -1;);
  }

  unsigned int value4hash() { return ((unsigned int) entity); }
  int compare(RHTNode *n) { return (((int) entity) - ((int) n->entity)); }

  OZ_Term getEntity() { return (entity); }
  OB_TIndex getOTI() { return (oti); }
};

//
//
class ResourceHashTable: public GenDistEntryTable<RHTNode> {
public:
  ResourceHashTable(int sizeAsPowerOf2)
    : GenDistEntryTable<RHTNode>(sizeAsPowerOf2) {}
  ~ResourceHashTable() {}

  //
  void add(OZ_Term entity, OB_TIndex oti) {
    // kost@ : this is what we can deal with: values or refs to vars;
    Assert((!oz_isRef(entity) && !oz_isVar(entity)) ||
	   (oz_isRef(entity) && oz_isVar(*tagged2Ref(entity))));
    // there can be at most one entry for a given 'entity':
    Assert(find(entity) == (OB_TIndex) RESOURCE_NOT_IN_TABLE);
    // the owner entry must be of the type 'ref':
    DebugCode(OwnerEntry *oe = ownerIndex2ownerEntry(oti););
    Assert(oe && oe->isRef());
    //
    RHTNode *n = new RHTNode(entity, oti);
    htAdd(n);
  }

  //
  OB_TIndex find(TaggedRef entity) {
    Assert((!oz_isRef(entity) && !oz_isVar(entity)) ||
	   (oz_isRef(entity) && oz_isVar(*tagged2Ref(entity))));
    //
    RHTNode ref(entity);
    RHTNode *found = htFind(&ref);

    //
    if (found) {
      Assert(found->getEntity() == entity);
      OB_TIndex oti = found->getOTI();
      OwnerEntry *oe = ownerIndex2ownerEntry(oti);

      //
      if (oe && oe->isRef() && oe->getRef() == entity) {
	return (oti);		// still upright;
      } else {
	htDel(found);		// something's changed;
	delete found;
	return ((OB_TIndex) RESOURCE_NOT_IN_TABLE);
      }
      Assert(0);
    } else {
      return ((OB_TIndex) RESOURCE_NOT_IN_TABLE);
    }
    Assert(0);
  }

  //
  void gcResourceTable();
};

extern ResourceHashTable *resourceTable;

#define RHT resourceTable

ConstTerm* gcDistResourceImpl(ConstTerm*);

#endif








