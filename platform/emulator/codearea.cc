/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------

  ------------------------------------------------------------------------
*/

#include "types.hh"

#include "builtins.hh"
#include "codearea.hh"
#include "misc.hh"
#include "records.hh"
#include "opcodes.hh"
#include "optostr.hh"

//
// Globals
//


#ifdef RECINSTRFETCH

int CodeArea::fetchedInstr = 0;
ProgramCounter CodeArea::ops[RECINSTRFETCH];



void CodeArea::recordInstr(ProgramCounter PC){
  ops[fetchedInstr] = PC;
  fetchedInstr++;
  if(fetchedInstr >= RECINSTRFETCH)
    fetchedInstr = 0;
}

#endif

HashTable CodeArea::atomTab(CHARTYPE,10000);
HashTable CodeArea::nameTab(CHARTYPE,1000);
int CodeArea::totalSize = 0; /* in bytes */
char **CodeArea::opToString = initOpToString();
CodeAreaList *CodeArea::allBlocks = NULL;

#ifdef THREADED
void **CodeArea::globalInstrTable = NULL;
#endif



Opcode CodeArea::stringToOp(char *s)
{
  for (int i=0; i < (Opcode) ERROR; i++) {
    if (strcmp(s,opToString[i]) == 0 ) {
      return (Opcode) i;
    }
  }

  return ERROR;
}

void CodeArea::showAtomNames()
{
  printf("\nAtom Tab:\n");
  atomTab.print();
}

inline Literal *addToLiteralTab(char *str, HashTable *table, Bool isName)
{
  Literal *found = (Literal *) table->ffind(str);

  if (found != (Literal *) htEmpty) {
    return found;
  }
  
  found = new Literal(str,isName);
  if (table->aadd(found,str)) {
    return found;
  }
  error("addToLiteralTab: failed");
  return NULL;
}


Literal *addToAtomTab(char *str)
{
  return addToLiteralTab(str,&CodeArea::atomTab,NO);
}

Literal *addToNameTab(char *str)
{
  return addToLiteralTab(str,&CodeArea::nameTab,OK);
}


AbstractionTable CodeArea::abstractionTab(4000);


AbstractionEntry *addAbstractionTab(int id)
{
  if (id == 0)
    return NULL;

  AbstractionEntry *found = (AbstractionEntry *) CodeArea::abstractionTab.ffind(id);

  if (found != (AbstractionEntry *) htEmpty) {
    return found;
  }
  
  found = new AbstractionEntry();
  if (CodeArea::abstractionTab.aadd(found,id)) {
    return found;
  }
  
  error("addAbstractionTab: failed");
  return NULL;
}


/*
  we store the absolute adress of the indices in the 
  instruction tables
  */

#ifdef THREADED
AdressOpcode CodeArea::opcodeToAdress(Opcode oc)
{
  return ToInt32(globalInstrTable[oc]);
}


Opcode CodeArea::adressToOpcode(AdressOpcode adr) 
{
  for(int i = 0; i < (int) ERROR; i++)
    if (ToInt32(globalInstrTable[i]) == adr)
      return (Opcode)i;
  return ERROR;
}

#else /* THREADED */
AdressOpcode CodeArea::opcodeToAdress(Opcode oc)
{
  return  oc;
}


Opcode CodeArea::adressToOpcode(AdressOpcode adr) 
{
  return adr;
}

#endif /* THREADED */


void AbstractionEntry::setPred(Abstraction *ab) 
{ 
  abstr = ab; 
  pc    = abstr->getPC();
  g     = abstr->getGRegs();
  
  // indexing on X[0] optimized !!!
  if (CodeArea::adressToOpcode((AdressOpcode) *pc) == SWITCHONTERMX &&
      getRegArg(pc+1) == 0) {
    indexTable = (IHashTable *) getAdressArg(pc+2);
  } else {
    indexTable = NULL;
  }
}


#define DISPATCH() PC += sizeOf(op); break


#ifdef FASTREGACCESS
inline Reg regToInt(Reg N) { return (N / sizeof(TaggedRef)); }
#else
inline Reg regToInt(Reg N) { return N; }
#endif


char *getBIName(ProgramCounter PC)
{
  BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC);
  return entry->getPrintName();

}


