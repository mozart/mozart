/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: schulte
  Last modified: $Date$ from $Author$
  Version: $Revision$

*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "am.hh"

#include "builtins.hh"

#include "genvar.hh"
#include "ofgenvar.hh"
#include "fdbuilti.hh"
#include "solve.hh"

/*
 * Dynamically expanded string buffer
 */


#define SAFETY_MARGIN   256
#define STRING_BUFFER_SIZE 1024

class StringBuffer {
  char static_buffer[STRING_BUFFER_SIZE+SAFETY_MARGIN];
  char * buffer;
  char * start;
  char * end;

  void ensure(int n)
  {
    while (buffer+n>end) resize();
  }

public:
  StringBuffer() {
    start  = static_buffer;
    end    = start + STRING_BUFFER_SIZE;
    buffer = start;
  }
  ~StringBuffer() {
    dispose();
  }
  void reset(void) {
    dispose();
    start  = static_buffer;
    end    = start + STRING_BUFFER_SIZE;
    buffer = start;
  }

  int size() { return buffer-start; }

  char *string() { *buffer=0; return start; }
  char *getPtr() { return buffer; }
  void setPtr(char *ptr) { buffer = ptr; }

  char *allocate(int n) {
    ensure(n);
    char *ret = buffer;
    buffer += n;
    return ret;
  }

  void resize(void);

  void dispose(void)
  {
    if (start!=static_buffer) delete start;
  }

  void put(char c)
  {
    ensure(1);
    *buffer++ = c;
  }
  void put2(char c1,char c2)
  {
    ensure(2);
    *buffer++ = c1;
    *buffer++ = c2;
  }
  void back()
  {
    // assert(buffer>start);
    buffer--;
  }
  void put_octal(char c) {
    unsigned char c1 = (((unsigned char) c & '\300') >> 6) + '0';
    unsigned char c2 = (((unsigned char) c & '\070') >> 3) + '0';
    unsigned char c3 = ((unsigned char) c & '\007') + '0';
    ensure(4);
    *buffer++ = '\\';
    *buffer++ = c1;
    *buffer++ = c2;
    *buffer++ = c3;
  }
  void put_string(char *s) {
    ensure(strlen(s));
    char c;
    while ((c = *s++)) *buffer++=c;
  }
  void put_int(int i) {
    int len;
    sprintf(buffer,"%d%n",i,&len);
    buffer += len;
    ensure(0);
  }
  void put_float(double f) {
    int len;
    sprintf(buffer,"%g%n",f,&len);
    buffer += len;
    ensure(0);
  }
};

void StringBuffer::resize(void)
{
  int new_size = (3 * (end - start)) / 2; 
  char *new_start = new char[new_size + SAFETY_MARGIN];
  
  end   = new_start + new_size;
  
  memcpy(new_start, start, buffer-start);
    
  dispose();

  buffer = (buffer - start) + new_start;
  start  = new_start;
}

StringBuffer tcl_buffer;

OZ_Term NameTclName,
  AtomTclOption, AtomTclList, AtomTclPosition,
  AtomTclQuote, AtomTclString, AtomTclVS,
  AtomTclBatch,
  AtomError,
  AtomDot, AtomTagPrefix, AtomVarPrefix, AtomImagePrefix,
  NameGroupVoid;

OZ_C_proc_begin(BIgetTclName,1) {
  return OZ_unify(OZ_getCArg(0), NameTclName);
} OZ_C_proc_end


