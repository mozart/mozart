/*
 *  Authors:
 *    Christian Schulte (schulte@dfki.de)
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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "runtime.hh"
#include "tcl_tk.hh"
#include "gc.hh"

#include "builtins.hh"

#include "genvar.hh"
#include "ofgenvar.hh"
#include "fdbuilti.hh"
#include "solve.hh"

#include "dictionary.hh"
#include "find-alive-entry.hh"

TaggedRef NameTclName, 
  AtomTclOption, AtomTclList, AtomTclPosition,
  AtomTclQuote, AtomTclString, AtomTclVS,
  AtomTclBatch, AtomTclColor,
  AtomError,
  AtomDot, AtomTagPrefix, AtomVarPrefix, AtomImagePrefix,
  NameTclClosed,
  NameTclSlaves,
  NameTclSlaveEntry;

OZ_Return raise_os_error()
{
  int xx = errno;
  return oz_raise(E_SYSTEM,E_OS,"os",2,OZ_int(xx),
		  OZ_string(OZ_unixError(xx)));
}

OZ_Return raise_type_error(TaggedRef tcl)
{
  oz_typeError(-1,"Tickle");
}

OZ_Return raise_closed(TaggedRef tcl) {
  return oz_raise(E_SYSTEM,E_TK,"alreadyClosed",1,tcl);
}

OZ_Return raise_toplevel(void) {
  return oz_raise(E_ERROR,E_KERNEL,"globalState",1,OZ_atom("io"));
}

#define CHECK_TOPLEVEL     \
if (!am.onToplevel())      \
  return raise_toplevel();



OZ_BI_define(BIgetTclNames,0,3) {
  OZ_out(0)=NameTclSlaves;
  OZ_out(1)=NameTclSlaveEntry;
  OZ_out(2)=NameTclName;
  return PROCEED;
} OZ_BI_end

/*
 * Locking
 */

#define ENTER_TCL_LOCK(TS) { \
  TaggedRef t = (TS)->getLock();                                  \
  DEREF(t, t_ptr, t_tag);                                         \
  if (isVariableTag(t_tag)) {                                          \
    am.addSuspendVarList(t_ptr);                                  \
    return SUSPEND;                                               \
  } else {                                                        \
    (TS)->setLock(oz_newVariable()); \
  }                                                               \
}

#define LEAVE_TCL_LOCK(TS) \
  (void) oz_unify((TS)->getLock(), NameUnit); // mm_u


/*
 * Dynamically expanded string buffer
 */

#define StateExit(Check) \
  { s = Check; if (s != PROCEED) goto exit; }
#define StateReturn(Check) \
  { OZ_Return s = Check; if (s != PROCEED) return s;  }



inline 
char hex_digit(unsigned int i) {
  return (i>9) ? (i - 10 + 'a') : (i + '0');
}

#define SAFETY_MARGIN      256
#define STRING_BUFFER_SIZE 2048

class TclSession {
  char * static_buffer;
  char * buffer;
  char * start;
  char * write_start;
  char * end;
  char * protect_start;

  int tcl_fd;

  TaggedRef tcl_lock;
  TaggedRef tcl_rets;
  TaggedRef tcl_dict;

  int widget_ctr;
  int tag_ctr;
  int var_ctr;
  int image_ctr;

  void ensure(int n) {
    while (buffer+n>end) 
      resize();
  }

public:
  TaggedRef getLock() { 
    return tcl_lock; 
  }

  void setLock(TaggedRef t) { 
    tcl_lock = t; 
  }

  void enterReturn(TaggedRef ret, TaggedRef cast) {
    TaggedRef newt = OZ_cons(OZ_cons(ret,cast),
			     oz_newVariable());
    
    (void) oz_unify(newt,tcl_rets); // mm_u
    tcl_rets = tail(newt);
  }

