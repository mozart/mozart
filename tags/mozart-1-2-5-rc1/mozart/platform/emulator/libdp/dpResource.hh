/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
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

#define RESOURCE_HASH_TABLE_DEFAULT_SIZE 25
#define RESOURCE_NOT_IN_TABLE 0-1

enum DPResourceType{
  UD_unknown = 0,

  UD_thread,
  UD_array,
  UD_dictionary,
  UD_last
};

extern char *dpresource_names[];

/************************************************************/
/*  Defines                                                 */
/************************************************************/

class DistResource: public Tertiary{
public:
  NO_DEFAULT_CONSTRUCTORS(DistResource)
  DistResource(int i):Tertiary(i,Co_Resource,Te_Proxy){}
  
};
/************************************************************/
/*  ResourceTable                                           */
/************************************************************/


class ResourceHashTable: public GenHashTable {
  int hash(TaggedRef entity){
    int val = abs((int) entity) ;
    return val;}

public:
  ResourceHashTable(int i):GenHashTable(i){}

  //
  void add(OZ_Term entity, int oti) {
    // kost@ : this is what we can deal with:
    Assert((!oz_isRef(entity) && !oz_isVar(entity)) ||
	   (oz_isRef(entity) && oz_isVar(*tagged2Ref(entity))));
    Assert(find(entity) == RESOURCE_NOT_IN_TABLE);
    int hvalue;
    GenHashBaseKey *ghbk;
    GenHashEntry *ghe;

    //
    hvalue = hash(entity);
    ghbk = (GenHashBaseKey*) entity;
    ghe  = (GenHashEntry*) oti;
    GenHashTable::htAdd(hvalue, ghbk, ghe);
  }

  //
  int find(TaggedRef entity) {
    // kost@ : this is what we can deal with:
    Assert((!oz_isRef(entity) && !oz_isVar(entity)) ||
	   (oz_isRef(entity) && oz_isVar(*tagged2Ref(entity))));
    int hvalue = hash(entity);
    GenHashNode *aux;

    //
  repeat:
    aux = htFindFirst(hvalue);
    while (aux){
      OZ_Term te;

      //
      te = (OZ_Term) aux->getBaseKey();

      //
      // Now, there are three cases: found, not found, and found a
      // dead entry;
      if (te == entity) {
	// that's the entry we're talking about: let's check whether
	// the corresponding oe entry is still alive:
	int oti;
	OwnerEntry *oe;

	//
	oti = (int) aux->getEntry();
	oe = OT->getEntry(oti);

	//
	if (oe && oe->isRef() && oe->getRef() == entity) {
	  return (oti);		// found!
	} else {
	  // The wrong one: that is, the current entry is outdated
	  // and should be removed;
	  (void) htSub(hvalue, aux);

	  // must start from scratch since 'htSub()' is NOT compatible
	  // with 'htFindFirst()' & Co.;
	  goto repeat;
	}
	Assert(0);

	//
      } if (oz_isRef(te) && !oz_isVar(*tagged2Ref(te))) {
	// bound variables can be (and should be) discarded as well;
	(void) htSub(hvalue, aux);
	goto repeat;
      } else {
	aux = htFindNext(aux, hvalue);
      }
    }

    //
    return (RESOURCE_NOT_IN_TABLE);
  }

  //
  void gcResourceTable();
};

extern ResourceHashTable *resourceTable;

#define RHT resourceTable

ConstTerm* gcDistResourceImpl(ConstTerm*);

#endif