OZ_Return isTcl(TaggedRef tcl) {
  DEREF(tcl, tcl_ptr, tcl_tag);

  if (isAnyVar(tcl_tag)) {
    OZ_suspendOn(makeTaggedRef(tcl_ptr));
  } else if (isInt(tcl_tag)) {
    return PROCEED;
  } else if (isFloat(tcl_tag)) {
    return PROCEED;
  } else if (isLiteral(tcl_tag)) {
    if (tagged2Literal(tcl)->isAtom()) {
      return PROCEED;
    } else if (literalEq(tcl,NameTrue) || literalEq(tcl,NameFalse)) {
      return PROCEED;
    }
    return FAILED;
  } else if (isObject(tcl)) {
    TaggedRef v = tagged2Object(tcl)->getFeature(NameTclName);
    if (v!=makeTaggedNULL()) {
      DEREF(v, v_ptr, v_tag);
      if (isAnyVar(v_tag)) {
	OZ_suspendOn(makeTaggedRef(v_ptr)); 
      } else {
	return PROCEED;
      }
    } else {
      return FAILED;
    }
  } else if (isSTuple(tcl)) {
    SRecord  * st = tagged2SRecord(tcl);
    TaggedRef l  = st->getLabel();
    
    if (isAtom(l)) {
      if (literalEq(l,AtomPair)) { 
	OZ_assertVirtualString(tcl);
	return PROCEED;
      } else if (literalEq(l,AtomTclVS)) {
	TaggedRef arg = st->getArg(0);

	DEREF(arg, arg_ptr, arg_tag);

	if (isAnyVar(arg_tag)) {
	  OZ_suspendOn(makeTaggedRef(arg_ptr)); 
	}

	OZ_assertVirtualString(arg);
	return PROCEED;
      } else if (literalEq(l,AtomTclBatch)) {
	TaggedRef batch = st->getArg(0);

	while (1) {
	  DEREF(batch, batch_ptr, batch_tag);

	  if (isAnyVar(batch_tag)) {
	    OZ_suspendOn(makeTaggedRef(batch_ptr)); 
	  } else if (isCons(batch_tag)) {
	    OZ_Return batch_state = isTcl(head(batch));

	    if (batch_state!=PROCEED) 
	      return batch_state;

	    batch = tail(batch);
	  } else if (isLiteral(batch_tag) && literalEq(batch,AtomNil)) {
	    return PROCEED;
	  } else {
	    return FAILED;
	  }
	}

      } else {
	for (int i=0; i < st->getWidth(); i++) {
	  OZ_Return argstate = isTcl(st->getArg(i));
	  if (argstate!=PROCEED) 
	    return argstate;
	}
	return PROCEED;
      }
    }
  } else if (isSRecord(tcl_tag)) {
    SRecord * sr = tagged2SRecord(tcl);
    if (tagged2Literal(sr->getLabel())->isAtom()) {
      for (int i=0; i < sr->getWidth(); i++) {
      OZ_Return argstate = isTcl(sr->getArg(i));
      if (argstate!=PROCEED) 
	return argstate;
      }
      return PROCEED;
    } else {
      return FAILED;
    }
    
  } else if (isCons(tcl_tag)) {
    OZ_assertString(tcl);
    return PROCEED;
  }
    
  return FAILED;

}


OZ_C_proc_begin(BIisTcl, 2) {  
  OZ_Return s = isTcl(OZ_getCArg(0)); 
  switch (s) {
  case FAILED:  
    return OZ_unify(OZ_getCArg(1), NameFalse);
  case PROCEED: 
    return OZ_unify(OZ_getCArg(1), NameTrue);
  default: 
    return s;
  }
} OZ_C_proc_end


OZ_C_proc_begin(BIisTclFilter, 3) {
  TaggedRef tcl = OZ_getCArg(0);

  DEREF(tcl, tcl_ptr, tcl_tag);

  if (isAnyVar(tcl_tag)) {
    OZ_suspendOn(makeTaggedRef(tcl_ptr));
  } else if (isLiteral(tcl_tag)) {
    return OZ_unify(OZ_getCArg(2), NameTrue);
  } else if (isSRecord(tcl_tag)) {
    OZ_Return s = PROCEED;
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef as = sr->getArityList(); /* arity list is already deref'ed */
    TaggedRef fs = deref(OZ_getCArg(1));

    while (isCons(as) && isCons(fs)) {
      TaggedRef a = head(as);
      TaggedRef f = deref(head(fs));

      if (isFeature(a)) {

	switch (featureCmp(a,f)) {
	case 0:
	  fs = deref(tail(fs));
	  as = tail(as);
	  break;
	case 1:
	  fs = deref(tail(fs));
	  break;
	case -1:
	  s = isTcl(sr->getFeature(a));
	  if (s!=PROCEED)
	    goto exit;
	  as = tail(as);
	  break;
	}

      }
    }

    while (isCons(as)) {
      TaggedRef a = head(as);

      if (tagged2Literal(a)->isAtom()) {
	s = isTcl(sr->getFeature(a));
	if (s!=PROCEED)
	  goto exit;
	as = tail(as);
      }
    }
  exit:
    switch (s) {
    case FAILED:  
      return OZ_unify(OZ_getCArg(2), NameFalse);
    case PROCEED: 
      return OZ_unify(OZ_getCArg(2), NameTrue);
    default: 
      return s;
    }
  }
  Assert(0);
  return FAILED;
} OZ_C_proc_end





