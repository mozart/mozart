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
  "Number of calls of propagate_p        ",
  "Number of susps of propagate_p        ",
  "Number of calls of checkSuspList      ",
  "Number of susps of checkSuspList    ",

  "Number of copied LITERALs             ",
  "Size of copied LITERALs               ",
  "Number of copied SCRIPTs              ",
  "Size of copied SCRIPTs                ",
  "Number of copied SUSPCONTINUATIONs    ",
  "Size of copied SUSPCONTINUATIONs      ",
  "Number of copied REFSARRAYs           ",
  "Size of copied REFSARRAYs             ",
  "Number of copied CFUNCCONTINUATIONs   ",
  "Size of copied CFUNCCONTINUATIONs     ",
  "Number of copied CONTINUATIONs        ",
  "Size of copied CONTINUATIONs          ",
  "Number of copied STUPLEs              ",
  "Size of copied STUPLEs                ",
  "Number of copied LTUPLEs              ",
  "Size of copied LTUPLEs                ",
  "Number of copied RECORDs              ",
  "Size of copied RECORDs                ",
  "Number of copied SUSPENSIONs          ",
  "Size of copied SUSPENSIONs            ",
  "Number of copied CONDSUSPLISTs        ",
  "Size of copied CONDSUSPLISTs          ",
  "Number of copied SUSPLISTs            ",
  "Size of copied SUSPLISTs              ",
  "Number of copied SVARs                ",
  "Size of copied SVARs                  ",
  "Number of copied FDVARs               ",
  "Size of copied FDVARs                 ",
  "Number of copied OFSVARs              ",
  "Size of copied OFSVARs                ",
  "Number of copied BOARDs               ",
  "Size of copied BOARDs                 ",
  "Number of copied ASKACTORs            ",
  "Size of copied ASKACTORs              ",
  "Number of copied WAITACTORs           ",
  "Size of copied WAITACTORs             ",
  "Number of copied SOLVEACTORs          ",
  "Size of copied SOLVEACTORs            ",

  "Number of boolean FDVARs              ",
  "Bytes saved for boolean FDVARs        ",
  "Number of bitvector domains           ",
  "Bytes saved for bitvector domains     ",
  "Number of interval list domains       ",
  "Bytes saved for interval list domains ",
};

ProfileHost FDProfiles;

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

void ProfileData::print(void) {
  for (int i = 0; i < no_high; i += 1)
    cout << "\t" << print_msg[i] << " = " << items[i] << endl;
}

void ProfileData::operator += (ProfileData &y) {
  for (int i = no_high; i--; ) {
    items[i] += y.items[i];
  }
}

void ProfileHost::print_total_average(void) {
  total.init();
  ProfileList * aux = head;
  int n = 0;
  while (aux) {
    total += *aux;
    n += 1;
    aux = aux->get_next();
  }

  printf("Average of %d distributions.\n", n - 1);
  for (int i = 0; i < no_high; i += 1)
    printf("\t%s = %u  (%.2f)\n",
           ProfileData::getPrintMsg(i), total.getItem(i),
           n ? float(total.getItem(i)) / n : 0.0);
  printf("Average of %d distributions.\n", n - 1);
}


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
