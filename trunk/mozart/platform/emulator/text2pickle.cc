#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>


#include "config.h"
#include "opcodes.hh"

static int line=1, col=0;

void OZ_error(const char *format, ...)
{
  va_list ap;
  va_start(ap,format);
  fprintf(stderr,"*** Error in line %d column %d\n",line,col);
  vfprintf(stderr,format,ap);
  fprintf(stderr,"\n");
  fflush(stderr);
  va_end(ap);
  exit(1);
}



/************************************************************/

const int TBSize = 1024;

class TextBlock {
public:
  unsigned char text[TBSize];
  TextBlock *next;
  TextBlock(): next(0) {}
};

class MsgBuffer {
  int mode;  
  TextBlock *first, *last;
  int pos;
public:
  int fd;

  MsgBuffer(int f, int m): fd(f), mode(m), pos(0) {
    first = last = new TextBlock();
  }
  int textmode()   { return mode!=0; }
  void put(unsigned char c) { 
    if (pos==TBSize){
      last->next = new TextBlock();
      last = last->next;
      pos = 0;      
    }
    last->text[pos++] = c;
  }

  void dump()
  {
    while(first->next) {
      write(fd,first->text,TBSize);
      first = first->next;
    }
    write(fd,first->text,pos);
  }
  unsigned long crc();
};


#define TEXT2PICKLE
#include "pickle.cc"


unsigned long MsgBuffer::crc()
{
  TextBlock *aux = first;
  unsigned long i = init_crc();
  while(aux->next) {
    i = update_crc(i,aux->text,TBSize);
    aux = aux->next;
  }
  i = update_crc(i,aux->text,pos);
  return i;
}

inline
int nextchar(FILE *in)
{
  int c = fgetc(in);
  if (c=='\n') {
    line++;
    col=0;
  }
  col++;
  return c;
}

inline
int skipBlanks(FILE *in)
{
  while(1) {
    int c = nextchar(in);
    if (c==EOF || !isspace(c))
      return c;
  }
}

inline
int getTag(FILE *in)
{
  int tag = skipBlanks(in);
  if (tag==EOF) return TAG_EOF;
  int colon = nextchar(in);
  Assert(colon == ':');
  return tag;
}

inline
unsigned scanInt(FILE *in)
{
  unsigned ret;
  int aux = fscanf(in,"%u",&ret);
  Assert(aux==1);
  return ret;
}


char *buf = 0;
int bufSize = -1;

inline
void setBuf(int i,char c)
{
  if (buf==0) {
    bufSize = 100;
    buf = (char *) malloc(bufSize*sizeof(char));
    setBuf(i,c);
  } else if (i >= bufSize) {
    bufSize *= 2;
    buf = (char *) realloc(buf, bufSize);
    setBuf(i,c);
  } else {
    Assert(buf);
    buf[i] = c;
  }
}

static inline
void scanQuotedString(FILE *in)
{
  int c = nextchar(in);
  int i = 0;
  while (c != '\'') {
    if (c == EOF) {
      OZ_error("end-of-file in string");
    } else if (c == '\\') {
      c = nextchar(in);
      switch (c) {
      case 'a':
	setBuf(i++,'\a');
	break;
      case 'b':
	setBuf(i++,'\b');
	break;
      case 'f':
	setBuf(i++,'\f');
	break;
      case 'n':
	setBuf(i++,'\n');
	break;
      case 'r':
	setBuf(i++,'\r');
	break;
      case 't':
	setBuf(i++,'\t');
	break;
      case 'v':
	setBuf(i++,'\v');
	break;
      case 'x':
	{
	  char hexstring[3];
	  hexstring[0] = nextchar(in);
	  c = nextchar(in);
	  hexstring[1] = c;
	  hexstring[2] = '\0';
	  if (c == EOF)
	    OZ_error("end-of-file in string");
	  char *end;
	  int hexnum = (int) strtol(hexstring, &end, 16);
	  if (hexnum == 0 || *end != '\0')
	    OZ_error("illegal number in hexadecimal notation");
	  setBuf(i++,hexnum);
	}
	break;
      case '\\':
      case '`':
      case '\"':
      case '\'':
      case '&':
	setBuf(i++,c);
	break;
      case EOF:
	OZ_error("end-of-file in string");
      case '0': case '1': case '2': case '3':
      case '4': case '5': case '6': case '7':
	{
	  char octstring[4];
	  octstring[0] = c;
	  octstring[1] = nextchar(in);
	  c = nextchar(in);
	  octstring[2] = c;
	  octstring[3] = '\0';
	  if (c == EOF)
	    OZ_error("end-of-file in string");
	  char *end;
	  int octnum = (int) strtol(octstring, &end, 8);
	  if (octnum == 0 || octnum > 255 || *end != '\0')
	    OZ_error("illegal number in octal notation");
	  setBuf(i++,octnum);
	}
	break;
      default:
	OZ_error("illegal character in string");
      }
    } else {
      setBuf(i++,c);
    }
    c = nextchar(in);
  }
  setBuf(i,'\0');
}

