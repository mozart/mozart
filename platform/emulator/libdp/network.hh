/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Konstantin Popov <kost@sics.se>
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

#ifndef __NETWORK_HH
#define __NETWORK_HH

#include "base.hh"
#include "dpBase.hh"
#include "msgType.hh"
#include "comm.hh"

#ifdef INTERFACE  
#pragma interface
#endif

class ComObj;
ComObj* createComObj(DSite*, int recCtr);
void comController_startGCComObjs();
void comController_gcComObjs();
void comController_finishGCComObjs();

//
// Run this when starting;
void initNetwork();


//
// Used by distpane

int getNORM_ComObj(ComObj*);
int getNOSM_ComObj(ComObj*);

int getComControllerInfo(int &size);
int getTransControllerInfo(int &size);
int getMsgContainerManagerInfo(int &size);

//
// Used when "disconnecting" a site
int openclose(int Type);

//
// ShutDwn stuff
int startNiceClose();
int niceCloseProgress();

#endif // __NETWORK_HH



