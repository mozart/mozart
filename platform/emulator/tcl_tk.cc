/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: schulte

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


TaggedRef NameTclName,
  AtomTclOption, AtomTclList, AtomTclPosition,
  AtomTclQuote, AtomTclString, AtomTclVS,
  AtomTclBatch, AtomTclColor,
  AtomError,
  AtomDot, AtomTagPrefix, AtomVarPrefix, AtomImagePrefix,
  NameGroupVoid,
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
if (!am.isToplevel())      \
  return raise_toplevel();


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
  return OZ_typeError(0,"List");
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

    if (isNil(group)) return OZ_unify(out,OZ_getCArg(1));
  }

  return OZ_typeError(0,"List");
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


OZ_C_proc_begin(BIgetTclNames,3) {
  (void) OZ_unify(OZ_getCArg(0), NameTclSlaves);
  (void) OZ_unify(OZ_getCArg(1), NameTclSlaveEntry);
  (void) OZ_unify(OZ_getCArg(2), NameTclName);
  return PROCEED;
} OZ_C_proc_end

/*
 * Locking
 */

#define ENTER_TCL_LOCK(TS) { \
  TaggedRef t = (TS)->getLock();                                  \
  DEREF(t, t_ptr, t_tag);                                         \
  if (isAnyVar(t_tag)) {                                          \
    am.addSuspendVarList(t_ptr);                                  \
    return SUSPEND;                                               \
  } else {                                                        \
    (TS)->setLock(makeTaggedRef(newTaggedUVar(am.currentBoard))); \
  }                                                               \
}

