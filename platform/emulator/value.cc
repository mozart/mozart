/*
 * FBPS Saarbr"ucken
 * Author: mehl
 */

#if defined(INTERFACE)
#pragma implementation "value.hh"
#endif

#include "runtime.hh"
#include "genvar.hh"
#include "dictionary.hh"
#include "ip.hh"

/*===================================================================
 * global names and atoms
 *=================================================================== */

TaggedRef  AtomNil, AtomCons, AtomPair, AtomVoid,
  AtomSucceeded, AtomAlt, AtomMerged, AtomFailed,
  AtomEntailed, AtomSuspended, AtomBlocked,
  AtomEmpty, AtomUpper, AtomLower, AtomDigit,
  AtomCharSpace, AtomPunct, AtomOther,
  NameTrue, NameFalse, AtomBool, AtomSup, AtomCompl,
  AtomMin, AtomMax, AtomMid,
  AtomNaive, AtomSize, AtomNbSusps,

  NameOoFreeFlag,NameOoAttr,NameOoFreeFeatR,NameOoUnFreeFeat,
  NameOoFastMeth,NameOoDefaults,NameOoRequiredArg,NameOoDefaultVar,
  NameOoPrintName,NameOoLocking,

  NameUnit,
  AtomKinded, AtomDet, AtomRecord, AtomLow, AtomFSet,
  // Atoms for System.get and System.set
  AtomActive, AtomAtoms, AtomBuiltins, AtomCommitted,
  AtomCloned, AtomCode, AtomCopy, AtomCreated, AtomDebug, AtomDepth,
  AtomFeed, AtomForeign, AtomFree, AtomFreelist, AtomGC, AtomHigh,
  AtomHints, AtomIdle, AtomInt, AtomInvoked, AtomLimits, AtomLoad,
  AtomLocation, AtomMedium, AtomNames, AtomOn, AtomPropagate,
  AtomPropagators, AtomRun, AtomRunnable, AtomShowSuspension,
  AtomStopOnToplevelFailure, AtomSystem, AtomThread,
  AtomTotal,
  AtomThreshold, AtomTolerance, AtomUser, AtomVariables, AtomWidth, AtomHeap,
  AtomDebugIP, AtomDebugPerdio,
  RecordFailure,
  E_ERROR, E_KERNEL, E_OBJECT, E_TK, E_OS, E_SYSTEM,
  BI_Unify,BI_Show,BI_send;



TaggedRef getUniqueName(char *s)
{
  CHECK_STRPTR(s);
  Literal *ret = addToNameTab(s);
  ret->setFlag(Lit_isUniqueName);
  return makeTaggedLiteral(ret);
}

