$cmode='dyn';

%builtins_all =
(
    ##* CompilerSupport

    'concatenateAtomAndInt' => { in  => ['+atom','+int'],
                                          out => ['+atom'],
                                          BI  => BIconcatenateAtomAndInt,
                                          native => true},

    'isBuiltin' => { in  => ['+value'],
                              out => ['+bool'],
                              BI  => BIisBuiltin,
                              native => true},

    'nameVariable' => { in  => ['value','+atom'],
                                 out => [],
                                 BI  => BInameVariable,
                                 native => true},

    'newNamedName' => { in  => ['+atom'],
                                 out => ['+literal'],
                                 BI  => BInewNamedName,
                                 native => true},

    'newCopyableName' => { in  => ['+atom'],
                                    out => ['+literal'],
                                    BI  => BInewCopyableName,
                                    native => true},

    'isCopyableName' => { in  => ['+value'],
                                   out => ['+bool'],
                                   BI  => BIisCopyableName,
                                   native => true},

    'isUniqueName' => { in  => ['+value'],
                                 out => ['+bool'],
                                 BI  => BIisUniqueName,
                                 native => true},

    'newPredicateRef' => { in  => [],
                                    out => ['+foreignPointer'],
                                    BI  => BInewPredicateRef,
                                    native => true},

    'newCopyablePredicateRef' => { in  => [],
                                            out => ['+foreignPointer'],
                                            BI  => BInewCopyablePredicateRef,
                                            native => true},

    'isCopyablePredicateRef' => { in  => ['+foreignPointer'],
                                           out => ['+bool'],
                                           BI  => BIisCopyablePredicateRef,
                                           native => true},
    ##* Assembler support

    'newCodeBlock'              => { in  => ['+int'],
                                             out => ['+foreignPointer'],
                                             BI  => BInewCodeBlock,
                                             native => true},

    'getOpcode'         => { in  => ['+atom'],
                                             out => ['+int'],
                                             BI  => BIgetOpcode,
                                             native => true},

    'getInstructionSize'        => { in  => ['+atom'],
                                             out => ['+int'],
                                             BI  => BIgetInstructionSize,
                                             native => true},

    'makeProc'          => { in  => ['+foreignPointer',
                                                     '+[value]'],
                                             out => ['+procedure/0'],
                                             BI  => BImakeProc,
                                             native => true},

    'addDebugInfo'              => { in  => ['+foreignPointer',
                                                     '+atom','+int'],
                                             out => [],
                                             BI  => BIaddDebugInfo,
                                             native => true},

    'storeOpcode'               => { in  => ['+foreignPointer','+int'],
                                             out => [],
                                             BI  => BIstoreOpcode,
                                             native => true},

    'storeNumber'               => { in  => ['+foreignPointer',
                                                     '+number'],
                                             out => [],
                                             BI  => BIstoreNumber,
                                             native => true},

    'storeLiteral'              => { in  => ['+foreignPointer',
                                                     '+literal'],
                                             out => [],
                                             BI  => BIstoreLiteral,
                                             native => true},

    'storeFeature'              => { in  => ['+foreignPointer',
                                                     '+feature'],
                                             out => [],
                                             BI  => BIstoreFeature,
                                             native => true},

    'storeConstant'             => { in  => ['+foreignPointer',
                                                     'value'],
                                             out => [],
                                             BI  => BIstoreConstant,
                                             native => true},

    'storeBuiltinname'  => { in  => ['+foreignPointer',
                                                     '+procedure'],
                                             out => [],
                                             BI  => BIstoreBuiltinname,
                                             native => true},

    'storeRegisterIndex'        => { in  => ['+foreignPointer','+int'],
                                             out => [],
                                             BI  => BIstoreRegisterIndex,
                                             native => true},

    'storeInt'          => { in  => ['+foreignPointer','+int'],
                                             out => [],
                                             BI  => BIstoreInt,
                                             native => true},

    'storeLabel'                => { in  => ['+foreignPointer','+int'],
                                             out => [],
                                             BI  => BIstoreLabel,
                                             native => true},

    'storePredicateRef' => { in  => ['+foreignPointer',
                                                     '+value'],
                                             out => [],
                                             BI  => BIstorePredicateRef,
                                             native => true},

    'storePredId'               => { in  => ['+foreignPointer','+atom',
                                                     '+value','+record',
                                                     '+value','+int'],
                                             out => [],
                                             BI  => BIstorePredId,
                                             native => true},

    'newHashTable'              => { in  => ['+foreignPointer','+int',
                                                     '+int'],
                                             out => ['+foreignPointer'],
                                             BI  => BInewHashTable,
                                             native => true},

    'storeHTScalar'             => { in  => ['+foreignPointer',
                                                     '+foreignPointer',
                                                     '+value','+int'],
                                             out => [],
                                             BI  => BIstoreHTScalar,
                                             native => true},

    'storeHTRecord'             => { in  => ['+foreignPointer',
                                                     '+foreignPointer',
                                                     '+literal','+value',
                                                     '+int'],
                                             out => [],
                                             BI  => BIstoreHTRecord,
                                             native => true},

    'storeRecordArity'  => { in  => ['+foreignPointer',
                                                     '+value'],
                                             out => [],
                                             BI  => BIstoreRecordArity,
                                             native => true},

    'storeGenCallInfo'  => { in  => ['+foreignPointer','+int',
                                                     '+bool','+literal',
                                                     '+bool','+value'],
                                             out => [],
                                             BI  => BIstoreGenCallInfo,
                                             native => true},

    'storeApplMethInfo' => { in  => ['+foreignPointer',
                                                     '+literal','+value'],
                                             out => [],
                                             BI  => BIstoreApplMethInfo,
                                             native => true},

    'storeGRegRef'              => { in  => ['+foreignPointer',
                                                     '+[tuple]'],
                                             out => [],
                                             BI  => BIstoreGRegRef,
                                             native => true},

    'storeLocation'             => { in  => ['+foreignPointer',
                                                     '+list#list'],
                                             out => [],
                                             BI  => BIstoreLocation,
                                             native => true},

    'storeCache'                => { in  => ['+foreignPointer',
                                                     'value'],
                                             out => [],
                                             BI  => BIstoreCache,
                                             native => true},


 );
