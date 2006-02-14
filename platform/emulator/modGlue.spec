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
     'transferRespKBR'	     =>  { in  => ['value'],
		                  out => [],
		                  BI  => BIkbrTransferResp},

     'createDHT'          =>  { in  => ['+port'],
		                  out => [],
		                  BI  => BIcreateDHT},

     'createSiteRef'      =>  { in  => [],
		                out => ['+string'],
		                BI  => BIcreateSiteRef},


     'connectDHT'          =>  { in  => ['+port','+string'],
		                  out => [],
		                  BI  => BIconnectDHT},

     'insertDHTitem'             => { in  => ['+int', 'value'],
  			             out => [],	
			             BI  => BIinsertDHTitem},

     'lookupDHTitem'             => { in  => ['+int'],
  			             out => [],	
			             BI  => BIlookupDHTitem},



     'getCRC'	            =>  { in  => ['+virtualString'],
		                  out => ['+int'],
		                  BI  => BIgetCRC},

     'marshalPort'           => { in  => ['+port'],
				  out => ['+string'],
				  BI  => BIportToMS},


     'unmarshalPort'           => { in  => ['+string'],
				  out => ['+port'],
				  BI  => BImsToPort},


     'sendcp' 		=> { in  => ['+int', '+int', '+int', '+int', '+int'],
			     out => [],
			     BI  => BIsendCping},


     'sendmpp' 		=> { in  => ['+int', '+int', '+int', '+int', '+int'],
			     out => [],
			     BI  => BIsendMpongPL},

     'sendmpt' 		=> { in  => ['+int', '+int', '+int', '+int', '+int' ,'value'],
			     out => [],
			     BI  => BIsendMpongTerm},


     'getMsgCntr'       => { in  => [],
			     out => ['+record'],
			     BI  => BIgetMsgCntr},
     
     'getOperCntr'      => { in  => [],
			     out => ['+record'],
			     BI  => BIgetOperCntr},
     
     'initIPConnection'    => { in  => ['+int','+string','+atom','+value','+port', '+int'],
				out => ['+record'],
				BI  => BIinitIPConnection},

     'getBroadcastAddresses' => { in  => [],
				  out => ['+[string]'],
				  bi  => BIgetBroadcastAddresses},

     'sockoptBroadcast'      => { in  => ['+int'],
				  out => [],
				  bi  => BIsockoptBroadcast},

     'handover'              => { in => ['+value','+value'],
                                  out => [],
				  BI => BIhandover},

     'handoverRoute'         => { in => ['+[value]', '+value'],
                                  out => [],
				  BI => BIhandoverRoute},

     'connFailed'            => { in => ['+int','+atom'],
				  out => [],
				  BI  => BIconnFailed},

     'printDPTables'	     => { in  => [],
				  out => [],
				  BI  => BIprintDPTables},

     'printDssMemoryAllocation'	     => { in  => [],
					  out => [],
					  BI  => BIprintDssMemoryAllocation},

     'setDGC'	             => { in  => ['value', '+atom'],
				  out => ['bool'],
				  BI  => BIsetDGC},


     'getDGC'	             => { in  => ['value'],
				  out => ['+record'],
				  BI  => BIgetDGC},

     'getDGCAlgs'            => { in  => [],
				  out => ['+record'],
				  BI  => BIgetDGCAlgs},

     'getDGCAlgInfo'         => { in  => ['+atom'],
				  out => ['+record'],
				  BI  => BIgetDGCAlgInfo},

     'setDGCAlg'	     => { in  => ['+atom','+bool'],
				  out => [],
				  BI  => BIsetDGCAlg},

     'setDGCAlgProp'         => { in  => ['+atom','+atom','value'],
				  out => [],
				  BI  => BIsetDGCAlgProp},

     'getMsgPriority'        =>  { in  => [],
                                   out => ['+record'],
                                   BI  => BIgetMsgPriority},

     'setMsgPriority'        =>  { in  => ['+atom','+atom'],
                                   out => [],
                                   BI  => BIsetMsgPriority},

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

     'getMaxRtt'             => { in  => [],
                                  out => ['+int'],
                                  BI  => BIgetMaxRtt},

     'setMaxRtt'             => { in  => ['+int'],
                                  out => [],
                                  BI  => BIsetMaxRtt},

     'migrateManager'        =>  { in  => ['value'],
                                   out => [],
                                   BI  => BImigrateManager},

     'createLogFile'         => { in  => ['+string'],
				  out => [],
				  BI  => BIcreateLogFile},

     'siteStatistics'	=> { in  => [],
			     out => ['+[value]'],
			     BI  => BIsiteStatistics},

     'getTablesInfo'	=> { in  => [],
			     out => ['+[value]'],
			     BI  => BItablesExtract},

     'getNetInfo'	=> { in  => [],
			     out => ['+[value]'],
			     BI  => BI_DistMemInfo},

     'perdioStatistics'	=> { in  => [],
			     out => ['+record'],
			     BI  => BIperdioStatistics},
				  
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



