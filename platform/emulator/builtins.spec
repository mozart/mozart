# -*- perl -*-

$cmode='stat';

%builtins_all =
(
    #* Access to all of them: the Builtin 'builtin'

    'builtin'   => { in  => ['+virtualString','+int'],
                     out => ['+procedure'],
                     BI  => BIbuiltin,
                     native => false},
    #* NATIVE IS HERE ONLY TEMPORARYLY FALSE IN ORDER TO DEBUG THE SYSTEM!

    'BootManager' => { in     => ['+virtualString'],
                       out    => ['+record'],
                       BI     => BIBootManager,
                       native => true},

    #* NATIVE IS HERE ONLY TEMPORARYLY FALSE IN ORDER TO DEBUG THE SYSTEM!

    #* Core

    ##* Type Tests

    'IsNumber'          => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisNumberB,
                             native => false},

    'IsInt'             => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisIntB,
                             native => false},

    'IsFloat'           => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisFloatB,
                             native => false},

    'IsRecord'          => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisRecordB,
                             native => false},

    'IsTuple'           => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisTupleB,
                             native => false},

    'IsLiteral'         => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisLiteralB,
                             native => false},

    'IsLock'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisLockB,
                             native => false},

    'IsCell'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisCellB,
                             native => false},

    'IsPort'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisPortB,
                             native => false},

    'IsProcedure'       => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisProcedureB,
                             native => false},

    'IsName'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisNameB,
                             native => false},

    'IsAtom'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisAtomB,
                             native => false},

    'IsBool'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisBoolB,
                             native => false},

    'IsUnit'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisUnitB,
                             native => false},

    'IsChunk'           => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisChunkB,
                             native => false},

    'IsRecordC'         => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisRecordCB,
                             native => false},

    'IsObject'          => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisObjectB,
                             native => false},

    'IsDictionary'      => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisDictionary,
                             native => false},

    'IsArray'           => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisArray,
                             native => false},

    'IsChar'            => { in  => ['+value'],
                             out => ['+bool'],
                             BI  => BIcharIs,
                             native => false},

    'IsString'          => { in  => ['+value'],
                             out => ['+bool'],
                             BI  => BIisString,
                             native => false},

    'IsVirtualString'   => { in  => ['!+value'],
                             out => ['+bool'],
                             BI  => BIvsIs,
                             native => false},

    'IsFree'            => { in  => ['value'],
                             out => ['+bool'],
                             bi  => BIisFree,
                             native => false},

    'IsKinded'          => { in  => ['value'],
                             out => ['+bool'],
                             bi  => BIisKinded,
                             native => false},

    'IsDet'             => { in  => ['value'],
                             out => ['+bool'],
                             bi  => BIisDet,
                             native => false},

    'Type.ofValue'      => { in  => ['+value'],
                             out => ['+atom'],
                             bi  => BItermType,
                             native => false},

    'fdIs'              => { in  => ['*value','bool'],
                             out => [],
                             bi  => BIfdIs,
                             native => false},

    'fsIsVarB'          => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIfsIsVarB,
                             native => false},

    'fsIsValueB'        => { in  => ['+value','bool'],
                             out => [],
                             bi  => BIfsIsValueB,
                             native => false},


    ##* Type Conversion

    'AtomToString'      => { in  => ['+atom'],
                             out => ['+string'],
                             bi  => BIatomToString,
                             native => false},

    'StringToAtom'      => { in  => ['+string'],
                             out => ['+atom'],
                             BI  => BIstringToAtom,
                             native => false},

    'IntToFloat'        => { in  => ['+int'],
                             out => ['+float'],
                             bi  => BIintToFloat,
                             native => false},

    'FloatToInt'        => { in  => ['+float'],
                             out => ['+int'],
                             bi  => BIfloatToInt,
                             native => false},

    'IntToString'       => { in  => ['+int'],
                             out => ['+string'],
                             BI  => BIintToString,
                             native => false}, # new style builtin

    'FloatToString'     => { in  => ['+float'],
                             out => ['+string'],
                             BI  => BIfloatToString,
                             native => false},

    'StringToInt'       => { in  => ['+string'],
                             out => ['+int'],
                             BI  => BIstringToInt,
                             native => false},

    'StringToFloat'     => { in  => ['+string'],
                             out => ['+float'],
                             BI  => BIstringToFloat,
                             native => false},


    'String.isInt'      => { in  => ['+string'],
                             out => ['+bool'],
                             BI  => BIstringIsInt,
                             native => false},

    'String.isFloat'    => { in  => ['+string'],
                             out => ['+bool'],
                             BI  => BIstringIsFloat,
                             native => false},

    'String.isAtom'     => { in  => ['+string'],
                             out => ['+bool'],
                             BI  => BIstringIsAtom,
                             native => false},


    ##* Operations on different units

    ###* Numbers (arithmetics)

    '/'         => { in  => ['+float','+float'],
                     out => ['+float'],
                     bi  => BIfdiv,
                     native => false},

    '*'         => { in  => ['+number','+number'],
                     out => ['+number'],
                     bi  => BImult,
                     native => false},

    'div'       => { in  => ['+int','+int'],
                     out => ['+int'],
                     bi  => BIdiv,
                     native => false},

    'mod'       => { in  => ['+int','+int'],
                     out => ['+int'],
                     bi  => BImod,
                     native => false},

    '-'         => { in  => ['+number','+number'],
                     out => ['+number'],
                     bi  => BIminus,
                     native => false},

    '+'         => { in  => ['+number','+number'],
                     out => ['+number'],
                     bi  => BIplus,
                     native => false},

    'Max'       => { in  => ['+comparable','+comparable'],
                     out => ['+comparable'],
                     bi  => BImax,
                     native => false},

    'Min'       => { in  => ['+comparable','+comparable'],
                     out => ['+comparable'],
                     bi  => BImin,
                     native => false},

    '<'         => { in  => ['+comparable','+comparable'],
                     out => ['+bool'],
                     bi  => BIlessFun,
                     negated => '>=',
                     native => false},

    '=<'        => { in  => ['+comparable','+comparable'],
                     out => ['+bool'],
                     bi  => BIleFun,
                     negated => '>',
                     native => false},

    '>'         => { in  => ['+comparable','+comparable'],
                     out => ['+bool'],
                     bi  => BIgreatFun,
                     negated => '=<',
                     native => false},

    '>='        => { in  => ['+comparable','+comparable'],
                     out => ['+bool'],
                     bi  => BIgeFun,
                     negated => '<',
                     native => false},

    '~'         => { in  => ['+number'],
                     out => ['+number'],
                     bi  => BIuminus,
                     native => false},

    '+1'        => { in  => ['+int'],
                     out => ['+int'],
                     bi  => BIadd1,
                     native => false},

    '-1'        => { in  => ['+int'],
                     out => ['+int'],
                     bi  => BIsub1,
                     native => false},

    'Exp'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIexp,
                     native => false},

    'Log'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIlog,
                     native => false},

    'Sqrt'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIsqrt,
                     native => false},

    'Sin'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIsin,
                     native => false},

    'Asin'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIasin,
                     native => false},

    'Cos'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIcos,
                     native => false},

    'Acos'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIacos,
                     native => false},

    'Tan'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BItan,
                     native => false},

    'Atan'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIatan,
                     native => false},

    'Ceil'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIceil,
                     native => false},

    'Floor'     => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIfloor,
                     native => false},

    'Abs'       => { in  => ['+number'],
                     out => ['+number'],
                     bi  => BIabs,
                     native => false},

    'Round'     => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIround,
                     native => false},

    'Atan2'     => { in  => ['+float','+float'],
                     out => ['+float'],
                     bi  => BIatan2,
                     native => false},

    'fPow'      => { in  => ['+float','+float'],
                     out => ['+float'],
                     bi  => BIfPow,
                     native => false},

    ###* Array/Dictionaries

    'NewArray'          => { in  => ['+int','+int','value'],
                             out => ['+array'],
                             BI  => BIarrayNew,
                             native => false},

    'Array.high'        => { in  => ['+array'],
                             out => ['+int'],
                             bi  => BIarrayHigh,
                             native => false},

    'Array.low'         => { in  => ['+array'],
                             out => ['+int'],
                             bi  => BIarrayLow,
                             native => false},

    'Get'               => { in  => ['+array','+int'],
                             out => ['value'],
                             bi  => BIarrayGet,
                             native => false},

    'Put'               => { in  => ['+array','+int','value'],
                             out => [],
                             bi  => BIarrayPut,
                             native => false},


    'NewDictionary'     => { in  => [],
                             out => ['+dictionary'],
                             BI  => BIdictionaryNew,
                             native => false},

    'Dictionary.isEmpty'=> { in  => ['+dictionary'],
                             out => ['+bool'],
                             bi  => BIdictionaryIsMt,
                             native => false},

    'Dictionary.get'    => { in  => ['+dictionary','+feature'],
                             out => ['value'],
                             bi  => BIdictionaryGet,
                             native => false},

    'Dictionary.condGet'=> { in  => ['+dictionary','+feature','value'],
                             out => ['value'],
                             bi  => BIdictionaryCondGet,
                             native => false},

    'Dictionary.put'    => { in  => ['+dictionary','+feature','value'],
                             out => [],
                             bi  => BIdictionaryPut,
                             native => false},

    'Dictionary.condPut'=> { in  => ['+dictionary','+feature','value'],
                             out => [],
                             bi  => BIdictionaryCondPut,
                             native => false},

    'Dictionary.exchange'=> { in  => ['+dictionary','+feature','value',
                                      'value'],
                              out => [],
                              BI  => BIdictionaryExchange,
                              native => false},

    'Dictionary.condExchange' => { in  => ['+dictionary','+feature','value',
                                           'value','value'],
                                   out => [],
                                   BI  => BIdictionaryCondExchange,
                                   native => false},

    'Dictionary.remove' => { in  => ['+dictionary','+feature'],
                             out => [],
                             bi  => BIdictionaryRemove,
                             native => false},

    'Dictionary.removeAll'=> { in  => ['+dictionary'],
                               out => [],
                               BI  => BIdictionaryRemoveAll,
                               native => false},

    'Dictionary.member' => { in  => ['+dictionary','+feature'],
                             out => ['+bool'],
                             bi  => BIdictionaryMember,
                             native => false},

    'Dictionary.keys' => { in  => ['+dictionary'],
                           out => ['+[feature]'],
                           BI  => BIdictionaryKeys,
                           native => false},

    'Dictionary.entries' => { in  => ['+dictionary'],
                              out => ['+[feature#value]'],
                              BI  => BIdictionaryEntries,
                              native => false},

    'Dictionary.items' => { in  => ['+dictionary'],
                            out => ['+[value]'],
                            BI  => BIdictionaryItems,
                            native => false},

    'Dictionary.clone' => { in  => ['+dictionary'],
                            out => ['+dictionary'],
                            BI  => BIdictionaryClone,
                            native => false},

    'Dictionary.markSafe' => { in  => ['+dictionary'],
                               out => [],
                               BI  => BIdictionaryMarkSafe,
                               native => false},

    ###* Groups


    'addFastGroup'      => { in  => ['+value','value'],
                             out => ['value'],
                             BI  => BIaddFastGroup,
                             native => false},

    'delFastGroup'      => { in  => ['value'],
                             out => [],
                             BI  => BIdelFastGroup,
                             native => false},

    'getFastGroup'      => { in  => ['+value'],
                             out => ['+value'],
                             BI  => BIgetFastGroup,
                             native => false},

    'delAllFastGroup'   => { in  => ['+value'],
                             out => ['+value'],
                             BI  => BIdelAllFastGroup,
                             native => false},

    ###* Locks, Cells, Ports

    'NewLock'           => { in  => [],
                             out => ['+lock'],
                             BI  => BInewLock,
                             native => false},

    'Lock'              => { in  => ['+lock'],
                             out => [],
                             BI  => BIlockLock,
                             native => false},

    'Unlock'            => { in  => ['+lock'],
                             out => [],
                             BI  => BIunlockLock,
                             native => false},


    'NewPort'           => { in  => ['value'],
                             out => ['+port'],
                             BI  => BInewPort,
                             native => false},

    'Send'              => { in  => ['+port','value'],
                             out => [],
                             BI  => BIsendPort,
                             native => false},

    'NewCell'           => { in  => ['value'],
                             out => ['+cell'],
                             BI  => BInewCell,
                             native => false},

    'Exchange'          => { in  => ['+cell','value','value'],
                             out => [],
                             bi  => BIexchangeCell,
                             native => false},

    'Access'            => { in  => ['+cell'],
                             out => ['value'],
                             bi  => BIaccessCell,
                             native => false},

    'Assign'            => { in  => ['+cell','value'],
                             out => [],
                             bi  => BIassignCell,
                             native => false},

    ###* Characters

    'Char.isAlNum'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsAlNum,
                             native => false},

    'Char.isAlpha'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsAlpha,
                             native => false},

    'Char.isCntrl'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsCntrl,
                             native => false},

    'Char.isDigit'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsDigit,
                             native => false},

    'Char.isGraph'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsGraph,
                             native => false},

    'Char.isLower'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsLower,
                             native => false},

    'Char.isPrint'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsPrint,
                             native => false},

    'Char.isPunct'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsPunct,
                             native => false},

    'Char.isSpace'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsSpace,
                             native => false},

    'Char.isUpper'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsUpper,
                             native => false},

    'Char.isXDigit'     => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsXDigit,
                             native => false},

    'Char.toLower'      => { in  => ['+char'],
                             out => ['+char'],
                             BI  => BIcharToLower,
                             native => false},

    'Char.toUpper'      => { in  => ['+char'],
                             out => ['+char'],
                             BI  => BIcharToUpper,
                             native => false},

    'Char.toAtom'       => { in  => ['+char'],
                             out => ['+atom'],
                             BI  => BIcharToAtom,
                             native => false},

    'Char.type'         => { in  => ['+char'],
                             out => ['+atom'],
                             BI  => BIcharType,
                             native => false},

    ###* Tuples, Records, OFS


    'Adjoin'            => { in  => ['+record','+record'],
                             out => ['+record'],
                             bi  => BIadjoin,
                             native => false},

    'AdjoinList'        => { in  => ['+record','+[feature#value]'],
                             out => ['+record'],
                             BI  => BIadjoinList,
                             native => false},

    'record'            => { in  => ['+literal','+[feature#value]'],
                             out => ['+record'],
                             BI  => BImakeRecord,
                             native => false},

    'Arity'             => { in  => ['+record'],
                             out => ['+[feature]'],
                             bi  => BIarity,
                             native => false},

    'AdjoinAt'          => { in  => ['+record','+feature','value'],
                             out => ['+record'],
                             BI  => BIadjoinAt,
                             native => false},

    'MakeTuple'         => { in  => ['+literal','+int'],
                             out => ['+tuple'],
                             bi  => BItuple,
                             native => false},

    'Label'             => { in  => ['*recordC'],
                             out => ['+literal'],
                             bi  => BIlabel,
                             native => false},

    'hasLabel'          => { in  => ['value'],
                             out => ['+bool'],
                             bi  => BIhasLabel,
                             native => false},

    'TellRecord'        => { in  => ['+literal','record'],
                             out => [],
                             BI  => BIrecordTell,
                             native => false},

    'WidthC'            => { in  => ['*record','int'],
                             out => [],
                             BI  => BIwidthC,
                             native => false},

    'monitorArity'      => { in  => ['*recordC','value','[feature]'],
                             out => [],
                             BI  => BImonitorArity,
                             native => false},

    'tellRecordSize'    => { in  => ['+literal','+int','record'],
                             out => [],
                             BI  => BIsystemTellSize,
                             native => false},

    ###* Records and Chunks

    '.'                 => { in  => ['*recordCOrChunk','+feature'],
                             out => ['value'],
                             bi  => BIdot,
                             native => false},

    '^'                 => { in  => ['*recordCOrChunk','+feature'],
                             out => ['value'],
                             bi  => BIuparrowBlocking,
                             native => false},

    'HasFeature'        => { in  => ['*recordCOrChunk','+feature'],
                             out => ['+bool'],
                             bi  => BIhasFeatureB,
                             native => false},

    'CondSelect'        => { in  => ['*recordCOrChunk','+feature','value'],
                             out => ['value'],
                             bi  => BImatchDefault,
                             native => false},

    'Width'             => { in  => ['+record'],
                             out => ['+int'],
                             bi  => BIwidth,
                             native => false},

    ###* Chunks

    'NewChunk'          => { in  => ['+record'],
                             out => ['+chunk'],
                             BI  => BInewChunk,
                             native => false},

    ###* Names

    'NewName'           => { in  => [],
                             out => ['+name'],
                             BI  => BInewName,
                             native => false},

    'NewUniqueName'     => { in  => ['+atom'],
                             out => ['+name'],
                             BI  => BInewUniqueName,
                             native => false},

    ###* Procedures

    'ProcedureArity'    => { in  => ['+procedure'],
                             out => ['+int'],
                             bi  => BIprocedureArity,
                             native => false},

    ###* Functions

    'funReturn'         => { in  => ['value'],
                             out => [],
                             doesNotReturn => 1,
                             BI  => BIfunReturn,
                             native => false},

    'getReturn'         => { in  => [],
                             out => ['value'],
                             BI  => BIgetReturn,
                             native => false},


    ###* Object-Oriented Primitives

    '@'                 => { in  => ['value'],
                             out => ['value'],
                             bi  => BIat,
                             native => false},

    '<-'                => { in  => ['value','value'],
                             out => [],
                             bi  => BIassign,
                             native => false},

    'ooExch'            => { in  => ['value','value'],
                             out => ['value'],
                             bi  => BIexchange,
                             native => false},

    'copyRecord'        => { in  => ['+record'],
                             out => ['+record'],
                             BI  => BIcopyRecord,
                             native => false},

    'makeClass'         => { in  => ['+dictionary','+record','+record',
                                     '+dictionary','+bool','+bool'],
                             out => ['+class'],
                             BI  => BImakeClass,
                             native => false},

    ','                 => { in  => ['+class','+record'],
                             out => [],
                             bi  => BIcomma,
                             native => false},

    'send'              => { in  => ['+record','+class','+object'],
                             out => [],
                             bi  => BIsend,
                             native => false},

    'getClass'          => { in  => ['+object'],
                             out => ['+class'],
                             bi  => BIgetClass,
                             native => false},

    'ooGetLock'         => { in  => ['lock'],
                             out => [],
                             bi  => BIooGetLock,
                             native => false},

    'newObject'         => { in  => ['+class'],
                             out => ['+object'],
                             bi  => BInewObject,
                             native => false},

    'New'               => { in  => ['+class','+record','value'],
                             out => [],
                             bi  => BINew,
                             native => false},

    ###* Spaces

    'Space.new'         => { in  => ['+procedure/1'],
                             out => ['+space'],
                             BI  => BInewSpace,
                             native => false},

    'IsSpace'           => { in  => ['+value'],
                             out => ['+bool'],
                             BI  => BIisSpace,
                             native => false},

    'Space.ask'         => { in  => ['+space'],
                             out => ['+tuple'],
                             BI  => BIaskSpace,
                             native => false},

    'Space.askVerbose'  => { in  => ['+space','!value'],
                             out => [],
                             BI  => BIaskVerboseSpace,
                             native => false},

    'Space.merge'       => { in  => ['+space'],
                             out => ['+value'],
                             BI  => BImergeSpace,
                             native => false},

    'Space.clone'       => { in  => ['+space'],
                             out => ['+space'],
                             BI  => BIcloneSpace,
                             native => false},

    'Space.commit'      => { in  => ['+space','+value'],
                             out => [],
                             BI  => BIcommitSpace,
                             native => false},

    'Space.inject'      => { in  => ['+space','+procedure/1'],
                             out => [],
                             BI  => BIinjectSpace,
                             native => false},


    ###* Threads

    'Thread.is'         => { in  => ['+value'],
                             out => ['+bool'],
                             BI  => BIthreadIs,
                             native => false},

    'Thread.id'         => { in  => ['+thread'],
                             out => ['+int'],
                             BI  => BIthreadID,
                             native => false},

    'Thread.setId'      => { in  => ['+thread','+int'],
                             out => [],
                             BI  => BIsetThreadID,
                             native => false},

    'Thread.parentId'   => { in  => ['+thread'],
                             out => ['+int'],
                             BI  => BIparentThreadID,
                             native => false},

    'Thread.this'       => { in  => [],
                             out => ['+thread'],
                             BI  => BIthreadThis,
                             native => false},

    'Thread.suspend'    => { in  => ['+thread'],
                             out => [],
                             BI  => BIthreadSuspend,
                             native => false},

    'Thread.unleash'    => { in  => ['+thread','+int'],
                             out => [],
                             BI  => BIthreadUnleash,
                             native => false},

    'Thread.resume'     => { in  => ['+thread'],
                             out => [],
                             BI  => BIthreadResume,
                             native => false},

    'Thread.injectException'=> { in  => ['+thread','+value'],
                                 out => [],
                                 BI  => BIthreadRaise,
                                 native => false},

    'Thread.preempt'    => { in  => ['+thread'],
                             out => [],
                             BI  => BIthreadPreempt,
                             native => false},

    'Thread.setPriority'=> { in  => ['+thread','+atom'],
                             out => [],
                             BI  => BIthreadSetPriority,
                             native => false},

    'Thread.getPriority'=> { in  => ['+thread'],
                             out => ['+atom'],
                             BI  => BIthreadGetPriority,
                             native => false},

    'Thread.isSuspended'=> { in  => ['+thread'],
                             out => ['+bool'],
                             BI  => BIthreadIsSuspended,
                             native => false},

    'Thread.state'      => { in  => ['+thread'],
                             out => ['+atom'],
                             BI  => BIthreadState,
                             native => false},

    'Thread.setRaiseOnBlock'=> { in  => ['+thread','+bool'],
                                 out => [],
                                 BI  => BIthreadSetRaiseOnBlock,
                                 native => false},

    'Thread.getRaiseOnBlock'=> { in  => ['+thread'],
                                 out => ['+bool'],
                                 BI  => BIthreadGetRaiseOnBlock,
                                 native => false},

    'Thread.taskStack'  => { in  => ['+thread','+int','+bool'],
                             out => ['+[record]'],
                             BI  => BIthreadTaskStack,
                             native => false},

    'Thread.taskStackError' => { in  => ['+thread','+bool'],
                             out => ['+[record]'],
                             BI  => BIthreadTaskStackError,
                             native => false},

    'Thread.frameVariables'=> { in  => ['+thread','+int'],
                                out => ['+record'],
                                BI  => BIthreadFrameVariables,
                                native => false},

    'Thread.location'   => { in  => ['+thread'],
                             out => ['+[atom]'],
                             BI  => BIthreadLocation,
                             native => false},


    'Thread.create'    => { in  => ['+procedure'],
                            out => [],
                            BI  => BIthreadCreate,
                            native => false},

    ###* Foreign Pointers

    'isForeignPointer'  => { in  => ['+value'],
                             out => ['+bool'],
                             BI  => BIisForeignPointer ,
                             native => false},

    'ForeignPointerToInt'=> { in  => ['+foreignPointer'],
                              out => ['+int'],
                              BI  => BIForeignPointerToInt,
                              native => false},


    ###* Bit Arrays

    'BitArray.new'      => { in  => ['+int','+int'],
                             out => ['+bitArray'],
                             BI  => BIbitArray_new,
                             native => false},

    'BitArray.is'       => { in  => ['+value'],
                             out => ['+bool'],
                             BI  => BIbitArray_is,
                             native => false},

    'BitArray.set'      => { in  => ['+bitArray','+int'],
                             out => [],
                             BI  => BIbitArray_set,
                             native => false},

    'BitArray.clear'    => { in  => ['+bitArray','+int'],
                             out => [],
                             BI  => BIbitArray_clear,
                             native => false},

    'BitArray.test'     => { in  => ['+bitArray','+int'],
                             out => ['+bool'],
                             BI  => BIbitArray_test,
                             native => false},

    'BitArray.low'      => { in  => ['+bitArray'],
                             out => ['+int'],
                             BI  => BIbitArray_low,
                             native => false},

    'BitArray.high'     => { in  => ['+bitArray'],
                             out => ['+int'],
                             BI  => BIbitArray_high,
                             native => false},

    'BitArray.clone'    => { in  => ['+bitArray'],
                             out => ['+bitArray'],
                             BI  => BIbitArray_clone,
                             native => false},

    'BitArray.or'       => { in  => ['+bitArray','+bitArray'],
                             out => [],
                             BI  => BIbitArray_or,
                             native => false},

    'BitArray.and'      => { in  => ['+bitArray','+bitArray'],
                             out => [],
                             BI  => BIbitArray_and,
                             native => false},

    'BitArray.card'     => { in  => ['+bitArray'],
                             out => ['+int'],
                             BI  => BIbitArray_card,
                             native => false},

    'BitArray.disjoint' => { in  => ['+bitArray','+bitArray'],
                             out => ['+bool'],
                             BI  => BIbitArray_disjoint,
                             native => false},

    'BitArray.nimpl'    => { in  => ['+bitArray','+bitArray'],
                             out => [],
                             BI  => BIbitArray_nimpl,
                             native => false},

    'BitArray.toList'   => { in  => ['+bitArray'],
                             out => ['+[int]'],
                             BI  => BIbitArray_toList,
                             native => false},

    'BitArray.complementToList' => { in  => ['+bitArray'],
                                     out => ['+[int]'],
                                     BI  => BIbitArray_complementToList,
                                     native => false},


    ##* Misc Operations
    ###* Equalities

    '=='                => { in  => ['*value','*value'],
                             out => ['+bool'],
                             bi  => BIeqB,
                             negated => '\\\\=',
                             native => false},

    '\\\\='             => { in  => ['*value','*value'],
                             out => ['+bool'],
                             bi  => BIneqB,
                             negated => '==',
                             native => false},

    ###* Other Misc Operations

    'Wait'              => { in  => ['+value'],
                             out => [],
                             bi  => BIisValue,
                             native => false},

    'WaitOr'            => { in  => ['value','value'],
                             out => [],
                             BI  => BIwaitOr,
                             native => false},

    'virtualStringLength'=> { in  => ['!virtualString','!+int'],
                              out => ['+int'],
                              BI  => BIvsLength,
                              native => false},

    'Length'            => { in  => ['+[value]'],
                             out => ['+int'],
                             BI  => BIlength,
                             native => false},

    'Not'               => { in  => ['+bool'],
                             out => ['+bool'],
                             bi  => BInot,
                             native => false},

    'And'               => { in  => ['+bool','+bool'],
                             out => ['+bool'],
                             bi  => BIand,
                             native => false},

    'Or'                => { in  => ['+bool','+bool'],
                             out => ['+bool'],
                             bi  => BIor,
                             native => false},

    'Value.status'      => { in  => ['value'],
                             out => ['+tuple'],
                             bi  => BIstatus,
                             native => false},

    ##* Exceptions

    'biExceptionHandler'=> { in  => ['value'],
                             out => [],
                             BI  => BIbiExceptionHandler,
                             native => true},

    'setDefaultExceptionHandler'=> { in  => ['+procedure/1'],
                                     out => [],
                                     BI  => BIsetDefaultExceptionHandler,
                                     native => true},

    'getDefaultExceptionHandler'=> { in  => [],
                                     out => ['+procedure/1'],
                                     BI  => BIgetDefaultExceptionHandler,
                                     native => true},

    'raise'             => { in  => ['value'],
                             out => [],
                             BI  => BIraise,
                             doesNotReturn => 1,
                             native => false},

    'raiseError'        => { in  => ['value'],
                             out => [],
                             BI  => BIraiseError,
                             doesNotReturn => 1,
                             native => false},

    'raiseDebug'        => { in  => ['value'],
                             out => [],
                             BI  => BIraiseDebug,
                             doesNotReturn => 1,
                             native => false},

    'Exception.raiseDebugCheck' => { in  => ['value'],
                                        out => ['+bool'],
                                        BI  => BIraiseDebugCheck,
                                        native => false},

    ##* Diffent Kinds of Special Variables

    ##* Statistics

    'statisticsPrint'   => { in  => ['+virtualString'],
                             out => [],
                             BI  => BIstatisticsPrint,
                             native => true},

    'statisticsPrintProcs'=> { in  => [],
                               out => [],
                               BI  => BIstatisticsPrintProcs,
                               native => true},

    'instructionsPrint' => { in  => [],
                             out => [],
                             BI  => BIinstructionsPrint,
                             ifdef=>'PROFILE_INSTR',
                             native => true},

    'instructionsPrintCollapsable' => { in  => [],
                             out => [],
                             BI  => BIinstructionsPrintCollapsable,
                             ifdef=>'PROFILE_INSTR',
                             native => true},

    'instructionsPrintReset' => { in  => [],
                             out => [],
                             BI  => BIinstructionsPrintReset,
                             ifdef=>'PROFILE_INSTR',
                             native => true},

    'biPrint'           => { in  => [],
                             out => [],
                             BI  => BIbiPrint,
                             ifdef=>'PROFILE_BI',
                             native => true},

    'halt'              => { in  => [],
                             out => [],
                             BI  => BIhalt,
                             ifdef=>'DEBUG_TRACE',
                             native => true},

    ##* System Printing Primitives

    'UnSitedPrintName'  => { in  => ['value'],
                             out => ['+atom'],
                             BI  => BIgetPrintName,
                             native => false},

    ##* Browser Support

    'getsBoundB'        => { in  => ['value','value'],
                             out => [],
                             BI  => BIgetsBoundB,
                             native => false},

    'addr'              => { in  => ['value'],
                             out => ['+int'],
                             BI  => BIaddr,
                             native => false},

    'recordCIsVarB'     => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIisRecordCVarB,
                             native => false},

    'deepFeed'          => { in  => ['+cell','value'],
                             out => [],
                             BI  => BIdeepFeed,
                             native => false},

    'chunkWidth'        => { in  => ['+chunk'],
                             out => ['+int'],
                             BI  => BIchunkWidth,
                             native => false},

    'getTermSize'       => { in  => ['value','+int','+int'],
                             out => ['+int'],
                             BI  => BIgetTermSize,
                             native => false},

    ##* Misc. System Procs

    'Alarm'             => { in  => ['+int','unit'],
                             out => [],
                             BI  => BIalarm,
                             native => false},

    'Delay'             => { in  => ['!+int'],
                             out => [],
                             BI  => BIdelay,
                             native => false},

    'Time.time'         => { in  => [],
                             out => ['+int'],
                             BI  => BItimeTime,
                             native => false},


    '='                 => { in  => ['value','value'],
                             out => [],
                             BI  => BIunify,
                             native => false},

    'fail'              => { in  => [],
                             out => [],
                             BI  => BIfail,
                             native => false},

    'nop'               => { in  => [],
                             out => [],
                             BI  => BInop,
                             native => false},

    'onToplevel'        => { in  => [],
                             out => ['+bool'],
                             BI  => BIonToplevel,
                             native => false},

    'getConstraints'    => { in  => ['+value','+[value]'],
                             out => [],
                             bi  => BIgetConstraints,
                             native => true},


    #* Dynamic Linking

    'dlOpen'            => { in  => ['+virtualString'],
                             out => ['+foreignPointer'],
                             BI  => BIdlOpen,
                             native => true},

    'dlClose'           => { in  => ['+foreignPointer'],
                             out => [],
                             BI  => BIdlClose,
                             native => true},

    'findFunction'      => { in  => ['+virtualString','+int',
                                     '+foreignPointer'],
                             out => [],
                             BI  => BIfindFunction,
                             native => true},

    'dlLoad'            => { in  => ['+virtualString'],
                             out => ['+foreignPointer#record'],
                             BI  => BIdlLoad,
                             native => true},


    #* Distribution

    'export'            => { in  => ['value'],
                             out => [],
                             BI  => BIexport,
                             module=>components,
                             native => false},

    'PerdioVar.is'      => { in  => ['value'],
                             out => ['+bool'],
                             BI  =>   PerdioVar_is,
                             module=> 'perdiovar',
                             native => false},

    'probe'             => { in  => ['value'],
                             out => [],
                             BI  => BIprobe,
                             native => true},

    'crash'             => { in  => [],
                             out => [],
                             BI  => BIcrash,
                             doesNotReturn=>1,
                             native => true},

    'installHW'         => { in  => ['value','value','value'],
                             out => [],
                             BI  => BIhwInstall,
                             native => true},

    'deInstallHW'       =>  { in  => ['value','value','value'],
                             out => [],
                             BI  => BIhwDeInstall,
                             native => true},



    'setNetBufferSize'  =>  { in  => ['+value'],
                             out => [],
                             BI  => BIsetNetBufferSize,
                             native => true},

    'getNetBufferSize'  =>  { in  => [],
                             out => ['value'],
                             BI  => BIgetNetBufferSize,
                             native => true},

    'getEntityCond'     =>  { in  => ['value'],
                             out => ['value'],
                             BI  => BIgetEntityCond,
                             native => true},



    'controlVarHandler' => { in  => ['+value'],
                             out => [],
                             BI  => BIcontrolVarHandler,
                             native => true},

    'dvset'             => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIdvset,
                             ifdef=>DEBUG_PERDIO,
                             module=>'perdio',
                             native => true},

    'tempSimulate'      => { in  => ['+int'],
                             out => ['+int'],
                             BI  => BIcloseCon,
                             module=>'perdio',
                             native => true},

    'startTmp'          => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIstartTmp,
                             module=>'perdio',
                             native => true},

    'siteStatistics'    => { in  => [],
                             out => ['+[value]'],
                             BI  => BIsiteStatistics,
                             module=>'perdio',
                             native => true},

    'printBorrowTable'  => { in  => [],
                             out => [],
                             BI  => BIprintBorrowTable,
                             module=>'perdio',
                             native => true},

    'printOwnerTable'   => { in  => [],
                             out => [],
                             BI  => BIprintOwnerTable,
                             module=>'perdio',
                             native => true},


    'portWait'         =>  { in  => ['+port','+int'],
                             out => [],
                             BI  => BIportWait,
                             module=>'perdio',
                             native => true},


    'perdioStatistics'  => { in  => [],
                             out => ['+record'],
                             BI  => BIperdioStatistics,
                             module=>'perdio' ,
                             native => true},


     'atRedo'           => { in  => ['+feature', 'value'],
                             out => [],
                             bi  => BIatRedo,
                             native => true},

    'slowNet'           => { in  => ['+int', '+int'],
                             out => [],
                             bi  => BIslowNet,
                             native => true},


    #* Virtual Sites

    'VirtualSite.newMailbox' => { in     => [],
                                  out    => ['+string'],
                                  BI     => BIVSnewMailbox,
                                  module => vs,
                                  native => true},

    'VirtualSite.initServer' => { in     => ['+string'],
                                  out    => [],
                                  BI     => BIVSinitServer,
                                  module => vs,
                                  native => true},

    'VirtualSite.removeMailbox' => { in     => ['+string'],
                                  out    => [],
                                  BI     => BIVSremoveMailbox,
                                  module => vs,
                                  native => true},

    #* Tools


    ##* Debugger


    ###* Debugger Internal

    'Debug.getStream'   => { in  => [],
                             out => ['value'],
                             BI  => BIgetDebugStream,
                             native => true},

    'Debug.setStepFlag' => { in  => ['+thread','+bool'],
                             out => [],
                             BI  => BIsetStepFlag,
                             native => true},

    'Debug.setTraceFlag'=> { in  => ['+thread','+bool'],
                             out => [],
                             BI  => BIsetTraceFlag,
                             native => true},

    'Debug.checkStopped'=> { in  => ['+thread'],
                             out => ['+bool'],
                             BI  => BIcheckStopped,
                             native => true},

    'Debug.print'       => { in  => ['value','+int'],
                             out => [],
                             BI  => BIdebugPrint,
                             ifdef=>'DEBUG_PRINT',
                             native => true},

    'Debug.printLong'   => { in  => ['value','+int'],
                             out => [],
                             BI  => BIdebugPrintLong,
                             ifdef=>'DEBUG_PRINT',
                             native => true},

    'procedureEnvironment'=> { in  => ['+procedure'],
                               out => ['+tuple'],
                               BI  => BIprocedureEnvironment,
                               native => true},

    'chunkArity'        => { in  => ['+chunk'],
                             out => ['+[feature]'],
                             BI  => BIchunkArity,
                             native => true},

    'Debug.inspect'     => { in  => ['value'],
                             out => ['+value'],
                             BI  => BIinspect,
                             native => true},

    ###* Debugger External

    'Debug.prepareDumpThreads'  => { in  => [],
                                     out => [],
                                     BI  => BIprepareDumpThreads,
                                     native => true},

    'Debug.dumpThreads' => { in  => [],
                             out => [],
                             BI  => BIdumpThreads,
                             native => true},

    'Debug.listThreads' => { in  => [],
                             out => ['+[thread]'],
                             BI  => BIlistThreads,
                             native => true},

    'Debug.breakpointAt'=> { in  => ['+atom','+int','+bool'],
                             out => ['+bool'],
                             BI  => BIbreakpointAt,
                             native => true},

    'Debug.breakpoint'  => { in  => [],
                             out => [],
                             BI  => BIbreakpoint,
                             native => true},

    'Debug.displayDef'  => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIdisplayDef,
                             native => true},

    'Debug.displayCode' => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIdisplayCode,
                             native => true},

    'Debug.procedureCode'=> { in  => ['+procedure'],
                              out => ['+int'],
                              BI  => BIprocedureCode,
                              native => true},

    'Debug.procedureCoord'=> { in  => ['+procedure'],
                               out => ['+record'],
                               BI  => BIprocedureCoord,
                               native => true},

    'Debug.livenessX'   => { in  => ['+int'],
                             out => ['+int'],
                             BI  => BIlivenessX,
                             native => true},


    #* Unclassified

    ##* Ozma

    'ozma_readProc'     => { in     => ['+virtualString'],
                             out    => ['+value'],
                             BI     => ozma_readProc,
                             ifdef  => MODULES_LINK_STATIC,
                             native => true},

    ###* ByNeed

    'ByNeed'            => { in  => ['value'],
                             out => ['value'],
                             BI  => BIbyNeed,
                             module=>'future',
                             native => false},

    ###
    ### Misc stuff
    ###

    ###
    ### Christian's private stuff
    ###

    'GetCloneDiff'      => { in  => ['+space'],
                             out => ['+value'],
                             BI  => BIgetCloneDiff,
                             ifdef=>'CS_PROFILE',
                             native => true},




);
