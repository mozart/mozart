/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Michael Mehl (1997)
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

#include "codearea.hh"
#include "indexing.hh"

#define GETREGARG(pc) XRegToInt(getXRegArg(pc))

#define ISREAD(i) { int _i=(i); if (_i<maxX && xUsage[_i]==0) xUsage[_i]=1; }

#define ISREAD_TO(args) { for (int _j=args; _j--;) { ISREAD(_j); } }

#define ISWRITE(i)					\
{							\
  int _i = (i);						\
  if (_i<maxX && xUsage[_i]==0) {			\
    current->writer=new Writer(_i,current->writer);	\
    xUsage[_i]=-1;					\
  }							\
}

#define ISLOC(bi,loc) \
{						\
  {for (int _j=bi->getInArity(); _j--; ) {      \
     ISREAD(loc->getInIndex(_j)); }}	        \
  {for (int _j=bi->getOutArity(); _j--; ) {     \
     ISWRITE(loc->getOutIndex(bi,_j));}}	\
}

#define BREAK           goto outerLoop;

#define PUSH(offset)    todo = new Segment(PC+offset,todo,current->writer);

#define CONTINUE(newpc) PUSH(newpc); BREAK;


class Writer {
public:
  int i;
  Writer *next;
  Writer(int i,Writer *next) : i(i),next(next) {}
};

class Segment {
public:
  ProgramCounter pc;
  Writer *writer;
  Segment *next;
  Segment(ProgramCounter pc,Segment *next, Writer *w);
  ~Segment();
};


Segment::Segment(ProgramCounter pc,Segment *next, Writer *w)
  : pc(pc),next(next)
{
  // copy writer
  writer = 0;
  while (w) {
    writer = new Writer(w->i,writer);
    w=w->next;
  }
}


Segment::~Segment()
{
  for (Writer *w=writer; w;) {
    Writer *n=w->next;
    delete w;
    w=n;
  }
  writer = 0;
} 

typedef unsigned long bitVec;

const int maxXSize = sizeof(bitVec)*8;

class LivenessCache : public AddressHashTable{
public:
  LivenessCache(): AddressHashTable(100) {}

  int findPC(ProgramCounter from, TaggedRef *X, int maxX)
  {
    void *aux = htFind(ToInt32(from));
    if (aux==htEmpty)
      return -1;
    
    int ret = 0;
    bitVec bits = (bitVec) aux;
    for (int i=0;i<maxX; i++) {
      if ((1<<i)&bits) {
	ret = i+1;
      } else {
	if (X) 
	  X[i]= taggedVoidValue;
      }
    }
    return ret;	
  }

  void addPC(ProgramCounter from, int *xUsage, int maxX) 
  {
    if (maxX>maxXSize)
      return;
    
    bitVec aux = 0;
    for (int i=0;i<maxX; i++) {
      if (xUsage[i]==1)
	aux |= (1<<i);
    }
    htAdd(ToInt32(from),(void*) aux);
  }
};

static LivenessCache livenesscache;


int CodeArea::livenessX(ProgramCounter from, TaggedRef *X,int maxX)
{
  int ret = livenesscache.findPC(from,X,maxX);
  if (ret != -1)
    return ret;

  int *xUsage = new int[maxX];
  for (int i=maxX; i--;) xUsage[i]=0;

  ret = livenessXInternal(from,X,maxX,xUsage);
  livenesscache.addPC(from,xUsage,ret);

  delete [] xUsage;
  return ret;
}