inline 
void tcl_put(char c) {
  tcl_buffer.put(c);
}

inline 
void tcl_back(void) {
  tcl_buffer.back();
}

inline 
void tcl_put2(char c1, char c2) {
  tcl_buffer.put2(c1,c2);
}

inline 
void tcl_put_octal(char c) {
  tcl_buffer.put_octal(c);
}

inline 
void tcl_put_quote(char c) {
  unsigned char uc = (unsigned char) c;
  switch (uc) {
  case '\a': tcl_put2('\\', 'a'); break;
  case '\b': tcl_put2('\\', 'b'); break;
  case '\f': tcl_put2('\\', 'f'); break;
  case '\n': tcl_put2('\\', 'n'); break;
  case '\r': tcl_put2('\\', 'r'); break;
  case '\t': tcl_put2('\\', 't'); break;
  case '\v': tcl_put2('\\', 'v'); break;
  case '{':   case '}':   case '\\':  case '$':
  case '[':   case ']':   case '"':   case ';':
  case ' ':
    tcl_put2('\\', c); break;
  default:
    if ((uc<33) || (uc>127)) {
      tcl_put_octal(c);
    } else {
      tcl_put(c);
    }
  }
}
  

inline
void cstring2buffer(char* s) {
  char c;
  while ((c = *s++))
    tcl_put(c);
}
 

inline
void atom2buffer(TaggedRef atom) {
  cstring2buffer(tagged2Literal(atom)->getPrintName());
}


inline
void int2buffer(TaggedRef i) {
  if (isSmallInt(i)) {
    tcl_buffer.put_int(smallIntValue(i));
  } else {
    char * s = toC(i);
    if (*s == '~') *s='-';
    tcl_buffer.put_string(s);
    delete [] s;
  }
}


inline
void float2buffer(TaggedRef f) {
  tcl_buffer.put_float(floatValue(f));
}


inline
void string2buffer(TaggedRef list) {
  do {
    tcl_put((char) smallIntValue(deref(head(list))));
    list = deref(tail(list));
  } while (isCons(list));
}


void vs2buffer(TaggedRef vs) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isInt(vs_tag)) {
    int2buffer(vs);
  } else if (isFloat(vs_tag)) {
    float2buffer(vs);
  } else if (isLiteral(vs_tag)) {
    Assert(tagged2Literal(vs)->isAtom());
    
    if (!literalEq(vs, AtomNil) && !literalEq(vs, AtomPair))
      atom2buffer(vs);
  } else if (isSTuple(vs)) {
    Assert(literalEq(tagged2SRecord(vs)->getLabel(),AtomPair));

    for (int i=0; i < tagged2SRecord(vs)->getWidth(); i++)
      vs2buffer(tagged2SRecord(vs)->getArg(i));
  } else if (isCons(vs_tag)) {
    string2buffer(vs);
  }
}


static char *protect_start;
inline
void start_protect(void) {
  protect_start = tcl_buffer.getPtr();
}


inline
void stop_protect(void) {
  if (protect_start == tcl_buffer.getPtr())
    tcl_put2('"','"');
}


inline
void protect_atom2buffer(TaggedRef atom) {
  if (literalEq(atom, AtomPair) || literalEq(atom, AtomNil))
    return;

  char *s = tagged2Literal(atom)->getPrintName();
  char c;

  while ((c = *s++))
    tcl_put_quote(c);
}


inline
void protect_string2buffer(TaggedRef list) {
  do {
    tcl_put_quote((char) smallIntValue(deref(head(list))));
    list = deref(tail(list));
  } while (isCons(list));
}


void protect_vs2buffer(TaggedRef vs) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isInt(vs_tag)) {
    int2buffer(vs);
  } else if (isFloat(vs_tag)) {
    float2buffer(vs);
  } else if (isLiteral(vs_tag)) {
    Assert(tagged2Literal(vs)->isAtom());
    
    if (!literalEq(vs, AtomNil) && !literalEq(vs, AtomPair))
      protect_atom2buffer(vs);
  } else if (isSTuple(vs)) {
    Assert(literalEq(tagged2SRecord(vs)->getLabel(),AtomPair));

    for (int i=0; i < tagged2SRecord(vs)->getWidth(); i++)
      protect_vs2buffer(tagged2SRecord(vs)->getArg(i));
  } else if (isCons(vs_tag)) {
    protect_string2buffer(vs);
  }
}


