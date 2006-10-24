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
     'is'	=> { in  => ['+value'],
		     out => ['+bool'],
		     bi  => BIisNumber},

     'abs'	=> { in  => ['+number'],
		     out => ['+number'],
		     bi  => BIabs},

     '*'	=> { in  => ['+number','+number'],
		     out => ['+number'],
		     bi  => BImult},

     '-'	=> { in  => ['+number','+number'],
		     out => ['+number'],
		     bi  => BIminus},

     '+'	=> { in  => ['+number','+number'],
		     out => ['+number'],
		     bi  => BIplus},

     '~'	=> { in  => ['+number'],
		     out => ['+number'],
		     bi  => BIuminus},
     );
1;;
