### -*- perl -*-
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

$module_init_fun_name = "parser_init";

%builtins_all =
(
 'file'			=> { in  => ['+virtualString','+record'],
			     out => ['+value'],
			     bi  => parser_parseFile},

 'virtualString'	=> { in  => ['+virtualString','+record'],
			     out => ['+value'],
			     bi  => parser_parseVirtualString},

 'expandFileName'	=> { in  => ['+virtualString'],
			     out => ['+value'],
			     bi  => parser_expandFileName},
 );