void tcl2buffer(TaggedRef);


inline
void feature2buffer(SRecord *st, OZ_Term a) {
  if (isSmallInt(a)) {
    tcl2buffer(st->getFeature(a));
  } else if (tagged2Literal(a)->isAtom()) {
    tcl_put('-');
    start_protect();
    protect_atom2buffer(a);
    stop_protect();
    tcl_put(' ');
    tcl2buffer(st->getFeature(a));
  }
}

void tuple2buffer(SRecord *st, int start = 0) {
  if (st->isTuple()) {
    if (start < st->getWidth()) {
      tcl2buffer(st->getArg(start));
      for (int i=start+1; i < st->getWidth(); i++) {
	tcl_put(' ');
	tcl2buffer(st->getArg(i));
      }
    }
  } else {
    TaggedRef as = st->getArityList();
    
    if (start==1 && isCons(as)) {
      Assert(smallIntValue(head(as))==1);
      as=tail(as);
    }
    if (!isCons(as))
      return;

    feature2buffer(st,head(as));
    as = tail(as);

    while (isCons(as)) {
      TaggedRef a = head(as);
      tcl_put(' ');
      feature2buffer(st,a);
      as = tail(as);
    }
  }
}


void tcl2buffer(TaggedRef tcl) {

  DEREF(tcl, tcl_ptr, tcl_tag);

  Assert(!isAnyVar(tcl_tag));

  if (isInt(tcl_tag)) {
    int2buffer(tcl);
  } else if (isFloat(tcl_tag)) {
    float2buffer(tcl);
  } else if (isLiteral(tcl_tag)) {
    if (tagged2Literal(tcl)->isAtom()) {
      start_protect();
      protect_atom2buffer(tcl);
      stop_protect();
    } else if (literalEq(tcl,NameTrue)) {
      tcl_put('1');
    } else if (literalEq(tcl,NameFalse)) {
      tcl_put('0');
    }
  } else if (isCons(tcl_tag)) {
    start_protect();
    protect_string2buffer(tcl);
    stop_protect();
  } else if (isSTuple(tcl)) {
    SRecord  * st = tagged2SRecord(tcl);
    TaggedRef l  = st->getLabel();
    
    if (literalEq(l,AtomPair)) {
      start_protect();
      protect_vs2buffer(tcl);
      stop_protect();
    } else if (literalEq(l,AtomTclOption)) {
      tuple2buffer(st);
    } else if (literalEq(l,AtomTclList)) {
      tcl_put('['); tuple2buffer(st); tcl_put(']');
    } else if (literalEq(l,AtomTclQuote)) {
      tcl_put('{'); tuple2buffer(st); tcl_put('}');
    } else if (literalEq(l,AtomTclString)) {
      tcl_put('"'); tuple2buffer(st); tcl_put('"');
    } else if (literalEq(l,AtomTclPosition)) {
      if (st->getWidth() > 1) {
	tcl_put('{'); tcl2buffer(st->getArg(0));
	tcl_put('.'); tuple2buffer(st, 1);
	tcl_put('}');
      }
    } else if (literalEq(l,AtomTclVS)) {
      vs2buffer(st->getArg(0));
    } else if (literalEq(l,AtomTclBatch)) {
      TaggedRef b = deref(st->getArg(0));

      if (isCons(b)) {
	tcl2buffer(head(b));
	b = deref(tail(b));

	while (isCons(b)) {
	  tcl_put(' ');
	  tcl2buffer(head(b));
	  b = deref(tail(b));
	}
      }
    } else {
      start_protect();
      protect_atom2buffer(st->getLabel());
      stop_protect();
      tcl_put(' '); tuple2buffer(st);
    }

  } else if (isObject(tcl)) {
    vs2buffer( tagged2Object(tcl)->getFeature(NameTclName) );
  } else if (isSRecord(tcl_tag)) { 
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef l  = sr->getLabel();

    if (!literalEq(l,AtomTclOption)) {
      start_protect();
      protect_atom2buffer(l);
      stop_protect();
      tcl_put(' ');
    }
    
    TaggedRef as = sr->getArityList();

    while (isCons(as)) {
      TaggedRef a = head(as);

      if (isSmallInt(a)) {
	tcl2buffer(sr->getFeature(a));
	tcl_put(' ');
      } else if (tagged2Literal(a)->isAtom()) {
	tcl_put('-');
	start_protect();
	protect_atom2buffer(a);
	stop_protect();
	tcl_put(' ');
	tcl2buffer(sr->getFeature(a));
	tcl_put(' ');
      }
      
      as = tail(as);
    }

    tcl_back();
  }
 
}