  TaggedRef genTopName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,AtomDot);
    s->setArg(1,makeInt(widget_ctr++));
    return makeTaggedSRecord(s);
  }

  TaggedRef genWidgetName(TaggedRef parent) {
    SRecord * s = SRecord::newSRecord(AtomPair,3);
    s->setArg(0,parent);
    s->setArg(1,AtomDot);
    s->setArg(2,makeInt(widget_ctr++));
    return makeTaggedSRecord(s);
  }

  TaggedRef genTagName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,AtomTagPrefix);
    s->setArg(1,makeInt(tag_ctr++));
    return makeTaggedSRecord(s);
  }

  TaggedRef genVarName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,AtomVarPrefix);
    s->setArg(1,makeInt(var_ctr++));
    return makeTaggedSRecord(s);
  }

  TaggedRef genImageName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,AtomImagePrefix);
    s->setArg(1,makeInt(image_ctr++));
    return makeTaggedSRecord(s);
  }

  
  TclSession(int fd, TaggedRef d, TaggedRef r) {
    widget_ctr = 0;
    tag_ctr    = 0;
    var_ctr    = 0;
    image_ctr  = 0;

    tcl_fd        = fd;
    tcl_lock      = NameUnit;
    tcl_rets      = r;
    tcl_dict      = d;

    static_buffer = new char[STRING_BUFFER_SIZE+SAFETY_MARGIN];
    start         = static_buffer;
    end           = start + STRING_BUFFER_SIZE;
    buffer        = start;
  }

  ~TclSession() {
    delete static_buffer;
    dispose();
  }

  void reset(void) {
    dispose();
    start  = static_buffer;
    end    = start + STRING_BUFFER_SIZE;
    buffer = start;
  }

  void start_write(void) {
    write_start = start;
  }
  
  void gc() {
    if (this) {
      OZ_collectHeapTerm(tcl_lock,tcl_lock);
      OZ_collectHeapTerm(tcl_dict,tcl_dict);
      OZ_collectHeapTerm(tcl_rets,tcl_rets);
    }
  }

  OZ_Return write(void);

  void resize(void);

  void dispose(void) {
    if (start!=static_buffer) 
      delete start;
  }

  void put(char c) {
    *buffer++ = c;
    ensure(0);
  }
  void put2(char c1,char c2) {
    *buffer++ = c1;
    *buffer++ = c2;
    ensure(0);
  }

  void start_protect(void) {
    protect_start = buffer;
  }

  void stop_protect(void) {
    if (protect_start == buffer)
      put2('"','"');
  }

  void put_quote(char c) {
    unsigned char uc = (unsigned char) c;
    switch (uc) {
    case '{':   case '}':   case '\\':  case '$':
    case '[':   case ']':   case '"':   case ';':
    case ' ':
      *buffer++ = '\\';
      *buffer++ = c; 
      break;
    default:
      if ((uc<33) || (uc>127)) {
	unsigned char c1 = ((uc & '\300') >> 6) + '0';
	unsigned char c2 = ((uc & '\070') >> 3) + '0';
	unsigned char c3 = (uc & '\007') + '0';
	*buffer++ = '\\';
	*buffer++ = c1;
	*buffer++ = c2;
	*buffer++ = c3;
      } else {
	*buffer++ = c;
      }
    }
  }
  
  /* Tcl Methods */

  void put_int(TaggedRef i) {
    if (oz_isSmallInt(i)) {
      int len;
      sprintf(buffer,"%d%n",smallIntValue(i),&len);
      buffer += len;
      ensure(0);
    } else {
      char * s = toC(i);
      if (*s == '~') *s='-';
      ensure(strlen(s));
      char c;
      while ((c = *s++)) 
	*buffer++=c;
    }
  }

  void put_float(TaggedRef f) {
    int len;
    sprintf(buffer,"%g%n",floatValue(f),&len);
    buffer += len;
    ensure(0);
  }

  void put_atom(TaggedRef atom) {
    if (literalEq(atom, AtomPair) || literalEq(atom, AtomNil))
      return;
    Assert(oz_isAtom(atom));
    Atom *l = (Atom*) tagged2Literal(atom);
    int n = l->getSize();
    const char *s = l->getPrintName();

    ensure(n);
    for (int i = 0; i < n; i++) {
      *buffer++ = *s++;
    }
  }

  void put_atom_quote(TaggedRef atom) {
    if (literalEq(atom, AtomPair) || literalEq(atom, AtomNil))
      return;

    Assert(oz_isAtom(atom));
    Atom* l = (Atom*)tagged2Literal(atom);
    int n = l->getSize();
    const char *s = l->getPrintName();
    char c;
    ensure(4*n);

    while ((c = *s++))
      put_quote(c);
    
  }

  OZ_Return put_string_quote(TaggedRef list) {
    while (1) {
      TaggedRef h = head(list);
      DEREF(h, h_ptr, h_tag);

      if (isVariableTag(h_tag)) {
	am.addSuspendVarList(h_ptr);
	return SUSPEND;
      }
      if (!isSmallIntTag(h_tag))
	return raise_type_error(list);
      int i = smallIntValue(h);
      if (i<0 || i>255)
	return raise_type_error(list);
	
      put_quote((char) i);
      ensure(0);

      TaggedRef t = tail(list);
      DEREF(t, t_ptr, t_tag);

      if (isVariableTag(t_tag)) {
	am.addSuspendVarList(t_ptr);
	return SUSPEND;
      }

      if (isLTupleTag(t_tag)) {
	list = t;
	continue;
      }

      if (isLiteralTag(t_tag) && oz_isNil(t)) 
	return PROCEED;

      return raise_type_error(list);
    }
  }

  OZ_Return put_string(TaggedRef list) {
    while (1) {
      TaggedRef h = head(list);
      DEREF(h, h_ptr, h_tag);

      if (isVariableTag(h_tag)) {
	am.addSuspendVarList(h_ptr);
	return SUSPEND;
      }
      if (!isSmallIntTag(h_tag))
	return raise_type_error(list);
      int i = smallIntValue(h);
      if (i<0 || i>255)
	return raise_type_error(list);
	
      put((char) i);

      TaggedRef t = tail(list);
      DEREF(t, t_ptr, t_tag);

      if (isVariableTag(t_tag)) {
	am.addSuspendVarList(t_ptr);
	return SUSPEND;
      }

      if (isLTupleTag(t_tag)) {
	list = t;
	continue;
      }

      if (isLiteralTag(t_tag) && oz_isNil(t)) 
	return PROCEED;

      return raise_type_error(list);
    }
  }

  OZ_Return put_feature(SRecord * sr, TaggedRef a) {
    if (oz_isSmallInt(a)) {
      return put_tcl(sr->getFeature(a));
    } if (oz_isAtom(a)) {
      put('-');
      put_atom(a);
      put(' ');
      return put_tcl(sr->getFeature(a));
    } else {
      return raise_type_error(makeTaggedSRecord(sr));
    }
  }
    
  OZ_Return put_tcl(TaggedRef tcl);
  OZ_Return put_tcl_filter(TaggedRef tcl, TaggedRef fs);
  OZ_Return put_tcl_return(TaggedRef tcl, TaggedRef * ret);
  OZ_Return put_vs(TaggedRef vs);
  OZ_Return put_vs_quote(TaggedRef vs);
  OZ_Return put_batch(TaggedRef batch, char delim);
  OZ_Return put_tuple(SRecord *st, int start = 0);
  OZ_Return put_record(SRecord * sr, TaggedRef as);
  OZ_Return put_record_or_tuple(TaggedRef tcl, int start); 

  OZ_Return close_hierarchy(Object * o);

};

