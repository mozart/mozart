/*
  Perdio Project, 
  DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5261
  SICS,
  Box 1263, S-16428 Sweden,Phone (+46) 8 7521500
  Author: brand

  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "genhashtbl.hh"
#endif

#include <math.h>
#include <malloc.h>
#include <unistd.h>
#include <stdio.h>

#include "types.hh"
#include "error.hh"
#include "genhashtbl.hh"
#include "perdio_debug.hh"

#ifdef DEBUG_PERDIO
#define IFDEBUG(A) A
#else
#define IFDEBUG(A) 
#endif

const int STEP=5;
const double MAXFULL=0.75; // The max. load of HashTable
const double IDEALFULL=0.40; 
const double MINFULL=0.20; // The min. load of HashTable
const int EXPANSION_FACTOR=2;  // load .375

/* ********************************************************************** */
/*      PRIME NUMBER FUNCTIONS                                            */
/* ********************************************************************** */

inline Bool isPrime2(int prime){
  if (prime%2 == 0) {return NO;}
  for(int i=3; i<=sqrt(prime); i+=2) {
    if (prime%i == 0) {return NO;}}
  return OK;}

int nextPrime2(int prime){
  if (prime <= STEP) {
    prime = STEP+2;}
  if (prime%2 == 0) {
    prime++;}
  while(!isPrime2(prime)) {
    prime += 2;}
  return prime;}

/* ********************************************************************** */
/*      GenHashNode
/* ********************************************************************** */

#ifdef DEBUG_PERDIO
int GenHashNode::getLength(){
  Assert(!isEmpty());
  int ct=1;
  GenHashNode *try0=next;
  while(try0){
    ct++;
    try0=try0->next;}
  return ct;}
#endif

/* ********************************************************************** */
/*      HIDDEN MEMBER FUNCTIONS                                           */
/* ********************************************************************** */


inline void 
GenHashTable::basic_htAdd(int ke,GenHashBaseKey* kb, GenHashEntry *en){
  int index=ke % tableSize;
  GenHashNode *main= &table[index];
  if(main->isEmpty()){
      main->set(ke,kb,en);
      PD(HASH,"add-direct at %d",index);
      return;}
  Assert(!(kb==main->getBaseKey()));
  GenHashNode *ne=manager->newGenHashNode();
  ne->setWithNext(ke,kb,en,main->getNext());
  PD(HASH,"add-collision at %d",index);
  main->setNext(ne);}

inline void GenHashTable::rehash(GenHashNode *old,int size){
  int i;
  GenHashNode *try0;
  GenHashNode *junk;
  for(i=0;i<size;i++){
    if(!(old[i].isEmpty())){
      basic_htAdd(old[i].key,old[i].basekey,old[i].entry);
      try0=old[i].next;
      while(try0!=NULL){
	basic_htAdd(try0->key,try0->basekey,try0->entry);
	junk=try0;
	try0=try0->next;
	manager->deleteGenHashNode(junk);}}}
  return;}

void GenHashTable::calc_percents(){
  top_percent = (int) (MAXFULL * tableSize);
  bottom_percent= (int) (MINFULL * tableSize);
  if(tableSize==minSize) bottom_percent=0;
  Assert(top_percent>bottom_percent);}  

void GenHashTable::resize(){
  int newSize=nextPrime2(tableSize*EXPANSION_FACTOR);
  PD(TABLE,"hash-table resize old=%d new=%d used=%d",
		tableSize,newSize,counter);
  PERDIO_DEBUG_DO1(TABLE2,resize_hash());
  GenHashNode *oldtable=table;
  table= (GenHashNode *) malloc(newSize * sizeof(GenHashNode));
  Assert(table!=NULL);
  if(table==NULL){
    error("Memory allocation: could not resize Hash Table");}
  init(0,newSize);
  int oldSize=tableSize;
  tableSize=newSize;
  rehash(oldtable,oldSize);
  calc_percents();
  PD(TABLE,"recomplete resize hash");
  PERDIO_DEBUG_DO1(TABLE2,resize_hash());
  free(oldtable);}

