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
     'is'         	=> { in  => ['+value'],
		             out => ['+bool'],
	        	     bi  => BIisDictionary},

     'new'         	=> { in  => [],
		             out => ['+dictionary'],
	        	     BI  => BIdictionaryNew},

     'get'        	=> { in  => ['+dictionary','+feature'],
		             out => ['value'],
		             bi  => BIdictionaryGet},

     'condGet'          => { in  => ['+dictionary','+feature','value'],
		             out => ['value'],
		             bi  => BIdictionaryCondGet},

     'put'	        => { in  => ['+dictionary','+feature','value'],
		             out => [],
		             bi  => BIdictionaryPut},

     'exchangeFun'	=> { in  => ['+dictionary','+feature','value'],
		             out => ['value'],
		             bi  => BIdictionaryExchange},

     'condExchangeFun'	=> { in  => ['+dictionary','+feature','value','value'],
		             out => ['value'],
		             bi  => BIdictionaryCondExchange},

     'remove'        	=> { in  => ['+dictionary','+feature'],
		             out => [],
        		     bi  => BIdictionaryRemove},

     'removeAll'        => { in  => ['+dictionary'],
		             out => [],
		             BI  => BIdictionaryRemoveAll},

     'member'	        => { in  => ['+dictionary','+feature'],
		             out => ['+bool'],
		             bi  => BIdictionaryMember},

     'keys'             => { in  => ['+dictionary'],
		             out => ['+[feature]'],
		             BI  => BIdictionaryKeys},

     'entries'          => { in  => ['+dictionary'],
		             out => ['+[feature#value]'],
		             BI  => BIdictionaryEntries},

     'items'            => { in  => ['+dictionary'],
		             out => ['+[value]'],
		             BI  => BIdictionaryItems},

     'clone'            => { in  => ['+dictionary'],
		             out => ['+dictionary'],
		             BI  => BIdictionaryClone},

     'isEmpty'          => { in  => ['+dictionary'],
		             out => ['+bool'],
		             BI  => BIdictionaryIsEmpty},

     'markSafe'         => { in  => ['+dictionary'],
		             out => [],
		             BI  => BIdictionaryMarkSafe},

     'toRecord'         => { in  => ['+literal','+dictionary'],
		             out => ['+record'],
		             BI  => BIdictionaryToRecord},

     'waitOr'           => { in  => ['+dictionary'],
		             out => ['+feature'],
		             BI  => BIdictionaryWaitOr},

     );;
1;;