// Some often used constants
void initLiterals()
{
  AtomNil   = makeTaggedAtom("nil");
  AtomCons  = makeTaggedAtom("|");
  AtomPair  = makeTaggedAtom("#");
  AtomVoid  = makeTaggedAtom("_");

  AtomBool  = makeTaggedAtom("bool");
  AtomSup   = makeTaggedAtom("sup");
  AtomCompl = makeTaggedAtom("compl");

  AtomEmpty     = makeTaggedAtom("");
  AtomUpper     = makeTaggedAtom("upper");
  AtomLower     = makeTaggedAtom("lower");
  AtomDigit     = makeTaggedAtom("digit");
  AtomCharSpace = makeTaggedAtom("space");
  AtomPunct     = makeTaggedAtom("punct");
  AtomOther     = makeTaggedAtom("other");

  AtomSucceeded    = makeTaggedAtom("succeeded");
  AtomAlt          = makeTaggedAtom("alternatives");
  AtomEntailed     = makeTaggedAtom("entailed");
  AtomSuspended    = makeTaggedAtom("suspended");
  AtomBlocked      = makeTaggedAtom("blocked");
  AtomMerged       = makeTaggedAtom("merged");
  AtomFailed       = makeTaggedAtom("failed");

  NameUnit         = getUniqueName("unit");

  NameTrue         = getUniqueName(NAMETRUE);
  NameFalse        = getUniqueName(NAMEFALSE);

  NameOoAttr        = getUniqueName("ooAttr");
  NameOoFreeFeatR   = getUniqueName("ooFreeFeatR");
  NameOoFreeFlag    = getUniqueName("ooFreeFlag");
  NameOoDefaultVar  = getUniqueName("ooDefaultVar");
  NameOoRequiredArg = getUniqueName("ooRequiredArg");
  NameOoFastMeth    = getUniqueName("ooFastMeth");
  NameOoUnFreeFeat  = getUniqueName("ooUnFreeFeat");
  NameOoDefaults    = getUniqueName("ooDefaults");
  NameOoPrintName   = getUniqueName("ooPrintName");
  NameOoLocking     = getUniqueName("ooLocking");

  AtomMin     = makeTaggedAtom("min");
  AtomMax     = makeTaggedAtom("max");
  AtomMid     = makeTaggedAtom("mid");
  AtomNaive   = makeTaggedAtom("naive");
  AtomSize    = makeTaggedAtom("size");
  AtomNbSusps = makeTaggedAtom("nbSusps");

  AtomLow          = makeTaggedAtom("low");

  // For system set and get
  AtomActive                = makeTaggedAtom("active");
  AtomAtoms                 = makeTaggedAtom("atoms");
  AtomBuiltins              = makeTaggedAtom("builtins");
  AtomCommitted             = makeTaggedAtom("committed");
  AtomCloned                = makeTaggedAtom("cloned");
  AtomCode                  = makeTaggedAtom("code");
  AtomCopy                  = makeTaggedAtom("copy");
  AtomCreated               = makeTaggedAtom("created");
  AtomDebug                 = makeTaggedAtom("debug");
  AtomDepth                 = makeTaggedAtom("depth");
  // AtomFailed
  AtomFeed                  = makeTaggedAtom("feed");
  AtomForeign               = makeTaggedAtom("foreign");
  AtomFree                  = makeTaggedAtom("free");
  AtomFreelist              = makeTaggedAtom("freelist");
  AtomGC                    = makeTaggedAtom("gc");
  AtomHigh                  = makeTaggedAtom("high");
  AtomHints                 = makeTaggedAtom("hints");
  AtomIdle                  = makeTaggedAtom("idle");
  AtomInt                   = makeTaggedAtom("int");
  AtomInvoked               = makeTaggedAtom("invoked");
  AtomLimits                = makeTaggedAtom("limits");
  AtomLoad                  = makeTaggedAtom("load");
  AtomLocation              = makeTaggedAtom("location");
  // AtomMax
  AtomMedium                = makeTaggedAtom("medium");
  // AtomMin
  AtomNames                 = makeTaggedAtom("names");
  AtomOn                    = makeTaggedAtom("on");
  AtomPropagate             = makeTaggedAtom("propagate");
  AtomPropagators           = makeTaggedAtom("propagators");
  AtomRun                   = makeTaggedAtom("run");
  AtomRunnable              = makeTaggedAtom("runnable");
  AtomShowSuspension        = makeTaggedAtom("showSuspension");
  // AtomSize
  AtomStopOnToplevelFailure = makeTaggedAtom("stopOnToplevelFailure");
  // AtomSucceeded
  AtomSystem                = makeTaggedAtom("system");
  AtomThread                = makeTaggedAtom("thread");
  AtomThreshold             = makeTaggedAtom("threshold");
  AtomTolerance             = makeTaggedAtom("tolerance");
  AtomTotal                 = makeTaggedAtom("total");
  AtomUser                  = makeTaggedAtom("user");
  AtomVariables             = makeTaggedAtom("variables");
  AtomWidth                 = makeTaggedAtom("width");
  AtomHeap                  = makeTaggedAtom("heap");

  // AtomFree                  = makeTaggedAtom("free");
  AtomKinded                = makeTaggedAtom("kinded");
  AtomDet                   = makeTaggedAtom("det");
  AtomRecord                = makeTaggedAtom("record");
  AtomFSet                  = makeTaggedAtom("fset");
  // AtomInt                   = makeTaggedAtom("int");

  AtomDebugIP               = makeTaggedAtom("debugIP");
  AtomDebugPerdio           = makeTaggedAtom("debugPerdio");


  RecordFailure = OZ_record(OZ_atom("failure"),
                            OZ_cons(OZ_atom("debug"),OZ_nil()));
  OZ_putSubtree(RecordFailure,OZ_atom("debug"),NameUnit);
  OZ_protect(&RecordFailure);

  E_ERROR = makeTaggedAtom("error");
  E_KERNEL= makeTaggedAtom("kernel");
  E_OBJECT= makeTaggedAtom("object");
  E_TK    = makeTaggedAtom("tk");
  E_OS    = makeTaggedAtom("os");
  E_SYSTEM= makeTaggedAtom("system");

  BI_Unify=makeTaggedConst(builtinTab.find("="));
  BI_Show=makeTaggedConst(builtinTab.find("Show"));
  BI_send=makeTaggedConst(builtinTab.find("Send"));
}




/*===================================================================
 * Literal
 *=================================================================== */


int Name::NameCurrentNumber = 0x200000;

char *Literal::getPrintName()
{
  if (isAtom())
    return ((Atom*)this)->getPrintName();

  if (getFlags()&Lit_isNamedName)
    return ((NamedName*)this)->printName;

  return "";
}


Atom *Atom::newAtom(char *str)
{
  Atom *ret = (Atom*) malloc(sizeof(Atom));
  ret->init();
  ret->printName = str;
  ret->setOthers(strlen(str));
  return ret;
}

