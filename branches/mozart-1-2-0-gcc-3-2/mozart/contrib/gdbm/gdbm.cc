#include "mozart.h"
#include "gdbm.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

class GDBM: public OZ_Extension {
public:
  GDBM_FILE db;
  char *filename;
  GDBM(char*f,void * d):OZ_Extension(),filename(f),db((GDBM_FILE) d){}
  //
  // Extension 
  //
  friend int oz_isGdbm(OZ_Term);
  static int id;
  virtual int getIdV() { return id; }
  virtual OZ_Term typeV() { return OZ_atom("gdbm"); }
  virtual OZ_Extension* gCollectV(void);
  virtual OZ_Extension* sCloneV(void) { Assert(0); return NULL; }
  virtual void gCollectRecurseV(void) {}
  virtual void sCloneRecurseV(void) {}
  virtual OZ_Term printV(int depth = 10);
  //
  void release();
  void close();
};

//
// Extension 
//

int GDBM::id;

inline int oz_isGdbm(OZ_Term t)
{
  t = OZ_deref(t);
  return OZ_isExtension(t) &&
    OZ_getExtension(t)->getIdV()==GDBM::id;
}

inline GDBM* tagged2Gdbm(OZ_Term t)
{
  Assert(oz_isGdbm(t));
  return (GDBM*) OZ_getExtension(OZ_deref(t));
}

OZ_Extension* GDBM::gCollectV(void)
{
  return(new GDBM(filename,db));
}

//
// Member Functions
//

void GDBM::close() {
  if (db!=0) { gdbm_close(db); db=0; }
}

void GDBM::release() {
  close();
  if (filename!=0) { free(filename); filename=0; }
}

OZ_Term GDBM::printV(int depth = 10)
{
  return OZ_mkTupleC("#",6,
		     OZ_atom("<"),
		     typeV(),
		     OZ_atom(" "),
		     OZ_atom((db==0)?"[closed] ":""),
		     OZ_atom(filename),
		     OZ_atom(">"));
}

//
// Builtins
//
// Exceptions have the form gdbm(PROC ARGS MSG)
// where PROC is an atom naming the operation
// ARGS is the list of input arguments
// MSG is an atom describing the problem
//
// Errors have the form error(gdbm(PROC ARGS MSG) debug:DEBUG)

#define GdbmException(PROC,MSG) \
OZ_raiseC("gdbm",3,OZ_atom(PROC),OZ_inAsList(),OZ_atom(MSG))

#define GdbmError(PROC,MSG) \
OZ_raiseErrorC("gdbm",3,OZ_atom(PROC),OZ_inAsList(),OZ_atom(MSG))

OZ_BI_define(cgdbm_open,4,1)
{
  OZ_declareBitString(1,flags);
  OZ_declareInt(      2,mode);
  OZ_declareInt(      3,block);
  OZ_declareVS(       0,name,len);

  int zflags = 0;
  if (OZ_BitStringGet(flags,0)) zflags  = GDBM_READER;
  if (OZ_BitStringGet(flags,1)) zflags  = GDBM_WRITER;
  if (OZ_BitStringGet(flags,2)) zflags  = GDBM_WRCREAT;
  if (OZ_BitStringGet(flags,3)) zflags  = GDBM_NEWDB;
  if (OZ_BitStringGet(flags,4)) zflags |= GDBM_FAST;

  GDBM_FILE file;
  file = gdbm_open(name,block,zflags,mode,NULL);
  if (file==NULL)
    return GdbmError("open",gdbm_strerror(gdbm_errno));
  else {
    GDBM* db = new GDBM(strdup(name),file);
    OZ_RETURN(OZ_extension(db));
  }
} OZ_BI_end

OZ_Return
datum2term(datum dat,OZ_Term& out)
{
  OZ_Datum d;
  d.data = dat.dptr;
  d.size = dat.dsize;
  // eventually OZ_datumToValue should take an OZ_Term& instead
  // of performing unify internally. when that happens, we can
  // dispense with the allocation of a new variable.
  out = OZ_newVariable();
  OZ_Return ret = OZ_datumToValue(d,out);
  if (d.data!=0) { free(d.data); d.data=0; }
  return ret;
}

OZ_Return
term2datum(OZ_Term in,datum& dat)
{
  OZ_Datum d;
  OZ_Return r = OZ_valueToDatum(in,&d);
  dat.dptr  = d.data;
  dat.dsize = d.size;
  return r;
}

#define OZ_declareGdbm(ARG,VAR,METH,CLOSEOK)			\
OZ_declareType(ARG,VAR,GDBM*,"gdbm",oz_isGdbm,tagged2Gdbm);	\
if (!CLOSEOK && VAR->db==0)					\
  return GdbmError(METH,"alreadyClosed");

OZ_BI_define(cgdbm_is,1,1)
{
  OZ_declareDetTerm(0,t);
  OZ_RETURN((oz_isGdbm(t))?OZ_true():OZ_false());
} OZ_BI_end

OZ_BI_define(cgdbm_get,2,1)
{
  OZ_declareGdbm(0,oz_db,"get",0);
  OZ_declareVS(1,oz_key,oz_size);
  datum val;
  val.dptr  = oz_key;
  val.dsize = oz_size;
  val = gdbm_fetch(oz_db->db,val);
  return (val.dptr==NULL) ? GdbmException("get","keyNotFound")
    : datum2term(val,OZ_out(0));
} OZ_BI_end

