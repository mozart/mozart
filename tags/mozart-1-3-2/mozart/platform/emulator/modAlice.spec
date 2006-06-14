###
### Author:
###   Leif Kornstaedt <kornstae@ps.uni-sb.de>
###
### Copyright:
###   Leif Kornstaedt, 2001
###
### Last change:
###   $Date$ by $Author$
###   $Revision$
###
### This file is part of Mozart, an implementation of Oz 3:
###   http://www.mozart-oz.org
###
### See the file "LICENSE" or
###   http://www.mozart-oz.org/LICENSE.html
### for information on usage and redistribution
### of this file, and for a DISCLAIMER OF ALL
### WARRANTIES.
###

# -*-perl-*-

%builtins_all =
    (
     'rpc'	        => { in  => ['value','value','value'],
		             out => [],
		             bi  => BIaliceRPC},
     );
1;;
