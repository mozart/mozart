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

#include "tagged.hh"

enum ProfileDataIndex {
  no_props = 0,
  no_vars,
  sz_vars,
  no_ent_props,
  no_touched_vars,
  from_home_to_home_hits,   // always hits
  from_home_to_deep_hits,   // always hits
  from_deep_to_home_misses, // always misses
  from_deep_to_deep_hits,
  from_deep_to_deep_misses,
  no_high
};

class ProfileData {
private:
  static char * print_msg[no_high];
  unsigned items[no_high];
public:
  ProfileData(void) {
    for (int i = no_high; i--; ) items[i] = 0;
  }
  void inc_item(ProfileDataIndex i) { items[i] += 1; }
  void print(void) {
    for (int i = 0; i < no_high; i += 1)
      cout << "\t" << print_msg[i] << " = " << items[i] << endl;
  }
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
  ProfileData average;
public:
  ProfileHost(void) : head(NULL), tail(NULL), curr(NULL) { add(); }
  void inc_item(ProfileDataIndex i) { tail->inc_item(i); }

  reset(void) { curr = head; }
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
    curr = NULL;
    add();
  }
  void get_average(void) {
    cout << "Not implemented yet." << endl;
  }
};


#if PROFILE_FD == 1
# define PROFILE_CODE1(CODE) {CODE}
#else
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