OZ_BI_define(cgdbm_condGet,3,1)
{
  OZ_declareGdbm(0,oz_db,"condGet",0);
  OZ_declareVS(1,oz_key,oz_size);
  OZ_declareTerm(2,oz_default);
  datum val;
  val.dptr  = oz_key;
  val.dsize = oz_size;
  val = gdbm_fetch(oz_db->db,val);
  if (val.dptr==NULL) OZ_RETURN(oz_default);
  else return datum2term(val,OZ_out(0));
} OZ_BI_end

OZ_BI_define(cgdbm_put,3,0)
{
  OZ_declareGdbm(   0,oz_db,"put",0);
  OZ_declareDetTerm(2,oz_val);
  OZ_declareVS(     1,oz_key,oz_size);
  datum key,val;
  key.dptr  = oz_key;
  key.dsize = oz_size;
  int ret;
  ret = term2datum(oz_val,val);
  if (ret!=PROCEED) return ret;
  ret = gdbm_store(oz_db->db,key,val,GDBM_REPLACE);
  free(val.dptr);
  return (ret==0) ? PROCEED :
    GdbmError("put",gdbm_strerror(gdbm_errno));
} OZ_BI_end

OZ_BI_define(cgdbm_firstkey,1,1)
{
  OZ_declareGdbm(0,oz_db,"firstkey",0);
  datum key;

  key = gdbm_firstkey(oz_db->db);
  if (key.dptr==NULL) OZ_RETURN(OZ_unit());
  else {
    OZ_Term res;
    char* s = new char[key.dsize+1];
    memcpy(s,key.dptr,key.dsize);
    s[key.dsize] = '\0';
    res = OZ_atom(s);
    delete s;
    free(key.dptr);
    OZ_RETURN(res);
  }
} OZ_BI_end

OZ_BI_define(cgdbm_nextkey,2,1)
{
  OZ_declareGdbm(0,oz_db,"nextkey",0);
  OZ_declareVS(  1,oz_key,oz_len);
  datum key,val;
  key.dptr  = oz_key;
  key.dsize = oz_len;
  val = gdbm_nextkey(oz_db->db,key);
  if (val.dptr==NULL) OZ_RETURN(OZ_unit());
  else {
    OZ_Term res;
    char *s = new char[val.dsize+1];
    memcpy(s,val.dptr,val.dsize);
    s[val.dsize] = '\0';
    res = OZ_atom(s);
    delete s;
    free(val.dptr);
    OZ_RETURN(res);
  }
} OZ_BI_end

OZ_BI_define(cgdbm_close,1,0)
{
  OZ_declareGdbm(0,oz_db,"close",1);
  oz_db->close();
  return PROCEED;
} OZ_BI_end

OZ_BI_define(cgdbm_free,1,0)
{
  OZ_declareGdbm(0,oz_db,"free",1);
  oz_db->release();
  return PROCEED;
} OZ_BI_end

OZ_BI_define(cgdbm_remove,2,0)
{
  OZ_declareGdbm(0,oz_db,"remove",0);
  OZ_declareVS(  1,oz_key,oz_len);
  datum key; int res;
  key.dptr  = oz_key;
  key.dsize = oz_len;
  res = gdbm_delete(oz_db->db,key);
  return (res==0) ? PROCEED :
    GdbmError("remove",gdbm_strerror(gdbm_errno));
} OZ_BI_end

OZ_BI_define(cgdbm_reorganize,1,0)
{
  OZ_declareGdbm(0,oz_db,"reorganize",0);
  int res;
  res = gdbm_reorganize(oz_db->db);
  return (res==0) ? PROCEED :
    GdbmError("reorganize",gdbm_strerror(gdbm_errno));
} OZ_BI_end

OZ_BI_define(cgdbm_member,2,1)
{
  OZ_declareGdbm(0,oz_db,"member",0);
  OZ_declareVS(  1,oz_key,oz_len);
  datum key;
  key.dptr  = oz_key;
  key.dsize = oz_len;
  OZ_RETURN((gdbm_exists(oz_db->db,key))?OZ_true():OZ_false());
} OZ_BI_end

extern "C" 
{
  OZ_C_proc_interface * oz_init_module(void) 
  {
    static OZ_C_proc_interface i_table[] = {
      {"is"		,1,1,cgdbm_is},
      {"open"		,4,1,cgdbm_open},
      {"get"		,2,1,cgdbm_get},
      {"condGet"	,3,1,cgdbm_condGet},
      {"put"		,3,0,cgdbm_put},
      {"firstkey"	,1,1,cgdbm_firstkey},
      {"nextkey"	,2,1,cgdbm_nextkey},
      {"close"		,1,0,cgdbm_close},
      {"free"		,1,0,cgdbm_free},
      {"remove"		,2,0,cgdbm_remove},
      {"reorganize"	,1,0,cgdbm_reorganize},
      {"member"		,2,1,cgdbm_member},
      {0,0,0,0}
    };
    GDBM::id = oz_newUniqueId();
    return i_table;
  }

} /* extern "C" */


char oz_module_name[] = "GDBM";