Name *Name::newName(Board *home)
{
  COUNT(numNewName);

  Name *ret = (Name*) heapMalloc(sizeof(Name));
  ret->init();
  ret->homeOrGName = ToInt32(home);
  ret->setOthers(NameCurrentNumber++);
  ret->setFlag(Lit_isName);
  return ret;
}


NamedName *NamedName::newNamedName(char *pn)
{
  COUNT(numNewNamedName);

  NamedName *ret = (NamedName*) malloc(sizeof(NamedName));
  ret->init();
  ret->homeOrGName = ToInt32(am.rootBoard);
  ret->setOthers(NameCurrentNumber++);
  ret->setFlag(Lit_isName|Lit_isNamedName);
  ret->printName = pn;
  return ret;
}


GName *Name::globalize()
{
  if (!hasGName()) {
    Assert(getBoard()==am.rootBoard);
    homeOrGName = ToInt32(newGName(makeTaggedLiteral(this),GNT_NAME));
    setFlag(Lit_hasGName);
  }
  return getGName();
}

void Name::import(GName *name)
{
  Assert(getBoard()==am.rootBoard);
  homeOrGName = ToInt32(name);
  setFlag(Lit_hasGName);
}


GName *Abstraction::globalize()
{
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_PROC));
  }
  return getGName();
}

GName *SChunk::globalize() {
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CHUNK));
  }
  return getGName();
}


/*===================================================================
 * ConstTerm
 *=================================================================== */

char *ObjectClass::getPrintName()
{
  TaggedRef aux = classGetFeature(NameOoPrintName);
  return aux ? tagged2Literal(aux)->getPrintName() : "???";
}


char *ConstTerm::getPrintName()
{
  switch (getType()) {
  case Co_Abstraction:
    return ((Abstraction *) this)->getPrintName();
  case Co_Object:
    return ((Object *) this)->getPrintName();
  case Co_Class:
    return ((ObjectClass *) this)->getPrintName();
  case Co_Builtin:
    return ((BuiltinTabEntry *)this)->getPrintName();
  default:
    return "UnknownConst";
  }
}

int ConstTerm::getArity()
{
  switch (getType()) {
  case Co_Abstraction: return ((Abstraction *) this)->getArity();
  case Co_Object:      return 1;
  case Co_Builtin:     return ((BuiltinTabEntry *)this)->getArity();
  default:             return -1;
  }
}

void Tertiary::setBoard(Board *b)
{
  if (getTertType() == Te_Local) {
    setPointer(b);
  } else {
    Assert(b==NULL || b==am.rootBoard);
  }
}



/*===================================================================
 * Object
 *=================================================================== */

/*
 * append two *det* lists
 *  NO ERROR CHECK!
 */
TaggedRef appendI(TaggedRef x,TaggedRef y)
{
  TaggedRef ret;
  TaggedRef *out=&ret;

  x=deref(x);
  while (isCons(x)) {
    LTuple *lt=new LTuple(head(x),makeTaggedNULL());
    *out=makeTaggedLTuple(lt);
    out=lt->getRefTail();
    x=deref(tail(x));
  }
  *out=y;
  return ret;
}

Bool member(TaggedRef elem,TaggedRef list)
{
  elem = deref(elem);
  list = deref(list);
  while (isCons(list)) {
    if (elem==deref(head(list)))
      return OK;
    list = deref(tail(list));
  }
  return NO;
}

/*
 * destructive reverse of a list
 */
TaggedRef reverseC(TaggedRef l)
{
  TaggedRef out=nil();
  l=deref(l);
  while (isCons(l)) {
    LTuple *lt=tagged2LTuple(l);
    TaggedRef next=deref(lt->getTail());
    lt->setTail(out);
    out = l;
    l = next;
  }
  Assert(isNil(l));
  return out;
}

void ObjectClass::globalize() {
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CLASS));
  }
}

TaggedRef Object::getArityList()
{
  TaggedRef ret = nil();

  SRecord *feat=getFreeRecord();
  if (feat) ret = feat->getArityList();

  SRecord *rec=getClass()->getUnfreeRecord();
  if (rec) ret=appendI(ret,rec->getArityList());
  return ret;
}

TaggedRef ObjectClass::getArityList()
{
  return features->getArityList();
}

int Object::getWidth()
{
  int ret = 0;
  SRecord *feat=getFreeRecord();
  if (feat) ret = feat->getWidth ();

  SRecord *rec=getClass()->getUnfreeRecord();
  if (rec) ret += rec->getWidth ();
  return ret;
}


int ObjectClass::getWidth()
{
  return features->getWidth();
}




Abstraction *ObjectClass::getMethod(TaggedRef label, SRecordArity arity, RefsArray X,
                               Bool &defaultsUsed)
{
  TaggedRef method;
  if (getfastMethods()->getArg(label,method)!=PROCEED)
    return NULL;

  DEREF(method,_1,_2);
  if (isAnyVar(method)) return NULL;
  Assert(isAbstraction(method));

  Abstraction *abstr = (Abstraction*) tagged2Const(method);
  defaultsUsed = NO;
  if (sameSRecordArity(abstr->getMethodArity(),arity))
    return abstr;
  defaultsUsed = OK;
  return lookupDefault(label,arity,X) ? abstr : (Abstraction*) NULL;
}

