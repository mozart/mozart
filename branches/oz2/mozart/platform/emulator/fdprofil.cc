/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "fdprofil.hh"
#endif

#ifdef PROFILE_FD

#include <iostream.h>
#include <limits.h>

#include "fdprofil.hh"

#include "am.hh"

char * ProfileData::print_msg1[no_high1] = {
  "Number of propagators                 ",
  "Number variables copied               ",
  "Size of variables copied              ",
  "Number of entailed propagators        ",
  "Number of failed propagators          ",
  "Number of suspended propagators       ",
  "Number of touched variables           ",
  "Number of failed fd unifications      ",
  "Number of succeeded fd unifications   ",
  "Propagation from HOME to HOME (hits)  ",
  "Propagation from HOME to DEEP (hits)  ",
  "Propagation from DEEP to HOME (misses)",
  "Propagation from DEEP to DEEP (hits)  ",
  "Propagation from DEEP to DEEP (misses)",
  "Number of calls of propagate_p        ",
  "Number of susps of propagate_p        ",
  "Number of calls of checkSuspList      ",
  "Number of susps of checkSuspList      "
};
  
char * ProfileData::print_msg2[no_high2] = {
  "LITERALs           ",
  "SCRIPTs            ",
  "SUSPCONTINUATIONs  ",
  "REFSARRAYs         ",
  "CONTINUATIONs      ",
  "STUPLEs            ",
  "LTUPLEs            ",
  "RECORDs            ",
  "SUSPENSIONs        ",
  "CONDSUSPLISTs      ",
  "SUSPLISTs          ",
  "SVARs              ",
  "FDVARs             ",
  "OFSVARs            ",
  "BOOLVARs           ",
#ifdef METAVAR
  "METAVARs           ",
#endif
  "BOARDs             ",
  "ASKACTORs          ",
  "WAITACTORs         ",  
  "SOLVEACTORs        "
};

ProfileHost FDProfiles;


//-----------------------------------------------------------------------------
// Built-ins

OZ_C_proc_begin(BIfdDiscard, 0)
{ 
  FDProfiles.discard();
  cout << "Profile dicarded." << endl;
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdTotalAverage, 0)
{ 
  FDProfiles.print_total_average();
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdGetNext, 1)
{ 
  cout << "Move on to next profile." << endl;
  return OZ_unify(OZ_getCArg(0), OZ_int(FDProfiles.next() ? 1 : 0));
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdPrint, 0)
{ 
  FDProfiles.print();
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdReset, 0)
{ 
  FDProfiles.reset();
  cout << "Current profile reset." << endl;  
  return PROCEED;
}
OZ_C_proc_end


//-----------------------------------------------------------------------------
// class ProfileData

void ProfileData::init(void) { 
  { for (int i = no_high1; i--; ) items1[i] = 0; }
  { for (int i = no_high2; i--; ) items2[i].no = items2[i].size = 0; }
}

void ProfileData::inc_item(int i) { 
  if (i < 0 || no_high1 <= i) error("index");
  items1[i] += 1;
}

void ProfileData::inc_item(int i, int by) { 
  if (i < 0 || no_high2 <= i) error("index");
  items2[i].size += by; 
  items2[i].no += 1;
}

char * ProfileData::getPrintMsg1(int i) {
  if (i < 0 || i >= no_high1) error("Index overflow.");
  return print_msg1[i];
}

char * ProfileData::getPrintMsg2(int i) {
  if (i < 0 || i >= no_high2) error("Index overflow.");
  return print_msg2[i];
}

void ProfileData::print(void) {
  int i;
  for (i = 0; i < no_high1; i += 1)
    cout << "\t" << print_msg1[i] << " = " << items1[i] << endl;
  for (i = 0; i < no_high2; i += 1)
    cout << "\t" << print_msg2[i] << ": no=" << items2[i].no 
	 << " size=" << items2[i].size << endl;
}


//-----------------------------------------------------------------------------
// class ProfileDataTotal

void ProfileDataTotal::init(void) { 
  ProfileData::init();
  for (int i = no_high2; i--; ) {
    max2[i] = 0;
    min2[i] = UINT_MAX; 
  }
}