OZ_Return ret_unix_error(TaggedRef out) {
  SRecord * err_tuple = SRecord::newSRecord(AtomError, 2);
    
  err_tuple->setArg(0, OZ_int(errno));
  err_tuple->setArg(1, OZ_atom(OZ_unixError(errno)));
  
  return OZ_unify(out,makeTaggedSRecord(err_tuple));
}


OZ_Return tcl_write(int fd, char * buff, int len, TaggedRef out) {
  int ret = osTestSelect(fd,SEL_WRITE);

  if (ret < 0)  { 
    tcl_buffer.reset();
    return ret_unix_error(out);
  } else if (ret==0) {
    return OZ_unifyInt(out,0);
  }  

  while ((ret = write(fd, buff, len)) < 0) {
    if (errno != EINTR) { 
      tcl_buffer.reset();
      return ret_unix_error(out);
    }
  }

  if (len == ret) {
    tcl_buffer.reset();

    return OZ_unify(out, NameTrue);
  }

  return OZ_unifyInt(out, ret);
}


OZ_C_proc_begin(BItclWrite,3) {  
  OZ_declareIntArg(0, fd);

  tcl_buffer.reset();
  tcl2buffer(OZ_getCArg(1));
  tcl_put('\n');

  return tcl_write(fd, tcl_buffer.string(), tcl_buffer.size(), 
		   OZ_getCArg(2));
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteBatch,3) {  
  OZ_declareIntArg(0, fd);
  TaggedRef batch = deref(OZ_getCArg(1));

  tcl_buffer.reset();

  while (isCons(batch)) {
    tcl2buffer(head(batch));
    tcl_put(';');
    batch = deref(tail(batch));
  }
  tcl_put('\n');

  return tcl_write(fd, tcl_buffer.string(), tcl_buffer.size(), 
		   OZ_getCArg(2));
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteTuple,4) {  
  OZ_declareIntArg(0, fd);

  tcl_buffer.reset();
  tcl2buffer(OZ_getCArg(1));
  tcl_put(' ');
  tuple2buffer(tagged2SRecord(deref(OZ_getCArg(2))));
  tcl_put('\n');

  return tcl_write(fd, tcl_buffer.string(), tcl_buffer.size(),
		   OZ_getCArg(3));
}
OZ_C_proc_end



OZ_C_proc_begin(BItclWriteTagTuple,5) {  
  OZ_declareIntArg(0, fd);
  TaggedRef tuple = deref(OZ_getCArg(3));

  tcl_buffer.reset();
  tcl2buffer(OZ_getCArg(1));
  tcl_put(' ');
  tcl2buffer(tagged2SRecord(tuple)->getArg(0));
  tcl_put(' ');
  tcl2buffer(OZ_getCArg(2));
  tcl_put(' ');
  tuple2buffer(tagged2SRecord(tuple),1);
  tcl_put('\n');

  return tcl_write(fd, tcl_buffer.string(), tcl_buffer.size(),
		   OZ_getCArg(4));
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteFilter,7) {  
  OZ_declareIntArg(0, fd);
  OZ_declareArg(1,tkclass);
  OZ_declareArg(2,id);
  OZ_declareArg(3,mess);
  OZ_declareArg(4,filter);
  OZ_declareArg(5,cont);

  tcl_buffer.reset();
  tcl2buffer(tkclass);
  tcl_put(' ');
  vs2buffer(id);
  tcl_put(' ');

  TaggedRef tr   = mess;

  DEREF(tr, t_p, tr_tag);

  if (isSRecord(tr_tag)) {
    SRecord   * sr = tagged2SRecord(tr);
    TaggedRef as   = sr->getArityList();
    TaggedRef fs   = deref(filter);
    
    while (isCons(as) && isCons(fs)) {
      TaggedRef a = head(as);
      TaggedRef f = deref(head(fs));
      
      if (isFeature(a)) {
	
	switch (featureCmp(a,f)) {
	case 0:
	  fs = deref(tail(fs));
	  as = tail(as);
	  break;
	case 1:
	  fs = deref(tail(fs));
	  break;
	case -1:
	  feature2buffer(sr,a);
	  tcl_put(' ');
	  as = tail(as);
	  break;
	}
	
      }

    }
    
    while (isCons(as)) {
      TaggedRef a = head(as);
      
      if (tagged2Literal(a)->isAtom()) {
	tcl_put('-');
	atom2buffer(a);
      tcl_put(' ');
      tcl2buffer(sr->getFeature(a));
      tcl_put(' ');
      as = tail(as);
      }
    }
  }
	
  tcl2buffer(cont);
  tcl_put('\n');

  return tcl_write(fd, tcl_buffer.string(), tcl_buffer.size(),
		   OZ_getCArg(6));
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteCont,3) {  
  OZ_declareIntArg(0, fd);
  OZ_declareIntArg(1, written);

  return tcl_write(fd, tcl_buffer.string() + written, 
		   tcl_buffer.size() - written, 
		   OZ_getCArg(2));
}
OZ_C_proc_end


