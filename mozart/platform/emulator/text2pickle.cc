/*
 *  Authors:
 *    Ralf Scheidhr (scheidhr@ps.uni-sb.de)
 * 
 *  Contributors:
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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

/* do not include extension.hh: otherwise HPUX does not find OZ_atom :-( */
#define __EXTENSIONHH

#include "config.h"
#include "base.hh"
#include "opcodes.cc"

// kost@ : that's broken 'cause of 'scanInt()'.
static int line=1, col=0;

void OZ_error(const char *format, ...)
{
#if defined(DEBUG_CHECK)
   fprintf(stderr, "Waiting 10 secs... hook up (pid %d)!\n", getpid());
   fflush(stderr);
   sleep(10);
#endif
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

//
// MarshalerBuffer is needed to make the compiler happy (PickleMarshalerBuffer
// is inherited from it), as well as to have additional debug checkss;
class MarshalerBuffer {
protected:
  TextBlock *first, *last;
  int pos;
public: 
  int fd;

  //
public:
  MarshalerBuffer(int f)
    : fd(f), pos(0) {
    first = last = new TextBlock();
  }

  void put(BYTE c) { 
    if (pos==TBSize){
      last->next = new TextBlock();
      last = last->next;
      pos = 0;      
    }
    last->text[pos++] = c;
  }

  BYTE get() { 
    OZ_error("no 'MarshalerBuffer::get()' in text2pickle!");
    return ((BYTE) 0);
  }
};

// 
// Here (in 'text2pickle'), the 'marshal*(PickleMarshalerBuffer *bs, ...)'
// functions are made to see this definition of PickleMarshalerBuffer instead
// of the 'pickeBase.hh's one!
class PickleMarshalerBuffer : public MarshalerBuffer {
private:
  int mode;

public:
  PickleMarshalerBuffer(int f, int m)
    : MarshalerBuffer(f), mode(m) {}

  int textmode() { return mode!=0; }
  void dump() {
    while(first->next) {
      int ret=write(fd,first->text,TBSize);
      if(ret<=0)_exit(1);
      first = first->next;
    }
    int ret=write(fd,first->text,pos);
    if(ret<=0)_exit(1);
  }
  unsigned long crc();
};


#define TEXT2PICKLE
//
#include "marshalerBase.cc"
#include "pickleBase.cc"

//
static
void marshalComment(PickleMarshalerBuffer *bs, char *s)
{
  if (bs->textmode()) {
    putTag(bs, TAG_COMMENT);
    while (*s) {
      bs->put(*s);
      s++;
    }
    bs->put('\n');
  }
}

//
static
void marshalLabelDef(PickleMarshalerBuffer *bs, char *lbl)
{
  if (bs->textmode()) {
    putTag(bs, TAG_LABELDEF);
    putString(bs, lbl);
  }
}

unsigned long PickleMarshalerBuffer::crc()
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
  static const int tableSize = 16411;  /* fixed size, cannot grow :-( */
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
  unsigned ttag;		// do nothing with labels (kept as an int);
} Tagvalue;


class TaggedPair {
public:
  int tag;
  Tagvalue val;
  TaggedPair *next;

  TaggedPair(int t, Tagvalue *v): tag(t), val(*v), next(NULL) {}
};

/************************************************************/

void pickle(TaggedPair *aux, PickleMarshalerBuffer *out)
{
  /* write new version number */
  Assert(aux->tag==TAG_STRING);

  // raph: The pickle is written in the *current* format with the
  // *current* version number.  unpickle() is responsible for checking
  // version compatibility.
  marshalString(out, MARSHALERVERSION);
  aux = aux->next;

  while(aux) {

    switch (aux->tag) {

    case TAG_LABELDEF:
      if (aux->val.labelDef->used) {
	marshalLabelDef(out, aux->val.labelDef->label);
      }
      break;

    case TAG_LABELREF:
      {
	Label *lbl = aux->val.labelRef.label;
	Assert(lbl->defined);
	// Assert(aux->val.labelRef.lastPC);
	if (out->textmode()) {
	  putTag(out, TAG_LABELREF);
	  putString(out, lbl->label);
	} else {
	  marshalLabel(out, 0, lbl->addr - aux->val.labelRef.lastPC);
	}
	break;
      }

    case TAG_CODESTART: 
      marshalCodeStart(out); 
      break;

    case TAG_INT:       marshalNumber(out, aux->val.num); break;
    case TAG_CODEEND:   marshalCodeEnd(out); break;
    case TAG_BYTE:      marshalByte(out, aux->val.num); break;
    case TAG_OPCODE:    marshalOpCode(out, 0, aux->val.opcode, 0); break;
    case TAG_STRING:    marshalString(out, aux->val.string); break;
    case TAG_COMMENT:   marshalComment(out, aux->val.string); break;
    case TAG_DIF:       marshalDIF(out, aux->val.mtag); break;
    case TAG_TERMDEF:   marshalTermDef(out, aux->val.ttag); break;
    case TAG_TERMREF:   marshalTermRef(out, aux->val.ttag); break;
    default:            Assert(0);
    }
    aux = aux->next;
  }

  if (!out->textmode()) {
    int headerSize;
    char *header = makeHeader(out->crc(),&headerSize);
    int ret=write(out->fd,header,headerSize);
    if(ret<0)_exit(1);
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

//
// kost@ : stack is needed for handling code areas: they can be
// nested. Note that "old" and "new" pickles are handled in the same
// way;
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

  Tagvalue val;

  /* old version */
  int tag = getTag(in);
  Assert(tag==TAG_STRING);
  val.string = strdup(scanString(in));
  AddPair(lastPair,tag,val);

  int major, minor;
  int aux = sscanf(val.string,"%d#%d",&major,&minor);
  if (aux !=2) {
    OZ_error("Version too new. Got: '%s', expected: '%s'.\n",
	     val.string, MARSHALERVERSION);
  }

  // check compatibility with current version: we currently are fully
  // backwards compatible
  if ((major << 16 | minor) > (MARSHALERMAJOR << 16 | MARSHALERMINOR)) {
    OZ_error("Compatibility check: unable convert from %s to %s.\n",
	     val.string, MARSHALERVERSION);
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
    case TAG_LABELDEF:
      val.labelDef = stack->labels.defLabel(scanString(in),PC);
      break;
    case TAG_EOF:       goto end;
    case TAG_TERMDEF:
    case TAG_TERMREF:   val.ttag = scanInt(in); break;
    default:
      OZ_error("unknown tag: '%c'\n",tag);
    }

    AddPair(lastPair,tag,val);
  }

 end:
  return ret;
}


/************************************************************/


int main(int argc, char **argv)
{
  int textmode = 0;
  int fd = STDOUT_FILENO;

  if (argc >= 2 && !strcmp(argv[1],"--textmode")) {
    /* out in textmode too: eliminates unused labels */
    textmode = 1;
    argv++;
    argc--;
  }
  if (argc >= 3 && !strcmp(argv[1],"-o")) {
    fd = open(argv[2],O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0777);
    if (fd == -1) {
      fprintf(stderr,"text2pickle: could not open output file %s\n",argv[2]);
      exit(1);
    }
    argv += 2;
    argc -= 2;
  }
  if (argc != 1) {
    fprintf(stderr,"Usage: text2pickle [--textmode] [-o <file>]\n");
    exit(2);
  }

  TaggedPair *aux = unpickle(stdin);

  PickleMarshalerBuffer fbuf(fd,textmode);
  pickle(aux,&fbuf);
}
