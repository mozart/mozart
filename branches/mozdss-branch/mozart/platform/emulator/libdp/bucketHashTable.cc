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


/* ********************************************************************** */
/*      BUCKET HASH TABLE                                                 */
/* ********************************************************************** */


const int STEP=5;
const double MAXFULL=0.75; // The max. load of HashTable
const double IDEALFULL=0.40; 
const double MINFULL=0.20; // The min. load of HashTable
const int EXPANSION_FACTOR=2;  // load .375
/* ********************************************************************** */
/*      PRIME NUMBER FUNCTIONS                                            */
/* ********************************************************************** */

inline Bool isMyPrime2(int prime){
  if (prime%2 == 0) {return NO;}
  for(int i=3; i<=sqrt(prime); i+=2) {
    if (prime%i == 0) {return NO;}}
  return OK;}

int nextMyPrime2(int prime){
  if (prime <= STEP) {
    prime = STEP+2;}
  if (prime%2 == 0) {
    prime++;}
  while(!isMyPrime2(prime)) {
    prime += 2;}
  return prime;}


inline void 
BucketHashTable::basic_htAdd(unsigned int pk,BucketHashNode *bhn){
  int index=pk % tableSize;
  BucketHashNode *ptr= table[index];
  bhn->setNext(ptr);
  table[index] = bhn;
}


  
inline void BucketHashTable::rehash(BucketHashNode **old,int size){
  BucketHashNode *tmp,*ptr;
  for(int i=0;i<size;i++){
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
  top_percent = (int) (MAXFULL * tableSize);
  bottom_percent= (int) (MINFULL * tableSize);
  if(tableSize==minSize) bottom_percent=0;
  Assert(top_percent>bottom_percent);}  

void BucketHashTable::resize(){
  int newSize=nextMyPrime2(tableSize*EXPANSION_FACTOR);
  BucketHashNode **oldtable=table;
  table= (BucketHashNode **) malloc(newSize * sizeof(BucketHashNode*));
  if(table==NULL){
    OZ_error("Memory allocation: could not resize Hash Table");}
  init(0,newSize);
  int oldSize=tableSize;
  tableSize=newSize;
  rehash(oldtable,oldSize);
  calc_percents();
  free(oldtable);}

void BucketHashTable::compactify(){
  printf("We dont compactify BucketHashTables\n");
  /*
  if(counter>=bottom_percent) return;
  if(tableSize==minSize) return;
  Assert(tableSize>=minSize);
  int newSize = nextMyPrime2((int) ( ((double) counter) / IDEALFULL));
  if(newSize<minSize) newSize=minSize;
  Assert(newSize<tableSize);
  GenHashNode *oldtable=table;
  table= (GenHashNode *) malloc(newSize * sizeof(GenHashNode));
  Assert(table!=NULL);
  if(table==NULL){
    return;}
  init(0,newSize);
  int oldSize=tableSize;
  tableSize=newSize;
  rehash(oldtable,oldSize);
  calc_percents();
  free(oldtable);
  */
}

/* ********************************************************************** */
/*      VISIBLE MEMBER FUNCTIONS                                           */
/* ********************************************************************** */

BucketHashTable::BucketHashTable(int s){
  tableSize = nextMyPrime2(s);
  counter = 0;
  calc_percents();
  minSize=tableSize;
  table = (BucketHashNode **) malloc(tableSize * sizeof(BucketHashNode*));
  init(0,tableSize);}


void BucketHashTable::htAdd(unsigned int pk,BucketHashNode *bhn){
  if (counter > top_percent) resize();  
  counter++;
  basic_htAdd(pk,bhn);}



void BucketHashTable::htSubPkEn(unsigned int pk,BucketHashNode *cur){
  unsigned int index=pk % tableSize; 
  counter--;
  BucketHashNode **try0=&table[index];
  while(*try0){
    if ((*try0)== cur){
      *try0=cur->next;
      delete cur;
      break;
    }
    try0 = &((*try0)->next);
  }
}
void* BucketHashTable::htSubPkSk(unsigned int pk,unsigned int sk){
  unsigned int index=pk % tableSize; 
  counter--;
  BucketHashNode *tmp, **try0=&table[index];
  while(*try0){
    if ((*try0)->prim_key==pk && (*try0)->sec_key==sk){
      tmp = *try0;
      *try0=tmp->next;
      return (void*) tmp;
    }
    try0 = &((*try0)->next);
  }
  return NULL;
}

BucketHashNode *BucketHashTable::htFindPk(unsigned int key){
  unsigned int index=key % tableSize; 
  return table[index];}


BucketHashNode *BucketHashTable::htFindPkSk(unsigned int pk,unsigned int sk){
  unsigned int index=pk % tableSize; 
  BucketHashNode *ans = table[index];
  while(ans && (ans->prim_key != pk || ans->sec_key != sk))
    ans = ans->next;
  return  ans;
} 


