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
     'new'		=> { in  => ['+procedure/1'],
			     out => ['+space'],
			     BI  => BInewSpace},

     'is'		=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => BIisSpace},

     'ask'		=> { in  => ['+space'],
			     out => ['+tuple'],
			     BI  => BIaskSpace},

     'askVerbose'	=> { in  => ['+space','!value'],
			     out => [],
			     BI  => BIaskVerboseSpace},

     'askUnsafe'	=> { in  => ['+space','value'],
			     out => [],
			     BI  => BIaskUnsafeSpace},

     'merge'	        => { in  => ['+space'],
			     out => ['+value'],
			     BI  => BImergeSpace},

     'clone'	        => { in  => ['+space'],
			     out => ['+space'],
			     BI  => BIcloneSpace},

     'commit'	        => { in  => ['+space','+value'],
			     out => [],
			     BI  => BIcommitSpace},

     'inject'	        => { in  => ['+space','+procedure/1'],
			     out => [],
			     BI  => BIinjectSpace},

     'choose'           => { in  => ['+int'],
                             out => ['value'],
                             BI  => BIregisterSpace},
     );
1;;
