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


#define SAFETY_MARGIN   256
#define TCL_BUFFER_SIZE 1024

static char tcl_static_buffer[TCL_BUFFER_SIZE+SAFETY_MARGIN];
static char * tcl_buffer;
static char * tcl_buffer_start;
static char * tcl_buffer_end;
static char * protect_start;


inline 
void init_tcl_buffer(void) {
  tcl_buffer_start = tcl_static_buffer;
  tcl_buffer_end   = tcl_buffer_start + TCL_BUFFER_SIZE;
  tcl_buffer       = tcl_buffer_start;
}

void resize_tcl_buffer(void) {
  int new_size = (3 * (tcl_buffer_end - tcl_buffer_start)) / 2; 
  char *new_tcl_buffer_start = 
    new char[new_size + SAFETY_MARGIN];
    
  tcl_buffer_end   = new_tcl_buffer_start + new_size;
    
  char *new_tcl_buffer = new_tcl_buffer_start;
  
  for (char *j=tcl_buffer_start; j<tcl_buffer ; new_tcl_buffer++, j++) 
    *new_tcl_buffer = *j;
    
  if (tcl_buffer_start!=tcl_static_buffer)
    delete tcl_buffer_start;

  tcl_buffer       = (tcl_buffer - tcl_buffer_start) + new_tcl_buffer_start;
  tcl_buffer_start = new_tcl_buffer_start;

}

inline
void delete_tcl_buffer(void) {
  if (tcl_buffer_start!=tcl_static_buffer)
    delete tcl_buffer_start;
}



OZ_C_proc_begin(BIgetTclName,1) {
  return OZ_unify(OZ_getCArg(0), NameTclName);
} OZ_C_proc_end



State isVirtualString(TaggedRef vs) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isAnyVar(vs_tag)) {
    return OZ_suspendOnVar(makeTaggedRef(vs_ptr));
  }

  if (isInt(vs_tag) || isFloat(vs_tag) || 
      (isLiteral(vs_tag) && tagged2Literal(vs)->isAtom()))
    return PROCEED;

  if (isSTuple(vs) && 
      sameLiteral(tagged2SRecord(vs)->getLabel(),AtomPair)) {
    for (int i=0; i < tagged2SRecord(vs)->getWidth(); i++) {
      State argstate = isVirtualString(tagged2SRecord(vs)->getArg(i));
      if (argstate!=PROCEED) 
	return argstate;
    }
    return PROCEED;
  }

  if (isLTuple(vs_tag)) {
    int len = isString(vs);

    if (len == -2) {
      return FAILED;
    } else if (len == -1) {
      return SUSPEND;
    } else {
      return PROCEED;
    }
  }

  return FAILED;

}


inline
State isTclLiteral(TaggedRef tcl) {
  if (tagged2Literal(tcl)->isAtom()) {
    return PROCEED;
  } else if (sameLiteral(tcl,NameTrue) || sameLiteral(tcl,NameFalse)) {
    return PROCEED;
  }
  return FAILED;
}