#define TCL_SESSION_INC 8
static  int tcl_session_free  = 0;
static  int tcl_session_max   = 0;

static TclSession **tcl_sessions=0;

typedef TclSession *TclSessionPtr;

void tcl_sessions_init() {

  int new_tcl_session_max = tcl_session_max+TCL_SESSION_INC;
  TclSession **new_tcl_sessions = new TclSessionPtr[new_tcl_session_max];

  if (tcl_session_max>0) {
    for (int i=0; i<tcl_session_max; i++)
      new_tcl_sessions[i] = tcl_sessions[i];
    delete tcl_sessions;
  }

  for (int i=tcl_session_max; i<new_tcl_session_max; i++)
    new_tcl_sessions[i] = 0;

  tcl_sessions=new_tcl_sessions;

  tcl_session_max=new_tcl_session_max;
}



void gc_tcl_sessions() {
  for (int i=tcl_session_free; i--; ) {
    tcl_sessions[i]->gc();
  }
}

int get_next_tcl_session() {
  if (tcl_session_free < tcl_session_max) {
    Assert(!tcl_sessions[tcl_session_free]);
    return tcl_session_free++;
  }

  for (int i=0; i<tcl_session_max; i++) {
    if (!tcl_sessions[i]) return i;
  }

  tcl_sessions_init();
  Assert(!tcl_sessions[tcl_session_free]);
  return tcl_session_free++;
}

OZ_Return TclSession::write() {
redo:
  int ret = osTestSelect(tcl_fd, SEL_WRITE);
  
  if (ret < 0)  { 
    reset();
    LEAVE_TCL_LOCK(this);
    return raise_os_error();
  } else if (ret==0) {
    goto wait_select;
  }  
  
  while ((ret = oswrite(tcl_fd, write_start, buffer-write_start)) < 0) {
    if (errno != EINTR) { 
      reset();
      LEAVE_TCL_LOCK(this);
      return raise_os_error();
    }
  }
  
  if (buffer - write_start == ret) {
    reset();
    LEAVE_TCL_LOCK(this);
    return PROCEED;
  }
  
  write_start += ret;
wait_select:
  TaggedRef var = oz_newVariable();
  
  (void) am.select(tcl_fd, SEL_WRITE, NameUnit, var);
  DEREF(var, var_ptr, var_tag);
  if (isVariableTag(var_tag)) {
    am.addSuspendVarList(var_ptr);
    return SUSPEND;
  } else {
    goto redo;
  }
}


void TclSession::resize(void) {
  int new_size = (3 * (end - start)) / 2; 
  char *new_start = new char[new_size + SAFETY_MARGIN];
  
  end   = new_start + new_size;
  
  memcpy(new_start, start, buffer-start);
    
  dispose();

  buffer = (buffer - start) + new_start;
  start  = new_start;
}



OZ_Return TclSession::put_tuple(SRecord *st, int start) {
  if (start < st->getWidth()) {
    StateReturn(put_tcl(st->getArg(start)));

    for (int i=start+1; i < st->getWidth(); i++) {
      put(' ');
      StateReturn(put_tcl(st->getArg(i)));
    }
  }
  return PROCEED;
}

OZ_Return TclSession::put_record(SRecord * sr, TaggedRef as) {
  TaggedRef a = head(as);
  
  StateReturn(put_feature(sr,a));
  as = tail(as);
  
  while (oz_isCons(as)) {
    a = head(as);
    put(' ');
    StateReturn(put_feature(sr,a));
    as = tail(as);
  }
  return PROCEED;
}

OZ_Return TclSession::put_batch(TaggedRef batch, char delim) { 	

  DEREF(batch, batch_ptr, batch_tag);
  
  if (isVariableTag(batch_tag)) {
    am.addSuspendVarList(batch_ptr);
    return SUSPEND;
  } else if (isLTupleTag(batch_tag)) {
    OZ_Return batch_state = put_tcl(head(batch));
    
    if (batch_state!=PROCEED) 
      return batch_state;
    
    batch = tail(batch);
  } else if (isLiteralTag(batch_tag) && literalEq(batch,AtomNil)) {
    return PROCEED;
  } else {
    return raise_type_error(batch);
  }
  
  while (1) {
    DEREF(batch, batch_ptr, batch_tag);
    
    if (isVariableTag(batch_tag)) {
      am.addSuspendVarList(batch_ptr);
      return SUSPEND;
    } else if (isLTupleTag(batch_tag)) {
      put(delim);
      OZ_Return batch_state = put_tcl(head(batch));
      
      if (batch_state!=PROCEED) 
	return batch_state;
      
      batch = tail(batch);
    } else if (isLiteralTag(batch_tag) && literalEq(batch,AtomNil)) {
      return PROCEED;
    } else {
      return raise_type_error(batch);
    }
  }
}

