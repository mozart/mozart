###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###   Christian Schulte <schulte@dfki.de>
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
###    http://mozart.ps.uni-sb.de
###
### See the file "LICENSE" or
###    http://mozart.ps.uni-sb.de/LICENSE.html
### for information on usage and redistribution 
### of this file, and for a DISCLAIMER OF ALL 
### WARRANTIES.
###

%builtins_all =
(

 'print'	        => { in     => ['value'],
			     out    => [],
			     bi     => BIprint},

 'show'		        => { in     => ['value'],
			     out    => [],
			     bi     => BIshow},
 
 'printName'	        => { in     => ['value'],
			     out    => ['+atom'],
			     BI     => BIgetPrintName},

 'printInfo'	        => { in     => ['virtualString'],
			     out    => [],
			     BI     => BIprintInfo},

 'printError'	        => { in     => ['virtualString'],
			     out    => [],
			     BI     => BIprintError},

 'showInfo'	        => { in     => ['virtualString'],
			     out    => [],
			     BI     => BIshowInfo},

 'showError'	        => { in     => ['virtualString'],
			     out    => [],
			     BI     => BIshowError},
 
 'valueToVirtualString' => { in     => ['value','+int','+int'],
			     out    => ['+string'],
			     BI     => BItermToVS},

 'exit'		        => { in  => ['+int'],
			     out => [],
			     BI  => BIshutdown,
			     doesNotReturn => 1},

 'gcDo'	                => { in  => [],
			     out => [],
			     BI  => BIgarbageCollection},
 
 'apply'	        => { in  => ['+procedureOrObject','+[value]'],
			     out => [],
			     BI  => BIapply},

 'eq'		        => { in  => ['value','value'],
			     out => ['+bool'],
			     BI  => BIsystemEq},
 
 'nbSusps'	        => { in  => ['value'],
			     out => ['+int'],
			     BI  => BIconstraints},

  'onToplevel'	        => { in  => [],
			     out => ['+bool'],
			     BI  => BIonToplevel},


 );