void GenHashTable::compactify(){
  if(counter>=bottom_percent) return;
  if(tableSize==minSize) return;
  Assert(tableSize>=minSize);
  int newSize = nextPrime2((int) ( ((double) counter) / IDEALFULL));
  if(newSize<minSize) newSize=minSize;
  Assert(newSize<tableSize);
  PD(TABLE,"compactify hash old=%d new=%d used=%d",
		tableSize,newSize,counter);
  PERDIO_DEBUG_DO1(TABLE2,resize_hash());
  GenHashNode *oldtable=table;
  table= (GenHashNode *) malloc(newSize * sizeof(GenHashNode));
  Assert(table!=NULL);
  if(table==NULL){
    PD(TABLE,"compactify hash failed");
    return;}
  init(0,newSize);
  int oldSize=tableSize;
  tableSize=newSize;
  rehash(oldtable,oldSize);
  calc_percents();
  PD(TABLE,"hash-table resize complete");
  PERDIO_DEBUG_DO1(TABLE2,resize_hash());
  free(oldtable);}

/* ********************************************************************** */
/*      VISIBLE MEMBER FUNCTIONS                                           */
/* ********************************************************************** */

GenHashTable::GenHashTable(int s){
  tableSize = nextPrime2(s);
  counter = 0;
  calc_percents();
  minSize=tableSize;
  table = (GenHashNode *) malloc(tableSize * sizeof(GenHashNode));
  manager = new GenHashNodeManager();
  init(0,tableSize);}

#ifdef DEBUG_PERDIO
GenHashNode *GenHashTable::getElem(int i){
  if(table[i].isEmpty()) return NULL;
  return &table[i];}
#endif

void GenHashTable::htAdd(int bigIndex,GenHashBaseKey* key,GenHashEntry *entry){
  if (counter > top_percent) resize();  
  counter++;
  basic_htAdd(bigIndex,key,entry);}

void GenHashTable::htSub(int bigIndex,GenHashNode *cur){
  int index=bigIndex % tableSize; 
  counter--;
  GenHashNode *try0=&table[index];
  if(try0==cur){
    if(cur->next==NULL){
      PD(HASH,"sub-direct at %d - no coll",index);
      cur->makeEmpty();}
    else{
      try0=cur->next;
      cur->copyFrom(try0);
      PD(HASH,"sub-direct at %d - with coll",index);
      manager->deleteGenHashNode(try0);}}
  else{
    while(try0->next!=cur) {Assert(try0!=NULL);try0=try0->next;}
    PD(HASH,"sub-cll chain at %d",index);
    try0->next=cur->next;
    manager->deleteGenHashNode(cur);}
  return;}

GenHashNode *GenHashTable::htFindFirst(int bigIndex){
  int index=bigIndex % tableSize; 
  Assert(index>=0);
  if(table[index].isEmpty()) {return NULL;}
  if(table[index].key==bigIndex) {return &table[index];}
  GenHashNode *try0=&table[index];
  while(try0!=NULL){
    if(try0->key==bigIndex) {return try0;}
    try0=try0->next;}
  return NULL;}

GenHashNode *GenHashTable::getByIndex(int &i){
  while(TRUE){
    if(i>=tableSize) return NULL;
    if(!table[i].isEmpty()) return &table[i];
    i++;
  }
}

GenHashNode *GenHashTable::getFirst(int &i){
  i=0;
  return getByIndex(i);}

GenHashNode *GenHashTable::htFindNext(GenHashNode *try0,int bigIndex){
  Assert(try0!=NULL);
  try0=try0->next;
  while(try0!=NULL){
    if(try0->key==bigIndex) {return try0;}
    try0=try0->next;}
  return NULL;}

GenHashNode *GenHashTable::getNext(GenHashNode *ghn,int &i){
  if(ghn->next!=NULL) return ghn->next;
  i++;
  return getByIndex(i);}


#ifdef DEBUG_PERDIO
void GenHashTable::printStatistics(){
  int maxx = 0, sum = 0, collpl = 0, coll = 0;
  for(int i = 0; i < tableSize; i++) {
    if (table[i].isEmpty())
      continue;
    int l = table[i].getLength();
    maxx = maxx > l ? maxx : l;
    sum += l;
    coll  += l > 1 ? l - 1 : 0;
    collpl += l > 1 ? 1 : 0;
  }
  printf("\nHashtable-Statistics:\n");
  printf("\tmaximum bucket length     : %d\n", maxx);
  printf("\tnumber of collision places: %d\n", collpl);
  printf("\tnumber of collisions      : %d\n", coll);
  printf("\t%d table entries have been used for %d literals (%d%%)\n", 
	 tableSize, counter, counter*100/tableSize);
  printf("\tfreelistHashNodememory    : %d\n",manager->length());
}
#endif