State isTcl(TaggedRef tcl) {
  DEREF(tcl, tcl_ptr, tcl_tag);

  if (isAnyVar(tcl_tag)) {
    return OZ_suspendOnVar(makeTaggedRef(tcl_ptr));
  } else if (isInt(tcl_tag)) {
    return PROCEED;
  } else if (isFloat(tcl_tag)) {
    return PROCEED;
  } else if (isLiteral(tcl_tag)) {
    return isTclLiteral(tcl);
  } else if (isObject(tcl)) {
    TaggedRef v = tagged2Object(tcl)->getFeature(NameTclName);
    if (v!=makeTaggedNULL()) {
      DEREF(v, v_ptr, v_tag);
      if (isAnyVar(v_tag)) {
	return OZ_suspendOnVar(makeTaggedRef(v_ptr)); 
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
      if (sameLiteral(l,AtomPair)) { 
	return isVirtualString(tcl);
      } else if (sameLiteral(l,AtomTclVS)) {
	TaggedRef arg = st->getArg(0);

	DEREF(arg, arg_ptr, arg_tag);

	if (isAnyVar(arg_tag)) {
	  return OZ_suspendOnVar(makeTaggedRef(arg_ptr)); 
	}

	return isVirtualString(arg);
      } else if (sameLiteral(l,AtomTclBatch)) {
	TaggedRef batch = st->getArg(0);

	while (1) {
	  DEREF(batch, batch_ptr, batch_tag);

	  if (isAnyVar(batch_tag)) {
	    return OZ_suspendOnVar(makeTaggedRef(batch_ptr)); 
	  } else if (isLTuple(batch_tag)) {
	    State batch_state = isTcl(tagged2LTuple(batch)->getHead());

	    if (batch_state!=PROCEED) 
	      return batch_state;

	    batch = tagged2LTuple(batch)->getTail();
	  } else if (isLiteral(batch_tag) && sameLiteral(batch,AtomNil)) {
	    return PROCEED;
	  } else {
	    return FAILED;
	  }
	}

      } else {
	for (int i=0; i < st->getWidth(); i++) {
	  State argstate = isTcl(st->getArg(i));
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
      State argstate = isTcl(sr->getArg(i));
      if (argstate!=PROCEED) 
	return argstate;
      }
      return PROCEED;
    } else {
      return FAILED;
    }
    
  } else if (isLTuple(tcl_tag)) {
    int len = isString(tcl);

    if (len == -2) {
      return FAILED;
    } else if (len == -1) {
      return SUSPEND;
    } else {
      return PROCEED;
    }
  }
    
  return FAILED;

}


OZ_C_proc_begin(BIisTcl, 2) {  
  State s = isTcl(OZ_getCArg(0)); 
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
    return OZ_suspendOnVar(makeTaggedRef(tcl_ptr));
  } else if (isLiteral(tcl_tag)) {
    return OZ_unify(OZ_getCArg(2), NameTrue);
  } else if (isSRecord(tcl_tag)) {
    State s = PROCEED;
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef as = deref(sr->getArityList());
    TaggedRef fs = deref(OZ_getCArg(1));

    while (isLTuple(as) && isLTuple(fs)) {
      TaggedRef a = deref(tagged2LTuple(as)->getHead());
      TaggedRef f = deref(tagged2LTuple(fs)->getHead());

      if (tagged2Literal(a)->isAtom()) {

	switch (atomcmp(a,f)) {
	case 0:
	  fs = deref(tagged2LTuple(fs)->getTail());
	  as = deref(tagged2LTuple(as)->getTail());
	  break;
	case 1:
	  fs = deref(tagged2LTuple(fs)->getTail());
	  break;
	case -1:
	  s = isTcl(sr->getFeature(a));
	  if (s!=PROCEED)
	    goto exit;
	  as = deref(tagged2LTuple(as)->getTail());
	  break;
	}

      }
    }

    while (isLTuple(as)) {
      TaggedRef a = deref(tagged2LTuple(as)->getHead());

      if (tagged2Literal(a)->isAtom()) {
	s = isTcl(sr->getFeature(a));
	if (s!=PROCEED)
	  goto exit;
	as = deref(tagged2LTuple(as)->getTail());
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
  *tcl_buffer = c; tcl_buffer++;
  if (tcl_buffer>tcl_buffer_end) 
    resize_tcl_buffer();
}

inline 
void tcl_back(void) {
  tcl_buffer--;
}

inline 
void tcl_put2(char c1, char c2) {
  *tcl_buffer = c1; tcl_buffer++;
  *tcl_buffer = c2; tcl_buffer++;
  if (tcl_buffer>tcl_buffer_end) 
    resize_tcl_buffer();
}

inline 
void tcl_put_octal(char c) {
  unsigned char c1 = (((unsigned char) c & '\300') >> 6) + '0';
  unsigned char c2 = (((unsigned char) c & '\070') >> 3) + '0';
  unsigned char c3 = ((unsigned char) c & '\007') + '0';
  *tcl_buffer = '\\'; tcl_buffer++;
  *tcl_buffer = c1;   tcl_buffer++;
  *tcl_buffer = c2;   tcl_buffer++;
  *tcl_buffer = c3;   tcl_buffer++;
  if (tcl_buffer>tcl_buffer_end) 
    resize_tcl_buffer();
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
    int len;
    sprintf(tcl_buffer,"%d%n",smallIntValue(i),&len);
    tcl_buffer += len;
    if (tcl_buffer>tcl_buffer_end) 
      resize_tcl_buffer();
  } else {
    char * s = OZ_intToCString(i);
    cstring2buffer(s);
    delete [] s;
  }
}


inline
void float2buffer(TaggedRef f) {
  int len;
  sprintf(tcl_buffer,"%g%n",floatValue(f),&len);
  tcl_buffer += len;
  if (tcl_buffer>tcl_buffer_end) 
    resize_tcl_buffer();
}


inline
void string2buffer(TaggedRef tail) {
  do {
    tcl_put((char) smallIntValue(deref(tagged2LTuple(tail)->getHead())));
    tail = deref(tagged2LTuple(tail)->getTail());
  } while (isLTuple(tail));
}


inline
void vs2buffer(TaggedRef vs) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isInt(vs_tag)) {
    int2buffer(vs);
  } else if (isFloat(vs_tag)) {
    float2buffer(vs);
  } else if (isLiteral(vs_tag)) {
    Assert(tagged2Literal(vs)->isAtom());
    
    if (!sameLiteral(vs, AtomNil) && !sameLiteral(vs, AtomPair))
      atom2buffer(vs);
  } else if (isSTuple(vs)) {
    Assert(sameLiteral(tagged2SRecord(vs)->getLabel(),AtomPair));

    for (int i=0; i < tagged2SRecord(vs)->getWidth(); i++)
      vs2buffer(tagged2SRecord(vs)->getArg(i));
  } else if (isLTuple(vs_tag)) {
    string2buffer(vs);
  }
}


inline
void start_protect(void) {
  protect_start = tcl_buffer;
}


inline
void stop_protect(void) {
  if (protect_start == tcl_buffer)
    tcl_put2('"','"');
}


inline
void protect_atom2buffer(TaggedRef atom) {
  if (sameLiteral(atom, AtomPair) || sameLiteral(atom, AtomNil))
    return;

  char *s = tagged2Literal(atom)->getPrintName();
  char c;

  while ((c = *s++))
    tcl_put_quote(c);
}


inline
void protect_string2buffer(TaggedRef tail) {
  do {
    tcl_put_quote((char) smallIntValue(deref(tagged2LTuple(tail)->getHead())));
    tail = deref(tagged2LTuple(tail)->getTail());
  } while (isLTuple(tail));
}


inline
void protect_vs2buffer(TaggedRef vs) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isInt(vs_tag)) {
    int2buffer(vs);
  } else if (isFloat(vs_tag)) {
    float2buffer(vs);
  } else if (isLiteral(vs_tag)) {
    Assert(tagged2Literal(vs)->isAtom());
    
    if (!sameLiteral(vs, AtomNil) && !sameLiteral(vs, AtomPair))
      protect_atom2buffer(vs);
  } else if (isSTuple(vs)) {
    Assert(sameLiteral(tagged2SRecord(vs)->getLabel(),AtomPair));

    for (int i=0; i < tagged2SRecord(vs)->getWidth(); i++)
      protect_vs2buffer(tagged2SRecord(vs)->getArg(i));
  } else if (isLTuple(vs_tag)) {
    protect_string2buffer(vs);
  }
}


