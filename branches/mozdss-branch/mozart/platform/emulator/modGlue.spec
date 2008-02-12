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
     'getCRC'	            =>  { in  => ['+virtualString'],
		                  out => ['+int'],
		                  BI  => BIgetCRC},

     'marshalPort'           => { in  => ['+port'],
				  out => ['+string'],
				  BI  => BIportToMS},

     'unmarshalPort'           => { in  => ['+string'],
				  out => ['+port'],
				  BI  => BImsToPort},

     'getMsgCntr'       => { in  => [],
			     out => ['+record'],
			     BI  => BIgetMsgCntr},

     'initIPConnection'    => { in  => ['+string','+int','+int','+port'],
				out => [],
				BI  => BIinitIPConnection},

     'getBroadcastAddresses' => { in  => [],
				  out => ['+[string]'],
				  bi  => BIgetBroadcastAddresses},

     'sockoptBroadcast'      => { in  => ['+int'],
				  out => [],
				  bi  => BIsockoptBroadcast},

     'setConnection'         => { in => ['+value','+int'],
                                  out => [],
				  BI => BIsetConnection},

     'acceptConnection'      => { in => ['+int'],
                                  out => [],
				  BI => BIacceptConnection},

     'handoverRoute'         => { in => ['+[value]', '+value'],
                                  out => [],
				  BI => BIhandoverRoute},

     'connFailed'            => { in => ['+value','+atom'],
				  out => [],
				  BI  => BIconnFailed},

     'setSiteState'          => { in => ['+value','+atom'],
				  out => [],
				  BI  => BIsetSiteState},

     'printDPTables'	     => { in  => [],
				  out => [],
				  BI  => BIprintDPTables},

     'printDssMemoryAllocation'	     => { in  => [],
					  out => [],
					  BI  => BIprintDssMemoryAllocation},

     'getRPC'                =>  { in  => [],
                                   out => ['value'],
                                   BI  => BIgetRPC},

     'getAnnotation'         =>  { in  => ['value'],
                                   out => ['+int','+int','+int'],
                                   BI  => BIgetAnnotation},

     'setAnnotation'         =>  { in  => ['value','+int','+int','+int'],
                                   out => [],
                                   BI  => BIsetAnnotation},

     'getFaultStream'        => { in  => ['value'],
				  out => ['list'],
				  BI  => BIgetFaultStream},

     'getFaultState'         => { in  => ['value'],
				  out => ['+atom'],
				  BI  => BIgetFaultState},

     'setFaultState'         => { in  => ['value','+atom'],
				  out => [],
				  BI  => BIsetFaultState},

     'kill'                  => { in  => ['value'],
				  out => [],
				  BI  => BIkill},

     'killLocal'             => { in  => ['value'],
				  out => [],
				  BI  => BIkillLocal},

     'migrateManager'        =>  { in  => ['value'],
                                   out => [],
                                   BI  => BImigrateManager},

     'createLogFile'         => { in  => ['+string'],
				  out => [],
				  BI  => BIcreateLogFile},

    'getEntityCond'	=>  { in  => ['value'],
			     out => ['value'],
			     BI  => BIgetEntityCond},

    'installFaultPort'	=>  { in  => ['+port'],
			     out => [],
			     BI  => BIinstallFaultPort},

    'distHandlerInstall'=>  { in  => ['value','+int','value'],
			     out => ['+bool'],
			     BI  => BIdistHandlerInstall},

    'distHandlerDeInstall'=>{ in  => ['value','+int','value'],
			     out => ['+bool'],
			     BI  => BIdistHandlerDeInstall},


    'setDssLogLevel'=>{ in  => ['+int'],
			     out => [],
			     BI  => BIsetDssLogLevel},


    'sendMsgToSite'=>{ in => ['+value', '+value'],
                       out => [],  
                       BI => BIsendMsgToSite},

    'getAllSites'=>{ in => [],
                     out => ['+[value]'],
                     BI => BIgetAllSites},

    'getConSites'=>{ in => [],
                     out => ['+[value]'],
                     BI => BIgetConSites},

    'getThisSite'=>{ in => [],
                     out => ['+value'],
                     BI => BIgetThisSite},

    'getSiteInfo'=>{ in => ['+value'],
                     out => ['+value'],
                     BI => BIgetSiteInfo},

    'getChannelStatus'=>{ in => ['+value'],
			  out => ['+value'],
			  BI => BIgetChannelStatus},
    
    );
