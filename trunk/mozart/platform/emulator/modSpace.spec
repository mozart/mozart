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

$module_init_fun_name = "space_init";

%builtins_all =
    (
     'new'		=> { in  => ['+procedure/1'],
			     out => ['+space'],
			     BI  => BInewSpace},

     'is'		=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => BIisSpace},

     'ask'		=> { in  => ['+space'],
			     out => ['+tuple'],
			     BI  => BIaskSpace},

     'askVerbose'	=> { in  => ['+space'],
			     out => ['value'],
			     BI  => BIaskVerboseSpace},

     'merge'	        => { in  => ['+space'],
			     out => ['+value'],
			     BI  => BImergeSpace},

     'clone'	        => { in  => ['+space'],
			     out => ['+space'],
			     BI  => BIcloneSpace},

     'commit1'	        => { in  => ['+space','+int'],
			     out => [],
			     BI  => BIcommit1Space},

     'commit2'	        => { in  => ['+space','+int','+int'],
			     out => [],
			     BI  => BIcommit2Space},

     'commit'	        => { in  => ['+space','+value'],
			     out => [],
			     BI  => BIcommitSpace},

     'inject'	        => { in  => ['+space','+procedure/1'],
			     out => [],
			     BI  => BIinjectSpace},

     'kill'	        => { in  => ['+space'],
			     out => [],
			     BI  => BIkillSpace},

     'choose'           => { in  => ['+int'],
                             out => ['value'],
                             BI  => BIchooseSpace},

     'waitStable'       => { in  => [],
                             out => [],
                             BI  => BIwaitStableSpace},
                 
     );
1;;
