### -*-perl-*-
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###
### Copyright:
###   Denys Duchier, 1998
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

#$module_init_fun_name = "ByteString_init";

%builtins_all =
    (

     'is'	=> { in  => ['+value'],
		     out => ['+bool'],
		     bi  => BIByteString_is},

     'make'	=> { in  => ['+string'],
		     out => ['+byteString'],
		     bi  => BIByteString_make},

     'get'	=> { in  => ['+byteString','+int'],
		     out => ['+int'],
		     bi  => BIByteString_get},

     'append'	=> { in  => ['+byteString','+byteString'],
		     out => ['+byteString'],
		     bi  => BIByteString_append},

     'slice'	=> { in  => ['+byteString','+int','+int'],
		     out => ['+byteString'],
		     bi  => BIByteString_slice},

     'width'	=> { in  => ['+byteString'],
		     out => ['+int'],
		     bi  => BIByteString_width},

     'toString'	=> { in  => ['+byteString'],
		     out => ['+string'],
		     bi  => BIByteString_toString},

     'toStringWithTail'
     =>		   { in  => ['+byteString','value'],
		     out => ['string'],
		     bi  => BIByteString_toStringWithTail},

     'strchr'	=> { in  => ['+byteString','+int','+int'],
		     out => ['+value'],
		     bi  => BIByteString_strchr},

     'cmp'	=> { in  => ['+byteString','+byteString'],
		     out => ['+int'],
		     bi  => BIByteString_cmp}
     );
1;;
