#include <oz.h>
#include "gdbm.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>

OZ_BI_define(cgdbm_open,4,1)
{
  OZ_declareNonvarIN(0,name);
  OZ_declareNonvarIN(1,flags);
  OZ_declareNonvarIN(2,mode);
  OZ_declareNonvarIN(3,block);
  int t_name  = 0;
  int t_flags = 0;
  int t_mode  = 0;
  int t_block = 0;
  char* c_name  = NULL;
  char* c_flags = NULL;
  int   c_mode;
  int   c_block;
  int   z_flags = 0;
  GDBM_FILE file;

  /* Perform type checks before any allocation so that we
     can return with type error and no memory leak */

  if (OZ_isAtom(name))                 t_name = 1;
  else if (OZ_isString(name,0))        t_name = 2;
  else if (OZ_isVirtualString(name,0)) t_name = 3;
  else return OZ_typeError(0,"Atom|String|VirtualString");

  if (OZ_isAtom(flags))                 t_flags = 1;
  else if (OZ_isString(flags,0))        t_flags = 2;
  else if (OZ_isVirtualString(flags,0)) t_flags = 3;
  else if (OZ_isUnit(flags))            t_flags = 4;
  else return OZ_typeError(1,"Atom|String|VirtualString|Unit");

  if (OZ_isInt(mode))       t_mode = 1;
  else if (OZ_isUnit(mode)) t_mode = 2;
  else return OZ_typeError(2,"Int|Unit");

  if (OZ_isInt(block))       t_block = 1;
  else if (OZ_isUnit(block)) t_block = 2;
  else return OZ_typeError(3,"Int|Unit");

  /* Perform type conversions */

  switch (t_name) {
  case 1: c_name=strdup(OZ_atomToC(name)); break;
  case 2: c_name=strdup(OZ_stringToC(name)); break;
  case 3: c_name=strdup(OZ_virtualStringToC(name)); break;
  default: return OZ_raise(OZ_atom("impossible"));
  }

  switch (t_flags) {
  case 1: c_flags = strdup(OZ_atomToC(flags)); break;
  case 2: c_flags = strdup(OZ_stringToC(flags)); break;
  case 3: c_flags = strdup(OZ_virtualStringToC(flags)); break;
  case 4: c_flags = NULL; break;
  default: return OZ_raise(OZ_atom("impossible"));
  }

  switch (t_mode) {
  case 1: c_mode = OZ_intToC(mode); break;
  case 2: c_mode = 0644; break;
  default: return OZ_raise(OZ_atom("impossible"));
  }

  switch (t_block) {
  case 1: c_block = OZ_intToC(block); break;
  case 2: c_block = 0; break;
  default: return OZ_raise(OZ_atom("impossible"));
  }

  if (c_flags != NULL) {
    char* s = c_flags;
    int fast = 0;
    while (*s)
      switch (*s++) {
      case 'r': z_flags = GDBM_READER; break;
      case 'w': z_flags = GDBM_WRITER; break;
      case 'c': z_flags = GDBM_WRCREAT; break;
      case 'n': z_flags = GDBM_NEWDB; break;
      case 'f': fast = 1; break;
      default: OZ_warning("Unknown flag `%c' to gdbm_open\n",s[-1]);
      }
    if (fast) z_flags |= GDBM_FAST;
    free(c_flags);
  }

  /* Open GDBM database */

  file = gdbm_open(c_name,c_block,z_flags,c_mode,NULL);
  free(c_name);
  if (file==NULL)
    return OZ_raiseC("gdbm",1,OZ_int(gdbm_errno));
  else {
    OZ_Term chunk = OZ_makeHeapChunk(sizeof(GDBM_FILE));
    char* s       = (char*) OZ_getHeapChunkData(chunk);
    memcpy(s,(char*)&file,sizeof(GDBM_FILE));
    OZ_RETURN(chunk);
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
  return OZ_datumToValue(d,out);
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

#define CHECK_GDBM(x) {                                         \
  if (!OZ_isChunk(x)                                            \
      || OZ_getHeapChunkSize(x)!=sizeof(GDBM_FILE))             \
    return OZ_typeError(0,"HeapChunk(GDBM_FILE)");              \
  if (*(GDBM_FILE*)OZ_getHeapChunkData(x)==(GDBM_FILE)0)        \
    return OZ_raiseErrorC("gdbm",2,OZ_atom("alreadyClosed"),x); \
}

OZ_BI_define(cgdbm_fetch,2,1)
{
  OZ_declareNonvarIN(0,oz_db);
  OZ_declareNonvarIN(1,oz_key);
  datum val;

  CHECK_GDBM(oz_db);

  /* the key string lives in a static area and won't need
     to be deallocated */
  if (OZ_isAtom(oz_key))
    val.dptr = OZ_atomToC(oz_key);
  else if (OZ_isString(oz_key,0))
    val.dptr = OZ_stringToC(oz_key);
  else if (OZ_isVirtualString(oz_key,0))
    val.dptr = OZ_virtualStringToC(oz_key);
  else return OZ_typeError(1,"Atom|String|VirtualString");

  val.dsize = strlen(val.dptr);
  val = gdbm_fetch(*(GDBM_FILE*)OZ_getHeapChunkData(oz_db),val);
  if (val.dptr==NULL) OZ_RETURN(OZ_unit());
  else return datum2term(val,OZ_out(0));
} OZ_BI_end

OZ_BI_define(cgdbm_store,4,1)
{
  OZ_declareNonvarIN(0,oz_db);
  OZ_declareNonvarIN(1,oz_key); int t_key = 0;
  OZ_declareNonvarIN(2,oz_val);
  OZ_declareNonvarIN(3,oz_rep); int rep;
  datum key,val;
  int ret;

  CHECK_GDBM(oz_db);

  if (OZ_isAtom(oz_key))                 t_key = 1;
  else if (OZ_isString(oz_key,0))        t_key = 2;
  else if (OZ_isVirtualString(oz_key,0)) t_key = 3;
  else return OZ_typeError(1,"Atom|String|VirtualString");


  if (OZ_isTrue(oz_rep)) rep = 1;
  else if (OZ_isFalse(oz_rep)) rep = 0;
  else return OZ_typeError(3,"Bool");

  switch (t_key) {
  case 1: key.dptr = strdup(OZ_atomToC(oz_key)); break;
  case 2: key.dptr = strdup(OZ_stringToC(oz_key)); break;
  case 3: key.dptr = strdup(OZ_virtualStringToC(oz_key)); break;
  default: return OZ_raise(OZ_atom("impossible"));
  }

  key.dsize = strlen(key.dptr);

  ret = term2datum(oz_val,val);
  if (ret!=PROCEED) {
    free(key.dptr);
    if (val.dptr) free(val.dptr);
    return ret;
  }

  ret = gdbm_store(*(GDBM_FILE*)OZ_getHeapChunkData(oz_db),
                   key,val,(rep)?GDBM_REPLACE:GDBM_INSERT);

  free(key.dptr);
  free(val.dptr);

  OZ_RETURN_INT(ret);
} OZ_BI_end

OZ_BI_define(cgdbm_firstkey,1,1)
{
  OZ_declareNonvarIN(0,oz_db);
  datum key;

  CHECK_GDBM(oz_db);

  key = gdbm_firstkey(*(GDBM_FILE*)OZ_getHeapChunkData(oz_db));
  if (key.dptr==NULL) OZ_RETURN(OZ_unit());
  else {
    OZ_Term res;
    char* s = (char*) malloc(key.dsize+1);
    memcpy(s,key.dptr,key.dsize);
    s[key.dsize] = '\0';
    res = OZ_atom(s);
    free(s);
    free(key.dptr);
    OZ_RETURN(res);
  }
} OZ_BI_end

OZ_BI_define(cgdbm_nextkey,2,1)
{
  OZ_declareNonvarIN(0,oz_db);
  OZ_declareNonvarIN(1,oz_key1);
  // OZ_Term oz_key2 = OZ_getCArg(2);
  datum key,val;

  CHECK_GDBM(oz_db);

  if (OZ_isAtom(oz_key1))
    key.dptr = strdup(OZ_atomToC(oz_key1));
  else if (OZ_isString(oz_key1,0))
    key.dptr = strdup(OZ_stringToC(oz_key1));
  else if (OZ_isVirtualString(oz_key1,0))
    key.dptr = strdup(OZ_virtualStringToC(oz_key1));
  else return OZ_typeError(1,"Atom|String|VirtualString");

  key.dsize = strlen(key.dptr);

  val = gdbm_nextkey(*(GDBM_FILE*)OZ_getHeapChunkData(oz_db),key);
  free(key.dptr);
  if (val.dptr==NULL) OZ_RETURN(OZ_unit());
  else {
    OZ_Term res;
    char *s = (char*) malloc(val.dsize+1);
    memcpy(s,val.dptr,val.dsize);
    s[val.dsize] = '\0';
    res = OZ_atom(s);
    free(s);
    free(val.dptr);
    OZ_RETURN(res);
  }
} OZ_BI_end

OZ_BI_define(cgdbm_close,1,0)
{
  OZ_declareNonvarIN(0,oz_db);

  if (!OZ_isChunk(oz_db)
      || OZ_getHeapChunkSize(oz_db)!=sizeof(GDBM_FILE))
    return OZ_typeError(0,"HeapChunk(GDBM_FILE)");

  gdbm_close(*(GDBM_FILE*)OZ_getHeapChunkData(oz_db));
  *(GDBM_FILE*)OZ_getHeapChunkData(oz_db) = (GDBM_FILE)0;
  return PROCEED;
} OZ_BI_end

OZ_BI_define(cgdbm_error,1,1)
{
  OZ_declareIntIN(0,oz_num);
  const char* s;

  s = gdbm_strerror(oz_num);
  OZ_RETURN_STRING(s);
} OZ_BI_end

OZ_BI_define(cgdbm_delete,2,1)
{
  OZ_declareNonvarIN(0,oz_db);
  OZ_declareNonvarIN(1,oz_key);
  datum key; int res;

  CHECK_GDBM(oz_db);

  if (OZ_isAtom(oz_key))
    key.dptr = OZ_atomToC(oz_key);
  else if (OZ_isString(oz_key,0))
    key.dptr = OZ_stringToC(oz_key);
  else if (OZ_isVirtualString(oz_key,0))
    key.dptr = OZ_virtualStringToC(oz_key);
  else return OZ_typeError(1,"Atom|String|VirtualString");

  key.dsize = strlen(key.dptr);

  res = gdbm_delete(*(GDBM_FILE*)OZ_getHeapChunkData(oz_db),key);
  OZ_RETURN_INT(res);
} OZ_BI_end

OZ_BI_define(cgdbm_reorganize,1,1)
{
  OZ_declareNonvarIN(0,oz_db);
  int res;

  CHECK_GDBM(oz_db);

  res = gdbm_reorganize(*(GDBM_FILE*)OZ_getHeapChunkData(oz_db));
  OZ_RETURN_INT(res);
} OZ_BI_end

OZ_BI_define(cgdbm_bitor,2,1)
{
  OZ_declareIntIN(0,n1);
  OZ_declareIntIN(1,n2);
  OZ_RETURN_INT(n1|n2);
} OZ_BI_end

OZ_C_proc_interface oz_interface[] = {
  {"open"       ,4,1,cgdbm_open},
  {"fetch"      ,2,1,cgdbm_fetch},
  {"store"      ,4,1,cgdbm_store},
  {"firstkey"   ,1,1,cgdbm_firstkey},
  {"nextkey"    ,2,1,cgdbm_nextkey},
  {"close"      ,1,0,cgdbm_close},
  {"error"      ,1,1,cgdbm_error},
  {"delete"     ,2,1,cgdbm_delete},
  {"reorganize" ,1,1,cgdbm_reorganize},
  {"bitor"      ,2,1,cgdbm_bitor},
  {0,0,0}
};
