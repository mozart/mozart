/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
 * 
 *  Contributors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Copyright:
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

#ifndef __CONNECTION_HH
#define __CONNECTION_HH

class DSite;
class ComObj;
class TransController;

// Get a connection(=working transObj) for comObj
void doConnect(ComObj *comObj);
// The transController delivers a transObj(= a right to use a resource)
void transObjReady(ComObj *comObj, TransObj *transObj);
// The comObj hands back a transObj that it is done with
void handback(ComObj *comObj, TransObj *transObj);
// The comObj informs that it is now done and does not need the connection
// it was waiting for.
void comObjDone(ComObj *comObj);

void changeTCPLimitImpl();

// Used by dpMiscModule to set parameters such as what transport layer to use
void setIPAddress__(int adr);
int  getIPAddress();
void setIPPort__(int port);
int getIPPort();

#endif
