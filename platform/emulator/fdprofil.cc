/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(__GNUC__)
#pragma implementation "fdprofil.hh"
#endif

#include "fdprofil.hh"
#include "fdbuilti.hh"

/*
  Statistics wanted:
  - number of propagators
  - number and size of variables
  - entailed propagators
  - touched variable per distribution
  - touched variables pre generic linear eq
  - hitrate of propagation and length of suspension lists

*/


char * ProfileData::print_msg[no_high] = {
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
};

ProfileHost FDProfiles;

OZ_C_proc_begin(BIfdDiscard, 0)
{
  FDProfiles.discard();
  cout << "Profile dicarded." << endl;
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdAverage, 0)
{
  FDProfiles.get_average();
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdGetNext, 1)
{
  cout << "Move on to next profile." << endl;
  return OZ_unify(OZ_getCArg(0), newSmallInt(FDProfiles.next() ? 1 : 0));
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


TaggedRefSet * TaggedRefSet::root = NULL;

Bool TaggedRefSet::add(TaggedRef t, TaggedRefSet * &st) {
  if (st) {
    if (t < st->item) {
      return add(t, st->left);
    } else if (st->item < t) {
      return add(t, st->right);
    } else {
//      cout << "failed(" << (void *) t << ") ";
      return FALSE;
    }
  } else {
    st = new TaggedRefSet(t);
//      cout << "succeeded(" << (void *) t << ") ";
    return TRUE;
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