OZ_Return TclSession::put_record_or_tuple(TaggedRef tcl, int start = 0) {
  SRecord * st = tagged2SRecord(oz_deref(tcl));

  if (st->isTuple()) {
    if (start < st->getWidth()) {
      StateReturn(put_tcl(st->getArg(start)));

      for (int i=start+1; i < st->getWidth(); i++) {
	put(' ');
	StateReturn(put_tcl(st->getArg(i)));
      }
    }
    return PROCEED;
  } else {
    TaggedRef as = st->getArityList();
    
    if (start==1 && oz_isCons(as)) {
      Assert(smallIntValue(head(as))==1);
      as=tail(as);
    }
    if (!oz_isCons(as))
      return PROCEED;

    StateReturn(put_feature(st,head(as)));
    
    as = tail(as);

    while (oz_isCons(as)) {
      TaggedRef a = head(as);
      put(' ');
      StateReturn(put_feature(st,a));
      as = tail(as);
    }
    return PROCEED;
  }
}

OZ_Return TclSession::put_vs(TaggedRef vs) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isVariableTag(vs_tag)) {
    am.addSuspendVarList(vs_ptr);
    return SUSPEND;
  } else if (isSmallIntTag(vs_tag) || oz_isBigInt(vs)) {
    put_int(vs);
    return PROCEED;
  } else if (isFloatTag(vs_tag)) {
    put_float(vs);
    return PROCEED;
  } else if (isLiteralTag(vs_tag)) {

    if (!tagged2Literal(vs)->isAtom())
      return raise_type_error(vs);

    put_atom(vs);
    return PROCEED;
  } else if (oz_isSTuple(vs)) {
    SRecord * sr = tagged2SRecord(vs);

    if (!literalEq(sr->getLabel(),AtomPair))
       return raise_type_error(vs);

    for (int i=0; i < sr->getWidth(); i++) {
      StateReturn(put_vs(sr->getArg(i)));
    }
    return PROCEED;
  } else if (isLTupleTag(vs_tag)) {
    return put_string(vs);
  } else {
    return raise_type_error(vs);
  }
}


OZ_Return TclSession::put_vs_quote(TaggedRef vs) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isVariableTag(vs_tag)) {
    am.addSuspendVarList(vs_ptr);
    return SUSPEND;
  } else if (isSmallIntTag(vs_tag) || oz_isBigInt(vs)) {
    put_int(vs);
    return PROCEED;
  } else if (isFloatTag(vs_tag)) {
    put_float(vs);
    return PROCEED;
  } else if (isLiteralTag(vs_tag)) {

    if (!tagged2Literal(vs)->isAtom())
      return raise_type_error(vs);

    put_atom_quote(vs);
    return PROCEED;
  } else if (oz_isSTuple(vs)) {
    SRecord * sr = tagged2SRecord(vs);

    if (!literalEq(sr->getLabel(),AtomPair))
      return raise_type_error(vs);

    for (int i=0; i < sr->getWidth(); i++) {
      StateReturn(put_vs_quote(sr->getArg(i)));
    }
    return PROCEED;
  } else if (isLTupleTag(vs_tag)) {
    return put_string_quote(vs);
  } else {
    return raise_type_error(vs);   
  }
}


