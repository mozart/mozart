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


$module_init_fun_name = "gebvp_init";

$module_name          = "GBD";

%builtins_all = 
(
 #
 # Built-ins
 #
 

## Relation propagators

	'rel_BV_BT_BV_BV' => {in => ['+value','+value','+value','+value'],
	    out => [],
	    bi => bool_rel_BV_BT_BV_BV},

## Builtins Propagators

'gbd_rel_4' => { in => ['+value', '+value', '+value', '+value'],
  out=>[],
  bi => gbd_rel_4 }
,

'gbd_rel_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gbd_rel_5 }
,

'gbd_rel_6' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gbd_rel_6 }
,

'gbd_linear_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gbd_linear_5 }
,

'gbd_linear_6' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gbd_linear_6 }
,

'gbd_linear_7' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gbd_linear_7 },


##Propagators

    'not'		 => { in  => ['+value','+value', 'int'],
			     out => [],
			     bi  => bool_not},

##    'eq'		 => { in  => ['+value','+value', 'int'],
##			     out => [],
##			     bi  => bool_eq},
##  'andA'		 => { in  => ['+value','+value','int'],
##			     out => [],
##			     bi  => bool_and_arr},
##    'orA'		 => { in  => ['+value','+value','int'],
##			     out => [],
##			     bi  => bool_or_arr},
##    'rel'		 => { in  => ['+value','int', '+value', 'int'],
##			     out => [],
##			     bi  => bool_rel},
##    'linear'		 => { in  => ['+value','int', '+value', 'int'],
##			     out => [],
##			     bi  => bool_linear},
 );
