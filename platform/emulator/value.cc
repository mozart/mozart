/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(VAL_ALL)
#pragma implementation "value.hh"
#endif

#include "value.hh"
#include "dictionary.hh"
#include "am.hh"
#include "gname.hh"
#include "controlvar.hh"

#include <stdarg.h>

/*===================================================================
 * global names and atoms
 *=================================================================== */

TaggedRef 
  RecordFailure,

  _NameTrue, _NameFalse,

  BI_Unify,BI_send,BI_Delay,
  BI_load, BI_fail, BI_skip, BI_url_load, BI_obtain_native,

  BI_dot,
  BI_exchangeCell,BI_assign,BI_atRedo,
  BI_controlVarHandler,
  BI_unknown,

  __UNUSED_DUMMY_END_MARKER;

Builtin *bi_raise, *bi_raiseError;

/*===================================================================
 * Literal
 *=================================================================== */


// mm2: what's the magic here?
int Name::NameCurrentNumber = 0x200000;

const char *Literal::getPrintName()
{
  if (isAtom())
    return ((Atom*)this)->getPrintName();

  if (getFlags()&Lit_isNamedName)
    return ((NamedName*)this)->printName;

  return "";
}


Atom *Atom::newAtom(const char *str)
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
  ret->setOthers(NameCurrentNumber += 1 << sizeOfCopyCount);
  ret->setFlag(Lit_isName);
  return ret;
}

NamedName *NamedName::newNamedName(const char *pn)
{
  COUNT(numNewNamedName);

  NamedName *ret = (NamedName*) malloc(sizeof(NamedName));
  ret->init();
  Assert(oz_onToplevel());
  ret->homeOrGName = ToInt32(am.currentBoard());
  ret->setOthers(NameCurrentNumber += 1 << sizeOfCopyCount);
  ret->setFlag(Lit_isName|Lit_isNamedName);
  ret->printName = pn;
  return ret;
}

NamedName *NamedName::newCopyableName(const char *pn)
{
  NamedName *ret = newNamedName(pn);
  ret->setFlag(Lit_isCopyableName);
  return ret;
}


NamedName *NamedName::generateCopy()
{
  COUNT(numNewNamedName);

  NamedName *ret = (NamedName*) malloc(sizeof(NamedName));
  ret->init();
  Assert(oz_onToplevel() && isCopyableName());
  ret->homeOrGName = ToInt32(am.currentBoard());
  int seqNumber = getOthers();
  seqNumber++;
  Assert(seqNumber);
  setOthers(seqNumber);
  ret->setOthers(seqNumber);
  ret->setFlag(getFlags() & ~Lit_isCopyableName);
  ret->printName = printName;
  return ret;
}


void Name::import(GName *name)
{
  Assert(oz_isRootBoard(GETBOARD(this)));
  homeOrGName = ToInt32(name);
  setFlag(Lit_hasGName);
}

void initLiterals()
{
  initAtomsAndNames();

  _NameTrue  = oz_uniqueName(NAMETRUE);
  _NameFalse = oz_uniqueName(NAMEFALSE);

  RecordFailure = OZ_recordInitC("failure",
				 oz_list(OZ_pairA("debug",NameUnit),0));

  OZ_protect(&RecordFailure);
}

/*===================================================================
 * ConstTerm
 *=================================================================== */

const char *ObjectClass::getPrintName() 
{ 
  TaggedRef aux = classGetFeature(NameOoPrintName);
  return aux ? tagged2Literal(oz_deref(aux))->getPrintName() : "???";
}

const char *ConstTerm::getPrintName()
{
  switch (getType()) {
  case Co_Abstraction:
    return ((Abstraction *) this)->getPrintName();
  case Co_Object:
    return ((Object *) this)->getPrintName();
  case Co_Class:
    return ((ObjectClass *) this)->getPrintName();
  case Co_Builtin:
    return ((Builtin *)this)->getPrintName();
  default:
    return "UnknownConst";
  }
}

int ConstTerm::getArity()
{
  switch (getType()) {
  case Co_Abstraction: return ((Abstraction *) this)->getArity();
  case Co_Object:      return 1;
  case Co_Builtin:     return ((Builtin *)this)->getArity();
  default:             return -1;
  }
}

