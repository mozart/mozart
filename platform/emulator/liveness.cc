/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kevin Glynn (glynn@info.ucl.ac.be)
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

/* We allocate a static usage block for G usage vectors.  This
** saves a lot of malloc/free and a lot of time for some benchmarks
*/
#define StaticYUsageVectorSize 100

static int yUsageVector[StaticYUsageVectorSize];

#ifdef DEBUG_LIVECALC
// Whenever liveness zeroes the contents of a register we actually
// place a fresh +ve/-ve int (maintained by yMarker/xMarker)
// there. This gives some debugging info when tracking down buggy
// liveness code ...
static int xMarker = -1;
static int yMarker = 1;

   #define VOID_XREG(X,i)           X[i] = makeTaggedSmallInt(xMarker--)
   #define VOID_YREG(Y,i)           Y->setArg(i,makeTaggedSmallInt(yMarker++))
#else
   #define VOID_XREG(X,i)           X[i] = makeTaggedNULL()
   #define VOID_YREG(Y,i)           Y->setArg(i,makeTaggedNULL())
#endif


#define GETREGARG(pc) XRegToInt(getXRegArg(pc))

#define ISREAD(i) { int _i=(i); if (_i<xMax && xUsage[_i]==0) xUsage[_i]=1; }

#define ISREAD_TO(args) { for (int _j=args; _j--;) { ISREAD(_j); } }

