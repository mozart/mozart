/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
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

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "genhashtbl.hh"
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "genhashtbl.hh"

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
/*      HIDDEN MEMBER FUNCTIONS                                           */
/* ********************************************************************** */


inline void 
GenHashTable::basic_htAdd(int ke,GenHashBaseKey* kb, GenHashEntry *en){
  int index=ke % tableSize;
  GenHashNode *main= &table[index];
  if(main->isEmpty()){
      main->set(ke,kb,en);
      return;}
  Assert(!(kb==main->getBaseKey()));
  GenHashNode *ne=manager->newGenHashNode();
  ne->setWithNext(ke,kb,en,main->getNext());
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
  GenHashNode *oldtable=table;
  table= (GenHashNode *) malloc(newSize * sizeof(GenHashNode));
  Assert(table!=NULL);
  if(table==NULL){
    OZ_error("Memory allocation: could not resize Hash Table");}
  init(0,newSize);
  int oldSize=tableSize;
  tableSize=newSize;
  rehash(oldtable,oldSize);
  calc_percents();
  free(oldtable);}

void GenHashTable::compactify(){
  if(counter>=bottom_percent) return;
  if(tableSize==minSize) return;
  Assert(tableSize>=minSize);
  int newSize = nextPrime2((int) ( ((double) counter) / IDEALFULL));
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

GenHashNode *GenHashTable::getElem(int i){
  if(table[i].isEmpty()) return NULL;
  return &table[i];}

void GenHashTable::htAdd(int bigIndex,GenHashBaseKey* key,GenHashEntry *entry){
  if (counter > top_percent) resize();  
  counter++;
  basic_htAdd(bigIndex,key,entry);}

Bool GenHashTable::htSub(int bigIndex,GenHashNode *cur){
  int index=bigIndex % tableSize; 
  counter--;
  GenHashNode *try0=&table[index];
  if(try0==cur){
    if(cur->next==NULL){
      cur->makeEmpty();}
    else{
      try0=cur->next;
      cur->copyFrom(try0);
      manager->deleteGenHashNode(try0);
      return FALSE;
    }}
  else{
    while(try0->next!=cur) {Assert(try0!=NULL);try0=try0->next;}
    try0->next=cur->next;
    manager->deleteGenHashNode(cur);}
  return TRUE;}

void GenHashTable::deleteFirst(GenHashNode *cur){
  GenHashNode *next=cur->next;
  if(next==NULL){
    cur->makeEmpty();
    return;}
  cur->copyFrom(next);
  manager->deleteGenHashNode(next);}

void GenHashTable::deleteNonFirst(GenHashNode *before,GenHashNode *cur){
  before->next=cur->next;
  manager->deleteGenHashNode(cur);}  

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

GenFreeListManager *genFreeListManager;
				  
				  


