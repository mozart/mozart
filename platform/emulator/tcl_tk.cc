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

TaggedRef tcl_dict;

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



/*
 * Locking
 */

TaggedRef tcl_lock = makeTaggedNULL();
TaggedRef tcl_rets = makeTaggedNULL();

#define ENTER_TCL_LOCK { \
  TaggedRef t = tcl_lock;                                     \
  DEREF(t, t_ptr, t_tag);                                     \
  if (isAnyVar(t_tag)) {                                      \
    am.addSuspendVarList(t_ptr);                              \
    return SUSPEND;                                           \
  } else {                                                    \
    tcl_lock = makeTaggedRef(newTaggedUVar(am.currentBoard)); \
  }                                                           \
}

#define LEAVE_TCL_LOCK (void) am.fastUnify(tcl_lock, NameUnit, OK);



int tcl_fd = 0;

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

class StringBuffer {
  char static_buffer[STRING_BUFFER_SIZE+SAFETY_MARGIN];
  char * buffer;
  char * start;
  char * write_start;
  char * end;
  char * protect_start;

  void ensure(int n) {
    while (buffer+n>end)
      resize();
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

  void start_write(void) {
    write_start = start;
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

};




OZ_Return StringBuffer::write() {
redo:
  int ret = osTestSelect(tcl_fd, SEL_WRITE);

  if (ret < 0)  {
    reset();
    LEAVE_TCL_LOCK;
    return raise_os_error();
  } else if (ret==0) {
    goto wait_select;
  }

  while ((ret = oswrite(tcl_fd, write_start, buffer-write_start)) < 0) {
    if (errno != EINTR) {
      reset();
      LEAVE_TCL_LOCK;
      return raise_os_error();
    }
  }

  if (buffer - write_start == ret) {
    reset();
    LEAVE_TCL_LOCK;
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


void StringBuffer::resize(void) {
  int new_size = (3 * (end - start)) / 2;
  char *new_start = new char[new_size + SAFETY_MARGIN];

  end   = new_start + new_size;

  memcpy(new_start, start, buffer-start);

  dispose();

  buffer = (buffer - start) + new_start;
  start  = new_start;
}



OZ_Return StringBuffer::put_tuple(SRecord *st, int start) {
  if (start < st->getWidth()) {
    StateReturn(put_tcl(st->getArg(start)));

    for (int i=start+1; i < st->getWidth(); i++) {
      put(' ');
      StateReturn(put_tcl(st->getArg(i)));
    }
  }
  return PROCEED;
}

OZ_Return StringBuffer::put_record(SRecord * sr, TaggedRef as) {
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

OZ_Return StringBuffer::put_batch(TaggedRef batch, char delim) {

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

OZ_Return StringBuffer::put_record_or_tuple(TaggedRef tcl, int start = 0) {
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

OZ_Return StringBuffer::put_vs(TaggedRef vs) {
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


OZ_Return StringBuffer::put_vs_quote(TaggedRef vs) {
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


OZ_Return StringBuffer::put_tcl(TaggedRef tcl) {
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


OZ_Return StringBuffer::put_tcl_filter(TaggedRef tcl, TaggedRef fs) {
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


OZ_Return StringBuffer::put_tcl_return(TaggedRef tcl, TaggedRef * ret) {
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




StringBuffer tcl_buffer;

OZ_C_proc_begin(BIgetTclNames,3) {
  (void) OZ_unify(OZ_getCArg(0), NameTclSlaves);
  (void) OZ_unify(OZ_getCArg(1), NameTclSlaveEntry);
  (void) OZ_unify(OZ_getCArg(2), NameTclName);
  return PROCEED;
} OZ_C_proc_end

OZ_C_proc_begin(BIsetTclDict, 1) {
  tcl_dict = deref(OZ_args[0]);
  return PROCEED;
} OZ_C_proc_end

OZ_C_proc_begin(BIsetTclFD,2) {
  tcl_lock = NameUnit;
  tcl_fd   = smallIntValue(deref(OZ_args[0]));
  tcl_rets = OZ_args[1];
  return PROCEED;
} OZ_C_proc_end




OZ_C_proc_begin(BItclWrite,1) {
  if (OZ_args[0] == NameTclClosed) {
    return tcl_buffer.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK;
    OZ_Return s;

    tcl_buffer.reset();
    StateExit(tcl_buffer.put_tcl(OZ_args[0]));
    tcl_buffer.put('\n');

    tcl_buffer.start_write();
    OZ_args[0] = NameTclClosed;
    return tcl_buffer.write();

  exit:
    tcl_buffer.reset();
    LEAVE_TCL_LOCK;
    return s;
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteReturn, 3) {
  if (OZ_args[0] == NameTclClosed) {
    return tcl_buffer.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK;
    OZ_Return s;

    tcl_buffer.reset();
    tcl_buffer.put2('o', 'z');
    tcl_buffer.put2('r', ' ');
    tcl_buffer.put('[');
    StateExit(tcl_buffer.put_tcl(OZ_args[0]));
    tcl_buffer.put2(']','\n');

    // Enter return variable and cast
    { TaggedRef newt = OZ_cons(OZ_cons(OZ_args[2],OZ_args[1]),
                             makeTaggedRef(newTaggedUVar(am.currentBoard)));

    (void) OZ_unify(newt,tcl_rets);
    tcl_rets = tail(newt);

    tcl_buffer.start_write();
    OZ_args[0] = NameTclClosed;
    return tcl_buffer.write();
    }
  exit:
    tcl_buffer.reset();
    LEAVE_TCL_LOCK;
    return s;
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteReturnMess, 4) {
  /*
   * OZ_args[0]: tickle object and modifier (for tags and marks)
   * OZ_args[1]: return message
   * OZ_args[2]: modifier to be put after arg 1 of above (may be unit)
   * OZ_args[3]: type cast for return value
   */
  if (OZ_args[0] == NameTclClosed) {
    return tcl_buffer.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK;
    OZ_Return s;
    TaggedRef ret = NameTclClosed;
    TaggedRef mess = deref(OZ_args[1]);
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

    tcl_buffer.reset();
    tcl_buffer.put2('o', 'z');
    tcl_buffer.put2('r', ' ');
    tcl_buffer.put('[');
    StateExit(tcl_buffer.put_tcl(OZ_args[0]));
    tcl_buffer.put(' ');
    StateExit(tcl_buffer.put_tcl(frst));
    tcl_buffer.put(' ');
    StateExit(tcl_buffer.put_tcl(OZ_args[2]));
    tcl_buffer.put(' ');

    StateExit(tcl_buffer.put_tcl_return(mess, &ret));
    tcl_buffer.put2(']','\n');

    // Enter return variable and cast
    { TaggedRef newt = OZ_cons(OZ_cons(ret,OZ_args[3]),
                            makeTaggedRef(newTaggedUVar(am.currentBoard)));

    (void) OZ_unify(newt,tcl_rets);
    tcl_rets = tail(newt);

    tcl_buffer.start_write();
    OZ_args[0] = NameTclClosed;
    return tcl_buffer.write();
    }

  exit:
    tcl_buffer.reset();
    LEAVE_TCL_LOCK;
    return s;
  }

}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteBatch,1) {
  if (OZ_args[0] == NameTclClosed) {
    return tcl_buffer.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK;
    OZ_Return s;

    tcl_buffer.reset();
    StateExit(tcl_buffer.put_batch(deref(OZ_args[0]),';'));
    tcl_buffer.put('\n');

    tcl_buffer.start_write();
    OZ_args[0] = NameTclClosed;
    return tcl_buffer.write();

  exit:
    tcl_buffer.reset();
    LEAVE_TCL_LOCK;
    return s;
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteTuple,2) {
  if (OZ_args[0] == NameTclClosed) {
    return tcl_buffer.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK;
    OZ_Return s;

    TaggedRef mess = deref(OZ_args[1]);
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

    tcl_buffer.reset();
    StateExit(tcl_buffer.put_tcl(OZ_args[0]));
    tcl_buffer.put(' ');
    StateExit(tcl_buffer.put_record_or_tuple(OZ_args[1]));
    tcl_buffer.put('\n');

    tcl_buffer.start_write();
    OZ_args[0] = NameTclClosed;
    return tcl_buffer.write();

  exit:
    tcl_buffer.reset();
    LEAVE_TCL_LOCK;
    return s;
  }

}
OZ_C_proc_end



OZ_C_proc_begin(BItclWriteTagTuple,3) {
  if (OZ_args[0] == NameTclClosed) {
    return tcl_buffer.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK;
    OZ_Return s;
    TaggedRef tuple = deref(OZ_args[2]);
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

    tcl_buffer.reset();
    StateExit(tcl_buffer.put_tcl(OZ_args[0]));
    tcl_buffer.put(' ');
    StateExit(tcl_buffer.put_tcl(fst));
    tcl_buffer.put(' ');
    StateExit(tcl_buffer.put_tcl(OZ_args[1]));
    tcl_buffer.put(' ');
    StateExit(tcl_buffer.put_record_or_tuple(tuple,1));
    tcl_buffer.put('\n');

    tcl_buffer.start_write();
    OZ_args[0] = NameTclClosed;
    return tcl_buffer.write();

  exit:
    tcl_buffer.reset();
    LEAVE_TCL_LOCK;
    return s;
  }
}
OZ_C_proc_end

OZ_C_proc_begin(BItclWriteFilter,5) {
  if (OZ_args[0] == NameTclClosed) {
    return tcl_buffer.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK;
    OZ_Return s;

    tcl_buffer.reset();
    StateExit(tcl_buffer.put_tcl(OZ_args[0]));
    tcl_buffer.put(' ');
    StateExit(tcl_buffer.put_vs(OZ_args[1]));
    tcl_buffer.put(' ');
    StateExit(tcl_buffer.put_tcl_filter(OZ_args[2], deref(OZ_args[3])));
    tcl_buffer.put(' ');
    StateExit(tcl_buffer.put_tcl(OZ_args[4]));
    tcl_buffer.put('\n');

    tcl_buffer.start_write();
    OZ_args[0] = NameTclClosed;
    return tcl_buffer.write();

  exit:
    tcl_buffer.reset();
    LEAVE_TCL_LOCK;
    return s;
  }
}
OZ_C_proc_end


OZ_Return close_hierarchy(Object * o) {
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


OZ_C_proc_begin(BItclClose,2) {
  if (OZ_args[0] == NameTclClosed) {
    return tcl_buffer.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TCL_LOCK;
    OZ_Return s;

    // Perform closing of objects
    TaggedRef to = OZ_args[1];
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
        LEAVE_TCL_LOCK;
        return PROCEED;
      }
    }

    // Create close tcl
    tcl_buffer.reset();
    StateExit(tcl_buffer.put_tcl(OZ_args[0]));
    tcl_buffer.put('\n');


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

    close_hierarchy(o);

    tcl_buffer.start_write();
    OZ_args[0] = NameTclClosed;
    return tcl_buffer.write();

  exit:
    tcl_buffer.reset();
    LEAVE_TCL_LOCK;
    return s;
  }
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
  {"getTclNames",        3, BIgetTclNames,        0},
  {"setTclFD",           2, BIsetTclFD,           0},
  {"setTclDict",         1, BIsetTclDict,         0},
  {"Tk.send",            1, BItclWrite,           0},
  {"tclWriteReturn",     3, BItclWriteReturn,     0},
  {"tclWriteReturnMess", 4, BItclWriteReturnMess, 0},
  {"Tk.batch",           1, BItclWriteBatch,      0},
  {"tclWriteTuple",      2, BItclWriteTuple,      0},
  {"tclWriteTagTuple",   3, BItclWriteTagTuple,   0},
  {"tclWriteFilter",     5, BItclWriteFilter,     0},

  {"tclClose",           2, BItclClose,           0},


  {"addFastGroup",        3, BIaddFastGroup,       0},
  {"delFastGroup",        1, BIdelFastGroup,       0},
  {"getFastGroup",        2, BIgetFastGroup,       0},
  {"delAllFastGroup",     2, BIdelAllFastGroup,    0},

  {"genTopName",    1, BIgenTopName,     0},
  {"genWidgetName", 2, BIgenWidgetName,  0},
  {"genTagName",    1, BIgenTagName,     0},
  {"genVarName",    1, BIgenVarName,     0},
  {"genImageName",  1, BIgenImageName,   0},

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
}