void Tertiary::setBoard(Board *b)
{
  if (getTertType() == Te_Local) {
    setPointer(b);
  } else {
    Assert(b==NULL || oz_isRootBoard(b));
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

  x=oz_deref(x);
  while (oz_isCons(x)) {
    LTuple *lt=new LTuple(oz_head(x),makeTaggedNULL());
    *out=makeTaggedLTuple(lt);
    out=lt->getRefTail();
    x=oz_deref(oz_tail(x));
  }
  *out=y;
  return ret;
}

Bool member(TaggedRef elem,TaggedRef list)
{
  elem = oz_deref(elem);
  list = oz_deref(list);
  while (oz_isCons(list)) {
    if (elem==oz_deref(oz_head(list)))
      return OK;
    list = oz_deref(oz_tail(list));
  }
  return NO;
}

/*
 * destructive reverse of a list
 */
TaggedRef reverseC(TaggedRef l)
{
  TaggedRef out=oz_nil();
  l=oz_deref(l);
  while (oz_isCons(l)) {
    LTuple *lt=tagged2LTuple(l);
    TaggedRef next=oz_deref(lt->getTail());
    lt->setTail(out);
    out = l;
    l = next;
  }
  Assert(oz_isNil(l));
  return out;
}

// make a copy of list and return its length
TaggedRef duplist(TaggedRef list, int &len)
{
  len = 0;
  TaggedRef ret = oz_nil();
  TaggedRef *aux = &ret;

  while(oz_isCons(list)) {
    len++;
    *aux = oz_cons(oz_head(list),*aux);
    aux = tagged2LTuple(*aux)->getRefTail();
    list = oz_tail(list);
  }
  return ret;
}

TaggedRef Object::getArityList() 
{
  TaggedRef ret = oz_nil();
  
  SRecord *rec=getClass()->getUnfreeRecord();
  if (rec) ret=rec->getArityList();
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

GName *Object::globalize(){
  if (!getGName1()) {
    setGName(newGName(makeTaggedConst(this),GNT_OBJECT));}
  return getGName1();
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
  if (oz_isVariable(method)) return NULL;
  Assert(oz_isAbstraction(method));
  
  Abstraction *abstr = (Abstraction*) tagged2Const(method);
  defaultsUsed = NO;
  if (sameSRecordArity(abstr->getMethodArity(),arity))
    return abstr;
  defaultsUsed = OK;
  return lookupDefault(label,arity,X) ? abstr : (Abstraction*) NULL;
}



/* X==NULL means: do not reorder X args */
Bool ObjectClass::lookupDefault(TaggedRef label, SRecordArity arity, RefsArray X)
{
  TaggedRef def;
  if (getDefMethods()->getArg(label,def)!=PROCEED)
    return NO;

  def = oz_deref(def);
  Assert(oz_isSRecord(def));
  SRecord *rec = tagged2SRecord(def);

  if (rec->isTuple()) {
    if (!sraIsTuple(arity)) {
      return NO;
    }
    int widthDefault  = rec->getWidth();
    int widthProvided = getTupleWidth(arity);
    if (widthDefault < widthProvided || 
	oz_eq(oz_deref(rec->getArg(widthProvided)),NameOoRequiredArg))
      return NO;

    if (X) {
      for (int i=widthProvided; i<widthDefault; i++) {
	if (oz_eq(oz_deref(rec->getArg(i)),NameOoDefaultVar)) {
	  X[i] = oz_newVariable();
	} else {
	  X[i] = rec->getArg(i);
	}
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
  for (argno = 0; oz_isCons(def); def = oz_tail(def), argno++) {
    TaggedRef feat  = oz_head(def);
    TaggedRef value = oz_deref(rec->getArg(argno));

    if (!oz_isNil(arityList) && featureEq(oz_head(arityList),feat)) {
      arityList = oz_tail(arityList);
      if (X)
	auxX[argno] = X[argnoProvided];
      argnoProvided++;
    } else if (oz_eq(value,NameOoDefaultVar)) {
      if (X)
	auxX[argno] = oz_newVariable();
    } else if (oz_eq(value,NameOoRequiredArg)) {
      return NO;
    } else {
      if (X)
	auxX[argno] = rec->getArg(argno);
    }
  }

  /* overspecified? */
  if (!oz_isNil(arityList))
    return NO;
  
  if (X) {
    while(argno>0) {
      argno--;
      X[argno] = auxX[argno];
    }
  }

  return OK;
}

TaggedRef ObjectClass::getFallbackNew() {
  TaggedRef fbs = oz_deref(classGetFeature(NameOoFallback));

  if (!oz_isSRecord(fbs)) 
    return 0;

  SRecord * sr = tagged2SRecord(fbs);

  TaggedRef fbn = oz_deref(sr->getFeature(AtomNew));

  if (!oz_isAbstraction(fbn) || tagged2Const(fbn)->getArity() != 3)
    return 0;

  return fbn;
}

TaggedRef ObjectClass::getFallbackApply() {
  TaggedRef fbs = oz_deref(classGetFeature(NameOoFallback));

  if (!oz_isSRecord(fbs)) 
    return 0;

  SRecord * sr = tagged2SRecord(fbs);

  TaggedRef fba = oz_deref(sr->getFeature(AtomApply));

  if (!oz_isAbstraction(fba) || tagged2Const(fba)->getArity() != 2)
    return 0;

  return fba;
}

/*===================================================================
 * Bigints
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


BigInt *newBigInt()                { return new BigInt(); }
BigInt *newBigInt(long i)          { return new BigInt(i); }
BigInt *newBigInt(unsigned long i) { return new BigInt(i); }
BigInt *newBigInt(int i)           { return new BigInt(i); }
BigInt *newBigInt(unsigned int i)  { return new BigInt(i); }
BigInt *newBigInt(char *s)         { return new BigInt(s); }



/*===================================================================
 * SRecord
 *=================================================================== */


void SRecord::initArgs()
{
  for (int i = getWidth(); i--; )
    args[i] = oz_newVariable();
}

/************************************************************************/
/*			Useful Stuff: Lists				*/
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
  while (oz_isCons(lista)) {
    if (!oz_isCons(listb)) return NO; 
    if ( !featureEq(oz_head(lista),oz_head(listb)) ) return NO;
    
    lista = oz_tail(lista);
    listb = oz_tail(listb);
  }
  Assert(oz_isNil(lista));
  return oz_isNil(listb);
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

  Assert(oz_isFeature(a));

  TaggedRef out;
  TaggedRef *ptr=&out;

  while (oz_isCons(list)) {
    TaggedRef oldhead = oz_head(list);
    CHECK_DEREF(oldhead);

    switch (featureCmp(a,oldhead)) {
    case 0:
      *ptr = list;
      return out;
    case -1:
      *ptr = oz_cons(a,list);
      return out;
    case 1:
      {
	LTuple *lt = new LTuple(oldhead,makeTaggedNULL());
	*ptr = makeTaggedLTuple(lt);
	ptr = lt->getRefTail();
	list=oz_tail(list);
      }
      break;
    default:
      OZ_error("insert");
      return 0;
    }
  }
  Assert(oz_isNil(list));
  *ptr=oz_cons(a,oz_nil());

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

  while (oz_isCons(ins)) {
    old = insert(oz_deref(oz_head(ins)),old);
    CHECK_DEREF(old);
    ins = oz_deref(oz_tail(ins));
    CHECK_NONVAR(ins);
  }

  Assert(oz_isNil(ins));

  return old;
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
  list = oz_deref(list);
  if (oz_isNil(list)) return OK;

  while(1) {
    TaggedRef cdr = oz_deref(oz_tail(list));
    if (oz_isNil(cdr)) return OK;
    if (featureCmp(oz_head(list),oz_head(cdr))!=-1) return NO;
    list = cdr;
  }
  return OK;
  
}


// mm2: optimize for already sorted list! (see isSorted)
// sort list using quicksort and duplicants
TaggedRef sortlist(TaggedRef list,int len)
{
  NEW_TEMP_ARRAY(TaggedRef*, r, len);
    
  // put pointers to elems of list in array r
  TaggedRef tmp = list;
  int i = 0;
  while (oz_isCons(tmp)) {
    r[i++] = tagged2LTuple(tmp)->getRef();
    tmp = oz_tail(tmp);
  }
    
  // sort array r using quicksort
  quicksort(r, r + (len - 1));

  // remove possible duplicate entries 
  TaggedRef pElem = list;
  TaggedRef cElem = tagged2LTuple(list)->getTail();
  int p = 0, c = 1;
  while (oz_isCons(cElem)) {
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
    
  DELETE_TEMP_ARRAY(r);
  return list;
}

// mm2: cycle test
TaggedRef packsort(TaggedRef list)
{
  list=oz_deref(list);
  if (oz_isNil(list)) {
    return oz_nil();
  }
  int len=0;

  TaggedRef tmp = list;
  
  while (oz_isCons(tmp)) {
    len++;
    LTuple *lt=tagged2LTuple(tmp);
    lt->setHead(oz_deref(lt->getHead()));
    tmp=oz_deref(lt->getTail());
    lt->setTail(tmp);
  }

 if (!oz_isNil(tmp)) return 0;

 return sortlist(list,len);
}


//************************************************************************
//                        Class Arity
//************************************************************************
  
/*	
 *	Precondition: entrylist is a list of different atoms.
 *	Construct a Arity holding these atoms, assigning them all
 *	different successive indices.
 */

Arity *Arity::newArity(TaggedRef entrylist , Bool itf)
{
  int w = oz_fastlength(entrylist);

  if (itf) {
    Arity *ar = (Arity *) (void *) new char[sizeof(Arity)];
    ar->next = NULL;
    ar->list = entrylist;
    ar->hashmask = 0;
    ar->width = w;
    return ar;
  }

  int size  = nextPowerOf2((int)(w*1.5));
  Arity *ar = (Arity *) (void *) new char[sizeof(Arity)+
					 sizeof(KeyAndIndex)*size];

  DebugCheckT(ar->numberOfCollisions = 0);
  ar->next = NULL;
  ar->list = entrylist;
  ar->width = w;
  ar->hashmask = size-1;
  int j=0;
  for (int i=0 ; i<size ; ar->table[i++].key = 0);
  while (oz_isCons(entrylist)) {
    const TaggedRef entry = oz_head(entrylist);
    const int hsh         = featureHash(entry);
    int i                 = ar->hashfold(hsh);
    const int step        = ar->scndhash(hsh);
    while (ar->table[i].key) {
      DebugCheckT(ar->numberOfCollisions++);
      i = ar->hashfold(i+step);
    }
    ar->table[i].key   = entry;
    ar->table[i].index = j++;
    entrylist = oz_tail(entrylist);
  }
  return ar;
}


int Arity::lookupInternal(TaggedRef entry)
{
  Assert(!isTuple());
  const int hsh  = featureHash(entry);

  int i          = hashfold(hsh);
  const int step = scndhash(hsh);
  while (1) {
    const TaggedRef key = table[i].key;
    if (!key) return -1;
    if (featureEq(key,entry)) {
      return table[i].index;
    }
    i = hashfold(i+step);
  }
}

/************************************************************************/
/*                          Class ArityTable                            */
/************************************************************************/

/*
 *	Initialize the aritytable.
 */

ArityTable aritytable(ARITYTABLESIZE);

/*
 *	Construct an ArityTable of given size. The size should be a power of 2
 *	in order to make the hashing work.
 */

ArityTable::ArityTable ( int n )
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
Bool ArityTable::hashvalue( TaggedRef list, int &ret )
{
  int i = 0;
  int len = 0;
  while(oz_isCons(list)){
    TaggedRef it=oz_head(list);
    if (len>=0 && oz_isSmallInt(it) && smallIntValue(it)==len+1) {
      len++;
    } else {
      len = -1;
    }
    i += featureHash(it);
    list = oz_tail(list);
  }
  Assert(oz_isNil(list));
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
  int hsh;
  int isTuple = hashvalue(list,hsh);

  Arity *ret;
  if ( table[hsh] == NULL ) {
    ret = Arity::newArity(list,isTuple);
    table[hsh] = ret;
  } else {
    Arity* c = table[hsh];
    while ( c->next != NULL ) {
      if ( listequal(c->list,list) ) return c;
      c = c->next;
    }
    if ( listequal(c->list,list) ) return c;
    ret = Arity::newArity(list,isTuple);
    c->next = ret;
  }
  return ret;
}


void ArityTable::printStat()
{
  int ec=0,ne=0,na=0,ac=0;
  for (int i = 0 ; i < size ; i ++) {
    Arity *ar = table[i];
    while (ar) {
      na++;
      ne += ar->getWidth();
      ec += ar->getCollisions();
      ar = ar->next;
      if (ar) ac++;
    }
  }
  printf("Aritytable statistics\n");
  printf("Arities:          %d\n", na);
  printf("Arity collisions: %d\n", ac);
  printf("Entries:          %d\n", ne);
  printf("Entry collisions: %d\n", ec);
}

/************************************************************************/
/*                      Class Record                          */
/************************************************************************/


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
  if (oz_isNil(lista)) {
    *out = listb;
    return ret;
  }

  if (oz_isNil(listb)) {
    *out = lista;
    return ret;
  }

  Assert(oz_isCons(lista) && oz_isCons(listb));

  TaggedRef a = oz_head(lista);
  TaggedRef b = oz_head(listb);
  TaggedRef newHead;

  switch (featureCmp(a,b)) {
    
  case 0:
    newHead = a;
    lista = oz_tail(lista);
    listb = oz_tail(listb);
    break;
  case -1:
    newHead = a;
    lista = oz_tail(lista);
    break;
  case 1:
  default:
    newHead = b;
    listb = oz_tail(listb);
    break;
  }

  LTuple *lt = new LTuple(newHead,makeTaggedNULL());
  *out = makeTaggedLTuple(lt);
  out = lt->getRefTail();
  goto loop;
}

TaggedRef oz_adjoin(SRecord *lrec, SRecord* hrecord)
{
  TaggedRef list1 = lrec->getArityList();
  TaggedRef list2 = hrecord->getArityList();

  // adjoin arities
  TaggedRef newArityList = merge(list1,list2);
  Arity *newArity = aritytable.find(newArityList);

  SRecord *newrec = SRecord::newSRecord(hrecord->getLabel(),newArity);

  // optimize case that right record completely overwrites left side.
  if (hrecord->isTuple()) {
    if (newArity->isTuple() && hrecord->getWidth() == newArity->getWidth()) {
      return SRecord::newSRecord(hrecord)->normalize();
    }
  } else if (newArity == hrecord->getRecordArity()) {
    return makeTaggedSRecord(SRecord::newSRecord(hrecord));
  }

  // copy left record to new record
  TaggedRef ar = list1;
  CHECK_DEREF(ar);
  while (oz_isCons(ar)) {
    TaggedRef a = oz_head(ar);
    CHECK_DEREF(a);
    newrec->setFeature(a,lrec->getFeature(a));
    ar = oz_tail(ar);
    CHECK_DEREF(ar);
  }

  TaggedRef har = list2;
  CHECK_DEREF(har);
  while (oz_isCons(har)) {
    TaggedRef a = oz_head(har);
    CHECK_DEREF(a);
    newrec->setFeature(a,hrecord->getFeature(a));
    har = oz_tail(har);
    CHECK_DEREF(har);
  }
  return newrec->normalize();
}

/*
 *	Construct a SRecord from SRecord old, and adjoin
 *	the pair (feature.value). This is the functionality of
 *	adjoinAt(old,feature,value) where old is a proper SRecord
 *	and feature is not contained in old.
 */

TaggedRef oz_adjoinAt(SRecord *rec, TaggedRef feature, TaggedRef value)
{
  if (rec->getIndex(feature) != -1) {
    SRecord *newrec = SRecord::newSRecord(rec);
    newrec->setFeature(feature,value);
    return newrec->normalize();
  } else {
    TaggedRef oldArityList = rec->getArityList();
    TaggedRef newArityList = insert(feature,oldArityList);
    Arity *arity = aritytable.find(newArityList);
    SRecord *newrec = SRecord::newSRecord(rec->getLabel(),arity);

    CHECK_DEREF(oldArityList);
    while (oz_isCons(oldArityList)) {
      TaggedRef a = oz_head(oldArityList);
      CHECK_DEREF(a);
      newrec->setFeature(a,rec->getFeature(a));
      oldArityList = oz_tail(oldArityList);
      CHECK_DEREF(oldArityList);
    }
    Assert(oz_isNil(oldArityList));
    newrec->setFeature(feature,value);
    return newrec->normalize();
  }
}

/* 
 * This is the functionality of adjoinlist(old,proplist). We assume
 * that arityList is the list of the keys in proplist. arityList
 * is computed by the builtin in order to ease error handling.
 */
TaggedRef oz_adjoinList(SRecord *lrec,TaggedRef arityList,TaggedRef proplist)
{
  TaggedRef newArityList = insertlist(arityList,lrec->getArityList());
  Arity *newArity = aritytable.find(newArityList);

  SRecord *newrec = SRecord::newSRecord(lrec->getLabel(),newArity);
  Assert(oz_fastlength(newArityList) == newrec->getWidth());

  TaggedRef ar = lrec->getArityList();
  CHECK_DEREF(ar);
  while (oz_isCons(ar)) {
    TaggedRef a = oz_head(ar);
    CHECK_DEREF(a);
    newrec->setFeature(a,lrec->getFeature(a));
    ar = oz_tail(ar);
    CHECK_DEREF(ar);
  }

  newrec->setFeatures(proplist);
  return newrec->normalize();
}


void SRecord::setFeatures(TaggedRef proplist)
{
  DEREF(proplist,_1,_2);
  CHECK_NONVAR(proplist);
  while (oz_isCons(proplist)) {
    TaggedRef pair = oz_head(proplist);
    DEREF(pair,_3,_4);
    CHECK_NONVAR(pair);
    proplist = oz_deref(oz_tail(proplist));
    CHECK_NONVAR(proplist);

    TaggedRef fea = oz_left(pair);
    DEREF(fea,_5,_6);
    CHECK_NONVAR(fea);

#ifdef DEBUG_CHECK
    if (!setFeature(fea, oz_right(pair))) {
      OZ_error("SRecord::setFeatures: improper feature: %s",
	    toC(oz_left(pair)));
    }
#else
    setFeature(fea, oz_right(pair));
#endif

  }

  Assert(oz_isNil(proplist));
}

	/*------------------------------*/
	/* 	Other Services.		*/
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
  if (!oz_isRef(oldVal) && oz_isVariable(oldVal)) {
    return oz_adjoinAt(this,feature,value);
  }
  setArg(i,value);
  return makeTaggedSRecord(this);
}

TaggedRef makeTupleArityList(int i)
{
  Assert(i>=0);
  TaggedRef out = oz_nil();
  while (i>0) {
    out=oz_cons(newSmallInt(i),out);
    i--;
  }
  return out;
}

/*
 * make LTuple to SRecord
 */
SRecord *makeRecord(TaggedRef t)
{
  if (oz_isSRecord(t)) return tagged2SRecord(t);
  Assert(oz_isLTuple(t));
  LTuple *lt=tagged2LTuple(t);
  SRecord *ret = SRecord::newSRecord(AtomCons,
				     aritytable.find(makeTupleArityList(2)));
  ret->setArg(0,lt->getHead());
  ret->setArg(1,lt->getTail());
  return ret;
}
  

/*===================================================================
 * Misc
 *=================================================================== */

DbgInfo *allDbgInfos = NULL;

PrTabEntry *PrTabEntry::allPrTabEntries = NULL;

void PrTabEntry::printPrTabEntries()
{
  PrTabEntry *aux = allPrTabEntries;
  unsigned int heapTotal = 0, callsTotal = 0, samplesTotal = 0;
  while(aux) {
    heapTotal    += aux->heapUsed;
    callsTotal   += aux->numCalled;
    samplesTotal += aux->samples;
    if (aux->numClosures || aux->numCalled || aux->heapUsed || aux->samples ||
	aux->szVars) {
      char *name = ozstrdup(toC(aux->printname)); // cannot have 2 toC in one line
      printf("%20.20s Created: %5u Called: %6u Heap: %5u B, Samples: %5u, Vars: %5u",
	     name,aux->numClosures,aux->numCalled,aux->heapUsed,
	     aux->samples,aux->szVars);
      printf(" %s(%d,%d)\n",
	     OZ_atomToC(aux->file),
	     aux->line,aux->colum);
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
  TaggedRef ret      = oz_nil();
  TaggedRef ps       = oz_atom("profileStats");
  TaggedRef samples  = oz_atom("samples");
  TaggedRef heap     = oz_atom("heap");
  TaggedRef calls    = oz_atom("calls");
  TaggedRef closures = oz_atom("closures");
  TaggedRef name     = oz_atom("name");
  TaggedRef line     = oz_atom("line");
  TaggedRef column   = oz_atom("column");
  TaggedRef file     = oz_atom("file");

  TaggedRef list = oz_cons(file,
		   oz_cons(line,
		   oz_cons(column,
		   oz_cons(name,
		   oz_cons(samples,
		   oz_cons(heap,
		   oz_cons(calls,
		   oz_cons(closures,
		   oz_nil()))))))));
  Arity *arity = aritytable.find(sortlist(list,oz_fastlength(list)));

  {
    PrTabEntry *aux = allPrTabEntries;
    while(aux) {
      if (aux->numClosures || aux->numCalled || aux->heapUsed || aux->samples) {
	SRecord *rec = SRecord::newSRecord(ps,arity);
	rec->setFeature(samples,oz_unsignedInt(aux->samples));
	rec->setFeature(calls,oz_unsignedInt(aux->numCalled));
	rec->setFeature(heap,oz_unsignedInt(aux->heapUsed));
	rec->setFeature(closures,oz_unsignedInt(aux->numClosures));
	rec->setFeature(line,OZ_int(aux->line));
	rec->setFeature(column,OZ_int(aux->colum));
	rec->setFeature(name,aux->printname);
	rec->setFeature(file,aux->file);
	ret = oz_cons(makeTaggedSRecord(rec),ret);
      }
      aux = aux->next;
    }
  }

  {
    OZ_PropagatorProfile *aux = OZ_PropagatorProfile::getFirst();
    TaggedRef noname = oz_atom("");
    while(aux) {
      if (aux->getSamples() || aux->getCalls()) {
	SRecord *rec = SRecord::newSRecord(ps,arity);
	rec->setFeature(samples,oz_unsignedInt(aux->getSamples()));
	rec->setFeature(calls,oz_unsignedInt(aux->getCalls()));
	rec->setFeature(heap,oz_unsignedInt(aux->getHeap()));
	rec->setFeature(closures,oz_int(0));
	rec->setFeature(line,oz_int(1));
	rec->setFeature(name,oz_atom(aux->getPropagatorName()));
	rec->setFeature(file,noname);
	ret = oz_cons(makeTaggedSRecord(rec),ret);
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
    aux->szVars      = 0;
    aux = aux->next;
  }
}

#include "opcodes.hh"
#include "codearea.hh"

void PrTabEntry::patchFileAndLine()
{
  Reg reg;
  int next;
  TaggedRef predName, fil;
  int lin,colu;
  CodeArea::getDefinitionArgs(PC-sizeOf(DEFINITION),reg,next,fil,lin,colu,predName);
  file  = fil;
  line  = lin;
  colum = colu;
}

CodeArea *PrTabEntry::getCodeBlock() 
{
  if (codeBlock == NULL) {
    codeBlock = CodeArea::findBlock(getPC());
  }
  return codeBlock;
}


int featureEqOutline(TaggedRef a, TaggedRef b)
{
  Assert(a != b); // already check in featureEq

  return bigIntEq(a,b);
}

//
//
// Virtual sites;
inline
Bool oz_isForeignPointer(TaggedRef term)
{
  term = oz_deref(term);
  return oz_isConst(term)
    && tagged2Const(term)->getType() == Co_Foreign_Pointer;
}

void* OZ_getForeignPointer(TaggedRef t)
{
  if (! oz_isForeignPointer(t)) {
    OZ_warning("Foreign pointer expected in OZ_getForeignPointer.\n Got 0x%x. Result unspecified.\n",t);
    return NULL;
  }
  return ((ForeignPointer*)tagged2Const(oz_deref(t)))->getPointer();
}

int OZ_isForeignPointer(TaggedRef t)
{
  return oz_isForeignPointer(oz_deref(t));
}

OZ_Term OZ_makeForeignPointer(void*p)
{
  ForeignPointer * fp = new ForeignPointer(p);
  return makeTaggedConst(fp);
}

ForeignPointer*
openForeignPointer(TaggedRef t)
{
  return (ForeignPointer*)tagged2Const(oz_deref(t));
}

TaggedRef oz_long(long i)
{
  return (new BigInt(i))->shrink();
}

TaggedRef oz_unsignedLong(unsigned long i)
{
  return (new BigInt(i))->shrink();
}

Board *oz_rootBoardOutline() { return oz_rootBoard(); }

/* 
 * simplified list generation, e.g.
 *  oz_list(oz_atom("a"),
 *          oz_atom("b"),
 *          oz_atom("c"),
 *          0)
 * returns the Oz list [a b c].
 */
OZ_Term oz_list(OZ_Term t1, ...)
{
  va_list ap;
  va_start(ap,t1);

  LTuple *lt=new LTuple();
  OZ_Term ret=makeTaggedLTuple(lt);
  lt->setHead(t1);
  while (1) {
    OZ_Term t2 = va_arg(ap,OZ_Term);
    if (!t2) break;
    LTuple *nl=new LTuple();
    lt->setTail(makeTaggedLTuple(nl));
    lt=nl;
    lt->setHead(t2);
  }

  lt->setTail(oz_nil());
  va_end(ap);
  return ret;
}

/*===================================================================
 * BuiltinTab
 *=================================================================== */

Builtin * cfunc2Builtin(void * f) {
  extern TaggedRef dictionary_of_builtins;
  OzDictionary * d = tagged2Dictionary(dictionary_of_builtins);
  for (int i=d->getFirst(); i>=0; i=d->getNext(i)) {
    TaggedRef v = d->getValue(i);
    if (v && oz_isBuiltin(v)) {
      Builtin * bi = tagged2Builtin(v);
      if (bi->getFun() == (OZ_CFun) f) return bi;
    }
  }
  return tagged2Builtin(BI_unknown);
}

/*===================================================================
 * LockLocal
 *=================================================================== */

void LockLocal::unlockComplex(){
  setLocker(pendThreadResumeFirst(&pending));
  return;}

void LockLocal::lockComplex(Thread *t){
  // mm2: ignoring the return is badly wrong
  (void) pendThreadAddToEndEmul(getPendBase(),t,getBoardInternal());}

void LockSecEmul::unlockPending(Thread* th){
  Assert(th!=NULL);
  PendThread** pt=&pending;
  while((*pt)->thread!=th){
    pt= &((*pt)->next);
    if((*pt)==NULL) return;}
  *pt=(*pt)->next;}

/*===================================================================
 * PendThread
 *=================================================================== */

OZ_Return pendThreadAddToEndEmul(PendThread **pt,Thread *t, Board *home)
{
  while(*pt!=NULL){pt= &((*pt)->next);}

  ControlVarNew(controlvar,home);
  *pt=new PendThread(t,NULL,0,0,controlvar,NOEX);
  SuspendOnControlVar;
}

Thread* pendThreadResumeFirst(PendThread **pt){
  PendThread *tmp=*pt;
  Assert(tmp!=NULL);
  ControlVarResume(tmp->controlvar);
  Thread *t=tmp->thread;
  Assert(t!=NULL);
  Assert(t!=(Thread*) 0x1);
  *pt=tmp->next;
  tmp->dispose();
  return t;}


// GARBAGE COLLECTION HACK
inline
void OZ_collectHeapTermUnsafe(TaggedRef & frm, TaggedRef & to) {
  if (frm)
    OZ_collectHeapTerm(frm,to);
  else
    to=frm;
}

void gcPendThreadEmul(PendThread **pt)
{
  PendThread *tmp;
  while (*pt!=NULL) {
    tmp=new PendThread((*pt)->thread->gcThread(),(*pt)->next);
    tmp->exKind = (*pt)->exKind;
    OZ_collectHeapTermUnsafe((*pt)->old,tmp->old);
    OZ_collectHeapTermUnsafe((*pt)->nw,tmp->nw);
    OZ_collectHeapTermUnsafe((*pt)->controlvar,tmp->controlvar);
    *pt=tmp;
    pt=&(tmp->next);
  }
}
