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

//
// kost@ : leftovers from 'genhashtbl.hh'.
//         For my taste, this should go too. Some other time?

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"

#define GenCast(X,XType,Y,NewType)\
{ XType tmp=X; Y= (NewType) tmp;}

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
