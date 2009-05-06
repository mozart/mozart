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
     # initialize the DP module.  The port in argument is used by the
     # Glue to request connections, etc.
     'initDP'                => { in  => ['+port'],
	                          out => [],
				  BI  => BIinitDP},

     # establish a connection to a site (1st arg) with the given pair
     # of file descriptors (2nd arg)
     'setConnection'         => { in  => ['+value','+int#+int'],
                                  out => [],
				  BI  => BIsetConnection},

     # establish an anonymous connection with the given socket
     # descriptor
     'acceptConnection'      => { in  => ['+int#+int'],
                                  out => [],
				  BI  => BIacceptConnection},

     # establish a DSS route (list of intermediate sites) to reach a
     # given site (2nd argument); not tested
     'handoverRoute'         => { in  => ['+[value]', '+value'],
                                  out => [],
				  BI  => BIhandoverRoute},

     # indicate that a connection could not be established for a given
     # site (1st arg).  The second argument states whether the
     # connection failure is permanent ('perm') or not ('temp').  The
     # failure is reported on the site's fault state.
     'connFailed'            => { in  => ['+value','+atom'],
				  out => [],
				  BI  => BIconnFailed},

     # force the fault state of a site to one of: ok, tempFail,
     # localFail, permFail.
     'setSiteState'          => { in  => ['+value','+atom'],
				  out => [],
				  BI  => BIsetSiteState},

     # defines the RPC wrapper used by the Glue.
     'setRPC'                => { in  => ['+procedure/3'],
                                  out => [],
                                  BI  => BIsetRPC},

     # return the annotation of a given entity (as a list)
     'getAnnotation'         => { in  => ['value'],
                                  out => ['+list'],
                                  BI  => BIgetAnnotation},

     # annotate an entity
     'annotate'              => { in  => ['value','+list'],
                                  out => [],
				  BI  => BIannotate},

     # return the fault stream of an entity
     'getFaultStream'        => { in  => ['value'],
				  out => ['list'],
				  BI  => BIgetFaultStream},

     # return the current fault state of an entity
     'getFaultState'         => { in  => ['value'],
				  out => ['+atom'],
				  BI  => BIgetFaultState},

     # force the fault state of an entity; use for debugging only
     'setFaultState'         => { in  => ['value','+atom'],
				  out => [],
				  BI  => BIsetFaultState},

     # kill an entity; the operation is asynchronous and not
     # guaranteed to succeed
     'kill'                  => { in  => ['value'],
				  out => [],
				  BI  => BIkill},

     # put the given entity in the fault state 'localFail' (unless it
     # is already permFail)
     'break'                 => { in  => ['value'],
				  out => [],
				  BI  => BIbreak},

     # attempt to migrate the coordinator of the given entity on the
     # current site
     'migrateManager'        => { in  => ['value'],
                                  out => [],
                                  BI  => BImigrateManager},

     # return the total number of sent and received messages.  The
     # output is a record of the form
     #
     #      msgStatistics(sent:      <number of messages sent>
     #                    received:  <number of messages received>
     #                    oswritten: <number of calls to write(2)>
     #                    osread:    <number of calls to read(2)>
     #                    cont:      <number of marshaler continuations>)
     #
     'getMsgCntr'            => { in  => [],
			          out => ['+record'],
				  BI  => BIgetMsgCntr},

     # print the mediator table, and DSS tables
     'printDPTables'	     => { in  => [],
				  out => [],
				  BI  => BIprintDPTables},

     # print the current memory usage of the DSS
     'printDssMemoryAllocation'	     => { in  => [],
					  out => [],
					  BI  => BIprintDssMemoryAllocation},

     # set the DSS log level (0: nothing, 1: normal, 2: important, 3:
     # behavior, 4: debug, 5: most, 6:too_much); see the internals of the DSS for
     # the exact meaning of these levels
     'setDssLogLevel'        => { in  => ['+int'],
			          out => [],
				  BI  => BIsetDssLogLevel},

     # define a file to send DSS log information; currently not implemented
     'createLogFile'         => { in  => ['+string'],
				  out => [],
				  BI  => BIcreateLogFile},

     # send a message (2nd arg) to a site (1st arg); the message is
     # delivered at the destination site (through the Oz port given to
     # initIPConnection)
     'sendSite'              => { in => ['+value', 'value'],
                                  out => [],  
				  BI => BIsendMsgToSite},

     # return the list of all currently known sites
     'getAllSites'           => { in => [],
                                  out => ['+[value]'],
				  BI => BIgetAllSites},

     # return the list of all sites currently connected to this site
     'getConSites'           => { in => [],
                                  out => ['+[value]'],
				  BI => BIgetConSites},

     # return the current site
     'getThisSite'           => { in => [],
                                  out => ['+value'],
				  BI => BIgetThisSite},

     # return site information (a byte string)
     'getSiteInfo'           => { in => ['+value'],
                                  out => ['+value'],
				  BI => BIgetSiteInfo},

     # set site information (info must be a virtual string)
     'setSiteInfo'           => { in => ['+value','+value'],
                                  out => [],
				  BI => BIsetSiteInfo},

     # return the connection status of the given site, as a record
     #
     #      cs(channel:       <whether a direct channel is used>
     #         circuit:       <whether a circuit is used>
     #         communicating: <whether currently communicating>)
     #
     'getChannelStatus'      => { in => ['+value'],
			          out => ['+value'],
				  BI => BIgetChannelStatus},

     # return a list of broadcast addresses available (used in module
     # Discovery)
     'getBroadcastAddresses' => { in  => [],
				  out => ['+[string]'],
				  bi  => BIgetBroadcastAddresses},

     # allow the broadcast messages for a given socket (module Discovery)
     'sockoptBroadcast'      => { in  => ['+int'],
				  out => [],
				  bi  => BIsockoptBroadcast},

    );