/*
 * Groups
 */


inline
TaggedRef findAliveEntry(TaggedRef group) {
  group = deref(group);

  while (isCons(group)) {
      TaggedRef ahead = deref(head(group));

      if (!(isLiteral(ahead) && literalEq(ahead,NameGroupVoid)))
	return group;
      
      group = deref(tail(group));
  }

  return group;
}


OZ_C_proc_begin(BIaddFastGroup,3)
{
  OZ_nonvarArg(0);
  TaggedRef group = deref(OZ_getCArg(0)); 

  if (isCons(group)) {
    TaggedRef member = cons(OZ_getCArg(1),findAliveEntry(tail(group)));
    tagged2LTuple(group)->setTail(member);
    return OZ_unify(member,OZ_getCArg(2));
  }
  return FAILED;
} 
OZ_C_proc_end


OZ_C_proc_begin(BIdelFastGroup,1)
{
  TaggedRef member = deref(OZ_getCArg(0));

  if (isCons(member)) {
    tagged2LTuple(member)->setHead(NameGroupVoid);
    tagged2LTuple(member)->setTail(findAliveEntry(tail(member)));
  }

  return PROCEED;
} 
OZ_C_proc_end

OZ_C_proc_begin(BIdelAndTestFastGroup,2)
{
  TaggedRef member = deref(OZ_getCArg(0));
  OZ_declareArg(1,out);

  if (isCons(member)) {
    if (literalEq(deref(head(member)),NameGroupVoid)) {
      return OZ_unify(out,NameFalse);
    } else {
      tagged2LTuple(member)->setHead(NameGroupVoid);
      tagged2LTuple(member)->setTail(findAliveEntry(tail(member)));
      return OZ_unify(out,NameTrue);
    }
  }

  /* e.g. toplevel is nobodys slave */
  return OZ_unify(out,NameTrue);
} 
OZ_C_proc_end


OZ_C_proc_begin(BIgetFastGroup,2)
{
  OZ_nonvarArg(0);
  TaggedRef group = OZ_getCArg(0); 

  DEREF(group, _1, _2);

  if (isCons(group)) {
    TaggedRef out = nil();

    group = deref(tail(group));
    
    while (isCons(group)) {
      TaggedRef ahead = deref(head(group));

      if (!(isLiteral(ahead) && literalEq(ahead,NameGroupVoid)))
	out = cons(ahead, out);
      
      group = deref(tail(group));
    }

    return isNil(group) ? OZ_unify(out,OZ_getCArg(1)) : FAILED;
  }
  
  return FAILED;

} 
OZ_C_proc_end


OZ_C_proc_begin(BIdelAllFastGroup,2)
{
  OZ_nonvarArg(0);
  TaggedRef group = OZ_getCArg(0); 

  DEREF(group, _1, _2);

  Assert(isCons(group));
  TaggedRef out = nil();

  group = deref(tail(group));

  while (isCons(group)) {
    TaggedRef ahead = deref(head(group));

    if (!(isLiteral(ahead) && literalEq(ahead,NameGroupVoid))) {
      out = cons(ahead, out);
      tagged2LTuple(group)->setHead(NameGroupVoid);
    }
      
    group = deref(tail(group));
  }

  Assert(isNil(group));
  return OZ_unify(out,OZ_getCArg(1));
}
OZ_C_proc_end



// ---------------------------------------------------------------------
// Counters
// ---------------------------------------------------------------------

