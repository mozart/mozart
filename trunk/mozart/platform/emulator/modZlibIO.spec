###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###
### Copyright:
###   Denys Duchier, 2003
###
### Last change:
###   $Date$
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
     'is'		=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => 'zlibio_is' },

     'new'		=> { in  => ['+int','+virtualString'],
			     out => ['+value'],
			     BI  => 'zlibio_new' },

     'close'		=> { in  => ['+value'],
			     out => [],
			     BI  => 'zlibio_close' },

     'readByteString'	=> { in  => ['+value','+int'],
			     out => ['+byteString','+int'],
			     BI  => 'zlibio_read_bytestring' },

     'read'		=> { in  => ['+value','+int'],
			     out => ['value','value','+int'],
			     BI  => 'zlibio_read' },

     'write'		=> { in  => ['+value','virtualString'],
			     out => ['+value'],
			     BI  => 'zlibio_write' },

     'flush'		=> { in  => ['+value','+int'],
			     out => [],
			     BI  => 'zlibio_flush' },
     );
