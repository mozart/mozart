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

#ifndef __GENHASHTABLEH
#define __GENHASHTABLEH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"

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
    Assert(unique(f));
#ifdef _MSC_VER
    /* MSVC++ put pointer to vritual tabke into first word */
    return FALSE;
#else
    if(no_free==cutoff){return FALSE;}
    Assert(no_free<cutoff);
    f->next=free;
    free=f;
    no_free++;
    return TRUE;
#endif
}

  FreeListEntry *getOne(){
    if(free==NULL) {return NULL;}
    FreeListEntry *f=free;
    free=free->next;
    no_free--;
    return f;}


  Bool unique(FreeListEntry *f){
    int len = no_free;
    FreeListEntry *ff = free;
    while(ff!=NULL){
      len --;
      if(ff == f){
	return FALSE;}
      ff = ff->next;}
    if(len != 0){
      return FALSE;}
    return TRUE;}
  
  int length(){
    DebugCode(int ct=0;
	      FreeListEntry *f=free;
	      while(f!=NULL){
		f= f->next;
		ct++;}
	      Assert(ct==no_free);
	      )
    return no_free;
  }
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
  int getLength();
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
    ghn = (GenHashNode*) (f);
    return ghn;}

  void deleteGenHashNode(GenHashNode* ghn){
    FreeListEntry *f;
    f = (FreeListEntry*)(void*)ghn;
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
  GenHashNode *getElem(int);
};



class Construct_7{
  void* one;
  void* two;
  void* three;
  void* four;
  void* five;
  void* six;
  void* seven;
public:
  Construct_7(){one=two=three=four=five=six=seven=NULL;}
};

class Construct_6{
  void* one;
  void* two;
  void* three;
  void* four;
  void* five;
  void* six;
public:
  Construct_6(){one=NULL;two=NULL;three=NULL;four=NULL;five=NULL;six=NULL;}
};


class Construct_5{
  void* one;
  void* two;
  void* three;
  void* four;
  void* five;
public:
  Construct_5(){one=NULL;two=NULL;three=NULL;four=NULL;five=NULL;}
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

#define CUTOFF_7  500
#define CUTOFF_6  500
#define CUTOFF_5  500
#define CUTOFF_4  500
#define CUTOFF_3  500
#define CUTOFF_2  500

class GenFreeListManager{
  FreeListManager* flm_7;
  FreeListManager* flm_6; // used by defer elements
  FreeListManager* flm_5;
  FreeListManager* flm_4;
  FreeListManager* flm_3; // used for OwnerCreditExtennsion,BorrowCreditExtension, 
                          // InformElem, Chain
  FreeListManager* flm_2; // used for ChainElem

  // 
public:
  
  GenFreeListManager(){
    flm_7=new FreeListManager(CUTOFF_7);
    flm_6=new FreeListManager(CUTOFF_6);
    flm_5=new FreeListManager(CUTOFF_5);
    flm_4=new FreeListManager(CUTOFF_4);
    flm_3=new FreeListManager(CUTOFF_3);
    flm_2=new FreeListManager(CUTOFF_2);}

  void putOne_7(FreeListEntry *f){
    if(flm_7->putOne(f)) return;
    Construct_7 *tmp=(Construct_7*) f;
    delete tmp;
    return;}

  void putOne_6(FreeListEntry *f){
    if(flm_6->putOne(f)) return;
    Construct_6 *tmp=(Construct_6*) f;
    delete tmp;
    return;}

  void putOne_5(FreeListEntry *f){
    if(flm_5->putOne(f)) return;
    Construct_5 *tmp=(Construct_5*) f;
    delete tmp;
    return;}


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

  FreeListEntry *getOne_2(){
    FreeListEntry* tmp=flm_2->getOne();
    if(tmp!=NULL) return tmp;
    return (FreeListEntry*) new Construct_2();}

  FreeListEntry *getOne_3(){
    FreeListEntry* tmp=flm_3->getOne();
    if(tmp!=NULL) return tmp;
    return (FreeListEntry*) new Construct_3();}

  FreeListEntry *getOne_4(){
    FreeListEntry* tmp=flm_4->getOne();
    if(tmp!=NULL) return tmp;
    return (FreeListEntry*) new Construct_4();}

  FreeListEntry *getOne_5(){
    FreeListEntry* tmp=flm_5->getOne();
    if(tmp!=NULL) return tmp;
    return (FreeListEntry*) new Construct_5();}

  FreeListEntry *getOne_6(){
    FreeListEntry* tmp=flm_6->getOne();
    if(tmp!=NULL) return tmp;
    return (FreeListEntry*) new Construct_6();}

  FreeListEntry *getOne_7(){
    FreeListEntry* tmp=flm_7->getOne();
    if(tmp!=NULL) return tmp;
    return (FreeListEntry*) new Construct_7();}
};

extern GenFreeListManager *genFreeListManager;



#endif