static int top_ctr    = 0;
static int widget_ctr = 0;
static int tag_ctr    = 0;
static int var_ctr    = 0;
static int image_ctr  = 0;

OZ_C_proc_begin(BIgenTopName,1) {
  SRecord * s = SRecord::newSRecord(AtomPair,2);
  s->setArg(0,AtomDot);
  s->setArg(1,makeInt(top_ctr++));
  return OZ_unify(OZ_getCArg(0),makeTaggedSRecord(s));
} OZ_C_proc_end

OZ_C_proc_begin(BIgenWidgetName,2) {
  TaggedRef parent = OZ_getCArg(0);

  DEREF(parent, p_ptr, p_tag);

  if (isAnyVar(p_tag))
    OZ_suspendOn(makeTaggedRef(p_ptr));

  SRecord * s = SRecord::newSRecord(AtomPair,3);
  s->setArg(0,parent);
  s->setArg(1,AtomDot);
  s->setArg(2,makeInt(widget_ctr++));
  return OZ_unify(OZ_getCArg(1),makeTaggedSRecord(s));
} OZ_C_proc_end

OZ_C_proc_begin(BIgenTagName,1) {
  SRecord * s = SRecord::newSRecord(AtomPair,2);
  s->setArg(0,AtomTagPrefix);
  s->setArg(1,makeInt(tag_ctr++));
  return OZ_unify(OZ_getCArg(0),makeTaggedSRecord(s));
} OZ_C_proc_end

OZ_C_proc_begin(BIgenVarName,1) {
  SRecord * s = SRecord::newSRecord(AtomPair,2);
  s->setArg(0,AtomVarPrefix);
  s->setArg(1,makeInt(var_ctr++));
  return OZ_unify(OZ_getCArg(0),makeTaggedSRecord(s));
} OZ_C_proc_end

OZ_C_proc_begin(BIgenImageName,1) {
  SRecord * s = SRecord::newSRecord(AtomPair,2);
  s->setArg(0,AtomImagePrefix);
  s->setArg(1,makeInt(image_ctr++));
  return OZ_unify(OZ_getCArg(0),makeTaggedSRecord(s));
} OZ_C_proc_end

// ---------------------------------------------------------------------
// Add to Builtin-Table
// ---------------------------------------------------------------------

static
BIspec tclTkSpec[] = {
  {"getTclName",       1, BIgetTclName,        0},
  {"isTcl",            2, BIisTcl,             0},
  {"isTclFilter",      3, BIisTclFilter,       0},
  {"tclWrite",         3, BItclWrite,          0},
  {"tclWriteBatch",    3, BItclWriteBatch,     0},
  {"tclWriteTuple",    4, BItclWriteTuple,     0},
  {"tclWriteTagTuple", 5, BItclWriteTagTuple,  0},
  {"tclWriteFilter",   7, BItclWriteFilter,    0},
  {"tclWriteCont",     3, BItclWriteCont,      0},

  {"addFastGroup",        3, BIaddFastGroup,	   0},
  {"delFastGroup",        1, BIdelFastGroup,	   0},
  {"delAndTestFastGroup", 2, BIdelAndTestFastGroup,0},
  {"getFastGroup",        2, BIgetFastGroup,	   0},
  {"delAllFastGroup",     2, BIdelAllFastGroup,    0},

  {"genTopName",    1, BIgenTopName,	 0},
  {"genWidgetName", 2, BIgenWidgetName,	 0},
  {"genTagName",    1, BIgenTagName,	 0},
  {"genVarName",    1, BIgenVarName,	 0},
  {"genImageName",  1, BIgenImageName,	 0},

  {0,0,0,0}
};


void BIinitTclTk() {
  BIaddSpec(tclTkSpec);

  NameTclName      = OZ_newName();
  AtomTclOption    = OZ_atom("o");
  AtomTclList      = OZ_atom("l");
  AtomTclPosition  = OZ_atom("p");
  AtomTclQuote     = OZ_atom("q");
  AtomTclString    = OZ_atom("s");
  AtomTclVS        = OZ_atom("v");
  AtomTclBatch     = OZ_atom("b");
  AtomError        = OZ_atom("error");
  AtomDot          = OZ_atom(".");
  AtomTagPrefix    = OZ_atom("t");
  AtomVarPrefix    = OZ_atom("v");
  AtomImagePrefix  = OZ_atom("i");

  NameGroupVoid    = OZ_newName();
}

