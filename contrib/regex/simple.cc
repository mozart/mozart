/* Copyright © by Denys Duchier, Dec 1997, Universität des Saarlandes
 * ===================================================================
 * Simple Regular Expression Interface
 *
 * {ReSimple.match +RE +TXT ?MATCHES}
 *      RE and TXT are both virtual strings.  MATCHES is false if the
 *      match fails, else it is a record indexed by integers; one
 *      integer per match group (0 for the whole match); each element
 *      of this record is a pair I#J indicating the index of the
 *      beginning of the match and of the (exclusive) end.
 * =================================================================== */

#ifndef MODULE
#define MODULE "regex_simple"
#endif

#ifndef PREFIX
#define PREFIX(m) regex_simple_ ## m
#endif

#ifndef ERRBUFSIZE
#define ERRBUFSIZE 200
#endif

#ifndef MAXMATCH
#define MAXMATCH 20
#endif

#define CONCAT(m,n) m #n

#include "oz.h"
#include <regex.h>

OZ_C_proc_begin(PREFIX(match),3)
{
  OZ_declareArg(2,RESULT);
  OZ_declareVirtualStringArg(0,RE);
  int errcode;
  regex_t preg;
  int must_free = 0;
  errcode = regcomp(&preg,RE,REG_EXTENDED|REG_NEWLINE);
  if (errcode) goto ouch;
  must_free = 1;
  OZ_declareVirtualStringArg(1,TXT);
  regmatch_t pmatch[MAXMATCH];
  errcode = regexec(&preg,TXT,preg.re_nsub+1,pmatch,0);
  if (errcode==REG_NOMATCH) return OZ_unify(RESULT,OZ_false());
  if (errcode) goto ouch;
  OZ_Term tuple;
  int N; N=0;
  for (int i=0;i<=preg.re_nsub;i++) {
    if (pmatch[i].rm_so == -1) break;
    N=i;
  }
  regfree(&preg);
  tuple = OZ_tupleC("match",N);
  for (int i=0;i<N;i++)
    OZ_putArg(tuple,i,OZ_pair2(OZ_int(pmatch[i+1].rm_so),
                               OZ_int(pmatch[i+1].rm_eo)));
  tuple = OZ_adjoinAt(tuple,OZ_int(0),OZ_pair2(OZ_int(pmatch[0].rm_so),
                                               OZ_int(pmatch[0].rm_eo)));
  return OZ_unify(RESULT,tuple);
ouch:
  char errbuf[ERRBUFSIZE];
  regerror(errcode,&preg,errbuf,ERRBUFSIZE);
  if (must_free) regfree(&preg);
  return OZ_raiseErrorC(MODULE,1,OZ_string(errbuf));
}
OZ_C_proc_end
