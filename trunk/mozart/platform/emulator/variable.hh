#ifndef __VARIABLEH
#define __VARIABLEH

#ifdef INTERFACE
#pragma interface
#endif

SuspList * addSuspToList(SuspList * list, SuspList * elem, Board * home);


class SVariable {
  
friend TaggedRef gcVariable(TaggedRef);
friend inline void addSuspSVar(TaggedRef, SuspList *);
friend inline void addSuspUVar(TaggedRefPtr, SuspList *);
friend inline void addSuspNotCVar(TaggedRefPtr, SuspList *);
friend void addSuspAnyVar(TaggedRefPtr, SuspList *);
  
protected:
  SuspList *suspList;
  Board *home;
public:

  USEFREELISTMEMORY;

  SVariable(Board *n)
  {
    suspList = NULL;
    home = n;
  };

  void dispose(void) {
    suspList->disposeList();
#ifdef DEBUG_CHECK
    suspList = (SuspList*)-1;
#else
    freeListDispose(this,sizeof(*this));
#endif
  }

  TaggedRef DBGmakeSuspList();

  // get home node without deref, for faster isLocal
  Board *getHome1() { return home; }
  Board *getBoardFast() { return home->getBoardFast (); }
  SuspList *getSuspList() { return suspList; }
  void setSuspList(SuspList *inSuspList) { suspList = inSuspList; }
  void unlinkSuspList() { suspList = NULL; }

  void addSuspension(Suspension *susp)
  {
    extern void updateExtSuspension(Board *home, Suspension *s);
    updateExtSuspension (getBoardFast(), susp);

    suspList = new SuspList(susp,suspList);
  }

  void print(ostream &stream, int depth, int offset, TaggedRef v);
  void printLong(ostream &stream, int depth, int offset, TaggedRef v);
};

SuspList * addSuspToList(SuspList * list, SuspList * elem, Board * home);

inline
void addSuspSVar(TaggedRef v, SuspList * el)
{
  SVariable * sv = tagged2SVar(v);
  sv->suspList = addSuspToList(sv->suspList, el, sv->home);
}

inline
void addSuspUVar(TaggedRefPtr v, SuspList * el)
{
  SVariable * sv = new SVariable(tagged2VarHome(*v));
  *v = makeTaggedSVar(sv);
  sv->suspList = addSuspToList(sv->suspList, el, sv->home);
}

inline
void addSuspNotCVar(TaggedRefPtr v, SuspList * el)
{
  Assert(tagTypeOf(*v) != CVAR);
  
  SVariable * sv;
  if (isSVar(*v)) {
    sv = tagged2SVar(*v);
  } else {
    sv = new SVariable(tagged2VarHome(*v));
    *v = makeTaggedSVar(sv);
  }
  sv->suspList = addSuspToList(sv->suspList, el, sv->home);
}

void addSuspAnyVar(TaggedRefPtr v, SuspList * el);

inline
SVariable *tagged2SuspVar(TaggedRef var)
{
  Assert(isSVar(var) || isCVar(var));
  return isSVar(var) ? tagged2SVar(var)
    /* cast needed, since CVariable unknown here
     * (void*) cast needed to make gcc quite */
    : (SVariable *) (void*) tagged2CVar(var); 
}

inline
SVariable *taggedBecomesSuspVar(TaggedRef *ref)
{
  TaggedRef val = *ref;
  Assert(isAnyVar(val));
  if (isUVar(val)){
    SVariable *ret  = new SVariable(tagged2VarHome(val));
    *ref = makeTaggedSVar(ret);
    return ret;
  }
  return tagged2SuspVar(val);
}

/*
 * Class VariableNamer: assign names to variables
 */

class VariableNamer {
private:
  static VariableNamer *allnames;
  TaggedRef var;
  TaggedRef name;
  VariableNamer *next;
  int length();
public:
  static void cleanup();
  static TaggedRef getName(TaggedRef var);
  static void addName(TaggedRef var, TaggedRef name);
};

#endif