OZ_Return TclSession::put_tcl(TaggedRef tcl) {
  DEREF(tcl, tcl_ptr, tcl_tag);
       
  if (isVariableTag(tcl_tag)) {
    am.addSuspendVarList(tcl_ptr);
    return SUSPEND;
  } else if (isSmallIntTag(tcl_tag) || oz_isBigInt(tcl)) {
    put_int(tcl);
    return PROCEED;
  } else if (isFloatTag(tcl_tag)) {
    put_float(tcl);
    return PROCEED;
  } else if (isLiteralTag(tcl_tag)) {
    if (tagged2Literal(tcl)->isAtom()) {
      start_protect();
      put_atom_quote(tcl);
      stop_protect();
      return PROCEED;
    } else if (literalEq(tcl,NameTrue)) {
      put('1');
      return PROCEED;
    } else if (literalEq(tcl,NameFalse)) {
      put('0');
      return PROCEED;
    } else if (literalEq(tcl,NameUnit)) {
      return PROCEED;
    } else {
      return raise_type_error(tcl);
    }
  } else if (oz_isObject(tcl)) {
    TaggedRef v = tagged2Object(tcl)->getFeature(NameTclName);
 
    if (v!=makeTaggedNULL()) {
      DEREF(v, v_ptr, v_tag);

      if (isVariableTag(v_tag)) {
	am.addSuspendVarList(v_ptr);
	return SUSPEND;
      } else if (isLiteralTag(v_tag) && literalEq(v,NameTclClosed)) {
	return raise_closed(tcl);
      } else {
	return put_vs(v);
      }
    } else {
      return raise_type_error(tcl);
    }

  } else if (oz_isSTuple(tcl)) {
    SRecord  * st = tagged2SRecord(tcl);
    TaggedRef l   = st->getLabel();
    
    if (oz_isAtom(l)) {
      if (literalEq(l,AtomCons)) { 
	return raise_type_error(tcl);	
      } else if (literalEq(l,AtomPair)) { 
	start_protect();
	StateReturn(put_vs_quote(tcl));
	stop_protect();
	return PROCEED;
      } else if (literalEq(l,AtomTclVS)) {
	TaggedRef arg = st->getArg(0);

	if (st->getWidth() != 1)
	  return raise_type_error(tcl);

	DEREF(arg, arg_ptr, arg_tag);

	if (isVariableTag(arg_tag)) {
	  am.addSuspendVarList(arg_ptr);
	  return SUSPEND;
	}

	return put_vs(arg);

      } else if (literalEq(l,AtomTclBatch)) {
	if (st->getWidth() != 1)
	  return raise_type_error(tcl);
	  
	return put_batch(st->getArg(0), ' ');
      } else if (literalEq(l,AtomTclColor)) {
	if (st->getWidth() != 3)
	  return raise_type_error(tcl);
	  
	put('#');
	for (int i=0; i < 3; i++) {
	  TaggedRef arg = st->getArg(i);
	  
	  DEREF(arg, arg_ptr, arg_tag);
	    
	  if (oz_isVariable(arg)) {
	    am.addSuspendVarList(arg_ptr);
	    return SUSPEND;
	  }
	    
	  if (!isSmallIntTag(arg_tag))
	    return raise_type_error(tcl);

	  int i = smallIntValue(arg);

	  if ((i < 0) || (i > 255))
	    return raise_type_error(tcl);

	  unsigned char c1 = hex_digit(((unsigned char) i & '\xF0') >> 4);
	  unsigned char c2 = hex_digit((unsigned char) i & '\x0F');
	  put2(c1,c2);
	}
	return PROCEED;
      } else if (literalEq(l,AtomTclOption)) {
	return put_tuple(st);
      } else if (literalEq(l,AtomTclList)) {
	put('['); 
	StateReturn(put_tuple(st)); 
	put(']');
	return PROCEED;
      } else if (literalEq(l,AtomTclQuote)) {
	put('{'); 
	StateReturn(put_tuple(st)); 
	put('}');
	return PROCEED;
      } else if (literalEq(l,AtomTclString)) {
	put('"'); 
	StateReturn(put_tuple(st)); 
	put('"');
	return PROCEED;
      } else if (literalEq(l,AtomTclPosition)) {
	put('{'); 
	StateReturn(put_tcl(st->getArg(0)));
	put('.'); 
	StateReturn(put_tuple(st, 1));
	put('}');
	return PROCEED;
      } else {
	put_atom(st->getLabel());
	put(' ');
	return put_tuple(st);
      }
    } else {
      return raise_type_error(tcl);
    }
  } else if (isSRecordTag(tcl_tag)) {
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef l  = sr->getLabel();
    TaggedRef as = sr->getArityList(); /* arity list is already deref'ed */
    
    if (tagged2Literal(l)->isAtom()) {

      if (literalEq(l,AtomPair) || literalEq(l,AtomCons) ||
	  literalEq(l,AtomTclVS) || literalEq(l,AtomTclBatch) ||
	  literalEq(l,AtomTclColor)) {
	return raise_type_error(tcl);
      } else if (literalEq(l,AtomTclOption)) {
	return put_record(sr, as);
      } else if (literalEq(l,AtomTclList)) {
	put('['); 
	StateReturn(put_record(sr, as)); 
	put(']');
	return PROCEED;
      } else if (literalEq(l,AtomTclQuote)) {
	put('{'); 
	StateReturn(put_record(sr, as)); 
	put('}');
	return PROCEED;
      } else if (literalEq(l,AtomTclString)) {
	put('"'); 
	StateReturn(put_record(sr, as)); 
	put('"');
	return PROCEED;
      } else if (literalEq(l,AtomTclPosition)) {
	put('{'); 
	StateReturn(put_feature(sr, head(as)));
	put('.');
	if (sr->getWidth() > 1)
	  StateReturn(put_record(sr, tail(as)));
	put('}');
	return PROCEED;
      } else {
	start_protect();
	put_atom(l);
	stop_protect();
	put(' ');
	return put_record(sr, as);
      } 
    } else {
      return raise_type_error(tcl);     
    }

  } else if (isLTupleTag(tcl_tag)) {
    start_protect();
    StateReturn(put_string_quote(tcl));
    stop_protect();
    return PROCEED;
  }
  
  return raise_type_error(tcl);

}


OZ_Return TclSession::put_tcl_filter(TaggedRef tcl, TaggedRef fs) {
  DEREF(tcl, tcl_ptr, tcl_tag);

  if (isLiteralTag(tcl_tag)) {
    return PROCEED;
  } else if (isSRecordTag(tcl_tag)) {
    OZ_Return s = PROCEED;
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef as = sr->getArityList(); /* arity list is already deref'ed */
    fs = oz_deref(fs);

    while (oz_isCons(as) && oz_isCons(fs)) {
      TaggedRef a = head(as);
      TaggedRef f = oz_deref(head(fs));

      if (oz_isName(a))
	return raise_type_error(tcl);	

      switch (featureCmp(a,f)) {
      case 0:
	fs = oz_deref(tail(fs));
	as = tail(as);
	break;
      case 1:
	fs = oz_deref(tail(fs));
	break;
      case -1:
	StateReturn(put_feature(sr,a));
	put(' ');
	as = tail(as);
	break;
      }

    }

    if (oz_isCons(as)) {
      return put_record(sr,as);
    } else {
      return PROCEED;
    }
  } else {
    return raise_type_error(tcl);    
  }
}


