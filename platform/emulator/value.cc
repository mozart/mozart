/*
 * FBPS Saarbr"ucken
 * Author: mehl
 * Last modified: $Date$ from $Author$
 * Version: $Revision$
 * State: $State$
 */

#if defined(INTERFACE)
#pragma implementation "value.hh"
#endif

#include "types.hh"
#include "tagged.hh"
#include "value.hh"

TaggedRef  AtomNil, AtomCons, AtomPair, AtomVoid,
  AtomLess, AtomGreater, AtomSame, AtomUncomparable,
  AtomInt, AtomFloat, AtomTuple, AtomProcedure, AtomCell,
  AtomChunk,
  AtomRecord, AtomAtom, AtomName, AtomUnknown,
  AtomClosed, AtomVariable,
  NameTrue, NameFalse, AtomBool, AtomSup,
  NameGroupVoid,
  NameTclName,
  AtomTclOption, AtomTclList, AtomTclPosition,
  AtomTclQuote, AtomTclString, AtomTclVS,
  AtomTclBatch,
  AtomError,
  AtomDot, AtomTagPrefix, AtomVarPrefix, AtomImagePrefix;


// Some often used constants
void initLiterals()
{
  AtomNil  = makeTaggedAtom(NameOfNil);
  AtomCons = makeTaggedAtom(NameOfCons);
  AtomPair = makeTaggedAtom(NameOfPair);
  AtomVoid = makeTaggedAtom(NameOfVoid);
  AtomBool = makeTaggedAtom(NameOfBool);
  AtomSup  = makeTaggedAtom(NameOfSup);

  AtomLess         = makeTaggedAtom("less");
  AtomSame         = makeTaggedAtom("same");
  AtomGreater      = makeTaggedAtom("greater");
  AtomUncomparable = makeTaggedAtom("uncomparable");
  AtomInt          = makeTaggedAtom("int");
  AtomFloat        = makeTaggedAtom("float");
  AtomTuple        = makeTaggedAtom("tuple");
  AtomProcedure    = makeTaggedAtom("procedure");
  AtomCell         = makeTaggedAtom("cell");
  AtomChunk        = makeTaggedAtom("chunk");
  AtomRecord       = makeTaggedAtom("record");
  AtomAtom         = makeTaggedAtom("atom");
  AtomName         = makeTaggedAtom("name");
  AtomUnknown      = makeTaggedAtom("unknown");
  AtomClosed       = makeTaggedAtom("closed");
  AtomVariable     = makeTaggedAtom("variable");

  NameTrue         = makeTaggedName(NAMETRUE);
  NameFalse        = makeTaggedName(NAMEFALSE);
  NameGroupVoid    = makeTaggedName(NAMEGROUPVOID);

  NameTclName      = makeTaggedName("TclName");
  AtomTclOption    = makeTaggedAtom("o");
  AtomTclList      = makeTaggedAtom("l");
  AtomTclPosition  = makeTaggedAtom("p");
  AtomTclQuote     = makeTaggedAtom("q");
  AtomTclString    = makeTaggedAtom("s");
  AtomTclVS        = makeTaggedAtom("v");
  AtomTclBatch     = makeTaggedAtom("b");
  AtomError        = makeTaggedAtom("error");

  AtomDot          = makeTaggedAtom(".");
  AtomTagPrefix    = makeTaggedAtom("t");
  AtomVarPrefix    = makeTaggedAtom("v");
  AtomImagePrefix  = makeTaggedAtom("i");
}




/*
 * Literal
 */

int Literal::LiteralCurrentNumber = 0x200000;


Literal::Literal(char *str, Bool flag)
{
  printName = ozstrdup(str);
  if (flag == OK)
    home = (Board *) ToPointer(ALLBITS);  // only top-level names;
  else
    home = (Board *) NULL;
  seqNumber = LiteralCurrentNumber++;
}

Literal::Literal (Board *hb)
{
  printName = "";
  home = hb;
  /* gcc bug workaround on linux: this was before
         seqNumber = LiteralCurrentNumber;
         LiteralCurrentNumber += sizeof(Literal);
   */
  seqNumber = LiteralCurrentNumber++;
}

/*
 * ConstTerm
 */

char *ConstTerm::getPrintName()
{
  switch (getType()) {
  case Co_Abstraction:
    return ((Abstraction *) this)->getPrintName();
  case Co_Object:
    return ((Object *) this)->getPrintName();
  case Co_Builtin:
    return ((Builtin *)this)->getPrintName();
  default:
    return "UnknownConst";
  }
}

/*
 * STuple
 */


/*
 * append two *det* lists
 *  NO ERROR CHECK!
 */
static
TaggedRef append(TaggedRef x,TaggedRef y)
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

TaggedRef Object::getArityList()
{
  TaggedRef ret = getFreeRecord()->getArityList();
  if (!isClass()) {
    SRecord *rec=getClass()->getUnfreeRecord();
    if (rec) ret=append(ret,rec->getArityList());
  }
  return ret;
}

