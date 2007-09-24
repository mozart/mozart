/*
 *  Authors:
 *    Erik Klintskog
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog,2001
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

#if defined(INTERFACE)
#pragma implementation "bucketHashTable.hh"
#endif

#include "bucketHashTable.hh"
#include <math.h>
#include <stdlib.h>


  // ********************************************************************** 
  //      BUCKET HASH TABLE                                                 
  // ********************************************************************** 

  namespace{
    const int    STEP    = 5;
    const double MAXFULL = 0.75; // The max. load of HashTable
    const double IDEALFULL= 0.40; 
    const double MINFULL = 0.20; // The min. load of HashTable
    const int    EXPANSION_FACTOR = 2;  // load .375


    // ********************************************************************** 
    //      PRIME NUMBER FUNCTIONS - (not the best but 'prime' should be small)
    // ********************************************************************** 
    
    inline bool if_isMyPrime2(const int& prime){
      if (prime%2 == 0) {return false;}
      for(int i=3; i<=sqrt(static_cast<double>(prime)); i+=2) {
	if (prime%i == 0) {return false;}}
      return true;
    }
    
    inline int if_nextMyPrime2(int prime){
      if (prime <= STEP) {
	prime = STEP+2;}
      if (prime%2 == 0) {
	prime++;}
      while(!if_isMyPrime2(prime)) {
	prime += 2;}
      return prime;
    }
  }

  inline void 
    BucketHashTable::basic_htAdd(const u32& pk,BucketHashNode* const bhn){
    BucketHashNode *next = table[pk % tableSize];
    Assert(next != bhn); 
    bhn->setNext(next); 
    table[pk % tableSize] = bhn;
    // We had some problems with this code. 
    Assert(bhn->getNext() != bhn); 
    Assert(bhn->getNext() == next); 
    
  }


  
  inline void BucketHashTable::rehash(BucketHashNode **old,int size){
    BucketHashNode *tmp,*ptr;
    for(register int i=0;i<size;i++){
      ptr = old[i];
      while(ptr) {
	tmp = ptr->next;
	int index = ptr->getPrimKey() % tableSize;
	ptr->next = table[index];
	table[index] = ptr; 
	ptr = tmp;
      }
    }
  }
	
  void BucketHashTable::calc_percents(){
    top_percent = (MAXFULL * tableSize);
    bottom_percent= (MINFULL * tableSize);
    if(tableSize==minSize) bottom_percent=0;
    Assert(top_percent>bottom_percent);}  

  void BucketHashTable::resize(){
    int newSize=if_nextMyPrime2(tableSize*EXPANSION_FACTOR);
    BucketHashNode **oldtable=table;
    //table = (BucketHashNode**) malloc(newSize * sizeof(BucketHashNode*));
    table = new BucketHashNode*[newSize];
    if(table==NULL){ dssError("Memory allocation: could not resize Hash Table");}
    init(0,newSize);
    int oldSize=tableSize;
    tableSize=newSize;
    rehash(oldtable,oldSize);
    calc_percents();
    delete [] oldtable;
    //free(oldtable);
  }


  // ********************************************************************** 
  //      VISIBLE MEMBER FUNCTIONS                                           
  // ********************************************************************** 

  BucketHashTable::BucketHashTable(int s):
    top_percent(0), bottom_percent(0), counter(0),
    minSize(0), tableSize(if_nextMyPrime2(s)), table(NULL){
    calc_percents();
    minSize=tableSize;
    //table = (BucketHashNode**) malloc(tableSize * sizeof(BucketHashNode*));
    //printf("fix tables when done\n");
    table = new BucketHashNode*[tableSize];
    init(0,tableSize);
  }

  BucketHashTable::~BucketHashTable(){
    clear();
    delete [] table;
    //free(table);
  }


  void BucketHashTable::htAdd(u32 pk,BucketHashNode *bhn){
    if (counter > top_percent) resize();
    counter++;
    basic_htAdd(pk,bhn);}



  void BucketHashTable::htSubEn(BucketHashNode *cur){
    counter--;
    BucketHashNode **try0=&table[(cur->getPrimKey())% tableSize];
    while(*try0){
      if ((*try0)!= cur)
	try0 = &((*try0)->next);
      else {
	*try0=cur->next;
	delete cur;
	break;
      }
    }
  }

  BucketHashNode* BucketHashTable::htSubPkSk(u32 pk, u32 sk){
    counter--;
    BucketHashNode *tmp, **try0=&table[pk % tableSize];
    while(*try0){
      if ((*try0)->prim_key != pk || (*try0)->sec_key != sk){
	try0 = &((*try0)->next);
      } else {
	tmp = *try0;
	*try0=tmp->next;
	return  tmp;
      }
    }
    return NULL;
  }


  BucketHashNode *BucketHashTable::htFindPkSk(const u32& pk, const u32& sk){
    BucketHashNode *ans = htFindPk(pk);
    while(ans && (ans->prim_key != pk || ans->sec_key != sk))
      ans = ans->next;
    return  ans;
  }

// The method returnd the "next" node, used when 
// traverseing all the nodes in the table. If called
// with NULL, the "first" node is returned. 
// If called with a node, the next node(acording to the
// buckethastable) is returned. When no more nodes exist, 
// the method returns NULL. 
//

BucketHashNode *BucketHashTable::htGetNext(BucketHashNode *n){
  int index = 0;
  if (n != NULL) {
    // return successor in bucket if there is one
    if (n->getNext() != NULL) return n->getNext();
    // otherwise look in the table "after" node n
    index = n->getPrimKey() % tableSize + 1;
  }
  // iterate until a nonempty bucket is found
  while (index < tableSize) {
    if (table[index] != NULL) return table[index];
    index++;
  }
  // table is exhausted, return NULL
  return NULL;
}
      
