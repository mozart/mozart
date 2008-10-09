###
### Authors:
###   Alberto Delgado <adelgado@cic.puj.edu.co> 
###
### Copyright:
###   Alberto Delgado, 2006-2007
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
 #
 # Built-ins
 #

## Distributor
    'distribute'    => { in =>  ['+int', '+int', '+value'],
        out => ['value'],
        bi =>  gbd_distribute }, 

 
    'bool'		=> { in  => ['+value'],
			     out => ['+int'],
			     bi  => new_boolvar,
                             fcp => ignore},

    'isVar'		=> { in  => ['+value'],
			     out => ['bool'],
			     bi  => boolvar_is,
                             fcp => ignore},

    'reflect.zero'        => { in  => ['*int'],
			      out => ['+int'],
			      bi  => boolvar_getZero},
    
    'reflect.one'        => { in  => ['*int'],
			      out => ['+int'],
			      bi  => boolvar_getOne},

    'reflect.size'       => { in  => ['*int'],
			      out => ['+int'],
			      bi  => boolvar_getSize}
 );