int CodeArea::livenessXInternal(ProgramCounter from, TaggedRef *X,int maxX, int *xUsage)
{
  if (maxX<=0) maxX = NumberOfXRegisters;

  Segment *todo = new Segment(from,0,0);
  Segment *seen = 0;
  Segment *current = 0;
  ProgramCounter PC;
  goto outerLoop2;

outerLoop:
  {
    // unset write flags
    for (Writer *w = current->writer; w;) {
      Writer *t=w->next;
      Assert(w->i<maxX);
      Assert(xUsage[w->i]==-1);
      xUsage[w->i]=0;
      delete w;
      w = t;
    }
    current->writer = 0; 
  }

outerLoop2:
  {
    // terminate if stack is empty
    if (!todo) {
      int nextI=0;
      for (int i=0; i<maxX; i++) {
	if (xUsage[i]==1) {
	  nextI=max(nextI,i+1);
	} else {
	  if (X)
	    X[i]= taggedVoidValue;
	}
      }

      while (seen) {
	Segment *n = seen->next;
	delete seen;
	seen = n;
      }
      return nextI;
    }

    // next from todo list
    current = todo;
    PC = current->pc;
    todo = current->next;

    // add current to segment list
    {
      Segment **prev = &seen;
      Segment *next = *prev;
      while (next) {
	if (PC == next->pc) {
	  delete current;
	  goto outerLoop2;
	} else if (PC > next->pc) {
	  prev = &next->next;
	  next = *prev;
	} else {
	  break;
	}
      }
      current->next = next;
      *prev = current;
    }

    // install writer flags
    for (Writer *w = current->writer; w;) {
      Writer *n = w->next;
      xUsage[w->i]=-1;
      w=n;
    }

  innerLoop:
    {
      Opcode op = getOpcode(PC);

      switch (op) {
      case FASTCALL:
      case FASTTAILCALL:
      case CALLPROCEDUREREF:
	{
	  AbstractionEntry *ae = (AbstractionEntry *) getAdressArg(PC+1);
	  Abstraction *ab = ae->getAbstr();
	  if (ab==0) {
	    ISREAD_TO(getPosIntArg(PC+2)>>1);
	  } else {
	    ISREAD_TO(ab->getArity());
	  }
	  BREAK;
	}
      case CALLGLOBAL:
      case CALLCONSTANT:
	{
	  int i  = getPosIntArg(PC+2)>>1;
	  ISREAD_TO(i);
	  BREAK;
	}

      case CALLBI:
	ISLOC(GetBI(PC+1),GetLoc(PC+2));
	break;

      case TESTBI:
	ISLOC(GetBI(PC+1),GetLoc(PC+2));
	PUSH(getLabelArg(PC+3));
	break;

      case INLINEPLUS1:
      case INLINEMINUS1:
	ISREAD(GETREGARG(PC+1));
	ISWRITE(GETREGARG(PC+2)); // must be last
	break;

      case INLINEPLUS:
      case INLINEMINUS:
	ISREAD(GETREGARG(PC+1));
	ISREAD(GETREGARG(PC+2));
	ISWRITE(GETREGARG(PC+3)); // must be last
	break;

      case INLINEDOT:
	ISREAD(GETREGARG(PC+1));
	ISWRITE(GETREGARG(PC+3));
	break;

      case INLINEAT:
	ISWRITE(GETREGARG(PC+2));
	break;

      case INLINEASSIGN:
	ISREAD(GETREGARG(PC+2));
	break;

      case TESTLE:
      case TESTLT:
	ISREAD(GETREGARG(PC+1));
	ISREAD(GETREGARG(PC+2));
	ISWRITE(GETREGARG(PC+3));
	PUSH(getLabelArg(PC+4));
	break;

      case RETURN:
      case ENDDEFINITION:
	BREAK;

      case EXHANDLER:
	CONTINUE(getLabelArg(PC+1));

      case LOCKTHREAD:
	ISREAD(GETREGARG(PC+2));
	break;

      case DEFINITIONCOPY:
      case DEFINITION:
	{
	  AssRegArray *list           = (AssRegArray*) getAdressArg(PC+5);
	  int size = list->getSize();
	  for (int i = 0; i < size; i++) {
	    if (((*list)[i].getKind()) == K_XReg)
	      ISREAD((*list)[i].getIndex());
	  }

	  if (op==DEFINITIONCOPY) { // Michael please check !!! 
	    ISREAD(GETREGARG(PC+1));
	  }
	  ISWRITE(GETREGARG(PC+1));
	  CONTINUE(getLabelArg(PC+2));
	}

      case BRANCH:
	{
	  int lbl = getLabelArg(PC+1);
	  CONTINUE(lbl);
	}

      case TAILSENDMSGX:
      case SENDMSGX:
	ISREAD(GETREGARG(PC+2));
	// fall through
      case TAILSENDMSGY:
      case TAILSENDMSGG:
      case SENDMSGY:
      case SENDMSGG:
	{
	  SRecordArity arity = (SRecordArity) getAdressArg(PC+3);
	  ISREAD_TO(getWidth(arity));
	  BREAK;
	}
      case CALLX:
      case TAILCALLX:
      case CONSCALLX:
      case TAILCONSCALLX:
	ISREAD(GETREGARG(PC+1));
	// fall through
      case CALLY:
      case CALLG:
      case TAILCALLG:
      case CONSCALLY:
      case CONSCALLG:
      case TAILCONSCALLG:
	ISREAD_TO(getPosIntArg(PC+2));
	BREAK;

      case DECONSCALLX:
      case TAILDECONSCALLX:
	ISREAD(GETREGARG(PC+1));
	// fall through
      case DECONSCALLY:
      case DECONSCALLG:
      case TAILDECONSCALLG:
	ISREAD_TO(2);
	BREAK;

      case CALLMETHOD:
	{
	  CallMethodInfo *cmi = (CallMethodInfo*)getAdressArg(PC+1);
	  ISREAD_TO(getWidth(cmi->arity));
	  BREAK;
	}

      case MOVEXX:
	ISREAD(GETREGARG(PC+1));
	ISWRITE(GETREGARG(PC+2));
	break;

      case MOVEXY:
	ISREAD(GETREGARG(PC+1));
	break;
      case MOVEYX:
      case MOVEGX:
	ISWRITE(GETREGARG(PC+2));
	break;
      case MOVEMOVEXYXY:
	ISREAD(GETREGARG(PC+1));
	ISREAD(GETREGARG(PC+3));
	break;
      case MOVEMOVEYXYX:
	ISWRITE(GETREGARG(PC+2));
	ISWRITE(GETREGARG(PC+4));
	break;
      case MOVEMOVEYXXY:
	ISWRITE(GETREGARG(PC+2));
	ISREAD(GETREGARG(PC+3));
	break;
      case MOVEMOVEXYYX:
	ISREAD(GETREGARG(PC+1));
	ISWRITE(GETREGARG(PC+4));
	break;
      case GETSELF:
	ISWRITE(GETREGARG(PC+1));
	break;
      case GETRETURNX:
      case CREATEVARIABLEX:
	ISWRITE(GETREGARG(PC+1));
	break;
      case CREATEVARIABLEMOVEX:
	ISWRITE(GETREGARG(PC+1));
	// fall through
      case CREATEVARIABLEMOVEY:
	ISWRITE(GETREGARG(PC+2));
	break;
      case UNIFYXX:
	ISREAD(GETREGARG(PC+1));
	ISREAD(GETREGARG(PC+2));
	break;
      case UNIFYXY:
      case UNIFYXG:
	ISREAD(GETREGARG(PC+1));
	break;
      case PUTRECORDX:
	ISWRITE(GETREGARG(PC+3));
	break;
      case PUTCONSTANTX:
	ISWRITE(GETREGARG(PC+2));
	break;
      case PUTLISTX:
	ISWRITE(GETREGARG(PC+1));
	break;
      case SETVARIABLEX:
	ISWRITE(GETREGARG(PC+1));
	break;
      case FUNRETURNX:
      case SETVALUEX:
	ISREAD(GETREGARG(PC+1));
	break;
      case GETRECORDX:
	ISREAD(GETREGARG(PC+3));
	break;
      case TESTBOOLX:
	PUSH(getLabelArg(PC+2));
	// fall through
      case TESTLITERALX:
      case TESTNUMBERX:
	ISREAD(GETREGARG(PC+1));
	// fall through
      case TESTLITERALY:
      case TESTLITERALG:
      case TESTBOOLY:
      case TESTBOOLG:
      case TESTNUMBERY:
      case TESTNUMBERG:
	PUSH(getLabelArg(PC+3));
	break;
      case TESTRECORDX:
	ISREAD(GETREGARG(PC+1));
	// fall through
      case TESTRECORDY:
      case TESTRECORDG:
	PUSH(getLabelArg(PC+4));
	break;
      case TESTLISTX:
	ISREAD(GETREGARG(PC+1));
	// fall through
      case TESTLISTY:
      case TESTLISTG:
	PUSH(getLabelArg(PC+2));
	break;
      case GETLITERALX:
      case GETNUMBERX:
	ISREAD(GETREGARG(PC+2));
	break;
      case GETLISTVALVARX:
	ISREAD(GETREGARG(PC+2));
	ISREAD(GETREGARG(PC+1));
	ISWRITE(GETREGARG(PC+3));
	break;
      case GETLISTX:
	ISREAD(GETREGARG(PC+1));
	break;
      case GETVARIABLEX:
	ISWRITE(GETREGARG(PC+1));
	break;

      case GETVARVARXX:
	ISWRITE(GETREGARG(PC+1));
	ISWRITE(GETREGARG(PC+2));
	break;
      case GETVARVARXY:
	ISWRITE(GETREGARG(PC+1));
	break;
      case GETVARVARYX:
	ISWRITE(GETREGARG(PC+2));
	break;
      case UNIFYVARIABLEX:
	ISWRITE(GETREGARG(PC+1));
	break;
      case UNIFYVALUEX:
	ISREAD(GETREGARG(PC+1));
	break;
      case UNIFYVALVARXX:
	ISREAD(GETREGARG(PC+1));
	ISWRITE(GETREGARG(PC+2));
	break;
      case UNIFYVALVARXY:
	ISREAD(GETREGARG(PC+1));
	break;
      case UNIFYVALVARYX:
      case UNIFYVALVARGX:
	ISWRITE(GETREGARG(PC+2));
	break;

      case MATCHX:
	ISREAD(GETREGARG(PC+1));
	// fall through
      case MATCHY:
      case MATCHG:
	{
	
	  IHashTable *table = (IHashTable *) getAdressArg(PC+2);
	  PUSH(table->lookupElse());
	  if (table->lookupLTuple()!=table->lookupElse()) 
	    PUSH(table->lookupLTuple());
	  for (int i = table->getSize(); i--; )
	    if (table->entries[i].val && 
		(table->entries[i].lbl != table->lookupElse()))
	      PUSH(table->entries[i].lbl);
	  BREAK;
	}

      default:
	// no usage of X registers
	break;
      }
      PC += sizeOf(op);
      goto innerLoop;
    }
  }
}

#ifdef DEBUG_LIVENESS
void checkLiveness(ProgramCounter PC, TaggedRef *X, int maxX)
{
  int m=CodeArea::livenessX(PC,X);
  if (m>maxX) {
    printf("liveness(%p)=%d > %d\n",PC,m,maxX);
  }
}
#endif
