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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __GENHASHTABLEH
#define __GENHASHTABLEH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"

#define GenCast(X,XType,Y,NewType)\
{ XType tmp=X; Y= (NewType) tmp;}

#define GENHASHNODE_CUTOFF 100

class FreeListEntry{
friend class FreeListManager;
private:
  FreeListEntry *next;
};

class FreeListManager{
  FreeListEntry* free;
  int cutoff;
  int no_free;

public:
  FreeListManager(int i):free(NULL),cutoff(i),no_free(0){}

  Bool putOne(FreeListEntry *f){
    if(no_free==cutoff){return FALSE;}
    Assert(no_free<cutoff);
    f->next=free;
    free=f;
    no_free++;
    return TRUE;}

  FreeListEntry *getOne(){
    if(free==NULL) {return NULL;}
    FreeListEntry *f=free;
    free=free->next;
    no_free--;
    return f;}

#ifdef DEBUG_PERDIO

  int length(){
    int ct=0;
    FreeListEntry *f=free;
    while(f!=NULL){
      f= f->next;
      ct++;}
    return ct;}
#endif
};

class GenHashBaseKey{
unsigned int dummy;
};

class GenHashEntry{
unsigned int dummy;
};

#define FREE_ENTRY  ((GenHashEntry *)~1)

class GenHashNode {
friend class GenHashTable;
protected:
  int key;
  GenHashBaseKey * basekey;
  GenHashEntry *entry;
  GenHashNode *next;
#ifdef DEBUG_PERDIO
  int getLength();
#endif
  Bool isEmpty()  {return entry==FREE_ENTRY;}

  void makeEmpty() {entry=FREE_ENTRY;}

  void b_set(int k,GenHashBaseKey *kb,GenHashEntry *e) {
    Assert(e!=FREE_ENTRY);
    key=k;
    basekey=kb;
    entry=e;}

  void set(int k,GenHashBaseKey *kb,GenHashEntry *e) {
    b_set(k,kb,e);
    next=NULL;}

  void setWithNext(int k,GenHashBaseKey *kb,GenHashEntry *e,GenHashNode *ne) {
    set(k,kb,e);
    next=ne;}

  void copyFrom(GenHashNode *from){
    key=from->key;
    next=from->next;
    basekey=from->basekey;
    entry=from->entry;}

public:
  void setNext(GenHashNode *n){next=n;}
  GenHashNode *getNext(){return next;}
  GenHashBaseKey *getBaseKey(){return basekey;}
  GenHashEntry *getEntry(){return entry;}
  void setEntry(GenHashEntry *e){ entry=e;}
  int getKey(){return key;}
};

class GenHashNodeManager: public FreeListManager{
public:
  GenHashNodeManager():FreeListManager(GENHASHNODE_CUTOFF){
    Assert(sizeof(GenHashNode)>=sizeof(void*));}
  
  GenHashNode *newGenHashNode(){
    FreeListEntry *f=getOne();
    if(f==NULL){return new GenHashNode();}
    GenHashNode *ghn;
    GenCast(f,FreeListEntry*,ghn,GenHashNode*);
    return ghn;}

  void deleteGenHashNode(GenHashNode* ghn){
    FreeListEntry *f;
    GenCast(ghn,GenHashNode*,f,FreeListEntry*);
    if(putOne(f)) {return;}
    delete ghn;
    return;}
};


class GenHashTable {
protected:
  int counter;      // number of entries
  double top_percent;      // if more than percent is used, we reallocate
  double bottom_percent;
  int minSize;
  int tableSize;
  GenHashNodeManager *manager;
  void init(int low,int high) {
    int i;
    for(i=low; i<high; i++) {
      table[i].makeEmpty();}}
  void basic_htAdd(int,GenHashBaseKey *,GenHashEntry *);
  void rehash(GenHashNode *,int);
  void resize();
  void calc_percents();
  GenHashNode* getByIndex(int&);

public:
  void compactify();
  GenHashNode * table; /* TODO -move to private */
  void clear(){
    counter=0;
    init(0,tableSize);
  }
  int getSize(){return tableSize;}
  int getUsed(){return counter;}
  GenHashTable(int);
  void htAdd(int,GenHashBaseKey*,GenHashEntry*);
  Bool htSub(int,GenHashNode*);
  GenHashNode* htFindFirst(int);
  GenHashNode* getFirst(int&);
  GenHashNode* getNext(GenHashNode*,int&);
  GenHashNode* htFindNext(GenHashNode*,int);
  GenHashBaseKey *htGetBaseKey(int i) {return table[i].getBaseKey();}
  GenHashEntry *htGetEntry(int i) {return table[i].getEntry();}
  int htGetKey(int i) {return table[i].getKey();}
  void deleteFirst(GenHashNode*);
  void deleteNonFirst(GenHashNode*,GenHashNode*);
#ifdef DEBUG_PERDIO
  void printStatistics();
  GenHashNode *getElem(int);
#endif
};



class Construct_4{
  void* one;
  void* two;
  void* three;
  void* four;
public:
  Construct_4(){one=NULL;two=NULL;three=NULL;four=NULL;}
};

class Construct_3{
  void* one;
  void* two;
  void* three;
public:
  Construct_3(){one=NULL;two=NULL;three=NULL;}
};

class Construct_2{
  void *one;
  void *two;
public:
  Construct_2(){one=NULL;two=NULL;}
};

#define CUTOFF_4  500
#define CUTOFF_3  500
#define CUTOFF_2  500

class GenFreeListManager{
  FreeListManager* flm_4;
  FreeListManager* flm_3; // used for OwnerCreditExtennsion,BorrowCreditExtension, 
                          // InformElem, Chain
  FreeListManager* flm_2; // used for ChainElem
  
public:
  
  GenFreeListManager(){
    flm_4=new FreeListManager(CUTOFF_4);
    flm_3=new FreeListManager(CUTOFF_3);
    flm_2=new FreeListManager(CUTOFF_2);}

  void putOne_4(FreeListEntry *f){
    if(flm_4->putOne(f)) return;
    Construct_4 *tmp=(Construct_4*) f;
    delete tmp;
    return;}

  void putOne_3(FreeListEntry *f){
    if(flm_3->putOne(f)) return;
    Construct_3 *tmp=(Construct_3*) f;
    delete tmp;
    return;}

  void putOne_2(FreeListEntry *f){
    if(flm_2->putOne(f)) return;
    Construct_2 *tmp=(Construct_2*) f;
    delete tmp;
    return;}

  FreeListEntry *getOne_3(){
    FreeListEntry* tmp=flm_3->getOne();
    if(tmp!=NULL) return tmp;
    return (FreeListEntry*) new Construct_3();}

  FreeListEntry *getOne_4(){
    FreeListEntry* tmp=flm_4->getOne();
    if(tmp!=NULL) return tmp;
    return (FreeListEntry*) new Construct_4();}

  FreeListEntry *getOne_2(){
    FreeListEntry* tmp=flm_2->getOne();
    if(tmp!=NULL) return tmp;
    return (FreeListEntry*) new Construct_2();}
};

extern GenFreeListManager *genFreeListManager;



#endif