Bool ObjectClass::lookupDefault(TaggedRef label, SRecordArity arity, RefsArray X)
{
  TaggedRef def;
  if (getDefMethods()->getArg(label,def)!=PROCEED)
    return NO;

  def = deref(def);
  Assert(isSRecord(def));
  SRecord *rec = tagged2SRecord(def);

  if (rec->isTuple()) {
    if (!sraIsTuple(arity)) {
      return NO;
    }
    int widthDefault  = rec->getWidth();
    int widthProvided = getTupleWidth(arity);
    if (widthDefault < widthProvided ||
        literalEq(deref(rec->getArg(widthProvided)),NameOoRequiredArg))
      return NO;

    for (int i=widthProvided; i<widthDefault; i++) {
      if (literalEq(deref(rec->getArg(i)),NameOoDefaultVar)) {
        X[i] = oz_newVariable();
      } else {
        X[i] = rec->getArg(i);
      }
    }
    return OK;
  }

  TaggedRef auxX[100];
  if (::getWidth(arity)>=100)
    return NO;

  TaggedRef arityList = sraGetArityList(arity);

  def = rec->getArityList();

  int argno;
  int argnoProvided = 0;
  for (argno = 0; isCons(def); def = tail(def), argno++) {
    TaggedRef feat  = head(def);
    TaggedRef value = deref(rec->getArg(argno));

    if (!isNil(arityList) && featureEq(head(arityList),feat)) {
      arityList = tail(arityList);
      auxX[argno] = X[argnoProvided];
      argnoProvided++;
    } else if (literalEq(value,NameOoDefaultVar)) {
      auxX[argno] = oz_newVariable();
    } else if (literalEq(value,NameOoRequiredArg)) {
      return NO;
    } else {
      auxX[argno] = rec->getArg(argno);
    }
  }

  /* overspecified? */
  if (!isNil(arityList))
    return NO;

  while(argno>0) {
    argno--;
    X[argno] = auxX[argno];
  }

  return OK;
}

/*===================================================================
 * Bigint memory management
 *=================================================================== */

static void *bigint_alloc(size_t size)
{
  return freeListMalloc(size);
}

static void bigint_dealloc(void *ptr, size_t size)
{
  freeListDispose(ptr,size);
}

static void *bigint_realloc(void *ptr, size_t old_size, size_t new_size)
{
  void *ret = bigint_alloc(new_size);
  memcpy(ret,ptr,old_size);
  bigint_dealloc(ptr,old_size);
  return ret;
}

void bigIntInit()
{
  mp_set_memory_functions(bigint_alloc,bigint_realloc,bigint_dealloc);
}

/*===================================================================
 * SRecord
 *=================================================================== */

/*
 * make an record
 *  the subtrees are initialized with new variables
 */
TaggedRef mkRecord(TaggedRef label,SRecordArity ff)
{
  SRecord *srecord = SRecord::newSRecord(label,ff,getWidth(ff));
  srecord->initArgs(am.currentUVarPrototype());
  return makeTaggedSRecord(srecord);
}

/************************************************************************/
/*                      Useful Stuff: Lists                             */
/************************************************************************/

/*
 * Precondition: lista and listb are increasing lists of features.
 * Test, whether the two lists are equal.
 * Completely deref'd
 *
 * mm2: possible bug: two different names may have the same Id
 *      (modulo counter).
 */

static
Bool listequal(TaggedRef lista, TaggedRef listb)
{
  while (isCons(lista)) {
    if (!isCons(listb)) return NO;
    if ( !featureEq(head(lista),head(listb)) ) return NO;

    lista = tail(lista);
    listb = tail(listb);
  }
  Assert(isNil(lista));
  return isNil(listb);
}

/*
 * Precondition: list is an increasing list of atoms.
 * If a is contained in list, return a list which is structally
 * equivalent to list. Otherwise, return the list which is obtained
 * by inserting a into list.
 * Everything is deref'd
 */

static
TaggedRef insert(TaggedRef a, TaggedRef list) {

  Assert(isFeature(a));

  TaggedRef out;
  TaggedRef *ptr=&out;

  while (isCons(list)) {
    TaggedRef oldhead = head(list);
    CHECK_DEREF(oldhead);

    switch (featureCmp(a,oldhead)) {
    case 0:
      *ptr = list;
      return out;
    case -1:
      *ptr = cons(a,list);
      return out;
    case 1:
      {
        LTuple *lt = new LTuple(oldhead,makeTaggedNULL());
        *ptr = makeTaggedLTuple(lt);
        ptr = lt->getRefTail();
        list=tail(list);
      }
      break;
    default:
      error("insert");
      return 0;
    }
  }
  Assert(isNil(list));
  *ptr=cons(a,nil());

  return out;
}

