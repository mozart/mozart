/* Copyright © by Denys Duchier, Dec 1997, Universität des Saarlandes
 * ===================================================================
 * POSIX Regular Expression Interface
 *
 * =================================================================== */

#ifndef MODULE
#define MODULE "regex"
#endif

#ifndef PREFIX
#define PREFIX(m) regex_ ## m
#endif

#ifndef ERRBUFSIZE
#define ERRBUFSIZE 200
#endif

#ifndef MAXMATCH
#define MAXMATCH 20
#endif

#include "oz.h"
#include <regex.h>

OZ_Return re_error(int code,regex_t* re,int must_free)
{
  char errbuf[ERRBUFSIZE];
  (void) regerror(code,re,errbuf,ERRBUFSIZE);
  if (must_free) delete re;
  return OZ_raiseErrorC(MODULE,1,OZ_string(errbuf));
}

OZ_C_proc_begin(PREFIX(compile),3)
{
  OZ_declareVirtualStringArg(0,RE);
  OZ_declareIntArg(1,CFLAGS);
  OZ_declareArg(2,RESULT);
  int errcode;
  regex_t *preg = new regex_t;
  errcode = regcomp(preg,RE,CFLAGS);
  if (errcode) return re_error(errcode,preg,1);
  return OZ_unify(RESULT,OZ_makeForeignPointer((void*)preg));
}
OZ_C_proc_end

OZ_C_proc_begin(PREFIX(execute),5)
{
  OZ_declareForeignPointerArg(0,RE);
  OZ_declareVirtualStringArg(1,TXT);
  OZ_declareIntArg(2,IDX);
  OZ_declareIntArg(3,EFLAGS);
  OZ_declareArg(4,RESULT);
  regex_t *preg = (regex_t*) RE;
  int errcode;
  regmatch_t pmatch[MAXMATCH];
  errcode = regexec(preg,&(TXT[IDX]),preg->re_nsub+1,pmatch,EFLAGS);
  if (errcode==REG_NOMATCH) return OZ_unify(RESULT,OZ_false());
  if (errcode) return re_error(errcode,preg,0);
  OZ_Term tuple;
  tuple = OZ_tupleC("match",preg->re_nsub);
  for (int i=preg->re_nsub;i>0;i--)
    OZ_putArg(tuple,i-1,OZ_pair2(OZ_int(pmatch[i].rm_so+IDX),
				 OZ_int(pmatch[i].rm_eo+IDX)));
  tuple = OZ_adjoinAt(tuple,OZ_int(0),OZ_pair2(OZ_int(pmatch[0].rm_so+IDX),
					       OZ_int(pmatch[0].rm_eo+IDX)));
  return OZ_unify(RESULT,tuple);
}
OZ_C_proc_end

OZ_C_proc_begin(PREFIX(free),1)
{
  OZ_declareForeignPointerArg(0,RE);
  delete (regex_t*) RE;
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(PREFIX(flags),1)
{
  OZ_Term tuple;
  tuple = OZ_tupleC("flag",6);
  OZ_putArg(tuple,0,OZ_int(REG_EXTENDED));
  OZ_putArg(tuple,1,OZ_int(REG_ICASE   ));
  OZ_putArg(tuple,2,OZ_int(REG_NEWLINE ));
  OZ_putArg(tuple,3,OZ_int(REG_NOSUB   ));
  OZ_putArg(tuple,4,OZ_int(REG_NOTBOL  ));
  OZ_putArg(tuple,5,OZ_int(REG_NOTEOL  ));
  return OZ_unify(OZ_getCArg(0),tuple);
}
OZ_C_proc_end