OZ_Return TclSession::put_tcl_return(TaggedRef tcl, TaggedRef * ret) {
  *ret = makeTaggedNULL();
  DEREF(tcl, tcl_ptr, tcl_tag);

  if (oz_isSTuple(tcl)) {
    SRecord * sr = tagged2SRecord(tcl);
    int w = sr->getWidth();

    if (w == 1)
      return raise_type_error(tcl);
      
    for (int i=1; i < w-1; i++) {
      put(' ');
      StateReturn(put_tcl(sr->getArg(i)));
    }

    *ret = sr->getArg(w-1);
    return PROCEED;

  } else if (isSRecordTag(tcl_tag)) {
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef as = tail(sr->getArityList()); /* arity list is already deref'ed */

    while (oz_isCons(as)) {
      TaggedRef a1  = head(as);
      TaggedRef ar  = tail(as);

      if (oz_isSmallInt(a1)) {
	if (oz_isCons(ar)) {
	  TaggedRef a2 = head(ar);

	  if (oz_isSmallInt(a2)) {
	    put(' ');
	    StateReturn(put_tcl(sr->getFeature(a1)));
	  } else {
	    *ret = sr->getFeature(a1);
	  }
	  
	} else {
	  *ret = sr->getFeature(a1);
	  return PROCEED;
	}

      } else if (oz_isAtom(a1)) {
	put2(' ','-');
	put_atom(a1);
	put(' ');
	StateReturn(put_tcl(sr->getFeature(a1)));
      } else {
	return raise_type_error(tcl);
      }

      as = ar;
    }
    
    Assert(*ret);
    return PROCEED;
  } else {
    return raise_type_error(tcl);
  }    

}


#define GET_TCL_SESSION						\
TclSession *ts;							\
{								\
  OZ_Term sid=oz_deref(OZ_args[0]);				\
  if (!oz_isVariable(oz_deref(tail(sid)))) {				\
    return oz_raise(E_SYSTEM,E_TK,"sessionAlreadyClosed",0);	\
  }								\
  ts = tcl_sessions[smallIntValue(head(sid))];			\
}


#define NEW_GET_TCL_SESSION					\
TclSession *ts;							\
{								\
  OZ_Term sid=oz_deref(OZ_in(0));				\
  if (!oz_isVariable(oz_deref(tail(sid)))) {				\
    return oz_raise(E_SYSTEM,E_TK,"sessionAlreadyClosed",0);	\
  }								\
  ts = tcl_sessions[smallIntValue(head(sid))];			\
}


OZ_BI_define(BIinitTclSession, 3,1) {
  int session_no = get_next_tcl_session();
  int fd = smallIntValue(oz_deref(OZ_in(0)));
  tcl_sessions[session_no] = new TclSession(fd, oz_deref(OZ_in(1)), OZ_in(2));
  OZ_Term sid = cons(newSmallInt(session_no),oz_newVariable());
  OZ_RETURN(sid);
} OZ_BI_end

OZ_BI_define(BIcloseTclSession, 1,0) {
  OZ_Term sid=oz_deref(OZ_in(0));

  if (oz_isVariable(oz_deref(tail(sid)))) {
    TclSession ** ts = &tcl_sessions[smallIntValue(head(sid))];
    delete *ts;
    *ts = (TclSession *) NULL;
    oz_unify(tail(sid),NameUnit); // mm_u
  }
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BItclWrite, 2,0) {
  NEW_GET_TCL_SESSION;
  if (OZ_in(1) == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts);
    OZ_Return s;

    ts->reset();
    StateExit(ts->put_tcl(OZ_in(1)));
    ts->put('\n');

    ts->start_write();
    OZ_in(1) = NameTclClosed;
    return ts->write();
  
  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }
} OZ_BI_end


OZ_BI_define(BItclWriteReturn, 4,0) {  
  NEW_GET_TCL_SESSION;
  if (OZ_in(1) == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts);
    OZ_Return s;

    ts->reset();
    ts->put2('o', 'z');
    ts->put2('r', ' ');
    ts->put('[');
    StateExit(ts->put_tcl(OZ_in(1)));
    ts->put2(']','\n');

    ts->enterReturn(OZ_in(3), OZ_in(2));
        
    ts->start_write();
    OZ_in(0) = NameTclClosed;
    return ts->write();
  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }
} OZ_BI_end