/* find the start of the definition where from points into */
ProgramCounter CodeArea::definitionStart(ProgramCounter from)
{
  ProgramCounter PC = from;

  int counter = 1;
  while (1) {
    Opcode op = adressToOpcode(getOP(PC));
    switch (op) {
    case CREATENAMEDVARIABLEX: 
    case CREATENAMEDVARIABLEY: 
    case CREATENAMEDVARIABLEG: 
      return NOCODE;

    case DEFINITION:
      counter++;
      DISPATCH();
      
    case ENDDEFINITION:
      counter--;
      if (counter == 0) {
	return getLabelArg(PC+1);
      }

      DISPATCH();

    default:
      DISPATCH();
    }
  }
  return NOCODE;
}

void displayCode(ProgramCounter from, int ssize) {
 CodeArea::display(from,ssize,stderr);
 fflush(stderr);
}

void CodeArea::getDefinitionArgs(ProgramCounter PC,
				 Reg &reg, ProgramCounter &next, TaggedRef &file,
				 TaggedRef &line, PrTabEntry *& pred)
{
  Assert(adressToOpcode(getOP(PC)) == DEFINITION);
  reg  = regToInt(getRegArg(PC+1));
  next = getLabelArg(PC+2);
  file = getLiteralArg(PC+3);
  line = getNumberArg(PC+4);
  pred = getPredArg(PC+5);
}


