/* -*- C++ -*-
 *  Authors:
 *    Per Brand (perbrand@sics.se)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

message(M_PORT_SEND,Index,Term)
message(M_REMOTE_SEND,Index,String,Term)

message(M_ASK_FOR_CREDIT,Index,Site)
message(M_OWNER_CREDIT,Index,Credit)
message(M_OWNER_SEC_CREDIT,Site,Index,Credit)
message(M_BORROW_CREDIT,Site,Index,Credit)

message(M_REGISTER,Index,Site)
message(M_REDIRECT,Site,Index,Term)
message(M_ACKNOWLEDGE,Site,Index)
message(M_SURRENDER,Index,Site,Term)

message(M_CELL_LOCK_GET,Index,Site)
message(M_CELL_LOCK_FORWARD,Site,Index,Site2)
message(M_CELL_LOCK_DUMP,Index,Site)

message(M_CELL_CONTENTS,Site,Index,Term)
message(M_CELL_READ,Index,Site)
message(M_CELL_REMOTEREAD,Site,Index,Site2)
message(M_CELL_READANS,Site,Index,Term)

message(M_LOCK_TOKEN,Site,Index)

message(M_CELL_CANTPUT,Index,Site,Term)
message(M_LOCK_CANTPUT,Index,Site)
message(M_CHAIN_ACK,Index,Site)
message(M_CHAIN_QUESTION,Index,Site)
message(M_CHAIN_ANSWER,Index,Site,Index2)
message(M_ASK_ERROR,Index,Index2);
message(M_TELL_ERROR,Site,Index,Index2);

message(M_GET_OBJECT,Index,Site)
message(M_GET_OBJECTANDCLASS,Index,Site)
message(M_SEND_OBJECT,Site,Index,Object)
message(M_SEND_OBJECTANDCLASS,Site,Index,ObjectAndClass)

message(M_FILE,String,Term)
message(M_REGISTER_VS,Site)
message(M_INIT_VS,Term)