OZ_BI_define(BItclWriteReturnMess, 5,0) {  
  NEW_GET_TCL_SESSION;
  /*
   * OZ_in(0): tickle object and modifier (for tags and marks)
   * OZ_in(1): return message
   * OZ_in(2): modifier to be put after arg 1 of above (may be unit)
   * OZ_in(3): type cast for return value
   */
  if (OZ_in(1) == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts);
    OZ_Return s;
    TaggedRef ret = NameTclClosed;
    TaggedRef mess = oz_deref(OZ_in(2));
    TaggedRef frst;

    Assert(!oz_isVariable(mess));

    if (!oz_isSRecord(mess)) {
      s = raise_type_error(mess);
      goto exit;
    }

    frst = tagged2SRecord(mess)->getFeature(newSmallInt(1));

    if (!frst) {
      s = raise_type_error(mess);;
      goto exit;
    }
      
    ts->reset();
    ts->put2('o', 'z');
    ts->put2('r', ' ');
    ts->put('[');
    StateExit(ts->put_tcl(OZ_in(1)));
    ts->put(' ');
    StateExit(ts->put_tcl(frst));
    ts->put(' ');
    StateExit(ts->put_tcl(OZ_in(3)));
    ts->put(' ');

    StateExit(ts->put_tcl_return(mess, &ret));
    ts->put2(']','\n');

    // Enter return variable and cast
    ts->enterReturn(ret, OZ_in(4));
    
    ts->start_write();
    OZ_in(0) = NameTclClosed;
    return ts->write(); 
  
  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }
    
} OZ_BI_end


OZ_BI_define(BItclWriteBatch,2,0) {  
  NEW_GET_TCL_SESSION;
  if (OZ_in(1) == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts)
    OZ_Return s;

    ts->reset();
    StateExit(ts->put_batch(oz_deref(OZ_in(1)),';'));
    ts->put('\n');

    ts->start_write();
    OZ_in(1) = NameTclClosed;
    return ts->write();
  
  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }
} OZ_BI_end


OZ_BI_define(BItclWriteTuple,3,0) {  
  NEW_GET_TCL_SESSION;
  if (OZ_in(1) == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts)
    OZ_Return s;

    TaggedRef mess = oz_deref(OZ_in(2));
    TaggedRef frst;

    if (!oz_isSRecord(mess)) {
      s = raise_type_error(mess);;
      goto exit;
    }

    Assert(!oz_isVariable(mess));

    frst = tagged2SRecord(mess)->getFeature(newSmallInt(1));

    if (!frst) {
      s = raise_type_error(mess);;
      goto exit;
    }
      
    ts->reset();
    StateExit(ts->put_tcl(OZ_in(1)));
    ts->put(' ');
    StateExit(ts->put_record_or_tuple(OZ_in(2)));
    ts->put('\n');

    ts->start_write();
    OZ_in(1) = NameTclClosed;
    return ts->write();
  
  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }

} OZ_BI_end



OZ_BI_define(BItclWriteTagTuple,4,0) {  
  NEW_GET_TCL_SESSION;
  if (OZ_in(1) == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts)
    OZ_Return s;
    TaggedRef tuple = oz_deref(OZ_in(3));
    TaggedRef fst;

    Assert(!oz_isVariable(tuple));

    if (!oz_isSRecord(tuple)) {
      s = raise_type_error(tuple);
      goto exit;
    }

    fst = tagged2SRecord(tuple)->getFeature(newSmallInt(1));
    
    if (!fst) {
      s = raise_type_error(tuple);
      goto exit;
    }

    ts->reset();
    StateExit(ts->put_tcl(OZ_in(1)));
    ts->put(' ');
    StateExit(ts->put_tcl(fst));
    ts->put(' ');
    StateExit(ts->put_tcl(OZ_in(2)));
    ts->put(' ');
    StateExit(ts->put_record_or_tuple(tuple,1));
    ts->put('\n');

    ts->start_write();
    OZ_in(1) = NameTclClosed;
    return ts->write();
    
  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }
} OZ_BI_end

OZ_BI_define(BItclWriteFilter,6,0) {  
  NEW_GET_TCL_SESSION;
  if (OZ_in(1) == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts)
    OZ_Return s;

    ts->reset();
    StateExit(ts->put_tcl(OZ_in(1)));
    ts->put(' ');
    StateExit(ts->put_vs(OZ_in(2)));
    ts->put(' ');
    StateExit(ts->put_tcl_filter(OZ_in(3), oz_deref(OZ_in(4))));
    ts->put(' ');
    StateExit(ts->put_tcl(OZ_in(5)));
    ts->put('\n');

    ts->start_write();
    OZ_in(1) = NameTclClosed;
    return ts->write();
  
  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }
} OZ_BI_end