void ProfileDataTotal::printTotal(unsigned n) {
  int i;
  printf("Average of %d distributions.\n", n);
  for (i = 0; i < no_high1; i += 1)
    printf("\t%s = %u  (%.2f)\n", print_msg1[i], items1[i],
	   n ? float(items1[i]) / n : 0.0);

  unsigned sum = 0;
  for (i = 0; i < no_high2; i += 1)
    sum += items2[i].size;

  printf("\n\n Bytes copied: %u\n", sum);

  for (i = 0; i < no_high2; i += 1)
    printf("\t%s = no=%u (%.2f) size=%u (%.0f%%) (%u/%.2f/%u)\n", 
	   print_msg2[i], 
	   items2[i].no, n ? float(items2[i].no) / n : 0.0,
	   items2[i].size, 100 * float(items2[i].size) / sum,
	   min2[i] == UINT_MAX ? 0 : min2[i], 
	   n ? float(items2[i].size) / n : 0.0, max2[i]);

  printf(" Bytes copied: %u\n", sum);

  printf("Average of %d distributions.\n", n);
}

inline unsigned min(unsigned a, unsigned b) {return (a < b && 0 < a) ? a : b;}
inline unsigned max(unsigned a, unsigned b) {return a > b ? a : b;}

void ProfileDataTotal::operator += (ProfileData &y) {
  int i;
  for (i = no_high1; i--; ) {
    items1[i] += y.items1[i];
  }
  for (i = no_high2; i--; ) {
    items2[i].no += y.items2[i].no;
    items2[i].size += y.items2[i].size;
    min2[i] = min(y.items2[i].size, min2[i]);
    max2[i] = max(y.items2[i].size, max2[i]);
  }
}

//-----------------------------------------------------------------------------
// class ProfileList

ProfileList::ProfileList(Board * b, int ident) 
  : board(b), next(NULL), id(ident) {}

void ProfileList::print(void) {
  cout << "Distribution " << id << " Board (" << board << "):" << endl;
  ProfileData::print();
}

void ProfileList::gc(void) { 
  if (board) board = board->gcBoard(); 
}


//-----------------------------------------------------------------------------
// class ProfileHost

ProfileHost::ProfileHost(void) 
  : head(NULL), tail(NULL), curr(NULL) { add(NULL); }

void ProfileHost::print_total_average(void) {
  total.init();
  ProfileList * aux = head;
  int n = 0;
  while (aux) {
    total += *aux;
    n += 1;
    aux = aux->get_next();
  }
  
  total.printTotal(n - 1);
}


void ProfileHost::printBoardStat(Board * b) {
  if (b) {
    ProfileList * aux = head;

    while (aux && (aux->getBoard()!= b))
      aux = aux->get_next();
    
    if (aux) {
      aux->print();
    } else {
      printf("Board 0x%x not found.\n", b);
    }
  } else {
    printf("Board == NULL.\n");
  }
}

ProfileList * ProfileHost::next(void) {
  if (curr == tail)
    return NULL;
  return curr = curr ? curr->get_next() : head;
}

void ProfileHost::print(void) {
  if (curr)
    curr->print();
  else
    cout << "curr == NULL" << endl;
}

void ProfileHost::add(Board * bb = NULL) {
  if (head) {
    ProfileList * aux = new ProfileList(bb, tail->get_id() + 1);
    tail->set_next(aux);
    tail = aux;
  } else {
    head = tail = new ProfileList(NULL, 0);
  }
}

void ProfileHost::discard(void) {
  ProfileList * aux = head;
  while (aux) {
    ProfileList * aux_next = aux->get_next();
    delete(aux);
    aux = aux_next;
  }
  head = tail = curr = NULL;
  add();
}

void ProfileHost::gc(void) {
  ProfileList * aux = head;
  while (aux) {
    aux->gc();
    aux = aux->get_next();
  }
}

//-----------------------------------------------------------------------------
// class TaggedRefSet

TaggedRefSet * TaggedRefSet::root = NULL;

TaggedRefSet::TaggedRefSet(OZ_Term t) 
  : item(t), left(NULL), right(NULL) {}

TaggedRefSet::TaggedRefSet(void) {
  if (root) {
    cout << "Previous set was not dicarded." << endl;
    discard();
  }
}

OZ_Boolean TaggedRefSet::add(OZ_Term t, TaggedRefSet * &st) {
  if (st) {
    if (t < st->item) {
      return add(t, st->left);
    } else if (st->item < t) {
      return add(t, st->right);
    } else {
//      cout << "failed(" << (void *) t << ") ";
      return OZ_FALSE;
    }
  } else {
    st = new TaggedRefSet(t);
//      cout << "succeeded(" << (void *) t << ") ";
    return OZ_TRUE;
  }
}

void TaggedRefSet::discard(TaggedRefSet * st) {
//  cout << "()" << endl;
  if (st) {
    discard(st->left);
    discard(st->right);
    delete st;
    if (st == root)
      root = NULL;
  }
}

void TaggedRefSet::print(TaggedRefSet * st) {
  if (st) {
    cout << (void *) st->item << endl;
    print(st->left);
    print(st->right);
  }
}

TaggedRefSet FDVarsTouched;

#endif
