#ifndef __VARIABLEH
#define __VARIABLEH

#ifdef INTERFACE
#pragma interface
#endif

SuspList * addSuspToList(SuspList * list, SuspList * elem, Board * home);
SuspList * addSuspToList(SuspList * list, Thread * elem, Board * home);

#define STORE_FLAG 1
#define REIFIED_FLAG 2

class SVariable {
  
friend TaggedRef gcVariable(TaggedRef);
friend inline void addSuspSVar(TaggedRef, SuspList *);
friend inline void addSuspSVar(TaggedRef, Thread *);
friend inline void addSuspUVar(TaggedRefPtr, SuspList *);
friend inline void addSuspUVar(TaggedRefPtr, Thread *);
friend inline void addSuspNotCVar(TaggedRefPtr, SuspList *);
friend inline void addSuspCVar(TaggedRef, Thread *);
friend void addSuspAnyVar(TaggedRefPtr, SuspList *);
  
protected:
  SuspList *suspList;
  Board *home;
public:

  USEFREELISTMEMORY;

  SVariable(Board * h) : suspList(NULL), home(h) {}

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
  Board *getHomeUpdate() {
    if (home->isCommitted()) {
      home=home->getBoardFast();
    }
    return home;
  }
  Board *getBoardFast() { return home->getBoardFast (); }
  SuspList *getSuspList() { return suspList; }
  void setSuspList(SuspList *inSuspList) { suspList = inSuspList; }
  void unlinkSuspList() { suspList = NULL; }

  void addSuspension (Thread *thr)
  {
    thr->updateExtThread(getBoardFast());

    suspList = new SuspList(thr, suspList);
  }

  void setStoreFlag(void) {
    suspList = (SuspList *) (((long) suspList) | STORE_FLAG);
  }
  void resetStoreFlag(void) {
    suspList = (SuspList *) (((long) suspList) & ~STORE_FLAG);
  }
  OZ_Boolean testStoreFlag(void) {
    return ((long)suspList) & STORE_FLAG;
  }
  OZ_Boolean testResetStoreFlag(void) {
    OZ_Boolean r = testStoreFlag();
    resetStoreFlag();
    return r;
  }
  
  void setReifiedFlag(void) {
    suspList = (SuspList *) (((long) suspList) | REIFIED_FLAG);
  }
  void resetReifiedFlag(void) {
    suspList = (SuspList *) (((long) suspList) & ~REIFIED_FLAG);
  }
  OZ_Boolean testReifiedFlag(void) {
    return ((long)suspList) & REIFIED_FLAG;
  }
  OZ_Boolean testResetReifiedFlag(void) {
    OZ_Boolean r = testReifiedFlag();
    resetReifiedFlag();
    return r;
  }

  void print(ostream &stream, int depth, int offset, TaggedRef v);
  void printLong(ostream &stream, int depth, int offset, TaggedRef v);
};

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
void addSuspSVar(TaggedRef v, Thread * el)
{
  SVariable * sv = tagged2SVar(v);
  sv->suspList = addSuspToList(sv->suspList, el, sv->home);
}

inline
void addSuspUVar(TaggedRefPtr v, Thread * el)
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
  char *name;
  VariableNamer *next;
  int length();
public:
  static void cleanup();
  static char *getName(TaggedRef var);
  static void addName(TaggedRef var, char *name);
};

char *getVarName(TaggedRef v);

#endif
