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

#include "base.hh"

class BucketHashNode {
friend class BucketHashTable;
protected:
  unsigned int prim_key;
  unsigned int sec_key;
  BucketHashNode *next;
public:
  BucketHashNode(){}
  
  BucketHashNode(unsigned int k,unsigned int kb) {
    prim_key=k;
    sec_key=kb;
    next=NULL;}

  BucketHashNode(unsigned int k,unsigned int kb, BucketHashNode* ne){
    prim_key=k;
    sec_key=kb;
    next=ne;}
  
  void setNext(BucketHashNode *n){next=n;}
  BucketHashNode *getNext(){return next;}
  BucketHashNode **getNextPtr(){return &next;}
  unsigned int getSecKey(){return sec_key;}
  unsigned int getPrimKey(){return prim_key;}
};



class BucketHashTable {
protected:
  int counter;      // number of entries
  double top_percent;      // if more than percent is used, we reallocate
  double bottom_percent;
  int minSize;
  int tableSize;
  
  void init(int low,int high) {
    int i;
    for(i=low; i<high; i++) {
      table[i] = NULL;}}
  void basic_htAdd(unsigned int,BucketHashNode *);
  void rehash(BucketHashNode**,int);
  void resize();
  void calc_percents();

public:
  void compactify();
  BucketHashNode **table;
  void clear(){
    counter=0;
    for(int i=0; i<tableSize; i++) 
      {
	BucketHashNode *tmp, *next = table[i];
	while(next)
	  {
	    tmp = next;
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
  void htAdd(unsigned int,BucketHashNode*);
  void htSubPkEn(unsigned int,BucketHashNode *);
  void* htSubPkSk(unsigned int,unsigned int);
  BucketHashNode* htFindPk(unsigned int);
  BucketHashNode* htFindPkSk(unsigned int,unsigned int);
  
};

#endif