/*
 * Precondition: old is an increasing list of features, ins is a list of
 * features. Return the list obtained by succesively inserting all the
 * elements of ins into old.
 * Old is deref'd
 */

static
TaggedRef insertlist(TaggedRef ins, TaggedRef old)
{
  CHECK_DEREF(old);
  DEREF(ins,_1,_2);
  CHECK_NONVAR(ins);

  while (isCons(ins)) {
    old = insert(deref(head(ins)),old);
    CHECK_DEREF(old);
    ins = deref(tail(ins));
    CHECK_NONVAR(ins);
  }

  Assert(isNil(ins));

  return old;
}

/*
 * Precondition: lista and listb are strictly increasing lists of features.
 * Return the merge of the two, without duplicates.
 * everything is deref'd
 */



static
TaggedRef merge(TaggedRef lista, TaggedRef listb)
{
  TaggedRef ret;
  TaggedRef *out = &ret;

 loop:
  if ( isNil(lista) ) {
    *out = listb;
    return ret;
  }

  if ( isNil(listb) ) {
    *out = lista;
    return ret;
  }

  Assert(isCons(lista) && isCons(listb));

  TaggedRef a = head(lista);
  TaggedRef b = head(listb);
  TaggedRef newHead;

  switch (featureCmp(a,b)) {

  case 0:
    newHead = a;
    lista = tail(lista);
    listb = tail(listb);
    break;
  case -1:
    newHead = a;
    lista = tail(lista);
    break;
  case 1:
  default:
    newHead = b;
    listb = tail(listb);
    break;
  }

  LTuple *lt = new LTuple(newHead,makeTaggedNULL());
  *out = makeTaggedLTuple(lt);
  out = lt->getRefTail();
  goto loop;
}


inline
void swap(TaggedRef** a, TaggedRef** b)
{
  register TaggedRef aux = **a;
  **a = **b;
  **b = aux;
}

static
void quicksort(TaggedRef** first, TaggedRef** last)
{
  register TaggedRef** i;
  register TaggedRef** j;

  if (first >= last)
    return;

  /* use middle element as pivot --> much better for sorted inputs */
  TaggedRef **middle = first + (last-first)/2;
  swap(first,middle);

  for (i = first, j = last; ; j--) {
    while (i != j && featureCmp(**i, **j) <= 0)
      j--;
    if (i == j)
      break;
    swap(i, j);
    do
      i++;
    while (i != j && featureCmp(**i, **j) <= 0);
    if (i == j)
      break;
    swap(i, j);
  } // for
  quicksort(first, i-1);
  quicksort(i+1, last);
}

Bool isSorted(TaggedRef list)
{
  list = deref(list);
  if (isNil(list)) return OK;

  while(1) {
    TaggedRef cdr = deref(tail(list));
    if (isNil(cdr)) return OK;
    if (featureCmp(head(list),head(cdr))!=-1) return NO;
    list = cdr;
  }
  return OK;

}


// sort list using quicksort and duplicants
TaggedRef sortlist(TaggedRef list,int len)
{
  TaggedRef** r = new TaggedRef*[len];

  // put pointers to elems of list in array r
  TaggedRef tmp = list;
  int i = 0;
  while (isCons(tmp)) {
    r[i++] = tagged2LTuple(tmp)->getRef();
    tmp = tail(tmp);
  }

  // sort array r using quicksort
  quicksort(r, r + (len - 1));

  // remove possible duplicate entries
  TaggedRef pElem = list;
  TaggedRef cElem = tagged2LTuple(list)->getTail();
  int p = 0, c = 1;
  while (isCons(cElem)) {
    LTuple* cElemPtr = tagged2LTuple(cElem);
    if (featureEq(*r[p], *r[c])) {
      tagged2LTuple(pElem)->setTail(cElemPtr->getTail());
    } else {
      pElem = cElem;
      p = c;
    }
    c += 1;
    cElem = cElemPtr->getTail();
  } // while

  delete r;
  return list;
}


static
TaggedRef packsort(TaggedRef list)
{
  if (isNil(list)) {
    return AtomNil;
  }
  int len=0;
  list=deref(list);

  TaggedRef prevEntry = makeTaggedNULL();
  TaggedRef tmp = list;

  while (isCons(tmp)) {
    len++;
    LTuple *lt=tagged2LTuple(tmp);
    lt->setHead(deref(lt->getHead()));
    tmp=deref(lt->getTail());
    lt->setTail(tmp);
  }

  if (!isNil(tmp)) return makeTaggedNULL();
  return sortlist(list,len);
}

Arity *mkArity(TaggedRef list)
{
  list=packsort(list);
  if (list == makeTaggedNULL()) return 0;
  return aritytable.find(list);
}

/************************************************************************/
/*                      Useful Stuff: Numbers                           */
/************************************************************************/

