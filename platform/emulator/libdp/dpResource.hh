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


class ResourceHashTable: public GenHashTable{
  int hash(TaggedRef entity){
    int val = abs((int) entity) ;
    return val;}
  void gcResourceTableRecurse(GenHashNode*, int);
  

public:
  ResourceHashTable(int i):GenHashTable(i){}
  
  void add(TaggedRef entity, int index){
    // kost@ : this is what we can deal with:
    Assert((!oz_isRef(entity) && !oz_isVariable(entity)) ||
	   (oz_isRef(entity) && oz_isVariable(*tagged2Ref(entity))));
    Assert(find(entity)==RESOURCE_NOT_IN_TABLE);
    int hvalue = hash(entity);
    GenHashTable::htAdd(hvalue,(GenHashBaseKey*)entity,
			(GenHashEntry*)index);}

  int find(TaggedRef entity){
    Assert((!oz_isRef(entity) && !oz_isVariable(entity)) ||
	   (oz_isRef(entity) && oz_isVariable(*tagged2Ref(entity))));
    int hvalue = hash(entity);
    GenHashNode *aux = htFindFirst(hvalue);
    while(aux){
      int OTI = (int) aux->getEntry();
      OwnerEntry *oe=OT->getEntry(OTI);
      // 
      // kost@ : that's how it was looking before:
      /* if(oe && (!oe->isTertiary()) && oe->getRef()==entity) { */
      // kost@: i've introduced now this assertion:
      // 'ref' owner entries are deallocated explicitly, so:
      Assert(oe);
      if(oe->isRef() && oe->getRef() == entity) {
	return OTI;
      } else {
	// kost@ : what??! why remove!?! It cann't be removed since
	// there can be multiple entries with the same hash value.
	// So, i've uncommented it:
	// if(htSub(hvalue,aux));
	;
	// I feel i know the motive for that: it was a sort of GC,
	// wasn't it? Outdated RHT entries (that is, those that did
	// not correspond to an alive&corrent OT entry anymore) were
	// eventually purged;
      }
      aux = htFindNext(aux, hvalue);
    }
    return RESOURCE_NOT_IN_TABLE;
  }

  // kost@ : explicit destruction of entries;
  void deleteFound(OZ_Term entity) {
    Assert((!oz_isRef(entity) && !oz_isVariable(entity)) ||
	   (oz_isRef(entity) && oz_isVariable(*tagged2Ref(entity))));
    int hvalue = hash(entity);
    GenHashNode *aux = htFindFirst(hvalue);
    while (aux) {
      OZ_Term te;
      GenCast(aux->getBaseKey(), GenHashBaseKey*, te, OZ_Term);   
      // Additionally, (all) bound variables can be discarded as well:
      // nobody will ever try to find them;
      if ((te == entity) ||
	  (oz_isRef(te) && !oz_isVariable(*tagged2Ref(te)))) {
	htSub(hvalue, aux);
	break;
      } else {
	aux = htFindNext(aux, hvalue);
      }
    }
  }
    
  
  void gcResourceTable();
};

extern ResourceHashTable *resourceTable;

#define RHT resourceTable

ConstTerm* gcDistResourceImpl(ConstTerm*);

#endif








