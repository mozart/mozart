###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###   Christian Schulte <schulte@ps.uni-sb.de>
###
### Copyright:
###   Denys Duchier, 1998
###   Christian Schulte, 1998
###
### Last change:
###   $Date$ by $Author$
###   $Revision$
###
### This file is part of Mozart, an implementation 
### of Oz 3:
###    http://www.mozart-oz.org
###
### See the file "LICENSE" or
###    http://www.mozart-oz.org/LICENSE.html
### for information on usage and redistribution 
### of this file, and for a DISCLAIMER OF ALL 
### WARRANTIES.
###

# -*-perl-*-

%builtins_all =
    (
     'close' 		=> { in  => ['+int'],
			     out => [],
			     BI  => BIclose},

     'crash'		=> { in  => [],
			     out => [],
			     BI  => BIcrash,
			     doesNotReturn=>1},

     'getMsgCntr'       => { in  => [],
			     out => ['+record'],
			     BI  => BIgetMsgCntr},
     
     'dvset'		=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIdvset},

     'slowNet'           => { in  => ['+int', '+int'],
			      out => [],
			      bi  => BIslowNet},

     'initIPConnection'    => { in  => ['+record'],
				out => ['+record'],
				BI  => BIinitIPConnection},

     'getBroadcastAddresses' => { in  => [],
				  out => ['+[string]'],
				  bi  => BIgetBroadcastAddresses},

     'sockoptBroadcast'      => { in  => ['+int'],
				  out => [],
				  bi  => BIsockoptBroadcast},

     'getConnGrant'          => { in  => ['+value','+value','+bool','value'],
                                  out => [],
				  BI  => BIgetConnGrant},

     'freeConnGrant'         => { in  => ['+value','+value'],
                                  out => [],
				  BI  => BIfreeConnGrant},

     'handover'              => { in => ['+value','+value','+value'],
                                  out => [],
				  BI => BIhandover},

     'getConnectWstream'     => { in => [],
                                  out => ['value'],
				  BI => BIgetConnectWstream},
     
     'setListenPort'         => { in => ['+int','+string'],
				  out => [],
				  BI  => BIsetListenPort},

     'connFailed'            => { in => ['+int','+atom'],
				  out => [],
				  BI  => BIconnFailed},

     'printDPTables'	     => { in  => [],
				  out => [],
				  BI  => BIprintDPTables},

     'createLogFile'         => { in  => ['+string'],
				  out => [],
				  BI  => BIcreateLogFile}
);



