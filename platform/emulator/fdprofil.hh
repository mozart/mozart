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

#ifndef PROFILE_FD
# define _PROFILE_CODE1(CODE)
# define PROFILE_CODE1(CODE)
# define FDPROFILE_GC(what,sz)
#else

#ifdef INTERFACE
#pragma interface
#endif

#include "oz.h"


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
#ifdef METAVAR
  cp_size_metavar,
#endif
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
  void init(void);
  ProfileData(void) { init(); }
  
  void inc_item(int i);
  void inc_item(int i, int by);

  void print(void);

  static char * getPrintMsg1(int i);
  static char * getPrintMsg2(int i);
};


class ProfileDataTotal : public ProfileData {
private:
  unsigned min2[no_high2], max2[no_high2];
public:
  ProfileDataTotal(void) {init();}
  void init(void);
  void printTotal(unsigned n);
  void operator += (ProfileData &y);
};

class Board;

class ProfileList : public ProfileData {
private:
  ProfileList * next;
  int id;
  Board * board;
public:
  ProfileList(Board * b, int ident);
  void set_next(ProfileList * n) { next = n; }
  ProfileList * get_next(void) { return next; }
  void print(void);
  int get_id(void) { return id; }

  Board * getBoard(void) { return board; } 
  void setBoard(Board * b) { board = b; } 

  void gc(void);
};

class ProfileHost {
private:
  ProfileList * head;
  ProfileList * tail;
  ProfileList * curr;
  ProfileDataTotal total;
public:
  ProfileHost(void);
  
  void inc_item(int i) { tail->inc_item(i); }
  void inc_item(int i, int by) { tail->inc_item(i, by); }

  void reset(void) { curr = head; }
  ProfileList * next(void);
  void print(void);
  void setBoard(Board * b) { tail->setBoard(b); }

  void add(Board * bb);
  void discard(void);
  void gc(void);
  void print_total_average(void);
  void printBoardStat(Board * b);
};


# define _PROFILE_CODE1(CODE) CODE
# define PROFILE_CODE1(CODE) {_PROFILE_CODE1(CODE); }
# define FDPROFILE_GC(what,sz) \
     if (opMode == IN_TC) {  FDProfiles.inc_item(what, sz); }

extern ProfileHost FDProfiles;

class TaggedRefSet {
private:
  static TaggedRefSet * root;
  TaggedRefSet * left, * right;
  OZ_Term item;
public:
  TaggedRefSet(void);
  TaggedRefSet(OZ_Term t);
  OZ_Boolean add(OZ_Term t, TaggedRefSet * &st = root);
  void discard(TaggedRefSet * st = root);
  void print(TaggedRefSet * st = root);
};

extern TaggedRefSet FDVarsTouched;

#endif

#endif
