/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __FDPROFIL_HH__
#define __FDPROFIL_HH__

#ifdef __GNUC__
#pragma interface
#endif

#include <iostream.h>
#include <limits.h>

#include "tagged.hh"

enum ProfileDataIndex1 {
  no_props1 = 0,
  no_vars_copied,
  size_vars_copied,
  no_ent_props,
  no_failed_props,
  no_susp_props,
  no_touched_vars,
  no_failed_fdunify_vars,
  no_succ_fdunify_vars,
  from_home_to_home_hits,   // always hits
  from_home_to_deep_hits,   // always hits
  from_deep_to_home_misses, // always misses
  from_deep_to_deep_hits,
  from_deep_to_deep_misses,
  no_calls_propagate_p,
  susps_per_propagate_p,
  no_calls_checksusplist,
  susps_per_checksusplist,

  no_high1
};

enum ProfileDataIndex2 {
  cp_size_literal,
  cp_size_script,
  cp_size_suspcont,
  cp_size_refsarray,
  cp_size_cfunccont,
  cp_size_cont,
  cp_size_stuple,
  cp_size_ltuple,
  cp_size_record,
  cp_size_susp,
  cp_size_condsusplist,
  cp_size_susplist,
  cp_size_svar,
  cp_size_fdvar,
  cp_size_ofsvar,
  cp_size_boolvar,
  cp_size_metavar,
  cp_size_board,
  cp_size_askactor,
  cp_size_waitactor,
  cp_size_solveactor,

  no_high2
};

class ProfileData {
friend class ProfileDataTotal;
private:
  static char * print_msg1[no_high1], * print_msg2[no_high2];
  unsigned items1[no_high1];
  struct {unsigned no; unsigned size;} items2[no_high2];
public:
  void init(void) { 
    { for (int i = no_high1; i--; ) items1[i] = 0; }
    { for (int i = no_high2; i--; ) items2[i].no = items2[i].size = 0; }
  }
  ProfileData(void) { init(); }
  
  void inc_item(int i) { 
    if (i < 0 || no_high1 <= i) error("index");
    items1[i] += 1;
  }
  void inc_item(int i, int by) { 
    if (i < 0 || no_high2 <= i) error("index");
    items2[i].size += by; 
    items2[i].no += 1;
  }
  
  void print(void);

  static char * getPrintMsg1(int i) {
    if (i < 0 || i >= no_high1) error("Index overflow.");
    return print_msg1[i];
  }
  static char * getPrintMsg2(int i) {
    if (i < 0 || i >= no_high2) error("Index overflow.");
    return print_msg2[i];
  }
};


class ProfileDataTotal : public ProfileData {
private:
  unsigned min2[no_high2], max2[no_high2];
public:
  ProfileDataTotal(void) {init();}

  void init(void) { 
    ProfileData::init();
    for (int i = no_high2; i--; ) {
      max2[i] = 0;
      min2[i] = UINT_MAX; 
    }
  }
  
  void printTotal(unsigned n);
  void operator += (ProfileData &y);
};

class ProfileList : public ProfileData {
private:
  ProfileList * next;
  int id;
public:
  ProfileList(int ident) : next(NULL), id(ident) {}
  void set_next(ProfileList * n) { next = n; }
  ProfileList * get_next(void) { return next; }
  void print(void) {
    cout << "Distribution " << id << " :" << endl;
    ProfileData::print();
  }
  int get_id(void) { return id; }
};

class ProfileHost {
private:
  ProfileList * head;
  ProfileList * tail;
  ProfileList * curr;
  ProfileDataTotal total;
public:
  ProfileHost(void) : head(NULL), tail(NULL), curr(NULL) { add(); }
  
  void inc_item(int i) { tail->inc_item(i); }
  void inc_item(int i, int by) { tail->inc_item(i, by); }

  void reset(void) { curr = head; }
  ProfileList * next(void) {
    if (curr == tail)
      return NULL;
    return curr = curr ? curr->get_next() : head;
  }
  void print(void) {
    if (curr)
      curr->print();
    else
      cout << "curr == NULL" << endl;
  }

  void add(void) {
    if (head) {
      ProfileList * aux = new ProfileList(tail->get_id() + 1);
      tail->set_next(aux);
      tail = aux;
    } else {
      head = tail = new ProfileList(0);
    }
  }
  void discard(void) {
    ProfileList * aux = head;
    while (aux) {
      ProfileList * aux_next = aux->get_next();
      delete(aux);
      aux = aux_next;
    }
    head = tail = curr = NULL;
    add();
  }
  void print_total_average(void);
};


#if PROFILE_FD 
# define _PROFILE_CODE1(CODE) CODE
# define PROFILE_CODE1(CODE) {_PROFILE_CODE1(CODE) }
#else
# define _PROFILE_CODE1(CODE)
# define PROFILE_CODE1(CODE)
#endif

extern ProfileHost FDProfiles;

class TaggedRefSet {
private:
  static TaggedRefSet * root;
  TaggedRefSet * left, * right;
  TaggedRef item;
public:
  TaggedRefSet(void) {
    if (root) {
      cout << "Previous set was not dicarded." << endl;
      discard();
    }
  }
  TaggedRefSet(TaggedRef t) : item(t), left(NULL), right(NULL) {}
  Bool add(TaggedRef t, TaggedRefSet * &st = root);
  void discard(TaggedRefSet * st = root);
  void print(TaggedRefSet * st = root);
};

extern TaggedRefSet FDVarsTouched;

#endif




