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

#ifndef SPREFIX
#define SPREFIX(m) "regex_" # m
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

OZ_BI_define(PREFIX(compile),2,1)
{
  OZ_declareVirtualStringIN(0,RE);
  OZ_declareIntIN(1,CFLAGS);
  int errcode;
  regex_t *preg = new regex_t;
  errcode = regcomp(preg,RE,CFLAGS);
  if (errcode) return re_error(errcode,preg,1);
  OZ_RETURN(OZ_makeForeignPointer((void*)preg));
}
OZ_BI_end

OZ_BI_define(PREFIX(execute),4,1)
{
  OZ_declareForeignPointerIN(0,RE);
  OZ_declareVirtualStringIN(1,TXT);
  OZ_declareIntIN(2,IDX);
  OZ_declareIntIN(3,EFLAGS);
  regex_t *preg = (regex_t*) RE;
  int errcode;
  regmatch_t pmatch[MAXMATCH];
  errcode = regexec(preg,&(TXT[IDX]),preg->re_nsub+1,pmatch,EFLAGS);
  if (errcode==REG_NOMATCH) OZ_RETURN(OZ_false());
  if (errcode) return re_error(errcode,preg,0);
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

OZ_BI_define(PREFIX(free),1,0)
{
  OZ_declareForeignPointerIN(0,RE);
  delete (regex_t*) RE;
  return PROCEED;
}
OZ_BI_end

OZ_BI_define(PREFIX(flags),0,1)
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

OZ_C_proc_interface oz_interface[] = {
  {"compile"    ,2,1,PREFIX(compile)},
  {"execute"    ,4,1,PREFIX(execute)},
  {"free"       ,1,0,PREFIX(free)},
  {"flags"      ,0,1,PREFIX(flags)},
  {0,0,0}
};
