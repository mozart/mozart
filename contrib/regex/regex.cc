/* Copyright © by Denys Duchier, Dec 1997, Universität des Saarlandes
 * ===================================================================
 * POSIX Regular Expression Interface
 *
 * =================================================================== */


#ifndef ERRBUFSIZE
#define ERRBUFSIZE 200
#endif

#ifndef MAXMATCH
#define MAXMATCH 50
#endif

#include "mozart.h"
#include <sys/types.h>

#ifdef HAVE_STRDUP
// quick fix to get it to compile at SICS on Solaris
#define __EXTENSIONS__
#include <string.h>
#else
inline char * strdup(const char *s) {
  char *ret = new char[strlen(s)+1];
  strcpy(ret,s);
  return ret;
}
#endif

extern "C" {
#include <regex.h>
}

class REGEX: public OZ_Extension {
public:
  char*    src;
  regex_t* re;
  REGEX(char*s,regex_t*r):OZ_Extension(),src(s),re(r){}
  //
  // Extension
  //
  static int id;
  virtual int getIdV() { return id; }
  virtual OZ_Term typeV() { return OZ_atom("regex"); }
  virtual OZ_Term printV(int depth = 10);
  virtual OZ_Extension* gCollectV(void);
  virtual OZ_Extension* sCloneV(void) { Assert(0); return NULL; }
  virtual void gCollectRecurseV(void) {}
  virtual void sCloneRecurseV(void) {}
  //
  void release() {
    delete src;
    regfree(re);
    delete re;
  }
};

//
// Extension
//

int REGEX::id;

inline int oz_isRegex(OZ_Term t)
{
  t = OZ_deref(t);
  return OZ_isExtension(t) &&
    OZ_getExtension(t)->getIdV()==REGEX::id;
}

inline REGEX* tagged2Regex(OZ_Term t)
{
  Assert(oz_isRegex(t));
  return (REGEX*) OZ_getExtension(OZ_deref(t));
}

OZ_Term REGEX::printV(int)
{
  return OZ_mkTupleC("#",5,
                     OZ_atom("<"),
                     typeV(),
                     OZ_atom(" "),
                     OZ_atom(src),
                     OZ_atom(">"));
}

OZ_Extension* REGEX::gCollectV(void)
{
  return new REGEX(src,re);
}

OZ_Return re_error(int code,regex_t* re,int must_free)
{
  char errbuf[ERRBUFSIZE];
  (void) regerror(code,re,errbuf,ERRBUFSIZE);
  // maybe free the regex_t buffer, if it was newly allocated
  if (must_free) { regfree(re); delete re; }
  return OZ_atom(errbuf);
}

//
// Builtins
//

#define RegexError(PROC,CODE,RE,FREE) \
OZ_raiseErrorC("regex",3,OZ_atom(PROC),OZ_inAsList(), \
               re_error(CODE,RE,FREE))

OZ_BI_define(regex_compile,2,1)
{
  OZ_declareInt(1,CFLAGS);
  OZ_declareVS( 0,RE,LEN);
  int errcode;
  regex_t *preg = new regex_t;
  errcode = regcomp(preg,RE,CFLAGS);
  if (errcode) return RegexError("compile",errcode,preg,1);
  OZ_RETURN(OZ_extension(new REGEX(strdup(RE),preg)));
}
OZ_BI_end

OZ_BI_define(regex_is,1,1)
{
  OZ_declareDetTerm(0,t);
  OZ_RETURN((oz_isRegex(t))?OZ_true():OZ_false());
} OZ_BI_end

#define OZ_declareRegex(ARG,VAR) \
OZ_declareType(ARG,VAR,REGEX*,"regex",oz_isRegex,tagged2Regex)

OZ_BI_define(regex_execute,4,1)
{
  OZ_declareRegex(0,RE)
  OZ_declareInt(  2,IDX);
  OZ_declareInt(  3,EFLAGS);
  OZ_declareVS(   1,TXT,LEN);

  regex_t *preg = RE->re;
  int errcode;
  regmatch_t pmatch[MAXMATCH];
  errcode = regexec(preg,&(TXT[IDX]),preg->re_nsub+1,pmatch,EFLAGS);
  if (errcode==REG_NOMATCH) OZ_RETURN(OZ_false());
  if (errcode) return RegexError("execute",errcode,preg,0);
  OZ_Term tuple;
  tuple = OZ_tupleC("match",preg->re_nsub);
  for (int i=preg->re_nsub;i>0;i--)
    OZ_putArg(tuple,i-1,OZ_pair2(OZ_int(pmatch[i].rm_so+IDX),
                                 OZ_int(pmatch[i].rm_eo+IDX)));
  tuple = OZ_adjoinAt(tuple,OZ_int(0),OZ_pair2(OZ_int(pmatch[0].rm_so+IDX),
                                               OZ_int(pmatch[0].rm_eo+IDX)));
  OZ_RETURN(tuple);
}
OZ_BI_end

OZ_BI_define(regex_free,1,0)
{
  OZ_declareRegex(0,RE);
  RE->release();
  return PROCEED;
}
OZ_BI_end

OZ_BI_define(regex_flags,0,1)
{
  OZ_Term tuple;
  tuple = OZ_tupleC("flag",6);
  OZ_putArg(tuple,0,OZ_int(REG_EXTENDED));
  OZ_putArg(tuple,1,OZ_int(REG_ICASE   ));
  OZ_putArg(tuple,2,OZ_int(REG_NEWLINE ));
  OZ_putArg(tuple,3,OZ_int(REG_NOSUB   ));
  OZ_putArg(tuple,4,OZ_int(REG_NOTBOL  ));
  OZ_putArg(tuple,5,OZ_int(REG_NOTEOL  ));
  OZ_RETURN(tuple);
}
OZ_BI_end

extern "C"
{
  OZ_C_proc_interface * oz_init_module(void)
  {
    static OZ_C_proc_interface i_table[] = {
      {"is"     ,1,1,regex_is},
      {"compile",2,1,regex_compile},
      {"execute",4,1,regex_execute},
      {"free"   ,1,0,regex_free},
      {"flags"  ,0,1,regex_flags},
      {0,0,0,0}
    };
    REGEX::id = oz_newUniqueId();
    return i_table;
  }
} /* extern "C" */