#define LEAVE_TCL_LOCK(TS) \
  (void) am.fastUnify((TS)->getLock(), NameUnit, OK);


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

  int top_ctr;
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
                             makeTaggedRef(newTaggedUVar(am.currentBoard)));

    (void) OZ_unify(newt,tcl_rets);
    tcl_rets = tail(newt);
  }

  TaggedRef genTopName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,AtomDot);
    s->setArg(1,makeInt(top_ctr++));
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
    top_ctr    = 0;
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
      gcTagged(tcl_lock, tcl_lock);
      gcTagged(tcl_dict, tcl_dict);
      gcTagged(tcl_rets, tcl_rets);
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
    if (isSmallInt(i)) {
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
    Assert(isAtom(atom));
    Atom *l = (Atom*) tagged2Literal(atom);
    int   n = l->getSize();
    char *s = l->getPrintName();

    ensure(n);
    for (int i = 0; i < n; i++) {
      *buffer++ = *s++;
    }
  }

  void put_atom_quote(TaggedRef atom) {
    if (literalEq(atom, AtomPair) || literalEq(atom, AtomNil))
      return;

    Assert(isAtom(atom));
    Atom* l = (Atom*)tagged2Literal(atom);
    int   n = l->getSize();
    char *s = l->getPrintName();
    char c;
    ensure(4*n);

    while ((c = *s++))
      put_quote(c);

  }

  OZ_Return put_string_quote(TaggedRef list) {
    while (1) {
      TaggedRef h = head(list);
      DEREF(h, h_ptr, h_tag);

      if (isAnyVar(h_tag)) {
        am.addSuspendVarList(h_ptr);
        return SUSPEND;
      }
      if (!isSmallInt(h_tag))
        return raise_type_error(list);
      int i = smallIntValue(h);
      if (i<0 || i>255)
        return raise_type_error(list);

      put_quote((char) i);
      ensure(0);

      TaggedRef t = tail(list);
      DEREF(t, t_ptr, t_tag);

      if (isAnyVar(t_tag)) {
        am.addSuspendVarList(t_ptr);
        return SUSPEND;
      }

      if (isLTuple(t_tag)) {
        list = t;
        continue;
      }

      if (isLiteral(t_tag) && isNil(t))
        return PROCEED;

      return raise_type_error(list);
    }
  }

  OZ_Return put_string(TaggedRef list) {
    while (1) {
      TaggedRef h = head(list);
      DEREF(h, h_ptr, h_tag);

      if (isAnyVar(h_tag)) {
        am.addSuspendVarList(h_ptr);
        return SUSPEND;
      }
      if (!isSmallInt(h_tag))
        return raise_type_error(list);
      int i = smallIntValue(h);
      if (i<0 || i>255)
        return raise_type_error(list);

      put((char) i);

      TaggedRef t = tail(list);
      DEREF(t, t_ptr, t_tag);

      if (isAnyVar(t_tag)) {
        am.addSuspendVarList(t_ptr);
        return SUSPEND;
      }

      if (isLTuple(t_tag)) {
        list = t;
        continue;
      }

      if (isLiteral(t_tag) && isNil(t))
        return PROCEED;

      return raise_type_error(list);
    }
  }

  OZ_Return put_feature(SRecord * sr, TaggedRef a) {
    if (isSmallInt(a)) {
      return put_tcl(sr->getFeature(a));
    } if (isLiteral(a) && tagged2Literal(a)->isAtom()) {
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
  TaggedRef var = makeTaggedRef(newTaggedUVar(am.currentBoard));

  (void) am.select(tcl_fd, SEL_WRITE, NameUnit, var);
  DEREF(var, var_ptr, var_tag);
  if (isAnyVar(var_tag)) {
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

  while (isCons(as)) {
    a = head(as);
    put(' ');
    StateReturn(put_feature(sr,a));
    as = tail(as);
  }
  return PROCEED;
}

OZ_Return TclSession::put_batch(TaggedRef batch, char delim) {

  DEREF(batch, batch_ptr, batch_tag);

  if (isAnyVar(batch_tag)) {
    am.addSuspendVarList(batch_ptr);
    return SUSPEND;
  } else if (isCons(batch_tag)) {
    OZ_Return batch_state = put_tcl(head(batch));

    if (batch_state!=PROCEED)
      return batch_state;

    batch = tail(batch);
  } else if (isLiteral(batch_tag) && literalEq(batch,AtomNil)) {
    return PROCEED;
  } else {
    return raise_type_error(batch);
  }

  while (1) {
    DEREF(batch, batch_ptr, batch_tag);

    if (isAnyVar(batch_tag)) {
      am.addSuspendVarList(batch_ptr);
      return SUSPEND;
    } else if (isCons(batch_tag)) {
      put(delim);
      OZ_Return batch_state = put_tcl(head(batch));

      if (batch_state!=PROCEED)
        return batch_state;

      batch = tail(batch);
    } else if (isLiteral(batch_tag) && literalEq(batch,AtomNil)) {
      return PROCEED;
    } else {
      return raise_type_error(batch);
    }
  }
}

OZ_Return TclSession::put_record_or_tuple(TaggedRef tcl, int start = 0) {
  SRecord * st = tagged2SRecord(deref(tcl));

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

    if (start==1 && isCons(as)) {
      Assert(smallIntValue(head(as))==1);
      as=tail(as);
    }
    if (!isCons(as))
      return PROCEED;

    StateReturn(put_feature(st,head(as)));

    as = tail(as);

    while (isCons(as)) {
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

  if (isAnyVar(vs_tag)) {
    am.addSuspendVarList(vs_ptr);
    return SUSPEND;
  } else if (isInt(vs_tag)) {
    put_int(vs);
    return PROCEED;
  } else if (isFloat(vs_tag)) {
    put_float(vs);
    return PROCEED;
  } else if (isLiteral(vs_tag)) {

    if (!tagged2Literal(vs)->isAtom())
      return raise_type_error(vs);

    put_atom(vs);
    return PROCEED;
  } else if (isSTuple(vs)) {
    SRecord * sr = tagged2SRecord(vs);

    if (!literalEq(sr->getLabel(),AtomPair))
       return raise_type_error(vs);

    for (int i=0; i < sr->getWidth(); i++) {
      StateReturn(put_vs(sr->getArg(i)));
    }
    return PROCEED;
  } else if (isCons(vs_tag)) {
    return put_string(vs);
  } else {
    return raise_type_error(vs);
  }
}


OZ_Return TclSession::put_vs_quote(TaggedRef vs) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isAnyVar(vs_tag)) {
    am.addSuspendVarList(vs_ptr);
    return SUSPEND;
  } else if (isInt(vs_tag)) {
    put_int(vs);
    return PROCEED;
  } else if (isFloat(vs_tag)) {
    put_float(vs);
    return PROCEED;
  } else if (isLiteral(vs_tag)) {

    if (!tagged2Literal(vs)->isAtom())
      return raise_type_error(vs);

    put_atom_quote(vs);
    return PROCEED;
  } else if (isSTuple(vs)) {
    SRecord * sr = tagged2SRecord(vs);

    if (!literalEq(sr->getLabel(),AtomPair))
      return raise_type_error(vs);

    for (int i=0; i < sr->getWidth(); i++) {
      StateReturn(put_vs_quote(sr->getArg(i)));
    }
    return PROCEED;
  } else if (isCons(vs_tag)) {
    return put_string_quote(vs);
  } else {
    return raise_type_error(vs);
  }
}


OZ_Return TclSession::put_tcl(TaggedRef tcl) {
  DEREF(tcl, tcl_ptr, tcl_tag);

  if (isAnyVar(tcl_tag)) {
    am.addSuspendVarList(tcl_ptr);
    return SUSPEND;
  } else if (isInt(tcl_tag)) {
    put_int(tcl);
    return PROCEED;
  } else if (isFloat(tcl_tag)) {
    put_float(tcl);
    return PROCEED;
  } else if (isLiteral(tcl_tag)) {
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
  } else if (isObject(tcl)) {
    TaggedRef v = tagged2Object(tcl)->getFeature(NameTclName);

    if (v!=makeTaggedNULL()) {
      DEREF(v, v_ptr, v_tag);

      if (isAnyVar(v_tag)) {
        am.addSuspendVarList(v_ptr);
        return SUSPEND;
      } else if (isLiteral(v_tag) && literalEq(v,NameTclClosed)) {
        return raise_closed(tcl);
      } else {
        return put_vs(v);
      }
    } else {
      return raise_type_error(tcl);
    }

  } else if (isSTuple(tcl)) {
    SRecord  * st = tagged2SRecord(tcl);
    TaggedRef l   = st->getLabel();

    if (isAtom(l)) {
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

        if (isAnyVar(arg_tag)) {
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

          if (isAnyVar(arg)) {
            am.addSuspendVarList(arg_ptr);
            return SUSPEND;
          }

          if (!isSmallInt(arg_tag))
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
  } else if (isSRecord(tcl_tag)) {
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

  } else if (isCons(tcl_tag)) {
    start_protect();
    StateReturn(put_string_quote(tcl));
    stop_protect();
    return PROCEED;
  }

  return raise_type_error(tcl);

}


OZ_Return TclSession::put_tcl_filter(TaggedRef tcl, TaggedRef fs) {
  DEREF(tcl, tcl_ptr, tcl_tag);

  if (isLiteral(tcl_tag)) {
    return PROCEED;
  } else if (isSRecord(tcl_tag)) {
    OZ_Return s = PROCEED;
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef as = sr->getArityList(); /* arity list is already deref'ed */
    fs = deref(fs);

    while (isCons(as) && isCons(fs)) {
      TaggedRef a = head(as);
      TaggedRef f = deref(head(fs));

      if (isLiteral(a) && !tagged2Literal(a)->isAtom())
        return raise_type_error(tcl);

      switch (featureCmp(a,f)) {
      case 0:
        fs = deref(tail(fs));
        as = tail(as);
        break;
      case 1:
        fs = deref(tail(fs));
        break;
      case -1:
        StateReturn(put_feature(sr,a));
        put(' ');
        as = tail(as);
        break;
      }

    }

    if (isCons(as)) {
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

  if (isSTuple(tcl)) {
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

  } else if (isSRecord(tcl_tag)) {
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef as = tail(sr->getArityList()); /* arity list is already deref'ed */

    while (isCons(as)) {
      TaggedRef a1  = head(as);
      TaggedRef ar  = tail(as);

      if (isSmallInt(a1)) {
        if (isCons(ar)) {
          TaggedRef a2 = head(ar);

          if (isSmallInt(a2)) {
            put(' ');
            StateReturn(put_tcl(sr->getFeature(a1)));
          } else {
            *ret = sr->getFeature(a1);
          }

        } else {
          *ret = sr->getFeature(a1);
          return PROCEED;
        }

      } else if (isLiteral(a1) && tagged2Literal(a1)->isAtom()) {
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


#define GET_TCL_SESSION                                         \
TclSession *ts;                                                 \
{                                                               \
  OZ_Term sid=deref(OZ_args[0]);                                \
  if (!isAnyVar(deref(tail(sid)))) {                            \
    return oz_raise(E_SYSTEM,E_TK,"sessionAlreadyClosed",0);    \
  }                                                             \
  ts = tcl_sessions[smallIntValue(head(sid))];                  \
}


OZ_C_proc_begin(BIinitTclSession, 4) {
  int session_no = get_next_tcl_session();
  int fd = smallIntValue(deref(OZ_args[0]));
  tcl_sessions[session_no] = new TclSession(fd, deref(OZ_args[1]), OZ_args[2]);
  OZ_Term sid = cons(newSmallInt(session_no),oz_newVariable());
  return OZ_unify(OZ_args[3],sid);
} OZ_C_proc_end

OZ_C_proc_begin(BIcloseTclSession, 1) {
  OZ_Term sid=deref(OZ_args[0]);

  if (isAnyVar(deref(tail(sid)))) {
    TclSession ** ts = &tcl_sessions[smallIntValue(head(sid))];
    delete *ts;
    *ts = (TclSession *) NULL;
    OZ_unify(tail(sid),NameUnit);
  }
  return PROCEED;
} OZ_C_proc_end


OZ_C_proc_begin(BItclWrite, 2) {
  GET_TCL_SESSION;
  if (OZ_args[1] == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts);
    OZ_Return s;

    ts->reset();
    StateExit(ts->put_tcl(OZ_args[1]));
    ts->put('\n');

    ts->start_write();
    OZ_args[1] = NameTclClosed;
    return ts->write();

  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteReturn, 4) {
  GET_TCL_SESSION;
  if (OZ_args[1] == NameTclClosed) {
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
    StateExit(ts->put_tcl(OZ_args[1]));
    ts->put2(']','\n');

    ts->enterReturn(OZ_args[3], OZ_args[2]);

    ts->start_write();
    OZ_args[0] = NameTclClosed;
    return ts->write();
  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteReturnMess, 5) {
  GET_TCL_SESSION;
  /*
   * OZ_args[0]: tickle object and modifier (for tags and marks)
   * OZ_args[1]: return message
   * OZ_args[2]: modifier to be put after arg 1 of above (may be unit)
   * OZ_args[3]: type cast for return value
   */
  if (OZ_args[1] == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts);
    OZ_Return s;
    TaggedRef ret = NameTclClosed;
    TaggedRef mess = deref(OZ_args[2]);
    TaggedRef frst;

    Assert(!isAnyVar(mess));

    if (!isSRecord(mess)) {
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
    StateExit(ts->put_tcl(OZ_args[1]));
    ts->put(' ');
    StateExit(ts->put_tcl(frst));
    ts->put(' ');
    StateExit(ts->put_tcl(OZ_args[3]));
    ts->put(' ');

    StateExit(ts->put_tcl_return(mess, &ret));
    ts->put2(']','\n');

    // Enter return variable and cast
    ts->enterReturn(ret, OZ_args[4]);

    ts->start_write();
    OZ_args[0] = NameTclClosed;
    return ts->write();

  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }

}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteBatch,2) {
  GET_TCL_SESSION;
  if (OZ_args[1] == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts)
    OZ_Return s;

    ts->reset();
    StateExit(ts->put_batch(deref(OZ_args[1]),';'));
    ts->put('\n');

    ts->start_write();
    OZ_args[1] = NameTclClosed;
    return ts->write();

  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteTuple,3) {
  GET_TCL_SESSION;
  if (OZ_args[1] == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts)
    OZ_Return s;

    TaggedRef mess = deref(OZ_args[2]);
    TaggedRef frst;

    if (!isSRecord(mess)) {
      s = raise_type_error(mess);;
      goto exit;
    }

    Assert(!isAnyVar(mess));

    frst = tagged2SRecord(mess)->getFeature(newSmallInt(1));

    if (!frst) {
      s = raise_type_error(mess);;
      goto exit;
    }

    ts->reset();
    StateExit(ts->put_tcl(OZ_args[1]));
    ts->put(' ');
    StateExit(ts->put_record_or_tuple(OZ_args[2]));
    ts->put('\n');

    ts->start_write();
    OZ_args[1] = NameTclClosed;
    return ts->write();

  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }

}
OZ_C_proc_end



OZ_C_proc_begin(BItclWriteTagTuple,4) {
  GET_TCL_SESSION;
  if (OZ_args[1] == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts)
    OZ_Return s;
    TaggedRef tuple = deref(OZ_args[3]);
    TaggedRef fst;

    Assert(!isAnyVar(tuple));

    if (!isSRecord(tuple)) {
      s = raise_type_error(tuple);
      goto exit;
    }

    fst = tagged2SRecord(tuple)->getFeature(newSmallInt(1));

    if (!fst) {
      s = raise_type_error(tuple);
      goto exit;
    }

    ts->reset();
    StateExit(ts->put_tcl(OZ_args[1]));
    ts->put(' ');
    StateExit(ts->put_tcl(fst));
    ts->put(' ');
    StateExit(ts->put_tcl(OZ_args[2]));
    ts->put(' ');
    StateExit(ts->put_record_or_tuple(tuple,1));
    ts->put('\n');

    ts->start_write();
    OZ_args[1] = NameTclClosed;
    return ts->write();

  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }
}
OZ_C_proc_end

OZ_C_proc_begin(BItclWriteFilter,6) {
  GET_TCL_SESSION;
  if (OZ_args[1] == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts)
    OZ_Return s;

    ts->reset();
    StateExit(ts->put_tcl(OZ_args[1]));
    ts->put(' ');
    StateExit(ts->put_vs(OZ_args[2]));
    ts->put(' ');
    StateExit(ts->put_tcl_filter(OZ_args[3], deref(OZ_args[4])));
    ts->put(' ');
    StateExit(ts->put_tcl(OZ_args[5]));
    ts->put('\n');

    ts->start_write();
    OZ_args[1] = NameTclClosed;
    return ts->write();

  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }
}
OZ_C_proc_end


OZ_Return TclSession::close_hierarchy(Object * o) {
  TaggedRef v = o->replaceFeature(NameTclName, NameTclClosed);

  if (v == makeTaggedNULL()) {
    return raise_type_error(makeTaggedConst(o));;
  }

  DEREF(v, v_ptr, v_tag);

  Assert(!isAnyVar(v_tag));
  // since the message has been assembled for closing already!

  if (isLiteral(v_tag) && literalEq(v,NameTclClosed)) {
    // okay, has been closed already
    return PROCEED;
  } else {
    TaggedRef slaves      = o->getFeature(NameTclSlaves);

    // close slaves
    if (slaves != makeTaggedNULL()) {
      slaves = deref(slaves);

      while (isCons(slaves)) {
        TaggedRef slave = deref(head(slaves));

        if (isSmallInt(slave)) {
          // this an entry in the event dictionary

          Assert(isDictionary(tcl_dict));
          tagged2Dictionary(tcl_dict)->remove(slave);

        } else if (isObject(slave)) {
          // this is an object which needs to be closed as well
          OZ_Return s = close_hierarchy(tagged2Object(slave));
          if (s != PROCEED)
            return s;
        }

        slaves = deref(tail(slaves));
      }
    }

    return PROCEED;
  }

}


OZ_C_proc_begin(BItclClose,3) {
  GET_TCL_SESSION;
  if (OZ_args[1] == NameTclClosed) {
    return ts->write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK(ts)
    OZ_Return s;

    // Perform closing of objects
    TaggedRef to = OZ_args[2];
    DEREF(to, to_ptr, to_tag);

    Assert(isObject(to));

    Object  * o = tagged2Object(to);
    TaggedRef v = o->getFeature(NameTclName);
    TaggedRef slave_entry;

    if (v == makeTaggedNULL()) {
      s = raise_type_error(to);;
      goto exit;
    }

    {
      DEREF(v, v_ptr, v_tag);

      if (isAnyVar(v_tag)) {
        am.addSuspendVarList(v_ptr);
        s = SUSPEND;
        goto exit;
      } else if (isLiteral(v_tag) && literalEq(v,NameTclClosed)) {
        LEAVE_TCL_LOCK(ts);
        return PROCEED;
      }
    }

    // Create close tcl
    ts->reset();
    StateExit(ts->put_tcl(OZ_args[1]));
    ts->put('\n');


    // okay, let us close it
    slave_entry = o->getFeature(NameTclSlaveEntry);

    // remove from parent
    if (slave_entry != makeTaggedNULL()) {
      slave_entry = deref(slave_entry);

      if (isCons(slave_entry)) {
        LTuple * l = tagged2LTuple(slave_entry);
        l->setHead(NameGroupVoid);
        l->setTail(findAliveEntry(tail(slave_entry)));
      }
    }

    ts->close_hierarchy(o);

    ts->start_write();
    OZ_args[1] = NameTclClosed;
    return ts->write();

  exit:
    ts->reset();
    LEAVE_TCL_LOCK(ts);
    return s;
  }
}
OZ_C_proc_end


// ---------------------------------------------------------------------
// Counters
// ---------------------------------------------------------------------

OZ_C_proc_begin(BIgenTopName,2) {
  GET_TCL_SESSION;
  return OZ_unify(OZ_getCArg(1),ts->genTopName());
} OZ_C_proc_end

OZ_C_proc_begin(BIgenWidgetName,3) {
  GET_TCL_SESSION;
  TaggedRef parent = OZ_getCArg(1);

  DEREF(parent, p_ptr, p_tag);

  if (isAnyVar(p_tag))
    OZ_suspendOn(makeTaggedRef(p_ptr));

  return OZ_unify(OZ_getCArg(2),ts->genWidgetName(parent));
} OZ_C_proc_end

OZ_C_proc_begin(BIgenTagName,2) {
  GET_TCL_SESSION;
  return OZ_unify(OZ_getCArg(1),ts->genTagName());
} OZ_C_proc_end

OZ_C_proc_begin(BIgenVarName,2) {
  GET_TCL_SESSION;
  return OZ_unify(OZ_getCArg(1),ts->genVarName());
} OZ_C_proc_end

OZ_C_proc_begin(BIgenImageName,2) {
  GET_TCL_SESSION;
  return OZ_unify(OZ_getCArg(1),ts->genImageName());
} OZ_C_proc_end

// ---------------------------------------------------------------------
// Add to Builtin-Table
// ---------------------------------------------------------------------

static
BIspec tclTkSpec[] = {
  {"getTclNames",        3, BIgetTclNames,        0},
  {"initTclSession",     4, BIinitTclSession,     0},
  {"closeTclSession",    1, BIcloseTclSession,    0},
  {"Tk.send",            2, BItclWrite,           0},
  {"tclWriteReturn",     4, BItclWriteReturn,     0},
  {"tclWriteReturnMess", 5, BItclWriteReturnMess, 0},
  {"Tk.batch",           2, BItclWriteBatch,      0},
  {"tclWriteTuple",      3, BItclWriteTuple,      0},
  {"tclWriteTagTuple",   4, BItclWriteTagTuple,   0},
  {"tclWriteFilter",     6, BItclWriteFilter,     0},

  {"tclClose",           3, BItclClose,           0},


  {"addFastGroup",        3, BIaddFastGroup,       0},
  {"delFastGroup",        1, BIdelFastGroup,       0},
  {"getFastGroup",        2, BIgetFastGroup,       0},
  {"delAllFastGroup",     2, BIdelAllFastGroup,    0},

  {"genTopName",    2, BIgenTopName,     0},
  {"genWidgetName", 3, BIgenWidgetName,  0},
  {"genTagName",    2, BIgenTagName,     0},
  {"genVarName",    2, BIgenVarName,     0},
  {"genImageName",  2, BIgenImageName,   0},

  {0,0,0,0}
};


void BIinitTclTk() {
  BIaddSpec(tclTkSpec);

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
  NameGroupVoid     = OZ_newName(); OZ_protect(&NameGroupVoid);
  NameTclSlaves     = OZ_newName(); OZ_protect(&NameTclSlaves);
  NameTclSlaveEntry = OZ_newName(); OZ_protect(&NameTclSlaveEntry);
  NameTclClosed     = OZ_newName(); OZ_protect(&NameTclClosed);

  tcl_sessions_init();
}