/*
 *      Return the truncate of the logarithm of i by base 2.
 */

inline
unsigned int intlog(unsigned int i)
{
  if ( i ) {
    unsigned int result = 0;
    while (i>>=1)
      result++;
    return result;
  } else {
    return 0;
  }
}

//************************************************************************
//                        Class Arity
//************************************************************************

/*
 *      Precondition: entrylist is a list of different atoms.
 *      Construct a Arity holding these atoms, assigning them all
 *      different successive indices.
 */

Arity::Arity ( TaggedRef entrylist , Bool itf)
  : isTupleFlag(itf)
{
  next = NULL;
  list = entrylist;
  DebugCheckT(numberofentries = 0);
  DebugCheckT(numberofcollisions = 0);
  if (isTupleFlag) {
    width=length(entrylist);
    DebugCheckT(indextable=0);
    DebugCheckT(keytable=0);
    return;
  }
  size = nextPowerOf2((unsigned int)(length(entrylist)*1.5));
  width = 0;
  hashmask = size-1;
  indextable = ::new int[size];
  keytable = ::new TaggedRef[size];
  for (int i=0 ; i<size ; keytable[i++] = makeTaggedNULL());
  while (isCons(entrylist)) {
    add(head(entrylist));
    entrylist = tail(entrylist);
  }
}


/*
 *      Precondition: entry is not contained in the Arity
 *      Insert entry, assigning a new index.
 */

void Arity::add( TaggedRef entry )
{
  Assert(!isTuple());
  int i=hashfold(featureHash(entry));
  int step=scndhash(entry);
  while ( keytable[i] != makeTaggedNULL() ) {
    DebugCheckT(numberofcollisions++);
    i = hashfold(i+step);
  }
  keytable[i] = entry;
  indextable[i] = width++;
  DebugCheckT(numberofentries++);
}


/************************************************************************/
/*                          Class ArityTable                            */
/************************************************************************/

/*
 *      Initialize the aritytable.
 */

ArityTable aritytable(ARITYTABLESIZE);

/*
 *      Construct an ArityTable of given size. The size should be a power of 2
 *      in order to make the hashing work.
 */

ArityTable::ArityTable ( unsigned int n )
{
  size = nextPowerOf2(n);
  table = ::new Arity*[size];
  for ( int i = 0 ; i < size; table[i++] = NULL ) ;
  hashmask = size-1;
}

/*
 * Compute the hashvalue of a list into aritytable.
 * For now, we just take the average of the hashvalues of the first three
 * entries in the list. The hashvalues of the entries are computed
 * analogously to the class Arity.
 * TODO: find a better hash heuristics!
 *
 * return NO, if no tuple
 *        OK, if tuple
 */

inline
Bool ArityTable::hashvalue( TaggedRef list, unsigned int &ret )
{
  int i = 0;
  int len = 0;
  while(isCons(list)){
    TaggedRef it=head(list);
    if (len>=0 && isSmallInt(it) && smallIntValue(it)==len+1) {
      len++;
    } else {
      len = -1;
    }
    i += featureHash(it);
    list = tail(list);
  }
  Assert(isNil(list));
  ret = hashfold(i);
  return len < 0 ? NO : OK;
}

/*
 * If list is already registered in aritytable, then return the associated
 * Arity. Otherwise, create a Hashtable, insert the new pair of
 * arity and Arity into aritytable, and return the new Arity.
 */
Arity *ArityTable::find( TaggedRef list)
{
  unsigned int hsh;
  int isTuple = hashvalue(list,hsh);

  Arity *ret;
  if ( table[hsh] == NULL ) {
    ret = ::new Arity(list,isTuple);
    table[hsh] = ret;
  } else {
    Arity* c = table[hsh];
    while ( c->next != NULL ) {
      if ( listequal(c->list,list) ) return c;
      c = c->next;
    }
    if ( listequal(c->list,list) ) return c;
    ret = ::new Arity(list,isTuple);
    c->next = ret;
  }
  return ret;
}

/************************************************************************/
/*                      Class Record                          */
/************************************************************************/

/*
 *      Construct a SRecord from SRecord old, and adjoin
 *      the pair (feature.value). This is the functionality of
 *      adjoinAt(old,feature,value) where old is a proper SRecord
 *      and feature is not contained in old.
 */

TaggedRef SRecord::adjoinAt(TaggedRef feature, TaggedRef value)
{
  if (getIndex(feature) != -1) {
    SRecord *newrec = newSRecord(this);
    newrec->setFeature(feature,value);
    return makeTaggedSRecord(newrec);
  } else {
    TaggedRef oldArityList = getArityList();
    TaggedRef newArityList = insert(feature,oldArityList);
    Arity *arity = aritytable.find(newArityList);
    SRecord *newrec = newSRecord(getLabel(),arity);

    CHECK_DEREF(oldArityList);
    while (isCons(oldArityList)) {
      TaggedRef a = head(oldArityList);
      CHECK_DEREF(a);
      newrec->setFeature(a,getFeature(a));
      oldArityList = tail(oldArityList);
      CHECK_DEREF(oldArityList);
    }
    Assert(isNil(oldArityList));
    newrec->setFeature(feature,value);
    return newrec->normalize();
  }
}

