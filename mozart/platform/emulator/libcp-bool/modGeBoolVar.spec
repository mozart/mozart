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


$module_init_fun_name = "module_init_geboolvar";

$module_init_fun      = "oz_init_module";

$module_name          = "GBD";

%builtins_all = 
(
 #
 # Built-ins
 #
 
    'bool'		=> { in  => ['+value'],
			     out => ['int'],
			     bi  => new_boolvar,
                             fcp => ignore},

    'isVar'		=> { in  => ['+value'],
			     out => ['bool'],
			     bi  => boolvar_is,
                             fcp => ignore},

    'reflect.min'        => { in  => ['*int'],
			      out => ['+int'],
			      bi  => boolvar_getMin},
    
    'reflect.max'        => { in  => ['*int'],
			      out => ['+int'],
			      bi  => boolvar_getMax},

    'reflect.size'       => { in  => ['*int'],
			      out => ['+int'],
			      bi  => boolvar_getSize},


##Propagators

    'not'		 => { in  => ['+value','+value', 'int'],
			     out => [],
			     bi  => bool_not},
    'eq'		 => { in  => ['+value','+value', 'int'],
			     out => [],
			     bi  => bool_eq},
    'and'		 => { in  => ['+value','+value', '+value', 'int'],
			     out => [],
			     bi  => bool_and},
    'andA'		 => { in  => ['+value','+value','int'],
			     out => [],
			     bi  => bool_and_arr},
    'or'		 => { in  => ['+value','+value', '+value', 'int'],
			     out => [],
			     bi  => bool_or},
    'orA'		 => { in  => ['+value','+value','int'],
			     out => [],
			     bi  => bool_or_arr},
    'imp'		 => { in  => ['+value','+value', '+value', 'int'],
			     out => [],
			     bi  => bool_imp},
    'eqv'		 => { in  => ['+value','+value', '+value', 'int'],
			     out => [],
			     bi  => bool_eqv},
    'xor'		 => { in  => ['+value','+value', '+value', 'int'],
			     out => [],
			     bi  => bool_xor},
    'rel'		 => { in  => ['+value','int', '+value', 'int'],
			     out => [],
			     bi  => bool_rel},
    'linear'		 => { in  => ['+value','int', '+value', 'int'],
			     out => [],
			     bi  => bool_linear},


 );