OZ_Return TclSession::close_hierarchy(Object * o) {
  TaggedRef v = o->replaceFeature(NameTclName, NameTclClosed);
    
  if (v == makeTaggedNULL()) {
    return raise_type_error(makeTaggedConst(o));;
  }

  DEREF(v, v_ptr, v_tag);

  Assert(!isVariableTag(v_tag)); 
  // since the message has been assembled for closing already!

  if (isLiteralTag(v_tag) && literalEq(v,NameTclClosed)) {
    // okay, has been closed already
    return PROCEED;
  } else {
    TaggedRef slaves      = o->getFeature(NameTclSlaves);

    // close slaves
    if (slaves != makeTaggedNULL()) {
      slaves = oz_deref(slaves);
      
      while (oz_isCons(slaves)) {
	TaggedRef slave = oz_deref(head(slaves));
	  
	if (oz_isSmallInt(slave)) {
	  // this an entry in the event dictionary

	  Assert(oz_isDictionary(tcl_dict));
	  tagged2Dictionary(tcl_dict)->remove(slave);
	  
	} else if (oz_isObject(slave)) {
	  // this is an object which needs to be closed as well
	  OZ_Return s = close_hierarchy(tagged2Object(slave));
	  if (s != PROCEED)
	    return s;
	}

	slaves = oz_deref(tail(slaves));
      }
    }

    return PROCEED;
  }

}

  
OZ_BI_define(BItclClose,3,0) {  
  NEW_GET_TCL_SESSION;
  if (OZ_in(1) == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts)
    OZ_Return s;

    // Perform closing of objects
    TaggedRef to = OZ_in(2);
    DEREF(to, to_ptr, to_tag);
      
    Assert(oz_isObject(to));

    Object  * o = tagged2Object(to);
    TaggedRef v = o->getFeature(NameTclName);
    TaggedRef slave_entry;    
	
    if (v == makeTaggedNULL()) {
      s = raise_type_error(to);;
      goto exit;
    }
	
    {
      DEREF(v, v_ptr, v_tag);
    
      if (isVariableTag(v_tag)) {
	am.addSuspendVarList(v_ptr);
	s = SUSPEND;
	goto exit;
      } else if (isLiteralTag(v_tag) && literalEq(v,NameTclClosed)) {
	LEAVE_TCL_LOCK(ts);
	return PROCEED;
      }
    }
	
    // Create close tcl
    ts->reset();
    StateExit(ts->put_tcl(OZ_in(1)));
    ts->put('\n');


    // okay, let us close it
    slave_entry = o->getFeature(NameTclSlaveEntry);

    // remove from parent
    if (slave_entry != makeTaggedNULL()) {
      slave_entry = oz_deref(slave_entry);
      
      if (oz_isCons(slave_entry)) {
	LTuple * l = tagged2LTuple(slave_entry);
	l->setHead(NameGroupVoid);
	l->setTail(findAliveEntry(tail(slave_entry)));
      }
    }
	  
    ts->close_hierarchy(o);

    ts->start_write();
    OZ_in(1) = NameTclClosed;
    return ts->write();
    
  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }
} OZ_BI_end


OZ_BI_define(BItclCloseWeb,2,0) {  
  NEW_GET_TCL_SESSION;
  CHECK_TOPLEVEL;

  // Perform closing of objects
  TaggedRef to = OZ_in(1);
  DEREF(to, to_ptr, to_tag);
      
  Assert(oz_isObject(to));

  Object  * o = tagged2Object(to);
  TaggedRef v = o->getFeature(NameTclName);
  TaggedRef slave_entry;    
	
  if (v == makeTaggedNULL()) {
    return raise_type_error(to);
  }
	
  {
    DEREF(v, v_ptr, v_tag);
    
    if (isVariableTag(v_tag)) {
      am.addSuspendVarList(v_ptr);
      return SUSPEND;
    } else if (isLiteralTag(v_tag) && literalEq(v,NameTclClosed)) {
      return PROCEED;
    }
	
    ts->close_hierarchy(o);

    OZ_in(1) = NameTclClosed;

    return PROCEED;
  }
} OZ_BI_end


// ---------------------------------------------------------------------
// Counters
// ---------------------------------------------------------------------

OZ_BI_define(BIgenTopName,1,1) {
  NEW_GET_TCL_SESSION;
  OZ_RETURN(ts->genTopName());
} OZ_BI_end

OZ_BI_define(BIgenWidgetName,2,1) {
  NEW_GET_TCL_SESSION;
  TaggedRef parent = OZ_in(1);

  DEREF(parent, p_ptr, p_tag);

  if (isVariableTag(p_tag))
    OZ_suspendOn(makeTaggedRef(p_ptr));

  OZ_RETURN(ts->genWidgetName(parent));
} OZ_BI_end

OZ_BI_define(BIgenTagName,1,1) {
  NEW_GET_TCL_SESSION;
  OZ_RETURN(ts->genTagName());
} OZ_BI_end

OZ_BI_define(BIgenVarName,1,1) {
  NEW_GET_TCL_SESSION;
  OZ_RETURN(ts->genVarName());
} OZ_BI_end

OZ_BI_define(BIgenImageName,1,1) {
  NEW_GET_TCL_SESSION;
  OZ_RETURN(ts->genImageName());
} OZ_BI_end

void BIinitTclTk() {

  AtomTclOption    = OZ_atom("o");
  AtomTclList      = OZ_atom("l");
  AtomTclPosition  = OZ_atom("p");
  AtomTclQuote     = OZ_atom("q");
  AtomTclString    = OZ_atom("s");
  AtomTclVS        = OZ_atom("v");
  AtomTclBatch     = OZ_atom("b");
  AtomTclColor     = OZ_atom("c");
  AtomError        = OZ_atom("error");
  AtomDot          = OZ_atom(".");
  AtomTagPrefix    = OZ_atom("t");
  AtomVarPrefix    = OZ_atom("v");
  AtomImagePrefix  = OZ_atom("i");
  
  NameTclName       = OZ_newName(); OZ_protect(&NameTclName);
  NameTclSlaves     = OZ_newName(); OZ_protect(&NameTclSlaves);
  NameTclSlaveEntry = OZ_newName(); OZ_protect(&NameTclSlaveEntry);
  NameTclClosed     = OZ_newName(); OZ_protect(&NameTclClosed);

  tcl_sessions_init();
}
