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
     'is'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisName},

     'new'		=> { in  => [],
			     out => ['+name'],
			     BI  => BInewName},

     'newUnique'	=> { in  => ['+atom'],
			     out => ['+name'],
			     BI  => BInewUniqueName},

     'newNamed'		=> { in  => ['+atom'],
			     out => ['+name'],
			     BI  => BInewNamedName},

     '<'		=> { in  => ['+name','+name'],
			     out => ['+bool'],
			     bi  => BInameLess},

     'hash'		=> { in  => ['+name'],
			     out => ['+int'],
			     bi  => BInameHash},

     'toString'		=> { in  => ['+name'],
			     out => ['+atom'],
			     bi  => BInameToString},
     );
1;;
