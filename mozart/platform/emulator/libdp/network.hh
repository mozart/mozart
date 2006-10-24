/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 *    Anna Neiderud (annan@sics.se)
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
ComObj* createComObj(DSite*);
void comController_startGCComObjs();
void comController_gcComObjs();
void comController_finishGCComObjs();

extern Bool ipIsbehindFW;

//
// Run this when starting;
void initNetwork();
// Do cleanups in debugmode
DebugCode(void exitNetwork();)

//
// Used by distpane

int getNORM_ComObj(ComObj*);
int getNOSM_ComObj(ComObj*);
int getLastRTT_ComObj(ComObj*);

int getComControllerInfo(int &size);
int getTransControllerInfo(int &size);
int getMsgContainerManagerInfo(int &size);

int getComControllerUnused();
int getTransControllerUnused();
int getMsgContainerManagerUnused();

//
// Used when "disconnecting" a site
int openclose(int Type);

//
// ShutDwn stuff
int startNiceClose();
int niceCloseProgress();

#endif // __NETWORK_HH