/* -----------------------------------------------------------------------
 * Bigint memory management
 * ----------------------------------------------------------------------- */

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

/*
 * SRecord
 */
/************************************************************************/
/*                      Useful Stuff: Lists                             */
/************************************************************************/

/*
 *      Precondition: lista and listb are increasing lists of literals.
 *      Test, whether the two lists are equal.
 * completely deref'd
 */


static
Bool listequal(TaggedRef lista, TaggedRef listb)
{
  while (isCons(lista)) {
    if (!isCons(listb)) return NO;
    if ( !sameLiteral(head(lista),head(listb)) ) return NO;

    lista = tail(lista);
    listb = tail(listb);
  }
  Assert(isNil(lista));
  return isNil(listb);
}

/*
 *      Precondition: list is an increasing list of atoms.
 *      If a is contained in list, return a list which is structally
 *      equivalent to list. Otherwise, return the list which is obtained
 *      by inserting a into list.

 * everything is deref'd
 */

static
TaggedRef insert(TaggedRef a, TaggedRef list) {

  Assert(isLiteral(a));

  if (isNil(list)) {
    return cons(a,list);
  }

  Assert(isCons(list));

  TaggedRef oldhead = head(list);
  CHECK_DEREF(oldhead);

  Assert(isLiteral(oldhead));

  switch (atomcmp(tagged2Literal(a),tagged2Literal(oldhead))) {

  case 0:
    return makeTaggedNULL();

  case -1:
    return cons(a,list);

  case 1:
  default:
    TaggedRef rest = insert(a,tail(list));
    return (rest == makeTaggedNULL()) ? rest : cons(oldhead,rest);
  }
}

/*
 *      Precondition: old is an increasing list of atoms, ins is a list of
 *      Atoms. Return the list obtained by succesively inserting all the
 *      elements of ins into old.
 * old is deref'd
 */

static
TaggedRef insertlist(TaggedRef ins, TaggedRef old)
{
  CHECK_DEREF(old);
  DEREF(ins,_1,_2);
  CHECK_NONVAR(ins);

  while (isCons(ins)) {
    TaggedRef aux = insert(headDeref(ins),old);
    old = aux ? aux : old;
    CHECK_DEREF(old);
    ins = tailDeref(ins);
    CHECK_NONVAR(ins);
  }

  Assert(isNil(ins));

  return(old);
}

/*
 *      Precondition: lista and listb are strictly increasing lists of atoms.
 *      Return the merge of the two, without duplicates.
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

  switch (atomcmp(tagged2Literal(a),tagged2Literal(b))) {

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


/*      Extract returns the list of heads of the list conslist in strictly
 *      increasing order. If pairlist is not a propertylist, return NULL.
 */
