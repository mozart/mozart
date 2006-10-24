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
    'getGlobals'	=> { in  => ['+procedure'],
			     out => ['+tuple'],
			     BI  => BIgetGlobals },

    'threadUnleash'	=> { in  => ['+thread','+int'],
			     out => [],
			     BI  => BIthreadUnleash},

    'getStream'	        => { in  => [],
			     out => ['value'],
			     BI  => BIgetDebugStream},

    'setStepFlag'	=> { in  => ['+thread','+bool'],
			     out => [],
			     BI  => BIsetStepFlag},

    'setTraceFlag'	=> { in  => ['+thread','+bool'],
			     out => [],
			     BI  => BIsetTraceFlag},

    'breakpointAt'	=> { in  => ['+atom','+int','+bool'],
			     out => ['+bool'],
			     BI  => BIbreakpointAt},

    'breakpoint'	=> { in  => [],
			     out => [],
			     BI  => BIbreakpoint},

    'procedureCoord'	=> { in  => ['+procedure'],
			     out => ['+record'],
			     BI  => BIprocedureCoord},

    'getId'		=> { in  => ['+thread'],
			     out => ['+int'],
			     BI  => BIthreadID},

    'setId'	        => { in  => ['+thread','+int'],
			     out => [],
			     BI  => BIsetThreadID},

    'getParentId'	=> { in  => ['+thread'],
			     out => ['+int'],
			     BI  => BIparentThreadID},

    'setRaiseOnBlock'	=> { in  => ['+thread','+bool'],
			     out => [],
			     BI  => BIthreadSetRaiseOnBlock},

    'getRaiseOnBlock'	=> { in  => ['+thread'],
			     out => ['+bool'],
			     BI  => BIthreadGetRaiseOnBlock},

    'getTaskStack'	=> { in  => ['+thread','+int','+bool'],
			     out => ['+[record]'],
			     BI  => BIthreadTaskStack},

    'getFrameVariables'	=> { in  => ['+thread','+int'],
			     out => ['+record'],
			     BI  => BIthreadFrameVariables},
 );