void CodeArea::display (ProgramCounter from, int sz, FILE* ofile)
{
  ProgramCounter PC;
  PC = from;

  Bool isEnd = NO;
  Opcode op;
  int i;

  for (i = 1; isEnd == NO; i++) {
    if (sz > 0 && i >= sz)
      isEnd = OK;
    fprintf(ofile, "0x%08x:  ", PC);
    op = adressToOpcode(getOP(PC));
    if (op <= ERROR) {
      fprintf(ofile, "%03d %s", op,opToString[(int)op]);
    }
    switch (op) {
    case ERROR:
    case FAILURE:
    case SUCCEED:
    case WAIT:
    case WAITTOP:
    case ASK: 
    case RETURN:
    case ELSECLAUSE:
    case WAITCLAUSE: 
    case ASKCLAUSE:
    case LASTCLAUSE:
    case DEALLOCATEL:
    case ALLOCATEL1:
    case ALLOCATEL2:
    case ALLOCATEL3:
    case ALLOCATEL4:
    case ALLOCATEL5:
    case ALLOCATEL6:
    case ALLOCATEL7:
    case ALLOCATEL8:
    case ALLOCATEL9:
    case ALLOCATEL10:
    case SHALLOWTHEN:
          /* Commands with no args.   */
      fprintf(ofile, "\n");       
      DISPATCH();
    case DEBUGINFO:
      {
	TaggedRef name       = getLiteralArg(PC+1);
	TaggedRef lineTerm   = getNumberArg(PC+2);
	TaggedRef absposTerm = getNumberArg(PC+3);
	int line,abspos;
	
	fprintf(ofile, "(%s,..,..)\n",OZ_toC(name));
	DISPATCH();
      }
    case PUTLISTX: 
    case PUTLISTY: 
    case PUTLISTG:
    case SETVALUEX: 
    case SETVALUEY:
    case SETVALUEG:
    case GETLISTX: 
    case GETLISTY: 
    case GETLISTG: 
    case UNIFYVALUEX: 
    case UNIFYVALUEY: 
    case UNIFYVALUEG:
    case GETVARIABLEX:
    case GETVARIABLEY:
    case GETVARIABLEG:
    case SETVARIABLEX:
    case SETVARIABLEY:
    case SETVARIABLEG:
    case UNIFYVARIABLEX: 
    case UNIFYVARIABLEY: 
    case UNIFYVARIABLEG: 
    case CREATEVARIABLEX: 
    case CREATEVARIABLEY: 
    case CREATEVARIABLEG: 
	  /* OP Reg       */
      fprintf(ofile, "(%d)\n", regToInt(getRegArg(PC+1)));
      DISPATCH();

    case CREATEVARIABLEMOVEX: 
    case CREATEVARIABLEMOVEY: 
    case CREATEVARIABLEMOVEG: 
	  /* OP Reg RegIndex       */
      fprintf (ofile,"(%d,X[%d])\n",
	       regToInt(getRegArg(PC+1)),
	       regToInt(getRegArg(PC+2)));
      DISPATCH();

    case GETLISTVALVARX: 
    case GETLISTVALVARY: 
    case GETLISTVALVARG: 
      /* OP RegIndex Reg RegIndex       */
      fprintf (ofile,"(X[%d],%d,X[%d])\n",
	       regToInt(getRegArg(PC+1)),
	       regToInt(getRegArg(PC+2)),
	       regToInt(getRegArg(PC+3)));
      DISPATCH();

    case MOVEMOVEXYXY:
    case MOVEMOVEYXXY:
    case MOVEMOVEYXYX:
    case MOVEMOVEXYYX:
	  /* ***type 1:    OP Reg       */
      fprintf (ofile,"(%d,%d,%d,%d)\n",
	      regToInt(getRegArg(PC+1)),regToInt(getRegArg(PC+2)),
	      regToInt(getRegArg(PC+3)),regToInt(getRegArg(PC+4)));
      DISPATCH();

    case SHALLOWTEST1:
      fprintf (ofile,
	       "(%s,X[%d],%0x%x,0x%x,%d)\n",
	       getBIName(PC+1),
	       regToInt(getRegArg(PC+2)),
	       getLabelArg(PC+3),
	       getPosIntArg(PC+4));
      DISPATCH();
      
    case TESTCONSTX:
    case TESTCONSTY:
    case TESTCONSTG:
    case TESTNUMBERX:
    case TESTNUMBERY:
    case TESTNUMBERG:
      {
	TaggedRef literal = getLiteralArg(PC+2);
	fprintf (ofile,
		 "(%d,%s,0x%x,0x%x,%d)\n",
		 regToInt(getRegArg(PC+1)),
		 OZ_toC(literal),
		 getLabelArg(PC+3),
		 getLabelArg(PC+4),
		 getPosIntArg(PC+5));
	DISPATCH();
      }

    case SHALLOWTEST2:
      fprintf (ofile,
	       "(%s,X[%d],X[%d],%0x%x,0x%x,%d)\n",
	       getBIName(PC+1),
	       regToInt(getRegArg(PC+2)),
	       regToInt(getRegArg(PC+3)),
	       getLabelArg(PC+4),
	       getPosIntArg(PC+5));
      DISPATCH();
      
    case INLINEREL1:
      fprintf (ofile,
	       "(%s,X[%d])\n",
	       getBIName(PC+1),
	       regToInt(getRegArg(PC+2)));
      DISPATCH();
      
    case INLINEFUN1:
    case INLINEREL2:
      fprintf (ofile,
	       "(%s,X[%d],X[%d])\n",
	       getBIName(PC+1),
	       regToInt(getRegArg(PC+2)),
	       regToInt(getRegArg(PC+3)));
      DISPATCH();
      
    case INLINEFUN2:
      fprintf (ofile,
	       "(%s,X[%d],X[%d],X[%d])\n",
	       getBIName(PC+1),
	       regToInt(getRegArg(PC+2)),
	       regToInt(getRegArg(PC+3)),
	       regToInt(getRegArg(PC+4)));
      DISPATCH();
      
    case INLINEEQEQ:
      fprintf (ofile,
	       "(%s,X[%d],X[%d],X[%d],%d)\n",
	       getBIName(PC+1),
	       regToInt(getRegArg(PC+2)),
	       regToInt(getRegArg(PC+3)),
	       regToInt(getRegArg(PC+4)),
	       getPosIntArg(PC+5));
      DISPATCH();
      
    case INLINEFUN3:
      fprintf (ofile,
	       "(%s,X[%d],X[%d],X[%d],X[%d])\n",
	       getBIName(PC+1),
	       regToInt(getRegArg(PC+2)),
	       regToInt(getRegArg(PC+3)),
	       regToInt(getRegArg(PC+4)),
	       regToInt(getRegArg(PC+5)));
      DISPATCH();
      
    case CALLBUILTIN:
      fprintf (ofile,
	       "(%s,%d)\n",
	       getBIName(PC+1),
	       getPosIntArg(PC+2));
      DISPATCH();
      
    case FASTCALL:
    case FASTTAILCALL:
      {
	/* type: OP PredicateRef */
	AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);
	Abstraction *abstr = entry->getAbstr();
	if (abstr) {  /* may be NULL during loading */
	  fprintf(ofile, "(%s,%d)\n", abstr->getPrintName(),abstr->getArity());
	} else {
	  fprintf(ofile, "(??,??)\n");
	}
	DISPATCH();
      }
    case APPLMETHX:
    case APPLMETHY:
    case APPLMETHG:
    case TAILAPPLMETHX:
    case TAILAPPLMETHY:
    case TAILAPPLMETHG:
    case SENDMSGX:
    case SENDMSGY:
    case SENDMSGG:
    case TAILSENDMSGX:
    case TAILSENDMSGY:
    case TAILSENDMSGG:
      {
	TaggedRef literal = getLiteralArg(PC+1);
	Reg reg        = regToInt(getRegArg(PC+2));
	int arity      = getPosIntArg(PC+3);
	fprintf(ofile, "(%s,%d,%d)\n", OZ_toC(literal),reg,arity);
	DISPATCH();
      }
    case CALLX:
    case CALLY:
    case CALLG:
    case TAILCALLX:
    case TAILCALLY:
    case TAILCALLG:
    case DETX: 
    case DETY: 
    case DETG: 
	  /* ***type 3a:    OP Reg Int      */
      {
	Reg reg = regToInt(getRegArg(PC+1));
	fprintf(ofile, "(%d,%d)\n", reg,getPosIntArg(PC+2));
      }
      DISPATCH();
    case SETVOID:
    case GETVOID:
    case UNIFYVOID:
    case ALLOCATEL: 
	  /* ***type 2:    OP PosInt    */
      fprintf(ofile, "(%d)\n",getPosIntArg(PC+1));
      DISPATCH();
    case SETINT:
    case UNIFYINT:
	  /* ***type 10:   OP Number       */
      {
	fprintf(ofile, "(");
	TaggedRef b = getNumberArg(PC+1);
	if (b == makeTaggedNULL())
	  fprintf(ofile, "(NULL)");
	else
	  fprintf(ofile, "%s", OZ_toC(b));
	fprintf(ofile, ")\n");
      }
      DISPATCH();
    case PUTINTX:
    case PUTINTY:
    case PUTINTG:
    case GETINTX:
    case GETINTY:
    case GETINTG:
	  /* ***type 3:    OP PosInt Reg   */
      {
	fprintf(ofile, "(");
	TaggedRef b = getNumberArg(PC+1);
	if (b == makeTaggedNULL())
	  fprintf(ofile, "(NULL)");
	else
	  fprintf(ofile, "%s", OZ_toC(b));
	fprintf(ofile, ",%d)\n", regToInt(getRegArg(PC+2)));
      }
      DISPATCH();
    case MOVEXX: 
    case MOVEXY: 
    case MOVEXG: 
    case MOVEYX: 
    case MOVEYY: 
    case MOVEYG: 
    case MOVEGX: 
    case MOVEGY: 
    case MOVEGG: 

    case UNIFYXX:
    case UNIFYXY:
    case UNIFYXG:
    case UNIFYYX:
    case UNIFYYY:
    case UNIFYYG:
    case UNIFYGX:
    case UNIFYGY:
    case UNIFYGG:

    case GETVARVARXX:
    case GETVARVARXY:
    case GETVARVARXG:
    case GETVARVARYX:
    case GETVARVARYY:
    case GETVARVARYG:
    case GETVARVARGX:
    case GETVARVARGY:
    case GETVARVARGG:

    case UNIFYVALVARXX:
    case UNIFYVALVARXY:
    case UNIFYVALVARXG:
    case UNIFYVALVARYX:
    case UNIFYVALVARYY:
    case UNIFYVALVARYG:
    case UNIFYVALVARGX:
    case UNIFYVALVARGY:
    case UNIFYVALVARGG:
	  /* ***type 4:    OP Reg Reg   */  
      {
	Reg reg = regToInt(getRegArg(PC+1));
	fprintf(ofile, "(%d,%d)\n", reg, regToInt(getRegArg(PC+2)));
      }
      DISPATCH();
    case SETCONSTANT: 
    case UNIFYCONSTANT:
	  /* ***type 5:    OP ConstantName  */
      {
	TaggedRef literal = getLiteralArg(PC+1);

	fprintf(ofile, "(%s)\n", OZ_toC(literal));
      }
      DISPATCH();
    case DEFINITION:
	  /* ***type 11:    OP predicate     */
      {
	Reg reg;
	ProgramCounter next;
	TaggedRef file, line;
	PrTabEntry *pred;
	getDefinitionArgs(PC,reg,next,file,line,pred);

	fprintf(ofile, "(X%d,0x%x,%s,%s,%s,[",reg,next,
		pred ? pred->getPrintName() : "(NULL)",
		OZ_toC(file), OZ_toC(line));

	AssRegArray &list = pred->gRegs;
	
	for (int k = 0; k < list.getSize(); k++) {
	  switch (list[k].kind) {
	  case XReg:
	    fprintf(ofile,"X%d ",list[k].number);
	    break;
	  case YReg:
	    fprintf(ofile,"Y%d ",list[k].number);
	    break;
	  case GReg:
	    fprintf(ofile,"G%d ",list[k].number);
	    break;
	  }
	}

	fprintf(ofile, "])\n");
      }
      DISPATCH();

    case PUTRECORDX: 
    case PUTRECORDY: 
    case PUTRECORDG: 
    case GETRECORDX: 
    case GETRECORDY: 
    case GETRECORDG: 
    case PUTSTRUCTUREX: 
    case PUTSTRUCTUREY: 
    case PUTSTRUCTUREG: 
    case GETSTRUCTUREX: 
    case GETSTRUCTUREY: 
    case GETSTRUCTUREG: 
	  /* ***type 6:    OP LiteralName Reg */
      {
	TaggedRef literal = getLiteralArg(PC+1);
	int n = getPosIntArg(PC+2);

	fprintf(ofile, "(%s,%i,%d)\n", OZ_toC(literal),
		 n, regToInt(getRegArg(PC+3)));
      }
      DISPATCH();
    case PUTCONSTANTX: 
    case PUTCONSTANTY: 
    case PUTCONSTANTG: 
    case GETCONSTANTX:
    case GETCONSTANTY:
    case GETCONSTANTG:
	  /* ***type 7:    OP ConstantName Reg */
      {
	TaggedRef literal = getLiteralArg(PC+1);

	fprintf(ofile, "(%s,%d)\n", OZ_toC(literal),
		 regToInt(getRegArg(PC+2)));
      }
      DISPATCH();

    case CREATENAMEDVARIABLEX: 
    case CREATENAMEDVARIABLEY: 
    case CREATENAMEDVARIABLEG: 
	  /* ***type 7:    OP ConstantName Reg */
      {
	TaggedRef literal = getLiteralArg(PC+2);

	fprintf(ofile, "(%s,%d)\n", OZ_toC(literal),
		 regToInt(getRegArg(PC+1)));
      }
      DISPATCH();
    case ENDDEFINITION:
    case BRANCH:
    case NEXTCLAUSE: 
    case THREAD:
    case SAVECONT:
	  /* ***type 8:    OP Label */
      fprintf(ofile, "(@ 0x%x)\n", getLabelArg (PC+1));
      DISPATCH();
    case BRANCHONVARX: 
    case BRANCHONVARY: 
    case BRANCHONVARG: 
    case BRANCHONNONVARX: 
    case BRANCHONNONVARY: 
    case BRANCHONNONVARG: 

	  /* ***type 9:    OP Reg Label */
      {
	Reg reg = regToInt(getRegArg(PC+1));
	
	fprintf(ofile, "(%d,@ 0x%x)\n", reg, getLabelArg (PC+2));
      }
      DISPATCH();

    case SWITCHONTERMX:
    case SWITCHONTERMY:
    case SWITCHONTERMG:
      {
	Reg reg = regToInt(getRegArg(PC+1));

	fprintf(ofile, "(%d", reg);
	fprintf(ofile, "...)\n");
      }
      
      DISPATCH();

    case SHALLOWGUARD:
    case CREATEOR:
    case CREATEENUMOR:
    case CREATECOND:
	  /* ***type 8:    OP Label Int*/
      {
	ProgramCounter lbl = getLabelArg(PC+1);
	int n = getPosIntArg(PC+2);
	fprintf(ofile, "(@ 0x%x, %d)\n", lbl, n);
      }
      DISPATCH();

    default:
      fflush(ofile);
      warning("Undefined command: %d (function void CodeArea::display)", op);
      isEnd = OK;
      DISPATCH();
    }
  }
}

#undef DISPATCH

