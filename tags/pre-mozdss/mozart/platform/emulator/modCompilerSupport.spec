### -*- perl -*-
###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###   Leif Kornstaedt <kornstae@ps.uni-sb.de>
###   Christian Schulte <schulte@ps.uni-sb.de>
###
### Copyright:
###   Denys Duchier, 1998
###   Leif Kornstaedt, 1998
###   Christian Schulte, 1998
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

$module_init_fun_name = "compiler_init";

%builtins_all =
(
    ##* Compiler Support

    'chunkArity'		=> { in  => ['+chunk'],
				     out => ['+[feature]'],
				     BI  => BIchunkArityCompiler},

    'featureLess'		=> { in  => ['+feature','+feature'],
				     out => ['+bool'],
				     BI  => BIfeatureLess},

    'concatenateAtomAndInt'	=> { in  => ['+atom','+int'],
				     out => ['+atom'],
				     BI  => BIconcatenateAtomAndInt},

    'isBuiltin'			=> { in  => ['+value'],
				     out => ['+bool'],
				     BI  => BIisBuiltin},

    'nameVariable'		=> { in  => ['value','+atom'],
				     out => [],
				     BI  => BInameVariable},

    'newCopyableName'		=> { in  => ['+atom'],
				     out => ['+literal'],
				     BI  => BInewCopyableName},

    'isCopyableName'		=> { in  => ['+value'],
				     out => ['+bool'],
				     BI  => BIisCopyableName},

    'isUniqueName'		=> { in  => ['+value'],
				     out => ['+bool'],
				     BI  => BIisUniqueName},

    'newProcedureRef'		=> { in  => [],
				     out => ['+foreignPointer'],
				     BI  => BInewProcedureRef},

    'newCopyableProcedureRef'	=> { in  => [],
				     out => ['+foreignPointer'],
				     BI  => BInewCopyableProcedureRef},

    'isCopyableProcedureRef'	=> { in  => ['+foreignPointer'],
				     out => ['+bool'],
				     BI  => BIisCopyableProcedureRef},

    'isLocalDet'                => { in  => ['+value'],
				     out => ['+bool'],
				     BI  => BIisLocalDet},

    ##* Assembler Support

    'getInstructionSizes'	=> { in  => [],
				     out => ['+record'],
				     BI  => BIgetInstructionSizes},

    'storeInstructions'	        => { in  => ['+int','+[value]','+[record]','+dictionary'],
				     out => ['+procedure/0'],
				     BI  => BIstoreInstructions},

 );
