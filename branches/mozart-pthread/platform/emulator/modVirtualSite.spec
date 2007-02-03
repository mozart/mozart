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

%builtins_all =
    (
     'newMailbox'	=> { in    => [],
			     out   => ['+string'],
			     BI    => BIVSnewMailbox},

     'initServer'	=> { in    => ['+string'],
			     out   => [],
			     BI    => BIVSinitServer},

     'removeMailbox'	=> { in    => ['+string'],
			     out   => [],
			     BI    => BIVSremoveMailbox},

     'processMailbox'	=> { in    => [],
			     out   => ['+bool'],
			     BI    => BIVSprocess_mailbox,
			     ifdef => DENYS_EVENTS },

     'processMessageQ'	=> { in    => [],
			     out   => ['+bool'],
			     BI    => BIVSprocess_messageq,
			     ifdef => DENYS_EVENTS },

     'processProbes'	=> { in    => [],
			     out   => ['+bool'],
			     BI    => BIVSprocess_probes,
			     ifdef => DENYS_EVENTS },

     'processGC'	=> { in    => [],
			     out   => ['+bool'],
			     BI    => BIVSprocess_gc,
			     ifdef => DENYS_EVENTS }
     
     );
