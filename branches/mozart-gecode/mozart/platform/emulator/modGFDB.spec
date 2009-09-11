### -*- mode: perl -*-
###
### Authors:
###   Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
###
### Copyright:
###   Gustavo Gutierrez, 2008-2009
###
### Contributors:
###   Andres Barco <anfelbar@univalle.edu.co>
###   Gustavo A. Gomez Farhat <gafarhat@univalle.edu.co>
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

###$module_init_fun_name = "module_init_geintvar";

###$module_init_fun	 = "oz_init_module";

###$module_name		 = "GFD";

%builtins_all = 
(
 #
 # Built-ins
 #
 
 ## Variable declaration
#  'int' => { 
#      in  => ['+value'],
#      out => ['int'],
#      bi  => new_intvar,
#      fcp => ignore
#  },

    'int' => { 
	in  => ['+value', 'int'],
	out => [],
	bi  => BINewIntVar},
    
    'decl' => { 
	in  => ['int'],
	out => [],
	bi  => BIDeclIntVar},

 ## Variable type testing
 'is' => { 
     in  => ['+value'],
     out => ['bool'],
     bi  => intvar_is,
     fcp => ignore
 },
 
 ## misc. operations
 'inf' => { 
     in  => [],
     out => ['int'],
     bi  => int_inf
 },

 'sup' => { 
     in  => [],
     out => ['int'],
     bi  => int_sup
 },
 
 ## Variable reflection
 'reflect.min' => { 
     in  => ['*int'],
     out => ['+int'],
     bi  => intvar_getMin
 },
 
 'reflect.max' => { 
     in  => ['*int'],
     out => ['+int'],
     bi  => intvar_getMax
 },

 'reflect.size' => {
     in  => ['*int'],
     out => ['+int'],
     bi  => intvar_getSize
 },
 
 'reflect.domList' => { 
     in  => ['+value'],
     out => ['+value'],
     bi  => int_domList
 },

 'reflect.dom' => { 
     in  => ['+value'],
     out => ['+value'],
     bi  => int_dom
 },
 
 'reflect.nextLarger' => { 
     in  => ['+value','int'],
     out => ['int'],
     bi  => int_nextLarger
 },
 
 'reflect.nextSmaller' => { 
     in  => ['+value','int'],
     out => ['int'],
     bi  => int_nextSmaller
 },

 'reflect.nbProp' => {
     in  => ['+value'],
     out => ['int'],
     bi  => intvar_propSusp
 },
 
 'reflect.mid' => { 
     in  => ['*int'],
     out => ['+int'],
     bi  => intvar_getMid
 },

 'reflect.width' => {
     in  => ['*int'],
     out => ['+int'],
     bi  => intvar_getWidth
 },

 'reflect.regretMin' => {
     in  => ['*int'],
     out => ['+int'],
     bi  => intvar_getRegretMin
 },
 
 'reflect.regretMax' => {
     in  => ['*int'],
     out => ['+int'],
     bi  => intvar_getRegretMax
 }
 
);
