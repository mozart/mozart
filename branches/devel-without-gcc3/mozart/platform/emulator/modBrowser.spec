###
### Authors:
###   Christian Schulte <schulte@ps.uni-sb.de>
###
### Copyright:
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

$module_init_fun_name = "browser_init";

%builtins_all =
(

    'getsBoundB'	=> { in  => ['value','value'],
			     out => [],
			     BI  => BIgetsBoundB},

    'addr'		=> { in  => ['value'],
			     out => ['int'],
			     BI  => BIaddr},

    'varSpace'		=> { in  => ['value'],
			     out => ['int'],
			     BI  => BIvarSpace},

    'recordCIsVarB'	=> { in  => ['value'],
			     out => ['bool'],
			     BI  => BIisRecordVarB},

    'chunkWidth'	=> { in  => ['+chunk'],
			     out => ['int'],
			     BI  => BIchunkWidth},

    'chunkArity'	=> { in  => ['+chunk'],
			     out => ['[feature]'],
			     BI  => BIchunkArityBrowser},

    'getTermSize'	=> { in  => ['value','+int','+int'],
			     out => ['int'],
			     BI  => BIgetTermSize},

    'procLoc'		=> { in  => ['value'],
			     out => ['atom','int','int'],
			     BI  => BIprocLoc},
    'shortName'		=> { in  => ['atom'],
			     out => ['atom'],
			     BI  => BIshortName},
 );