static inline
char *scanString(FILE *in)
{
  int c = nextchar(in);
  if (c == '\'') {
    scanQuotedString(in);
  } else if (oz_isalnum(c)) {
    int i = 0;
    while (oz_isalnum(c)) {
      setBuf(i++,c);
      c = nextchar(in);
    }
    if (c!=EOF && !isspace(c))
      OZ_error("illegal character in string");
    setBuf(i,'\0');
  } else {
    OZ_error("string expected");
  }
  return buf;
}

inline
char *scanComment(FILE *in)
{
  int i = 0;
  while(1) {
    int c = nextchar(in);
    if (c==EOF || c=='\n') {
      setBuf(i,0);
      return strdup(buf);
    }
    setBuf(i++,c);
  }
}

inline
MarshalTag char2Tag(char *s)
{
  for(int i=0; i<=DIF_LAST; i++) {
    if (strcmp(dif_names[i].name,s)==0)
      return dif_names[i].tag;
  }
  Assert(0);
  return (MarshalTag) 0; // make gcc happy
}


/************************************************************/

class Label {
public:
  char *label;          /* symbolic name */
  ProgramCounter addr;  /* absolute address (relativ to zero) */
  char used, defined;   /* for consistency checks */
  Label *next;

  Label(char *l, ProgramCounter  a, Label *n):
    label(strdup(l)), addr(a), used(0), defined(0), next(n) {}
};

unsigned hash(char *s)
{
  // 'hashfunc' is taken from 'Aho,Sethi,Ullman: Compilers ...', page 436
  const char *p = s;
  unsigned h = 0, g;
  for(; *p; p++) {
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }
  return h;
}


class LabelTable {
  static const int tableSize = 1024*3;  /* fixed size, cannot grow :-( */
  Label *table[tableSize];

public:
  LabelTable() 
  {
    for (int i=0; i<tableSize; i++) {
      table[i] = NULL;
    }
  }
 
  Label *addLabel(char *lbl, ProgramCounter addr)
  {
    int key = hash(lbl)%tableSize;
    table[key] = new Label(lbl,addr,table[key]);
    return table[key];
  }

  Label *findLabel(char *lbl)
  {
    int key = hash(lbl)%tableSize;
    Label *aux = table[key];
    while(aux) {
      if (strcmp(lbl,aux->label)==0)
	return aux;
      aux = aux->next;
    }
    return NULL;
  }

  Label *defLabel(char *lbl, ProgramCounter addr)
  {
    Label *aux = findLabel(lbl);
    if (aux==NULL) {
      aux = addLabel(lbl,addr);
    } else {
      Assert(aux->defined==0);
      aux->addr = addr;
    }
    aux->defined = 1;
    return aux;
  }

  Label *useLabel(char *lbl)
  {  
    Label *aux = findLabel(lbl);
    if (aux==0) 
      aux = addLabel(lbl,0);
    aux->used = 1;
    return aux;
  }
};



/************************************************************/

class TermTag {
public:
  char *label;
  int value;
  TermTag *next;

  static int tagCounter;

  TermTag(char *lbl, TermTag *nxt): label(strdup(lbl)), next(nxt) 
  {
    value = tagCounter++;
  }
};

int TermTag::tagCounter = 0;


class TermTagTable {
  static const int tableSize = 1024;  /* fixed size, cannot grow :-( */
  TermTag *table[tableSize];
public:
  TermTagTable() 
  {
    for (int i=0; i<tableSize; i++) {
      table[i] = NULL;
    }
  }
 
  TermTag *find(char *lbl)
  {
    int key = hash(lbl)%tableSize;
    TermTag *aux = table[key];
    while(aux) {
      if (strcmp(lbl,aux->label)==0)
	return aux;
      aux = aux->next;
    }
    return NULL;
  }

  TermTag *add(char *lbl)
  {
    Assert(!find(lbl));
    int key = hash(lbl)%tableSize;
    table[key] = new TermTag(lbl,table[key]);
    return table[key];
  }
};


/************************************************************/

typedef 
union {
  unsigned num;
  ProgramCounter pc;
  char *string;
  struct { 
    Label *label; 
    ProgramCounter lastPC; // address of instr containing the ref to label
  } labelRef;
  Label *labelDef;
  Opcode opcode;
  MarshalTag mtag;
  TermTag *ttag;
} Tagvalue;


class TaggedPair {
public:
  int tag;
  Tagvalue val;
  TaggedPair *next;

  TaggedPair(int t, Tagvalue *v): tag(t), val(*v), next(NULL) {}
};

/************************************************************/

