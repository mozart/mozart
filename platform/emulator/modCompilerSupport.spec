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

    'newNamedName'		=> { in  => ['+atom'],
				     out => ['+literal'],
				     BI  => BInewNamedName},

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

    'storeInstr'	        => { in  => ['+record','+foreignPointer','+dictionary'],
				     out => [],
				     BI  => BIstoreInstr},

    'allocateCodeBlock'		=> { in  => ['+int','+[value]'],
				     out => ['+foreignPointer','+procedure/0'],
				     BI  => BIallocateCodeBlock},

    'getOpcode'			=> { in  => ['+atom'],
				     out => ['+int'],
				     BI  => BIgetOpcode},

    'addDebugInfo'		=> { in  => ['+foreignPointer','+atom','+int'],
				     out => [],
				     BI  => BIaddDebugInfo},

    'storeOpcode'		=> { in  => ['+foreignPointer','+int'],
				     out => [],
				     BI  => BIstoreOpcode},

    'storeNumber'		=> { in  => ['+foreignPointer','+number'],
				     out => [],
				     BI  => BIstoreNumber},

    'storeLiteral'		=> { in  => ['+foreignPointer','+literal'],
				     out => [],
				     BI  => BIstoreLiteral},

    'storeFeature'		=> { in  => ['+foreignPointer','+feature'],
				     out => [],
				     BI  => BIstoreFeature},

    'storeConstant'		=> { in  => ['+foreignPointer','value'],
				     out => [],
				     BI  => BIstoreConstant},

    'storeBuiltinname'		=> { in  => ['+foreignPointer',
					     '+virtualString'],
				     out => [],
				     BI  => BIstoreBuiltinname},

    'storeXRegisterIndex'	=> { in  => ['+foreignPointer','+int'],
				     out => [],
				     BI  => BIstoreXRegisterIndex},

    'storeYRegisterIndex'	=> { in  => ['+foreignPointer','+int'],
				     out => [],
				     BI  => BIstoreYRegisterIndex},

    'storeGRegisterIndex'	=> { in  => ['+foreignPointer','+int'],
				     out => [],
				     BI  => BIstoreGRegisterIndex},

    'storeInt'			=> { in  => ['+foreignPointer','+int'],
				     out => [],
				     BI  => BIstoreInt},

    'storeLabel'		=> { in  => ['+foreignPointer','+int'],
				     out => [],
				     BI  => BIstoreLabel},

    'storeProcedureRef'		=> { in  => ['+foreignPointer','+value'],
				     out => [],
				     BI  => BIstoreProcedureRef},

    'storePredId'		=> { in  => ['+foreignPointer','+atom',
					     '+value','+record',
					     '+value','+int'],
				     out => [],
				     BI  => BIstorePredId},

    'newHashTable'		=> { in  => ['+foreignPointer','+int','+int','+value'],
				     out => [],
				     BI  => BInewHashTable},

    'storeRecordArity'		=> { in  => ['+foreignPointer','+value'],
				     out => [],
				     BI  => BIstoreRecordArity},

    'storeCallMethodInfo'	=> { in  => ['+foreignPointer','+int',
					     '+literal', '+bool','+value'],
				     out => [],
				     BI  => BIstoreCallMethodInfo},

    'storeGRegRef'		=> { in  => ['+foreignPointer','+[tuple]'],
				     out => [],
				     BI  => BIstoreGRegRef},

    'storeLocation'		=> { in  => ['+foreignPointer','+list#list'],
				     out => [],
				     BI  => BIstoreLocation},

    'storeCache'		=> { in  => ['+foreignPointer','value'],
				     out => [],
				     BI  => BIstoreCache},

 );