void tcl2buffer(TaggedRef);


inline
void tuple2buffer(SRecord *st, int start = 0) {
  if (start < st->getWidth()) {
    tcl2buffer(st->getArg(start));
    for (int i=start+1; i < st->getWidth(); i++) {
      tcl_put(' ');
      tcl2buffer(st->getArg(i));
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
    } else if (sameLiteral(tcl,NameTrue)) {
      tcl_put('1');
    } else if (sameLiteral(tcl,NameFalse)) {
      tcl_put('0');
    }
  } else if (isLTuple(tcl_tag)) {
    start_protect();
    protect_string2buffer(tcl);
    stop_protect();
  } else if (isSTuple(tcl)) {
    SRecord  * st = tagged2SRecord(tcl);
    TaggedRef l  = st->getLabel();
    
    if (sameLiteral(l,AtomPair)) {
      start_protect();
      protect_vs2buffer(tcl);
      stop_protect();
    } else if (sameLiteral(l,AtomTclOption)) {
      tuple2buffer(st);
    } else if (sameLiteral(l,AtomTclList)) {
      tcl_put('['); tuple2buffer(st); tcl_put(']');
    } else if (sameLiteral(l,AtomTclQuote)) {
      tcl_put('{'); tuple2buffer(st); tcl_put('}');
    } else if (sameLiteral(l,AtomTclString)) {
      tcl_put('"'); tuple2buffer(st); tcl_put('"');
    } else if (sameLiteral(l,AtomTclPosition)) {
      if (st->getWidth() > 1) {
	tcl_put('{'); tcl2buffer(st->getArg(0));
	tcl_put('.'); tuple2buffer(st, 1);
	tcl_put('}');
      }
    } else if (sameLiteral(l,AtomTclVS)) {
      vs2buffer(st->getArg(0));
    } else if (sameLiteral(l,AtomTclBatch)) {
      TaggedRef b = deref(st->getArg(0));

      if (isLTuple(b)) {
	tcl2buffer(tagged2LTuple(b)->getHead());
	b = deref(tagged2LTuple(b)->getTail());

	while (isLTuple(b)) {
	  tcl_put(' ');
	  tcl2buffer(tagged2LTuple(b)->getHead());
	  b = deref(tagged2LTuple(b)->getTail());
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

    if (!sameLiteral(l,AtomTclOption)) {
      start_protect();
      protect_atom2buffer(l);
      stop_protect();
      tcl_put(' ');
    }
    
    TaggedRef as = deref(sr->getArityList());

    while (isLTuple(as)) {
      TaggedRef a = deref(tagged2LTuple(as)->getHead());

      if (tagged2Literal(a)->isAtom()) {
	tcl_put('-');
	start_protect();
	protect_atom2buffer(a);
	stop_protect();
	tcl_put(' ');
	tcl2buffer(sr->getFeature(a));
	tcl_put(' ');
      }
      
      as = deref(tagged2LTuple(as)->getTail());
    }

    tcl_back();
  }
 
}


State ret_unix_error(TaggedRef out) {
  SRecord * err_tuple = SRecord::newSRecord(AtomError, 2);
    
  err_tuple->setArg(0, OZ_CToInt(errno));
  err_tuple->setArg(1, OZ_CToString(OZ_unixError(errno)));
  
  return OZ_unify(out,makeTaggedSRecord(err_tuple));
}


State tcl_write(int fd, char * buff, int len, TaggedRef out) {
  int ret = osTestSelect(fd,SEL_WRITE);

  if (ret < 0)  { 
    delete_tcl_buffer();
    return ret_unix_error(out);
  } else if (ret==0) {
    return OZ_unifyInt(out,0);
  }  

  while ((ret = write(fd, buff, len)) < 0) {
    if (errno != EINTR) { 
      delete_tcl_buffer();
      return ret_unix_error(out);
    }
  }

  if (len == ret) {
    delete_tcl_buffer();

    return OZ_unify(out, NameTrue);
  }

  return OZ_unifyInt(out, ret);
}


OZ_C_proc_begin(BItclWrite,3) {  
  OZ_declareIntArg(0, fd);

  init_tcl_buffer();
  tcl2buffer(OZ_getCArg(1));
  tcl_put('\n');

  return tcl_write(fd, tcl_buffer_start, tcl_buffer-tcl_buffer_start, 
		   OZ_getCArg(2));
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteBatch,3) {  
  OZ_declareIntArg(0, fd);
  TaggedRef batch = deref(OZ_getCArg(1));

  init_tcl_buffer();

  while (isLTuple(batch)) {
    tcl2buffer(tagged2LTuple(batch)->getHead());
    tcl_put(';');
    batch = deref(tagged2LTuple(batch)->getTail());
  }
  tcl_put('\n');

  return tcl_write(fd, tcl_buffer_start, tcl_buffer-tcl_buffer_start, 
		   OZ_getCArg(2));
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteTuple,4) {  
  OZ_declareIntArg(0, fd);

  init_tcl_buffer();
  tcl2buffer(OZ_getCArg(1));
  tcl_put(' ');
  tuple2buffer(tagged2SRecord(deref(OZ_getCArg(2))));
  tcl_put('\n');

  return tcl_write(fd, tcl_buffer_start, tcl_buffer-tcl_buffer_start, 
		   OZ_getCArg(3));
}
OZ_C_proc_end



OZ_C_proc_begin(BItclWriteTagTuple,5) {  
  OZ_declareIntArg(0, fd);
  TaggedRef tuple = deref(OZ_getCArg(3));

  init_tcl_buffer();
  tcl2buffer(OZ_getCArg(1));
  tcl_put(' ');
  tcl2buffer(tagged2SRecord(tuple)->getArg(0));
  tcl_put(' ');
  tcl2buffer(OZ_getCArg(2));
  tcl_put(' ');
  tuple2buffer(tagged2SRecord(tuple),1);
  tcl_put('\n');

  return tcl_write(fd, tcl_buffer_start, tcl_buffer-tcl_buffer_start, 
		   OZ_getCArg(4));
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteFilter,7) {  
  OZ_declareIntArg(0, fd);

  init_tcl_buffer();
  tcl2buffer(OZ_getCArg(1));
  tcl_put(' ');
  vs2buffer(OZ_getCArg(2));
  tcl_put(' ');
  
  TaggedRef tr   = OZ_getCArg(3);

  DEREF(tr, t_p, tr_tag);

  if (isSRecord(tr_tag)) {
    SRecord   * sr = tagged2SRecord(tr);
    TaggedRef as   = deref(sr->getArityList());
    TaggedRef fs   = deref(OZ_getCArg(4));
    
    while (isLTuple(as) && isLTuple(fs)) {
      TaggedRef a = deref(tagged2LTuple(as)->getHead());
      TaggedRef f = deref(tagged2LTuple(fs)->getHead());
      
      if (tagged2Literal(a)->isAtom()) {
	
	switch (atomcmp(a,f)) {
	case 0:
	  fs = deref(tagged2LTuple(fs)->getTail());
	  as = deref(tagged2LTuple(as)->getTail());
	  break;
	case 1:
	  fs = deref(tagged2LTuple(fs)->getTail());
	  break;
	case -1:
	  tcl_put('-');
	  atom2buffer(a);
	  tcl_put(' ');
	  tcl2buffer(sr->getFeature(a));
	  tcl_put(' ');
	  as = deref(tagged2LTuple(as)->getTail());
	  break;
	}
	
      }

    }
    
    while (isLTuple(as)) {
      TaggedRef a = deref(tagged2LTuple(as)->getHead());
      
      if (tagged2Literal(a)->isAtom()) {
	tcl_put('-');
	atom2buffer(a);
      tcl_put(' ');
      tcl2buffer(sr->getFeature(a));
      tcl_put(' ');
      as = deref(tagged2LTuple(as)->getTail());
      }
    }
  }
	
  tcl2buffer(OZ_getCArg(5));
  tcl_put('\n');

  return tcl_write(fd, tcl_buffer_start, tcl_buffer-tcl_buffer_start, 
		   OZ_getCArg(6));
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteCont,3) {  
  OZ_declareIntArg(0, fd);
  OZ_declareIntArg(1, written);

  return tcl_write(fd, tcl_buffer_start + written, 
		   tcl_buffer-tcl_buffer_start-written, 
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
      TaggedRef head = headDeref(group);

      if (!(isLiteral(head) && sameLiteral(head,NameGroupVoid)))
	return group;
      
      group = tailDeref(group);
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


OZ_C_proc_begin(BIgetFastGroup,2)
{
  OZ_nonvarArg(0);
  TaggedRef group = OZ_getCArg(0); 

  DEREF(group, _1, _2);

  if (isCons(group)) {
    TaggedRef out = nil();

    group = tailDeref(group);
    
    while (isCons(group)) {
      TaggedRef head = headDeref(group);

      if (!(isLiteral(head) && sameLiteral(head,NameGroupVoid)))
	out = cons(head, out);
      
      group = tailDeref(group);
    }

    return isNil(group) ? OZ_unify(out,OZ_getCArg(1)) : FAILED;
  }
  
  return FAILED;

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
    return OZ_suspendOnVar(makeTaggedRef(p_ptr));

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
  {"getTclName",       1, BIgetTclName,       NO, 0},
  {"isTcl",            2, BIisTcl,            NO, 0},
  {"isTclFilter",      3, BIisTclFilter,      NO, 0},
  {"tclWrite",         3, BItclWrite,         NO, 0},
  {"tclWriteBatch",    3, BItclWriteBatch,    NO, 0},
  {"tclWriteTuple",    4, BItclWriteTuple,    NO, 0},
  {"tclWriteTagTuple", 5, BItclWriteTagTuple, NO, 0},
  {"tclWriteFilter",   7, BItclWriteFilter,   NO, 0},
  {"tclWriteCont",     3, BItclWriteCont,     NO, 0},

  {"addFastGroup", 3, BIaddFastGroup,	NO, 0},
  {"delFastGroup", 1, BIdelFastGroup,	NO, 0},
  {"getFastGroup", 2, BIgetFastGroup,	NO, 0},

  {"genTopName",    1, BIgenTopName,	NO, 0},
  {"genWidgetName", 2, BIgenWidgetName,	NO, 0},
  {"genTagName",    1, BIgenTagName,	NO, 0},
  {"genVarName",    1, BIgenVarName,	NO, 0},
  {"genImageName",  1, BIgenImageName,	NO, 0},

  {0,0,0,0,0}
};


void BIinitTclTk() {
  BIaddSpec(tclTkSpec);
}