TaggedRef SRecord::adjoin(SRecord* hrecord)
{
  TaggedRef list1 = this->getArityList();
  TaggedRef list2 = hrecord->getArityList();

  // optimize case that left record is literal
  if (isNil(list1)) {
  overwrite:
    return makeTaggedSRecord(newSRecord(hrecord));
  }

  // optimize case that right record is literal
  if (isNil(list2)) {
    return makeTaggedSRecord(this->replaceLabel(hrecord->getLabel()));
  }

  // adjoin arities
  TaggedRef newArityList = merge(list1,list2);
  Arity *newArity = aritytable.find(newArityList);

  SRecord *newrec = newSRecord(hrecord->getLabel(),newArity);

  // optimize case that right record completely overwrites left side.
  if (hrecord->isTuple()) {
    if (newArity->isTuple() && hrecord->getWidth() == newArity->getWidth()) {
      goto overwrite;
    }
  } else if (newArity == hrecord->getRecordArity()) {
    goto overwrite;
  }

  // copy left record to new record
  TaggedRef ar = list1;
  CHECK_DEREF(ar);
  while (isCons(ar)) {
    TaggedRef a = head(ar);
    CHECK_DEREF(a);
    newrec->setFeature(a,getFeature(a));
    ar = tail(ar);
    CHECK_DEREF(ar);
  }

  TaggedRef har = list2;
  CHECK_DEREF(har);
  while (isCons(har)) {
    TaggedRef a = head(har);
    CHECK_DEREF(a);
    newrec->setFeature(a,hrecord->getFeature(a));
    har = tail(har);
    CHECK_DEREF(har);
  }
  return newrec->normalize();
}

/*
 * This is the functionality of adjoinlist(old,proplist). We assume
 * that arityList is the list of the keys in proplist. arityList
 * is computed by the builtin in order to ease error handling.
 */
TaggedRef SRecord::adjoinList(TaggedRef arityList,TaggedRef proplist)
{
  TaggedRef newArityList = insertlist(arityList,getArityList());
  Arity *newArity = aritytable.find(newArityList);

  SRecord *newrec = SRecord::newSRecord(getLabel(),newArity);
  Assert(length(newArityList) == newrec->getWidth());

  TaggedRef ar = getArityList();
  CHECK_DEREF(ar);
  while (isCons(ar)) {
    TaggedRef a = head(ar);
    CHECK_DEREF(a);
    newrec->setFeature(a,getFeature(a));
    ar = tail(ar);
    CHECK_DEREF(ar);
  }

  newrec->setFeatures(proplist);
  return newrec->normalize();
}


void SRecord::setFeatures(TaggedRef proplist)
{
  DEREF(proplist,_1,_2);
  CHECK_NONVAR(proplist);
  while (isCons(proplist)) {
    TaggedRef pair = head(proplist);
    DEREF(pair,_3,_4);
    CHECK_NONVAR(pair);
    proplist = deref(tail(proplist));
    CHECK_NONVAR(proplist);

    TaggedRef fea = oz_left(pair);
    DEREF(fea,_5,_6);
    CHECK_NONVAR(fea);

#ifdef DEBUG_CHECK
    if (!setFeature(fea, oz_right(pair))) {
      error("SRecord::setFeatures: improper feature: %s",
            toC(oz_left(pair)));
    }
#else
    setFeature(fea, oz_right(pair));
#endif

  }

  Assert(isNil(proplist));
}

        /*------------------------------*/
        /*      Other Services.         */
        /*______________________________*/


Bool SRecord::setFeature(TaggedRef feature,TaggedRef value)
{
  CHECK_FEATURE(feature);

  int i = getIndex(feature);
  if ( i == -1 ) {
    return NO;
  }
  setArg(i,value);
  return OK;
}

TaggedRef SRecord::replaceFeature(TaggedRef feature,TaggedRef value)
{
  CHECK_FEATURE(feature);

  int i = getIndex(feature);
  if ( i == -1 ) {
    return makeTaggedNULL();
  }

  TaggedRef oldVal = args[i];
  if (!isRef(oldVal) && isAnyVar(oldVal)) {
    return adjoinAt(feature,value);
  }
  setArg(i,value);
  return makeTaggedSRecord(this);
}

TaggedRef makeTupleArityList(int i)
{
  Assert(i>=0);
  TaggedRef out = nil();
  while (i>0) {
    out=cons(newSmallInt(i),out);
    i--;
  }
  return out;
}

/*
 * make LTuple to SRecord
 */