void pickle(TaggedPair *aux, MsgBuffer *out)
{
  /* write new version number */
  Assert(aux->tag==TAG_STRING);
  marshalString(PERDIOVERSION,out);
  aux = aux->next;

  while(aux) {

    switch (aux->tag) {

    case TAG_LABELDEF:
      if (aux->val.labelDef->used) {
	marshalLabelDef(aux->val.labelDef->label,out);
      }
      break;

    case TAG_LABELREF:
      {
	Label *lbl = aux->val.labelRef.label;
	Assert(lbl->defined);
	if (out->textmode()) {
	  putTag(TAG_LABELREF,out);
	  putString(lbl->label,out);
	} else {
	  marshalLabel(0, lbl->addr - aux->val.labelRef.lastPC, out);
	}
	break;
      }

    case TAG_CODESTART: 
      {
	int codesize = (aux->val.pc-(ProgramCounter)0)*sizeof(ByteCode);
	marshalCodeStart(codesize,out); 
	break;
      }

    case TAG_INT:       marshalNumber(aux->val.num,out); break;
    case TAG_CODEEND:   marshalCodeEnd(out); break;
    case TAG_BYTE:      marshalByte(aux->val.num,out); break;
    case TAG_OPCODE:    marshalOpCode(0,aux->val.opcode,out,0); break;
    case TAG_STRING:    marshalString(aux->val.string,out); break;
    case TAG_COMMENT:   putComment(aux->val.string,out); break;
    case TAG_DIF:       marshalDIF(out,aux->val.mtag); break;
    case TAG_TERMDEF:   marshalTermDef(aux->val.ttag->value,out); break;
    case TAG_TERMREF:   marshalTermRef(aux->val.ttag->value,out); break;
    default:            Assert(0);
    }
    aux = aux->next;
  }

  if (!out->textmode()) {
    int headerSize;
    char *header = makeHeader(out->crc(),&headerSize);
    write(out->fd,header,headerSize);
  }
  out->dump();
}

/************************************************************/

class CodeInfo {
public:
  ProgramCounter PC;      /* save value of PC of surrounding block */
  TaggedPair **lastPair;  /*  */
  LabelTable labels;      /* labels of this block */
  CodeInfo *next;
  CodeInfo(ProgramCounter pc, TaggedPair **p, CodeInfo *nxt):
    PC(pc), lastPair(p), next(nxt) {}
};


CodeInfo *stack = NULL;

void enterBlock(ProgramCounter PC, TaggedPair **p)
{
  stack = new CodeInfo(PC,p,stack);
}

ProgramCounter leaveBlock(ProgramCounter PC)
{
  CodeInfo *aux = stack;
  stack = stack->next;
  (*(aux->lastPair))->val.pc = PC;
  PC = aux->PC;
  delete aux;
  return PC;
}




#define AddPair(last,tag,val)			\
  *last = new TaggedPair(tag,&val);		\
  last  = & (*last)->next;


TaggedPair *unpickle(FILE *in)
{
  TaggedPair *ret = NULL;
  TaggedPair **lastPair = &ret; /* pointer to the end for adding new items */

  TermTagTable termTags;
  Tagvalue val;

  /* old version */
  int tag = getTag(in);
  Assert(tag==TAG_STRING);
  val.string = strdup(scanString(in));
  AddPair(lastPair,tag,val);

  int major, minor;
  int aux = sscanf(val.string,"%d#%d",&major,&minor);
  if (aux !=2) {
    OZ_error("Version too new. Got: '%s', expected: '%s'.\n",val.string,PERDIOVERSION);
  }

  ProgramCounter PC = 0;
  ProgramCounter lastPC;
  while (1) {
    tag = getTag(in);
    switch (tag) {
      
    case TAG_OPCODE:
      val.opcode = stringToOpcode(scanString(in));
      lastPC = PC;
      PC += sizeOf(val.opcode);
      break;
      
    case TAG_LABELREF:
      val.labelRef.label  = stack->labels.useLabel(scanString(in));
      val.labelRef.lastPC = lastPC;
      break;

    case TAG_CODESTART: 
      enterBlock(PC,lastPair);
      PC     = 0;
      val.pc = 0; /* leaveBlock will update to contain address of last instr */
      break;

    case TAG_CODEEND:   
      val.pc = PC;
      PC     = leaveBlock(lastPC);
      break;

    case TAG_INT:       val.num = scanInt(in); break;
    case TAG_BYTE:      val.num = scanInt(in); break;
    case TAG_STRING:    val.string = strdup(scanString(in)); break;
    case TAG_COMMENT:   val.string = scanComment(in); break;
    case TAG_DIF:       val.mtag = char2Tag(scanString(in)); break;
    case TAG_LABELDEF:  val.labelDef = stack->labels.defLabel(scanString(in),PC); break;
    case TAG_EOF:       goto end;
    case TAG_TERMDEF:   val.ttag = termTags.add(scanString(in)); break;
    case TAG_TERMREF:   val.ttag = termTags.find(scanString(in)); Assert(val.ttag); break;

    default:
      OZ_error("unknown tag: '%c'\n",tag);
    }

    AddPair(lastPair,tag,val);
  }

 end:
  return ret;
}


/************************************************************/


main(int argc, char **argv)
{
  int textmode = 0;
  if (argc == 2 && strcmp(argv[1],"--textmode")==0) {
    /* out in textmode too: eliminates unused labels */
    textmode = 1;
  }

  TaggedPair *aux = unpickle(stdin);

  MsgBuffer fbuf(STDOUT_FILENO,textmode);
  pickle(aux,&fbuf);
}