/*
 * Auxiliary functions for extract.
 */

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
  for (i = first, j = last; ; j--) {
    while (i != j && atomcmp(**i, **j) <= 0)
      j--;
    if (i == j)
      break;
    swap(i, j);
    do
      i++;
    while (i != j && atomcmp(**i, **j) <= 0);
    if (i == j)
      break;
    swap(i, j);
  } // for
  quicksort(first, i-1);
  quicksort(i+1, last);
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
    if (atomcmp(*r[p], *r[c]) == 0) {
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
 *      Return the minimum of 2 and the least power of 2 which is greater
 *      or equal to i.
 */

inline
unsigned int nextpowerof2 ( unsigned int i )
{
  if ( i && --i ) {
    unsigned int result = 2;
    while (i>>=1)
      result<<=1;
    return result;
  } else {
    return 2;
  }
}

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

Arity::Arity ( TaggedRef entrylist )
{
  next = NULL;
  list = entrylist;
  size = nextpowerof2((unsigned int)(lengthOfList(entrylist)*1.5));
  nextindex = 0;
  numberofentries = 0;
  numberofcollisions = 0;
  hashmask = size-1;
  indextable = ::new int[size];
  keytable = ::new Literal*[size];
  for (int i=0 ; i<size ; keytable[i++] = NULL);
  while (isCons(entrylist)) {
    add(tagged2Literal(head(entrylist)));
    entrylist = tail(entrylist);
  }
}


/*
 *      Precondition: entry is not contained in the Arity
 *      Insert entry, assigning a new index.
 */

void Arity::add( Literal *entry )
{
  int i=hashfold(entry->hash());
  int step=scndhash(entry);
  while ( keytable[i] != NULL ) {
    numberofcollisions++;
    i = hashfold(i+step);
  }
  keytable[i] = entry;
  indextable[i] = nextindex++;
  numberofentries++;
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
  size = nextpowerof2(n);
  table = ::new Arity*[size];
  for ( int i = 0 ; i < size; table[i++] = NULL ) ;
  hashmask = size-1;
}

/*
 *      Compute the hashvalue of a list into aritytable.
 *      For now, we just take the average of the hashvalues of the first three
 *      entries in the list. The hashvalues of the entries are computed
 *      analogously to the class Arity.
 *      TODO: find a better hash heuristics!
 */

inline
unsigned int ArityTable::hashvalue( TaggedRef list )
{
  int i = 0;
  while(isCons(list)){
    i += tagged2Literal(head(list))->hash();
    list = tail(list);
  }
  Assert(isNil(list));
  return hashfold(i);
}

/*
 *      If list is already registered in aritytable, then return the associated
 *      Arity. Otherwise, create a Hashtable, insert the new pair of
 *      arity and Arity into aritytable, and return the new Arity.
 */

Arity* ArityTable::find( TaggedRef list )
{
  Arity* ret;
  unsigned int hsh = hashvalue(list);
  if ( table[hsh] == NULL ) {
    ret = ::new Arity(list);
    table[hsh] = ret;
  } else {
    Arity* c = table[hsh];
    while ( c->next != NULL ) {
      if ( listequal(c->list,list) ) return c;
      c = c->next;
    }
    if ( listequal(c->list,list) ) return c;
    ret = ::new Arity(list);
    c->next = ret;
  }
  return(ret);
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

SRecord *SRecord::adjoinAt(TaggedRef feature, TaggedRef value)
{
  TaggedRef oldArityList = getArityList();
  TaggedRef newArityList = insert(feature,oldArityList);
  SRecord *newrec;
  if (!newArityList) {
    newrec = this->copySRecord();
  } else {
    newrec = newSRecord(getLabel(),aritytable.find(newArityList));

    CHECK_DEREF(oldArityList);
    while (isCons(oldArityList)) {
      TaggedRef a = head(oldArityList);
      CHECK_DEREF(a);
      newrec->setFeature(a,getFeature(a));
      oldArityList = tail(oldArityList);
      CHECK_DEREF(oldArityList);
    }

    Assert(isNil(oldArityList));
  }

  newrec->setFeature(feature,value);
  return newrec;
}

SRecord *SRecord::adjoin(SRecord* hrecord)
{
  TaggedRef list1 = this->getArityList();
  TaggedRef list2 = hrecord->getArityList();

  if (isNil(list1)) {
    return hrecord->copySRecord();
  }
  if (isNil(list2)) {
    return this->replaceLabel(hrecord->getLabel());
  }

  TaggedRef newArityList = merge(list1,list2);
  Arity *newArity = aritytable.find(newArityList);
  SRecord *newrec = newSRecord(hrecord->getLabel(),newArity);

  if (newArity != hrecord->theArity) {
    TaggedRef ar = list1;
    CHECK_DEREF(ar);
    while (isCons(ar)) {
      TaggedRef a = head(ar);
      CHECK_DEREF(a);
      newrec->setFeature(a,getFeature(a));
      ar = tail(ar);
      CHECK_DEREF(ar);
    }
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
  return newrec;
}

/*
 *      This is the functionality of adjoinlist(old,proplist). We assume
 *      that addarity is the sorted list of the keys in proplist. addlist
 *      is computed by the builtin in order to ease error handling.
 */
SRecord *SRecord::adjoinList(TaggedRef arityList,TaggedRef proplist)
{
  TaggedRef newArityList = insertlist(arityList,getArityList());
  Arity *newArity = aritytable.find(newArityList);
  SRecord *newrec = SRecord::newSRecord(getLabel(),newArity);
  Assert(lengthOfList(newArityList) == newrec->theArity->getSize());


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
  return newrec;
}

void SRecord::setFeatures(TaggedRef proplist)
{
  DEREF(proplist,_1,_2);
  CHECK_NONVAR(proplist);
  while (isCons(proplist)) {
    TaggedRef pair = head(proplist);
    DEREF(pair,_3,_4);
    CHECK_NONVAR(pair);
    proplist = tailDeref(proplist);
    CHECK_NONVAR(proplist);

    TaggedRef fea = left(pair);
    DEREF(fea,_5,_6);
    CHECK_NONVAR(fea);

#ifdef DEBUG_CHECK
    if (!setFeature(fea, right(pair))) {
      error("SRecord::setFeatures: improper feature: %s",
            OZ_toC(leftDeref(pair)));
    }
#else
    setFeature(fea, right(pair));
#endif

  }

  Assert(isNil(proplist));
}

        /*------------------------------*/
        /*      Other Services.         */
        /*______________________________*/


Bool SRecord::setFeature(TaggedRef feature,TaggedRef value)
{
  CHECK_LIT(feature);
  Assert(theArity!=NULL);

  int i = theArity->find(tagged2Literal(feature));
  if ( i == -1 ) {
    return NO;
  }
  setArg(i,value);
  return OK;
}

SRecord *SRecord::replaceFeature(TaggedRef feature,TaggedRef value)
{
  CHECK_LIT(feature);

  int i = theArity->find(tagged2Literal(feature));
  if ( i == -1 ) {
    return NULL;
  }

  TaggedRef oldVal = args[i];
  if (!isRef(oldVal) && isAnyVar(oldVal)) {
    return adjoinAt(feature,value);
  }
  setArg(i,value);
  return this;
}