#define ISWRITE(i)					\
{							\
  int _i = (i);						\
  if (_i<xMax && xUsage[_i]==0) {			\
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

#define GETREGARGG(pc) GRegToInt(getGRegArg(pc))

#define ISREADG(i) { int _i=(i); if (_i<gMax && gUsage[_i]==0) gUsage[_i]=1; }

    // for g registers store Writer with -ve values: G_0..G_n == -1..-(n+1)
#define ISWRITEG(i)					\
{							\
  int _i = (i);						\
  if (_i<gMax && gUsage[_i]==0) {			\
    current->writer=new Writer(-(_i+1),current->writer);	\
    gUsage[_i]=-1; 					\
  }							\
}

#define GETREGARGY(pc) YRegToInt(getYRegArg(pc))

#define ISREADY(i) { int _i=(i); if (_i<yMax && yUsage[_i]==0) yUsage[_i]=1; }

#define ISWRITEY(i)					\
{							\
  int _i = (i);						\
  if (_i<yMax && yUsage[_i]==0) {			\
    current->writer=new Writer(_i,current->writer);	\
    yUsage[_i]=-1; 					\
  }							\
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

Bool is_element(int x, Writer *l) {

  for (Writer *tmp = l; tmp; tmp = tmp->next) {
    if (tmp->i == x)
       return TRUE;
  }
  return FALSE;
} 

Bool is_subset(Writer *l, Writer *r) {

  for (Writer *tmp = l; tmp; tmp = tmp->next) {
    if (!is_element(tmp->i, r))
      return FALSE;
  }
  return TRUE;
} 

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
typedef uint32 bitVec;

const int xMaxSize = sizeof(bitVec)*8;

class LivenessCache : public AddressHashTable{
public:
  LivenessCache(): AddressHashTable(100) {}

  // We can pass in either X regs, Y regs or a usage array and findPC does the
  // appropriate thing.
  int findPC(ProgramCounter from, int max, TaggedRef *X, RefsArray *Y, int *usage)
  {
    void *aux = htFind(from);
    if (aux==htEmpty)
      return -1;
    
    int ret = 0;
    bitVec bits = (bitVec) ToInt32(aux);
    for (int i=0;i<max; i++) {
      if ((1<<i)&bits) {
	ret = i+1;
	if (usage) 
	  usage[i] = 1;
      } else {
        // Reg i is not live
        if (X) {
#ifdef DEBUG_LIVECALC
          printf("X Register (cached) [%d] => %d\n", i, xMarker);
#endif
          VOID_XREG(X,i);
        }
        else {
          if (Y) {
#ifdef DEBUG_LIVECALC
            printf("Y Register (cached) [%d] => %d\n", i, yMarker);
#endif
            VOID_YREG(Y,i);
          }
        }
      }
    }
    // Length of live register vector (i.e. last live register + 1)
    return ret;	
  }

  void addPC(ProgramCounter from, int max, int *usage)
  {
    // If there are more than 32 registers we silently don't cache results
    if (max>xMaxSize)
      return;
    
    bitVec aux = 0;
    for (int i=0;i<max; i++) {
      if (usage[i]==1)
	aux |= (1<<i);
    }
    htAdd(from, (void *) ToPointer(aux));
  }
};

// Caches for X,G,Y registers
static LivenessCache livenesscache;
static LivenessCache livenessGcache;
static LivenessCache livenessYcache;

// Call to re-initialise all caches to empty (e.g. if code page is reused)
void resetLivenessCaches()
{
  livenesscache.mkEmpty();
  livenessGcache.mkEmpty();
  livenessYcache.mkEmpty();
}

int CodeArea::livenessX(ProgramCounter from, TaggedRef *X,int xMax)
{

  if (xMax<=0) xMax = NumberOfXRegisters;

  int regLen = livenesscache.findPC(from,xMax,X, (RefsArray *) NULL, (int *) NULL);
  if (regLen != -1) 
    return regLen;

  // Gotta do it ourselves
  int *xUsage = new int[xMax];
  for (int i=xMax; i--;) xUsage[i]=0;
  
  regLen = livenessXInternal(from,xMax,xUsage);
    
  // Save for next time
  livenesscache.addPC(from,regLen,xUsage);

  // "Zero" dead X registers
  if (X) {
    for (int i=0; i<xMax; i++) {
      if (xUsage[i]!=1) {
#ifdef DEBUG_LIVECALC
	printf("X Register [%d] => %d\n", i, xMarker);
#endif
	VOID_XREG(X,i);
      }
    }
  }
  delete [] xUsage;
  return regLen;
}

int CodeArea::livenessXInternal(ProgramCounter from, int xMax, int *xUsage)
{

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
      Assert(w->i<xMax);
      Assert(xUsage[w->i]==-1);
      xUsage[w->i]=0;
      w = t;
    }
  }

outerLoop2:
  {
    // terminate if stack is empty
    if (!todo) {
      // Tidy up PC cache
      while (seen) {
	Segment *n = seen->next;
	delete seen;
	seen = n;
      }
      // Calculate length of live register vector
      int nextI=0;
      for (int i=xMax; i; i--) {
	if (xUsage[i-1]==1) {
	  nextI=i;
          break;
	}
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
	if (PC == next->pc && is_subset(next->writer, current->writer)) {
	  // If we have already 'seen' the current segment then
	  // we can skip it (since we have already collected its
	  // liveness info).  But note: we can only skip the 'current'
	  // segment when every writer in the 'seen' writer set is a member
	  // of the 'current' writer set.
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
	ISREAD(GETREGARG(PC+1));
	// fall through
      case TESTBOOLY:
      case TESTBOOLG:
	PUSH(getLabelArg(PC+2));
	PUSH(getLabelArg(PC+3));
	break;
      case TESTLITERALX:
      case TESTNUMBERX:
	ISREAD(GETREGARG(PC+1));
	// fall through
      case TESTLITERALY:
      case TESTLITERALG:
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
void checkLiveness(ProgramCounter PC, TaggedRef *X, int xMax)
{
  int m=CodeArea::livenessX(PC,X);
  if (m>xMax) {
    printf("liveness(%p)=%d > %d\n",PC,m,xMax);
  }
}
#endif

// Calculates usage vectors for G and Y registers from this PC onwards.
// aFrame points to the frame beneath the one we are analysing
void CodeArea::livenessGY(ProgramCounter from, Frame* aFrame,
                          int yMax, RefsArray* Y, 
                          int gMax, int* gUsage)
{

  Frame* curr_tos = aFrame;
  ProgramCounter pc = from;
  int *yUsage = yUsageVector;
  int yLiveLen = -1;
  int gLiveLen = -1;
  Bool yZeroed = 0;              // Y registers zeroed in situ
  Bool tidyYusage = 0;           // yUsage allocated
  Bool continuations = 0;        // CATCH and LOCK frames in same context

#ifndef GY_CACHE_OFF
  // Set 'continuations' if there are continuation frames (CATCH or LOCK) beneath us
  while (1) {

    GetFrame(curr_tos, xPC, xY, xCAP);
    if (xPC == C_EMPTY_STACK) {
      break;
    } else if (xPC == C_CATCH_Ptr || xPC == C_LOCK_Ptr) {
      continuations = 1;
      break;
    } else if (xPC == C_SET_SELF_Ptr || xPC == C_SET_ABSTR_Ptr ||
               xPC == C_DEBUG_CONT_Ptr || xPC == C_CALL_CONT_Ptr) {
      continue;
    } else { // Found parent frame
      break;
    }
  }

  // Check cache for G info
  gLiveLen = livenessGcache.findPC(pc,gMax,(TaggedRef *) NULL, 
                                   (RefsArray *) NULL, gUsage);
  // Check cache for Y info
  if (continuations) {
    // Since there are other entry points on the stack we need a yUsage 
    // structure to union liveness info.
    if (yMax > StaticYUsageVectorSize) {
      // Static block not big enough 
      yUsage = new int[yMax];
      tidyYusage = 1;
    }
    // Initialise Y registers to not used
    for (int i=yMax; i--;) yUsage[i]=0;
    yLiveLen = livenessYcache.findPC(pc,yMax,(TaggedRef *) NULL, 
                                     (RefsArray *) NULL, yUsage);
  }
  else {
    // No continuations on stack so directly void y registers from cache
    yLiveLen = livenessYcache.findPC(pc,yMax,(TaggedRef *) NULL, 
                                     Y, (int *) NULL);
    if (yLiveLen != -1) {
      // Y registers found and voided
      yZeroed = 1;
    }
  }
#endif

  if (gLiveLen == -1 || yLiveLen == -1) {
    // Gotta do it ourselves
    // Must initialise Y registers for liveness calcn
    if (!tidyYusage) {
      if (yMax > StaticYUsageVectorSize) {
        // Static block not big enough 
        yUsage = new int[yMax];
        tidyYusage = 1;
      }
      for (int i=yMax; i--;) yUsage[i]=0;
    }

    livenessGYInternal(pc,yMax,yUsage,gMax, gUsage, &yLiveLen, &gLiveLen);

#ifndef GY_CACHE_OFF
    // Save for next time
     livenessGcache.addPC(pc,gLiveLen,gUsage);
     livenessYcache.addPC(pc,yLiveLen,yUsage);
#endif
  }

  // There may be catch frames beneath this one.  If these frames
  // catch an exception their handler code will run in the same context
  // as this frame.  We must 'or' in liveness information from these
  // continuations too.
#ifndef GY_CACHE_OFF
  curr_tos = aFrame;
  while (continuations) {
#else
  while (1) {
#endif
    //printf("Look for continuations\n");
    GetFrame(curr_tos, xPC, xY, xCAP);
    if (xPC == C_EMPTY_STACK) {
      break;
    } else if (xPC == C_CATCH_Ptr || xPC == C_LOCK_Ptr) {
      // The actual continuation is in the frame beneath
      int gTmpLen = -1;
      int yTmpLen = -1;

      GetFrame(curr_tos, xPC, xY, xCAP);

#ifndef GY_CACHE_OFF
      gTmpLen = livenessGcache.findPC(xPC,gMax,(TaggedRef *) NULL, 
                                      (RefsArray *) NULL, gUsage);
      yTmpLen = livenessYcache.findPC(xPC,yMax,(TaggedRef *) NULL, 
                                      (RefsArray *) NULL, yUsage);
#endif
      if (gTmpLen == -1 || yTmpLen == -1) {

        livenessGYInternal(xPC,yMax,yUsage,gMax, gUsage, &yTmpLen, &gTmpLen);
#ifndef GY_CACHE_OFF
        // Save for next time
        livenessGcache.addPC(xPC,gTmpLen,gUsage);
        livenessYcache.addPC(xPC,yTmpLen,yUsage);
#endif
        gLiveLen = max(gLiveLen, gTmpLen); 
        yLiveLen = max(yLiveLen, yTmpLen); 
      }
      continue;
    } else if (xPC == C_SET_SELF_Ptr || xPC == C_SET_ABSTR_Ptr ||
	       xPC == C_DEBUG_CONT_Ptr || xPC == C_CALL_CONT_Ptr) {
      continue;
    } else { // Found parent frame - finished
      break;
    }
  }

  // Zero Y registers
  if (!yZeroed && Y) {
    for (int i=0; i<yMax; i++) {
      if (yUsage[i]!=1) {
#ifdef DEBUG_LIVECALC
        printf("Y Register [%d] => %d\n", i, yMarker);
#endif
        VOID_YREG(Y,i);
      }
    }
  }

   // Either yZeroed or yUsage initialised at this point

  if (tidyYusage)
    delete [] yUsage;
  return;
}

void CodeArea::livenessGYInternal(ProgramCounter from, int yMax, int *yUsage, int gMax, int *gUsage, 
                                                      int *yLiveLen, int *gLiveLen)
{

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
      if (w->i < 0) {
	// G Register
	Assert((-w->i)-1 < gMax);
	Assert(gUsage[(-w->i)-1]==-1);
	gUsage[(-w->i)-1]=0;
      }
      else {
	// Y Register
	Assert(w->i<yMax);
	Assert(yUsage[w->i]==-1);
	yUsage[w->i]=0;
      }
      w = t;
    }
  }

outerLoop2:
  {
    // terminate if stack is empty
    if (!todo) {
      // Tidy up PC cache
      while (seen) {
	Segment *n = seen->next;
	delete seen;
	seen = n;
      }
      // Calculate length of live register vector
      // (Could do this in ISREADY)
      // keving:  see comment on livenessXinternal
      *yLiveLen=0;
      for (int i=yMax; i; i--) {
	if (yUsage[i-1]==1) {
	  *yLiveLen=i;
          break;
	}
      }
      *gLiveLen=0;
      for (int i=gMax; i; i--) {
	if (gUsage[i-1]==1) {
	  *gLiveLen=i;
          break;
	}
      }
      return;
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

	if (PC == next->pc && is_subset(next->writer, current->writer)) {
	  // If we have already 'seen' the current segment then
	  // we can skip it (since we have already collected its
	  // liveness info).  But note: we can only skip the 'current'
	  // segment when every writer in the 'seen' writer set is a member
	  // of the 'current' writer set.
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
      if (w->i < 0)
	// G registers
	gUsage[(-w->i)-1]=-1;
      else
	yUsage[w->i]=-1;
      w=n;
    }

  innerLoop:
    {
      Opcode op = getOpcode(PC);
      //trace printf("        Checking 0x%x\n", PC);

      switch (op) {

      case ENDOFFILE:
	BREAK;

      case DEFINITION: 
      case DEFINITIONCOPY: 	  
	{
	  AssRegArray *list           = (AssRegArray*) getAdressArg(PC+5);
	  int size = list->getSize();
	  for (int i = 0; i < size; i++) {
	    if (((*list)[i].getKind()) == K_YReg)
	      // Copying from Y reg to G reg
	      ISREADY((*list)[i].getIndex());
	    if (((*list)[i].getKind()) == K_GReg)
	      // Copying from G reg to G reg
	      ISREADG((*list)[i].getIndex());
	  }
	  CONTINUE(getLabelArg(PC+2));
	}


      case ENDDEFINITION:
      case RETURN:
      case TAILCALLX:
      case FASTTAILCALL:
      case TAILSENDMSGX:
	BREAK;

      case MOVEXY:
      case PUTCONSTANTY:
      case UNIFYVALVARXY:
      case GETVARVARXY:
	ISWRITEY(GETREGARGY(PC+2));
	break;
	
      case MOVEGX:
	ISREADG(GETREGARGG(PC+1));
	break;

      case MOVEGY:
      case UNIFYVALVARGY:
	ISREADG(GETREGARGG(PC+1));
	ISWRITEY(GETREGARGY(PC+2));
	break;

      case MOVEYX:
      case SETVALUEY:
      case GETLISTY:
      case UNIFYVALUEY:
      case UNIFYVALVARYX:
      case CALLY:
      case DECONSCALLY:
      case CONSCALLY:
	ISREADY(GETREGARGY(PC+1));
	break;
	
      case SETVALUEG:
      case GETLISTG:
      case UNIFYVALUEG:
      case UNIFYVALVARGX:
      case CALLG:
      case TAILCALLG:
      case SETSELFG:
      case DECONSCALLG:
      case CONSCALLG:
	ISREADG(GETREGARGG(PC+1));
	break;

      case TAILCONSCALLG:
      case TAILDECONSCALLG:
	ISREADG(GETREGARGG(PC+1));
	BREAK;

      case CALLGLOBAL:
	ISREADG(GETREGARGG(PC+1));
	if (getPosIntArg(PC+2)&1)
	  // TailCall
	  BREAK;
	break;

      case CALLCONSTANT:
      case CALLPROCEDUREREF:
	if (getPosIntArg(PC+2)&1)
	  // TailCall
	  BREAK;
	break;


      case MOVEYY:
      case UNIFYVALVARYY:
	ISREADY(GETREGARGY(PC+1));
	ISWRITEY(GETREGARGY(PC+2));
	break; 

      case MOVEMOVEXYXY:
	ISWRITEY(GETREGARGY(PC+2));
	ISWRITEY(GETREGARGY(PC+4));
	break;

      case MOVEMOVEYXYX:
	ISREADY(GETREGARGY(PC+1));
	ISREADY(GETREGARGY(PC+3));
	break;

      case MOVEMOVEXYYX:
	ISWRITEY(GETREGARGY(PC+2));
	ISREADY(GETREGARGY(PC+3));
	break;

      case MOVEMOVEYXXY:
	ISREADY(GETREGARGY(PC+1));
	ISWRITEY(GETREGARGY(PC+4));
	break;

      case CREATEVARIABLEY:
      case CREATEVARIABLEMOVEY:
      case PUTLISTY:
      case SETVARIABLEY:
      case UNIFYVARIABLEY:
      case GETVARIABLEY:
      case GETVARVARYX:
      case CLEARY:
	ISWRITEY(GETREGARGY(PC+1));
	break;

      case UNIFYXY:
      case GETLITERALY:
      case GETNUMBERY:
      case SENDMSGY:
	ISREADY(GETREGARGY(PC+2));
	break;

      case TAILSENDMSGY:
	ISREADY(GETREGARGY(PC+2));
	BREAK;

      case UNIFYXG:
      case GETLITERALG:
      case GETNUMBERG:
      case SENDMSGG:
	ISREADG(GETREGARGG(PC+2));
	break;

      case TAILSENDMSGG:
	ISREADG(GETREGARGG(PC+2));
	BREAK;


      case PUTRECORDY:
	// label, arity, dest
	ISWRITEY(GETREGARGY(PC+3));
	break;

      case GETRECORDY:
	ISREADY(GETREGARGY(PC+3));
	break;

      case GETRECORDG:
	ISREADG(GETREGARGG(PC+3));
	break;

      case BRANCH:
	CONTINUE(getLabelArg(PC+1));

      case EXHANDLER:
      case LOCKTHREAD:
	PUSH(getLabelArg(PC+1));
        break;

      case GETRETURNX:
      case FUNRETURNX:
	// Dunno.  These instr. not currently implemented 
	// (and not documented?).  livenessX ignores them,
	// but isn't there a RETURN (and WRITE) here?
	break;

      case GETRETURNY:
	// As previous,  I've extrapolated from liveness X
	ISWRITEY(GETREGARGY(PC+1));
	break;

      case FUNRETURNY:
	// As previous,  I've extrapolated from liveness X
	ISREADY(GETREGARGY(PC+1));
	break;

      case GETRETURNG:
	// As previous,  I've extrapolated from liveness X
	ISWRITEG(GETREGARGG(PC+1));
	break;

      case FUNRETURNG:
	// As previous,  I've extrapolated from liveness X
	ISREADG(GETREGARGG(PC+1));
	break;

      case GETVARVARYY:
	ISWRITEY(GETREGARGY(PC+1));
	ISWRITEY(GETREGARGY(PC+2));
	break;

      case TESTLITERALY:
      case TESTNUMBERY:
	ISREADY(GETREGARGY(PC+1));
	// fall through
      case TESTLITERALX:
      case TESTNUMBERX:
      case TESTBI:
	PUSH(getLabelArg(PC+3));
	break;

      case TESTLITERALG:
      case TESTNUMBERG:
	ISREADG(GETREGARGG(PC+1));
	PUSH(getLabelArg(PC+3));
	break;

      case TESTRECORDY:
	ISREADY(GETREGARGY(PC+1));
	// fall through
      case TESTRECORDX:
      case TESTLT:
      case TESTLE:
	PUSH(getLabelArg(PC+4));
	break;

      case TESTRECORDG:
	ISREADG(GETREGARGG(PC+1));
	PUSH(getLabelArg(PC+4));
	break;

      case TESTLISTY:
	ISREADY(GETREGARGY(PC+1));
	// fall through
      case TESTLISTX:
	PUSH(getLabelArg(PC+2));
	break;

      case TESTLISTG:
	ISREADG(GETREGARGG(PC+1));
	PUSH(getLabelArg(PC+2));
	break;

      case TESTBOOLY:
	ISREADY(GETREGARGY(PC+1));
	// fall through
      case TESTBOOLX:
	PUSH(getLabelArg(PC+2));
	PUSH(getLabelArg(PC+3));
	break;

      case TESTBOOLG:
	ISREADG(GETREGARGG(PC+1));
	PUSH(getLabelArg(PC+2));
	PUSH(getLabelArg(PC+3));
	break;

      case MATCHY:
	ISREADY(GETREGARGY(PC+1));
	// fall through
      case MATCHX:
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

      case MATCHG:
	ISREADG(GETREGARGG(PC+1));
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

      case CALLMETHOD:
	{
	  CallMethodInfo *cmi = (CallMethodInfo*)getAdressArg(PC+1);
	  ISREADG(cmi->regIndex);
	  break;
	}

      default:
	// no usage of Y registers, no alt exec paths
	break;
      }
      PC += sizeOf(op);
      goto innerLoop;
    }
  }
}
