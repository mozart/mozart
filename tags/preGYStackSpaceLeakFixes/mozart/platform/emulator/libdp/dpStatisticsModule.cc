/*
 *  Authors:
 *    Andreas Sundstroem (andreas@sics.se)
 *    Erik Klintskog (erik@sics.se)
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "base.hh"
#include "dpBase.hh"

#include "perdio.hh"
#include "table.hh"
#include "dpMarshaler.hh"

#include "builtins.hh"
#include "os.hh"


OZ_BI_define(BItablesExtract,0,1)
{
  initDP();

  OZ_Term borrowlist;
  OZ_Term ownerlist;
  OZ_Term ret;

  borrowlist = BT->extract_info();
  ownerlist =OT->extract_info();
  ret=oz_cons(borrowlist,oz_cons(ownerlist, oz_nil()));
  OZ_RETURN(ret);
} OZ_BI_end

/*
OZ_BI_define(BItablesBTinfo,0,1)
{
  initDP();
  for (int i = getSize(); i--; ) {
  BorrowEntry *be = getFirstNode(i); 
    while (be) {
      ans = oz_cons(be->extract_info(),ans);
      be = be->getNext();
    }
  }
}
*/

OZ_BI_define(BIsiteStatistics,0,1)
{
  int i;
  OZ_Term sitelist = oz_nil(); 

  initDP();
  for (int i = primarySiteTable->getSize(); i--; ) {
    DSite *site = (DSite *) primarySiteTable->getFirstNode(i);
    while (site) {
      sitelist = oz_cons(site->getOzRep(), sitelist);
      site = site->getNext();
    }
  }

  for (int i = secondarySiteTable->getSize(); i--; ) {
    DSite *site = (DSite *) secondarySiteTable->getFirstNode(i);
    while (site) {
      sitelist = oz_cons(site->getOzRep(), sitelist);
      site = site->getNext();
    }
  }

  OZ_RETURN(sitelist);
} OZ_BI_end






OZ_Term makeMemRec(OZ_Term type, int size, int nr){
  return OZ_recordInit(oz_atom("mem"),
		       oz_cons(oz_pairA("type", type),
			       oz_cons(oz_pairAI("size",size),
				       oz_cons(oz_pairAI("nr",nr),oz_nil()))));
}


			       
OZ_BI_define(BI_DistMemInfo,0,1)
{
  initDP();
  int comObjNr, comObjSz, 
      transObjNr, transObjSz, 
      msgContainerNr, msgContainerSz,
      comObjUnused, transObjUnused, msgContainerUnused;
  
  comObjNr = getComControllerInfo(comObjSz);
  transObjNr = getTransControllerInfo(transObjSz);
  msgContainerNr = getMsgContainerManagerInfo(msgContainerSz);

  comObjUnused = getComControllerUnused();
  transObjUnused = getTransControllerUnused();
  msgContainerUnused = getMsgContainerManagerUnused();

  OZ_RETURN(oz_cons(makeMemRec(oz_atom("ComObjects"),comObjSz,comObjNr),
    	      oz_cons(makeMemRec(oz_atom("TransObjects"),transObjSz,
				 transObjNr),
		oz_cons(makeMemRec(oz_atom("MsgContainers"),msgContainerSz,
				   msgContainerNr),
	       oz_cons(makeMemRec(oz_atom("Unused ComObjects"),comObjSz,
				  comObjUnused),
	       oz_cons(makeMemRec(oz_atom("Unused TransObjects"),transObjSz,
				  transObjUnused),
	       oz_cons(makeMemRec(oz_atom("Unused MsgContainers"),msgContainerSz,
				  msgContainerUnused),
		       oz_nil())))))));
} OZ_BI_end