SRecord *makeRecord(TaggedRef t)
{
  if (isSRecord(t)) return tagged2SRecord(t);
  Assert(isLTuple(t));
  LTuple *lt=tagged2LTuple(t);
  SRecord *ret = SRecord::newSRecord(AtomCons,
                                     aritytable.find(makeTupleArityList(2)));
  ret->setArg(0,lt->getHead());
  ret->setArg(1,lt->getTail());
  return ret;
}


/*===================================================================
 * Space
 *=================================================================== */

SolveActor * Space::getSolveActor() {
  return ((SolveActor *) solve->getActor());
}

Bool Space::isFailed() {
  if (!solve) return OK;
  if (solve == (Board *) 1) return NO;
  return solve->isFailed();
}

Bool Space::isMerged() {
  if (solve == (Board *) 1) return OK;
  return NO;
}

char *toC(OZ_Term term)
{
  return OZ_toC(term,ozconf.errorPrintDepth,ozconf.errorPrintWidth);
}

/*===================================================================
 * Locks
 *=================================================================== */



/*===================================================================
 * Misc
 *=================================================================== */

DbgInfo *allDbgInfos = NULL;

PrTabEntry *PrTabEntry::allPrTabEntries = NULL;

void PrTabEntry::printPrTabEntries()
{
  PrTabEntry *aux = allPrTabEntries;
  int heapTotal = 0, callsTotal = 0, samplesTotal = 0;
  while(aux) {
    heapTotal    += aux->heapUsed;
    callsTotal   += aux->numCalled;
    samplesTotal += aux->samples;
    if (aux->numClosures || aux->numCalled || aux->heapUsed || aux->samples) {
      char *name = ozstrdup(toC(aux->printname)); // cannot have 2 toC in one line
      printf("%20.20s Created: %5d Called: %6d Heap: %5d B, Samples: %5d %s(%d)\n",
             name,aux->numClosures,aux->numCalled,aux->heapUsed,
             aux->samples,toC(aux->fileName),aux->lineno);
      delete name;
    }
    aux = aux->next;
  }

  printf("\n=============================================================\n\n");
  printf("    Total calls: %d, total heap: %d KB, samples: %d\n\n",
         callsTotal,heapTotal/KB,samplesTotal);
}


TaggedRef PrTabEntry::getProfileStats()
{
  TaggedRef ret      = nil();
  TaggedRef ps       = oz_atom("profileStats");
  TaggedRef samples  = oz_atom("samples");
  TaggedRef heap     = oz_atom("heap");
  TaggedRef calls    = oz_atom("calls");
  TaggedRef closures = oz_atom("closures");
  TaggedRef name     = oz_atom("name");
  TaggedRef line     = oz_atom("line");
  TaggedRef file     = oz_atom("file");

  TaggedRef list = cons(file,
                        cons(line,
                             cons(name,
                                  cons(samples,
                                       cons(heap,
                                            cons(calls,
                                                 cons(closures,nil())))))));
  Arity *arity = aritytable.find(sortlist(list,length(list)));

  {
    PrTabEntry *aux = allPrTabEntries;
    while(aux) {
      if (aux->numClosures || aux->numCalled || aux->heapUsed || aux->samples) {
        SRecord *rec = SRecord::newSRecord(ps,arity);
        rec->setFeature(samples,oz_int(aux->samples));
        rec->setFeature(calls,oz_int(aux->numCalled));
        rec->setFeature(heap,oz_int(aux->heapUsed));
        rec->setFeature(closures,oz_int(aux->numClosures));
        rec->setFeature(line,oz_int(aux->lineno));
        rec->setFeature(name,aux->printname);
        rec->setFeature(file,aux->fileName);
        ret = cons(makeTaggedSRecord(rec),ret);
      }
      aux = aux->next;
    }
  }

  {
    OZ_CFunHeader *aux = OZ_CFunHeader::getFirst();
    TaggedRef noname = oz_atom("");
    while(aux) {
      if (aux->getSamples() || aux->getCalls()) {
        SRecord *rec = SRecord::newSRecord(ps,arity);
        rec->setFeature(samples,oz_int(aux->getSamples()));
        rec->setFeature(calls,oz_int(aux->getCalls()));
        rec->setFeature(heap,oz_int(0));
        rec->setFeature(closures,oz_int(0));
        rec->setFeature(line,oz_int(0));
        rec->setFeature(name,oz_atom(builtinTab.getName((void *)(aux->getHeader()))));
        rec->setFeature(file,noname);
        ret = cons(makeTaggedSRecord(rec),ret);
      }
      aux = aux->getNext();
    }
  }

  return ret;
}


void PrTabEntry::profileReset()
{
  PrTabEntry *aux = allPrTabEntries;
  while(aux) {
    aux->numClosures = 0;
    aux->numCalled   = 0;
    aux->heapUsed    = 0;
    aux->samples     = 0;
    aux = aux->next;
  }
}
