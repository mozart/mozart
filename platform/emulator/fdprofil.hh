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


  cp_no_literal,
  cp_size_literal,
  cp_no_script,
  cp_size_script,
  cp_no_suspcont,
  cp_size_suspcont,
  cp_no_refsarray,
  cp_size_refsarray,
  cp_no_cfunccont,
  cp_size_cfunccont,
  cp_no_cont,
  cp_size_cont,
  cp_no_stuple,
  cp_size_stuple,
  cp_no_ltuple,
  cp_size_ltuple,
  cp_no_record,
  cp_size_record,
  cp_no_susp,
  cp_size_susp,
  cp_no_condsusplist,
  cp_size_condsusplist,
  cp_no_susplist,
  cp_size_susplist,
  cp_no_svar,
  cp_size_svar,
  cp_no_fdvar,
  cp_size_fdvar,
  cp_no_ofsvar,
  cp_size_ofsvar,
  cp_no_board,
  cp_size_board,
  cp_no_askactor,
  cp_size_askactor,
  cp_no_waitactor,
  cp_size_waitactor,
  cp_no_solveactor,
  cp_size_solveactor,

  fd_bool,
  fd_bool_saved,
  fd_bitvector,
  fd_bitvector_saved,
  fd_intervals,
  fd_intervals_saved,

  no_high
};

class ProfileData {
private:
  static char * print_msg[no_high];
  unsigned items[no_high];
public:
  void init(void) { for (int i = no_high; i--; ) items[i] = 0; }
  ProfileData(void) { init(); }
  void inc_item(ProfileDataIndex i, int by) { items[i] += by; }
  void print(void);
  void operator += (ProfileData &y);
  unsigned getItem(int i) {return items[i];}
  static char * getPrintMsg(int i) {
    if (i < 0 || i >= no_high) error("Index overflow.");
    return print_msg[i];
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
  ProfileData total;
public:
  ProfileHost(void) : head(NULL), tail(NULL), curr(NULL) { add(); }
  void inc_item(ProfileDataIndex i, int by = 1) { tail->inc_item(i, by); }

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
    head = tail = curr = NULL;
    add();
  }
  void print_total_average(void);
};


#if PROFILE_FD == 1
# define _PROFILE_CODE1(CODE) CODE
# define PROFILE_CODE1(CODE) {_PROFILE_CODE1(CODE)}
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