//
// The names from marshalBase is not printable. 
// A new set of names are defined here.
// If incompatibilites should ocour please update this
// array to the same number of entries as the master copy.
// Erik 
const struct {
  MarshalTag tag;
  char *name;
} dif_Mynames[] = {
  { DIF_UNUSED0,         "unused0"},
  { DIF_SMALLINT,        "smallint"},
  { DIF_BIGINT,          "bigint"},
  { DIF_FLOAT,           "float"},
  { DIF_ATOM_DEF,        "atom_def"},
  { DIF_NAME_DEF,        "name_def"},
  { DIF_UNIQUENAME_DEF,  "uniquename_def"},
  { DIF_RECORD_DEF,      "record_def"},
  { DIF_TUPLE_DEF,       "tuple_def"},
  { DIF_LIST_DEF,        "list_def"},
  { DIF_REF,             "ref"},
  { DIF_UNUSED1,         "unused1"},
  { DIF_OWNER_DEF,       "owner_def"},
  { DIF_UNUSED2,         "unused2"},
  { DIF_PORT_DEF,        "port_def"}, // 
  { DIF_CELL_DEF,        "cell_def"},
  { DIF_LOCK_DEF,        "lock_def"},
  { DIF_VAR_DEF,         "var_def"},
  { DIF_BUILTIN_DEF,     "builtin_def"},
  { DIF_DICT_DEF,        "dict_def"},
  { DIF_OBJECT_DEF,      "object_def"},
  { DIF_UNUSED3,         "unused3"},
  { DIF_UNUSED4,         "unused4"},
  { DIF_CHUNK_DEF,       "chunk_def"},
  { DIF_PROC_DEF,        "proc_def"},
  { DIF_CLASS_DEF,       "class_def"},
  { DIF_ARRAY_DEF,       "array_def"},
  { DIF_FSETVALUE,       "fsetvalue"},
  { DIF_ABSTRENTRY,      "abstrentry"},
  { DIF_UNUSED5,         "unused5"},
  { DIF_UNUSED6,         "unused6"},
  { DIF_SITE,            "site"},
  { DIF_UNUSED7,         "unused7"},
  { DIF_SITE_PERM,       "site_perm"},
  { DIF_UNUSED8,         "unused8"},
  { DIF_COPYABLENAME_DEF,"copyablename_def"},
  { DIF_EXTENSION_DEF,   "extension_def"},
  { DIF_RESOURCE_DEF,    "resource_def"},
  { DIF_RESOURCE,        "resource"},
  { DIF_FUTURE_DEF,      "future_def"},
  { DIF_VAR_AUTO_DEF,    "automatically_registered_var_def"},
  { DIF_FUTURE_AUTO_DEF, "automatically_registered_future_def"},
  { DIF_EOF,             "eof"},
  { DIF_CODEAREA,        "code_area_segment"},
  { DIF_VAR_OBJECT_DEF,  "var_object_exported_def"},
  { DIF_SYNC,            "sync"},
  { DIF_CLONEDCELL_DEF,  "clonedcell_def"},
  { DIF_STUB_OBJECT_DEF, "object_exported_def"},
  { DIF_SUSPEND,         "marshaling_suspended"},
  { DIF_LIT_CONT,        "dif_literal_continuation"},
  { DIF_EXT_CONT,        "dif_extension_continuation"},
  { DIF_SITE_SENDER,     "site_opt"},
  { DIF_RECORD,          "record"},
  { DIF_TUPLE,           "tuple"},
  { DIF_LIST,            "list"},
  { DIF_PORT,            "port"},
  { DIF_CELL,            "cell"},
  { DIF_LOCK,            "lock"},
  { DIF_BUILTIN,         "builtin"},
  { DIF_DICT,            "dict"},
  { DIF_OBJECT,          "object"},
  { DIF_CHUNK,           "chunk"},
  { DIF_PROC,	         "proc"},
  { DIF_CLASS,           "class"},
  { DIF_EXTENSION,       "extension"},
  { DIF_STUB_OBJECT,     "object_exported"},
  { DIF_BIGINT_DEF,      "bigint_def"},
  { DIF_CLONEDCELL,      "clonedcell"},
  { DIF_ARRAY,           "array"},
  { DIF_ATOM,            "atom"},
  { DIF_NAME,  	         "name"},
  { DIF_UNIQUENAME,      "uniquename"},
  { DIF_COPYABLENAME,    "copyablename"},
  { DIF_OWNER,           "owner"},
  { DIF_VAR,             "var"},
  { DIF_FUTURE,          "future"},
  { DIF_VAR_AUTO,        "automatically_registered_var"},
  { DIF_FUTURE_AUTO,     "automatically_registered_future"},
  { DIF_VAR_OBJECT,      "var_object_exported"},
  { DIF_LAST,            "last"}
};


OZ_BI_define(BIperdioStatistics,0,1)
{
  initDP();

  OZ_Term dif_send_ar=oz_nil();
  OZ_Term dif_recv_ar=oz_nil();
  int i;
  for (i=0; i<DIF_LAST; i++) {
    dif_send_ar=oz_cons(oz_pairAI(dif_Mynames[i].name,dif_counter[i].getSend()),
			dif_send_ar);
    dif_recv_ar=oz_cons(oz_pairAI(dif_Mynames[i].name,dif_counter[i].getRecv()),
			dif_recv_ar);
  }
  OZ_Term dif_send=OZ_recordInit(oz_atom("dif"),dif_send_ar);
  OZ_Term dif_recv=OZ_recordInit(oz_atom("dif"),dif_recv_ar);
  


  OZ_Term mess_send_ar=oz_nil();
  OZ_Term mess_recv_ar=oz_nil();
  for (i=0; i<M_LAST; i++) {
    mess_send_ar=oz_cons(oz_pairAI(mess_names[i],mess_counter[i].getSend()),
			 mess_send_ar);
    mess_recv_ar=oz_cons(oz_pairAI(mess_names[i],mess_counter[i].getRecv()),
			 mess_recv_ar);
  }
  OZ_Term mess_send=OZ_recordInit(oz_atom("messages"),mess_send_ar);
  OZ_Term mess_recv=OZ_recordInit(oz_atom("messages"),mess_recv_ar);


  
  OZ_Term send_ar=oz_nil();
  send_ar = oz_cons(oz_pairA("dif",dif_send),send_ar);
  send_ar = oz_cons(oz_pairA("messages",mess_send),send_ar);
  OZ_Term send=OZ_recordInit(oz_atom("send"),send_ar);

  OZ_Term recv_ar=oz_nil();
  recv_ar = oz_cons(oz_pairA("dif",dif_recv),recv_ar);
  recv_ar = oz_cons(oz_pairA("messages",mess_recv),recv_ar);
  OZ_Term recv=OZ_recordInit(oz_atom("recv"),recv_ar);

  
  OZ_Term ar=oz_nil();
  ar=oz_cons(oz_pairA("send",send),ar);
  ar=oz_cons(oz_pairA("recv",recv),ar);
  OZ_RETURN(OZ_recordInit(oz_atom("perdioStatistics"),ar));
} OZ_BI_end


#ifndef MODULES_LINK_STATIC

#include "modDPStatistics-if.cc"

#endif
