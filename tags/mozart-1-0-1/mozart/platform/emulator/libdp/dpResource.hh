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
  friend void ConstTerm::gcConstRecurse(void);
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
    Assert(find(entity)==RESOURCE_NOT_IN_TABLE);
    int hvalue = hash(entity);
    GenHashTable::htAdd(hvalue,(GenHashBaseKey*)entity,
			(GenHashEntry*)index);}
  
  int find(TaggedRef entity){
    int hvalue = hash(entity);
    GenHashNode *aux = htFindFirst(hvalue);
    while(aux){
      int OTI = (int) aux->getEntry();
      OwnerEntry *oe=OT->getEntry(OTI);
      if(oe && (!oe->isTertiary()) &&oe->getRef()==entity){
	return OTI;}
      else{
	if(htSub(hvalue,aux));}
      aux = htFindNext(aux, hvalue);}
    return RESOURCE_NOT_IN_TABLE;
  }
  
  void gcResourceTable();
};

extern ResourceHashTable *resourceTable;

#define RHT resourceTable

ConstTerm* gcDistResourceImpl(ConstTerm*);

#endif








