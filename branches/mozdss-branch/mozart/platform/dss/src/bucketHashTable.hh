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

#ifndef __BUCKET_HH
#define __BUCKET_HH

#ifdef INTERFACE  
#pragma interface
#endif


#include "base.hh"


  class BucketHashNode {
    friend class BucketHashTable;
  private:
    BucketHashNode(const BucketHashNode& bhn):prim_key(0), sec_key(0), next(NULL){} //shouldn't be used
    BucketHashNode& operator=(const BucketHashNode& bhn){ next = NULL; return *this; } // shouldn't be used
  protected:
    u32 prim_key;
    u32 sec_key;
    BucketHashNode*    next;
  public:

    BucketHashNode(const u32& k, const u32& kb):prim_key(k), sec_key(kb),next(NULL){}
    BucketHashNode(const u32& k, const u32& kb, BucketHashNode* ne):prim_key(k), sec_key(kb),next(ne){}
   
    virtual ~BucketHashNode(){}
    
    void setNext(BucketHashNode* const n){next=n;}
    BucketHashNode *getNext() const {return next;}
    BucketHashNode **getNextPtr() {return &next;}
    void setKeys(u32 p, u32 s) { prim_key = p; sec_key = s;}
    u32 getSecKey()  const {return sec_key;}
    u32 getPrimKey() const {return prim_key;}
  };



class BucketHashTable {
  protected:
    double top_percent;      // if more than percent is used, we reallocate
    double bottom_percent;
    int counter;      // number of entries
    int minSize;
  public:
    int tableSize;
    BucketHashNode **table;
  private:
    // shouldn't be used
    BucketHashTable(const BucketHashTable& bht):
      top_percent(0), bottom_percent(0), counter(0), minSize(0), tableSize(0), table(NULL){}
    BucketHashTable& operator=(const BucketHashTable&){ return *this; };
  protected:
    void init(const int& low, const int& high) {
      int i;
      for(i=low; i<high; i++) {
	table[i] = NULL;}
    }
    void basic_htAdd(const u32&, BucketHashNode* const);
    void rehash(BucketHashNode**, int);
    void resize();
    void calc_percents();

  public:
    void clear(){
      counter=0;
      for(int i=0; i<tableSize; i++){
	  BucketHashNode *next = table[i];
	  while(next) {
	      BucketHashNode *tmp = next;
	      next = next->next;
	      delete tmp;
	  }
	  table[i]=NULL;
	}
    }
  
  int getSize(){return tableSize;}
    int getUsed(){return counter;}
    BucketHashNode* getBucket(int i){return table[i];}
    BucketHashNode** getBucketPtr(int i){return &table[i];}

    BucketHashTable(int);
    virtual ~BucketHashTable();
    
    void htAdd(u32,BucketHashNode*);
    void htSubEn(BucketHashNode *);

    BucketHashNode* htFindPk(const u32& key){ return table[key % tableSize]; }
    BucketHashNode* htSubPkSk(u32, u32);
    BucketHashNode* htFindPkSk(const u32&, const u32&);
  
  BucketHashNode* htGetNext(BucketHashNode *n);
};

#endif
