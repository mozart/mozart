###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###   Michael Mehl <mehl@dfki.de>
###   Christian Schulte <schulte@ps.uni-sb.de>
###
### Copyright:
###   Denys Duchier, 1998
###   Michael Mehl, 1998
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
     'is'           => { in  => ['+value'],
		         out => ['+bool'],
		         bi  => BIisArray},

     'new'          => { in  => ['+int','+int','value'],
		         out => ['+array'],
		         BI  => BIarrayNew},

     'high'         => { in  => ['+array'],
		         out => ['+int'],
		         bi  => BIarrayHigh},

     'low'          => { in  => ['+array'],
		         out => ['+int'],
		         bi  => BIarrayLow},

     'get'          => { in  => ['+array','+int'],
		         out => ['value'],
		         bi  => BIarrayGet},

     'put'          => { in  => ['+array','+int','value'],
		         out => [],
		         bi  => BIarrayPut},

     'exchangeFun'  => { in  => ['+array','+int','value'],
		         out => ['value'],
		         bi  => BIarrayExchange}
     );
1;;
