/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: many

  Variables
  ------------------------------------------------------------------------
*/

#ifndef __VARIABLEH
#define __VARIABLEH

#ifdef INTERFACE
#pragma interface
#endif

#define AddSuspToList0(List,Thread,Home)                \
{                                                       \
  if ((List) && ((List)->getElem() == Thread)) {        \
  } else {                                              \
    List = new SuspList(Thread, List);                  \
    if (Home) checkExtThread(Thread,Home);              \
  }                                                     \
}

#ifdef DEBUG_STABLE
#define AddSuspToList(List,Thread,Home)                         \
{                                                               \
  AddSuspToList0(List,Thread,Home);                             \
                                                                \
  if (board_constraints_thr != Thread) {                        \
    board_constraints_thr = Thread;                             \
    board_constraints = new SuspList(board_constraints_thr,     \
                                     board_constraints);        \
  }                                                             \
}
#else
#define AddSuspToList(List,Thread,Home) AddSuspToList0(List,Thread,Home)
#endif


#define STORE_FLAG 1
#define REIFIED_FLAG 2

class SVariable {
friend TaggedRef gcVariable(TaggedRef);
protected:
  SuspList *suspList;
  Board *home;
public:

  USEFREELISTMEMORY;

  SVariable(Board * h) : suspList(NULL), home(h) {}

  void dispose(void) {
    suspList->disposeList();
    freeListDispose(this,sizeof(*this));
  }

  // get home node without deref, for faster isLocal
  Board *getHome1() { return home; }
  Board *getHomeUpdate() {
    if (home->isCommitted()) {
      home=home->derefBoard();
    }
    return home;
  }
  Board *getBoardInternal() { return home; }
  SuspList *getSuspList() { return suspList; }
  void setSuspList(SuspList *inSuspList) { suspList = inSuspList; }
  void unlinkSuspList() { suspList = NULL; }

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

  void addSuspSVar(Thread * el, int unstable)
  {
    AddSuspToList(suspList,el,unstable?home:0);
  }

  void print(ostream &stream, int depth, int offset, TaggedRef v);
  void printLong(ostream &stream, int depth, int offset, TaggedRef v);
};

inline
void addSuspSVar(TaggedRef v, Thread * el,int unstable=TRUE)
{
  tagged2SVar(v)->addSuspSVar(el,unstable);
}

inline
void addSuspUVar(TaggedRefPtr v, Thread * el, int unstable=TRUE)
{
  SVariable *sv = new SVariable(tagged2VarHome(*v));
  *v = makeTaggedSVar(sv);
  sv->addSuspSVar(el,unstable);
}

extern
void addSuspCVarOutline(TaggedRef v, Thread *el, int unstable);
inline
void addSuspAnyVar(TaggedRefPtr v, Thread *thr,int unstable=TRUE)
{
  TaggedRef t = *v;
  if (isSVar(t)) {
    addSuspSVar(t,thr,unstable);
  } else if (isCVar(t)) {
    addSuspCVarOutline(t,thr,unstable);
  } else {
    addSuspUVar(v,thr,unstable);
  }
}

inline
SVariable *tagged2SuspVar(TaggedRef var)
{
  Assert(isSVar(var) || isCVar(var));
  return isSVar(var) ? tagged2SVar(var)
    /* cast needed, since CVariable unknown here
     * (void*) cast needed to make gcc quite */
    : (SVariable *) (void*) tagged2CVar(var);
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
