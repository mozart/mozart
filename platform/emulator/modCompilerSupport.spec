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
###   http://mozart.ps.uni-sb.de
###
### See the file "LICENSE" or
###   http://mozart.ps.uni-sb.de/LICENSE.html
### for information on usage and redistribution
### of this file, and for a DISCLAIMER OF ALL
### WARRANTIES.
###

%builtins_all =
(
    ##* Compiler Support

    'chunkArity'                => { in  => ['+chunk'],
                                     out => ['+[feature]'],
                                     BI  => BIchunkArityCompiler},

    'featureLess'               => { in  => ['+feature','+feature'],
                                     out => ['+bool'],
                                     BI  => BIfeatureLess},

    'concatenateAtomAndInt'     => { in  => ['+atom','+int'],
                                     out => ['+atom'],
                                     BI  => BIconcatenateAtomAndInt},

    'isBuiltin'                 => { in  => ['+value'],
                                     out => ['+bool'],
                                     BI  => BIisBuiltin},

    'nameVariable'              => { in  => ['value','+atom'],
                                     out => [],
                                     BI  => BInameVariable},

    'newNamedName'              => { in  => ['+atom'],
                                     out => ['+literal'],
                                     BI  => BInewNamedName},

    'newCopyableName'           => { in  => ['+atom'],
                                     out => ['+literal'],
                                     BI  => BInewCopyableName},

    'isCopyableName'            => { in  => ['+value'],
                                     out => ['+bool'],
                                     BI  => BIisCopyableName},

    'isUniqueName'              => { in  => ['+value'],
                                     out => ['+bool'],
                                     BI  => BIisUniqueName},

    'newPredicateRef'           => { in  => [],
                                     out => ['+foreignPointer'],
                                     BI  => BInewPredicateRef},

    'newCopyablePredicateRef'   => { in  => [],
                                     out => ['+foreignPointer'],
                                     BI  => BInewCopyablePredicateRef},

    'isCopyablePredicateRef'    => { in  => ['+foreignPointer'],
                                     out => ['+bool'],
                                     BI  => BIisCopyablePredicateRef},

    'isLocalDet'                => { in  => ['+value'],
                                     out => ['+bool'],
                                     BI  => BIisLocalDet},

    ##* Assembler Support

    'allocateCodeBlock'         => { in  => ['+int','+[value]'],
                                     out => ['+foreignPointer','+procedure/0'],
                                     BI  => BIallocateCodeBlock},

    'getOpcode'                 => { in  => ['+atom'],
                                     out => ['+int'],
                                     BI  => BIgetOpcode},

    'getInstructionSize'        => { in  => ['+atom'],
                                     out => ['+int'],
                                     BI  => BIgetInstructionSize},

    'addDebugInfo'              => { in  => ['+foreignPointer','+atom','+int'],
                                     out => [],
                                     BI  => BIaddDebugInfo},

    'storeOpcode'               => { in  => ['+foreignPointer','+int'],
                                     out => [],
                                     BI  => BIstoreOpcode},

    'storeNumber'               => { in  => ['+foreignPointer','+number'],
                                     out => [],
                                     BI  => BIstoreNumber},

    'storeLiteral'              => { in  => ['+foreignPointer','+literal'],
                                     out => [],
                                     BI  => BIstoreLiteral},

    'storeFeature'              => { in  => ['+foreignPointer','+feature'],
                                     out => [],
                                     BI  => BIstoreFeature},

    'storeConstant'             => { in  => ['+foreignPointer','value'],
                                     out => [],
                                     BI  => BIstoreConstant},

    'storeBuiltinname'          => { in  => ['+foreignPointer',
                                             '+virtualString'],
                                     out => [],
                                     BI  => BIstoreBuiltinname},

    'storeRegisterIndex'        => { in  => ['+foreignPointer','+int'],
                                     out => [],
                                     BI  => BIstoreRegisterIndex},

    'storeInt'                  => { in  => ['+foreignPointer','+int'],
                                     out => [],
                                     BI  => BIstoreInt},

    'storeLabel'                => { in  => ['+foreignPointer','+int'],
                                     out => [],
                                     BI  => BIstoreLabel},

    'storePredicateRef'         => { in  => ['+foreignPointer','+value'],
                                     out => [],
                                     BI  => BIstorePredicateRef},

    'storePredId'               => { in  => ['+foreignPointer','+atom',
                                             '+value','+record',
                                             '+value','+int'],
                                     out => [],
                                     BI  => BIstorePredId},

    'newHashTable'              => { in  => ['+foreignPointer','+int','+int'],
                                     out => ['+foreignPointer'],
                                     BI  => BInewHashTable},

    'storeHTScalar'             => { in  => ['+foreignPointer',
                                             '+foreignPointer',
                                             '+value','+int'],
                                     out => [],
                                     BI  => BIstoreHTScalar},

    'storeHTRecord'             => { in  => ['+foreignPointer',
                                             '+foreignPointer',
                                             '+literal','+value','+int'],
                                     out => [],
                                     BI  => BIstoreHTRecord},

    'storeRecordArity'          => { in  => ['+foreignPointer','+value'],
                                     out => [],
                                     BI  => BIstoreRecordArity},

    'storeGenCallInfo'          => { in  => ['+foreignPointer','+int',
                                             '+bool','+literal',
                                             '+bool','+value'],
                                     out => [],
                                     BI  => BIstoreGenCallInfo},

    'storeApplMethInfo'         => { in  => ['+foreignPointer',
                                             '+literal','+value'],
                                     out => [],
                                     BI  => BIstoreApplMethInfo},

    'storeGRegRef'              => { in  => ['+foreignPointer','+[tuple]'],
                                     out => [],
                                     BI  => BIstoreGRegRef},

    'storeLocation'             => { in  => ['+foreignPointer','+list#list'],
                                     out => [],
                                     BI  => BIstoreLocation},

    'storeCache'                => { in  => ['+foreignPointer','value'],
                                     out => [],
                                     BI  => BIstoreCache},

 );
