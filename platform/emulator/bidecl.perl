#! /usr/local/bin/perl
###
### Here we declare all builtins in a general format which can be used
### to generate both the table of builtins used by the emulator and the
### information used by the Oz compiler.
###
###     bidecl.perl -ctable
###             generates the table of builtins for the emulator
###     bidecl.perl -cdecl
###             generates the extern declarations for the above table
###     bidecl.perl -oztable
###             generates the table of builtins for the Oz compiler
###
### ADDITIONAL OPTIONS
###
###     -include M1,M2,...,Mn
###     -exclude M1,M2,...,Mn
###
###             include (resp. exclude) only these modules. by default
###     all modules are included.  Only one of these options may be
###     present (actually there can be several of them as long as they
###     are all -include or all -exclude.  Their arguments are unioned).
###
### Each entry has the form: 'NAME' => { ... }, where 'NAME' is the
### string by which the builtin is known to the emulator.  The associative
### array describing the builtin contains at least:
###     in  => [...],
###     out => [...],
### describing respectively the input and output arguments.  Each argument
### is described by a type, possibly annotated by a prefix indicating
### the determinacy condition.  Thus an integer argument might be specified
### in one of these ways:
###      'int'          possibly non-determined
###     '+int'          determined
###     '*int'          kinded (e.g. an FD variable)
### '+int' (resp. '*int') indicates that the builtin will suspend until
### this argument is determined (resp. kinded).
###
### Furthermore, there are builtins that overwrite their input arguments.
### This should be indicated by the prefix `!'. Thus '!+value' indicates
### an argument for which the builtin will suspend until it is determined
### and which may be overwriten by its execution.
###
### For most builtins it is OK if the output registers are not all distinct
### from the input registers: the compiler takes advantage of the possibility
### to eliminate certain moves.  However there are builtins that will not
### function properly unless certain output registers are guaranteed to
### be distinct from the input registers.  An output argument may be
### annotated with ^ to indicate that it needs its own register.
###
### The annotations +,*,! and ^ may be given in arbitrary order.
###
### A type may be simple or complex:
###
### SIMPLE    ::= unit
###             | atom
###             | nilAtom
###             | array
###             | bool
###             | cell
###             | char
###             | chunk
###             | comparable
###             | class
###             | dictionary
###             | feature
###             | float
###             | foreignPointer
###             | fset
###             | int
###             | fdint
###             | intC
###             | literal
###             | lock
###             | name
###             | number
###             | object
###             | port
###             | procedure
###             | procedure/0
###             | procedure/1
###             | procedure/2
###             | procedure/3
###             | procedure/4
###             | procedure/5
###             | procedure/6
###             | procedure/>6
###             | procedureOrObject
###             | unaryProcOrObject
###             | record
###             | recordOrChunk
###             | recordC
###             | recordCOrChunk
###             | space
###             | thread
###             | tuple
###             | pair
###             | cons
###             | list
###             | string
###             | virtualString
###             | value
###
### COMPLEX   ::= [SIMPLE]              (list of SIMPLE)
###             | [SIMPLE#SIMPLE]       (list of pairs of SIMPLE)
###             |  SIMPLE#SIMPLE        (pair or SIMPLE)
###
### determinacy annotations for subtypes of complex types are not yet
### supported.
###
### Old style builtins have at least: bi => OLDBI, where OLDBI is the name
### of the C procedure that implements it (normally defined using
### OZ_C_proc_begin(OLDBI,...)).  ibi => OLDIBI, is for the case where the
### builtin also has an inline version implemented by OLDIBI.  Whether this
### is an inline fun or rel is determined by the output arity: 0 means rel,
### 1 means fun.
###
### New style builtins have only: BI => NEWBI, where NEWBI is the name of
### the C procedure that implements it and defined using
### OZ_BI_define(NEWBI,...,...).
###
### Old style boolean funs sometimes have a corresponding rel that can be
### used in shallow guards.  This is indicated by: 'shallow' => 'REL' where
### REL is the string by which the rel is known to the emulator.
###
### eqeq => 1, indicates that the builtin must be specially compiled using
### the eqeq instruction.
###
### ifdef => MACRO, indicates that the entry for this builtin in the
### emulator's table should be bracketed by #ifdef MACRO ... #endif.
### Actually MACRO can be of the form M1,M2,...,Mn in which case there
### will be n bracketing, one for each macro M1 to Mn.
###
### ifndef => M1,...,Mn is similar for #ifndef ... #endif.  both ifdef
### and ifndef may be present.
###
### doesNotReturn => 1, indicates that the builtin does not return and
### therefore that the code following it will never be executed.  For
### example 'raise'.
###
### module => M, indicates that the builtin belongs to module M.  This
### permits selective inclusion or exclusion through command line options
### -include or -exclude.
###
### native => true|false specifies whether the builtin
### is non-exportable.


$builtins = {
    #* Access to all of them: the Builtin 'builtin'

    'builtin'   => { in  => ['+virtualString','+int'],
                     out => ['+procedure'],
                     BI  => BIbuiltin,
                     native => true},


    #* Core

    ##* Type Tests

    'IsNumber'          => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisNumberB,
                             ibi => BIisNumberBInline,
                             shallow => isNumberRel,
                             native => false},

    'IsInt'             => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisIntB,
                             ibi => BIisIntBInline,
                             shallow => isIntRel,
                             native => false},

    'IsFloat'           => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisFloatB,
                             ibi => BIisFloatBInline,
                             shallow => isFloatRel,
                             native => false},

    'IsRecord'          => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisRecordB,
                             ibi => isRecordBInline,
                             shallow => isRecordRel,
                             native => false},

    'IsTuple'           => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisTupleB,
                             ibi => isTupleBInline,
                             shallow => isTupleRel,
                             native => false},

    'IsLiteral'         => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisLiteralB,
                             ibi => isLiteralBInline,
                             shallow => isLiteralRel,
                             native => false},

    'IsLock'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisLockB,
                             ibi => isLockBInline,
                             shallow => isLockRel,
                             native => false},

    'IsCell'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisCellB,
                             ibi => isCellBInline,
                             shallow => isCellRel,
                             native => false},

    'IsPort'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisPortB,
                             ibi => isPortBInline,
                             shallow => isPortRel,
                             native => false},

    'IsProcedure'       => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisProcedureB,
                             ibi => isProcedureBInline,
                             shallow => isProcedureRel,
                             native => false},

    'IsName'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisNameB,
                             ibi => isNameBInline,
                             shallow => isNameRel,
                             native => false},

    'IsAtom'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisAtomB,
                             ibi => isAtomBInline,
                             shallow => isAtomRel,
                             native => false},

    'IsBool'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisBoolB,
                             ibi => isBoolBInline,
                             shallow => isBoolRel,
                             native => false},

    'IsUnit'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisUnitB,
                             ibi => isUnitBInline,
                             shallow => isUnitRel,
                             native => false},

    'IsChunk'           => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisChunkB,
                             ibi => isChunkBInline,
                             shallow => isChunkRel,
                             native => false},

    'IsRecordC'         => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisRecordCB,
                             ibi => isRecordCBInline,
                             shallow => isRecordCRel,
                             native => false},

    'IsObject'          => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisObjectB,
                             ibi => BIisObjectBInline,
                             shallow => isObjectRel,
                             native => false},

    'IsDictionary'      => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisDictionary,
                             ibi => isDictionaryInline,
                             native => false},


    'IsArray'           => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisArray,
                             ibi => isArrayInline,
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
                             ibi => isFreeInline,
                             shallow => IsFreeRel,
                             native => false},

    'IsKinded'          => { in  => ['value'],
                             out => ['+bool'],
                             bi  => BIisKinded,
                             ibi => isKindedInline,
                             shallow => IsKindedRel,
                             native => false},

    'IsDet'             => { in  => ['value'],
                             out => ['+bool'],
                             bi  => BIisDet,
                             ibi => isDetInline,
                             shallow => IsDetRel,
                             native => false},

    'isNumberRel'       => { in  => ['+value'],
                             out => [],
                             bi  => BIisNumber,
                             ibi => BIisNumberInline,
                             native => false},

    'isIntRel'          => { in  => ['+value'],
                             out => [],
                             bi  => BIisInt,
                             ibi => BIisIntInline,
                             native => false},

    'isFloatRel'        => { in  => ['+value'],
                             out => [],
                             bi  => BIisFloat,
                             ibi => BIisFloatInline,
                             native => false},

    'isRecordRel'       => { in  => ['+value'],
                             out => [],
                             bi  => BIisRecord,
                             ibi => isRecordInline,
                             native => false},

    'isTupleRel'        => { in  => ['+value'],
                             out => [],
                             bi  => BIisTuple,
                             ibi => isTupleInline,
                             native => false},

    'isLiteralRel'      => { in  => ['+value'],
                             out => [],
                             bi  => BIisLiteral,
                             ibi => isLiteralInline,
                             native => false},

    'isCellRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisCell,
                             ibi => isCellInline,
                             native => false},

    'isPortRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisPort,
                             ibi => isPortInline,
                             native => false},

    'isProcedureRel'    => { in  => ['+value'],
                             out => [],
                             bi  => BIisProcedure,
                             ibi => isProcedureInline,
                             native => false},

    'isNameRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisName,
                             ibi => isNameInline,
                             native => false},

    'isAtomRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisAtom,
                             ibi => isAtomInline,
                             native => false},

    'isLockRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisLock,
                             ibi => isLockInline,
                             native => false},

    'isBoolRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisBool,
                             ibi => isBoolInline,
                             native => false},

    'isUnitRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisUnit,
                             ibi => isUnitInline,
                             native => false},

    'isChunkRel'        => { in  => ['+value'],
                             out => [],
                             bi  => BIisChunk,
                             ibi => isChunkInline,
                             native => false},

    'isRecordCRel'      => { in  => ['+value'],
                             out => [],
                             bi  => BIisRecordC,
                             ibi => isRecordCInline,
                             native => false},

    'isObjectRel'       => { in  => ['+value'],
                             out => [],
                             bi  => BIisObject,
                             ibi => BIisObjectInline,
                             native => false},

    'IsFreeRel'         => { in  => ['value'],
                             out => [],
                             bi  => BIisFreeRel,
                             ibi => isFreeRelInline,
                             native => false},

    'IsKindedRel'       => { in  => ['value'],
                             out => [],
                             bi  => BIisKindedRel,
                             ibi => isKindedRelInline,
                             native => false},

    'IsDetRel'          => { in  => ['value'],
                             out => [],
                             bi  => BIisDetRel,
                             ibi => isDetRelInline,
                             native => false},

    'Type.ofValue'      => { in  => ['+value'],
                             out => ['+atom'],
                             bi  => BItermType,
                             ibi => BItermTypeInline,
                             native => false},


    ##* Type Conversion

    'AtomToString'      => { in  => ['+atom'],
                             out => ['+string'],
                             bi  => BIatomToString,
                             ibi => atomToStringInline,
                             native => false},

    'StringToAtom'      => { in  => ['+string'],
                             out => ['+atom'],
                             BI  => BIstringToAtom,
                             native => false},

    'IntToFloat'        => { in  => ['+int'],
                             out => ['+float'],
                             bi  => BIintToFloat,
                             ibi => BIintToFloatInline,
                             native => false},

    'FloatToInt'        => { in  => ['+float'],
                             out => ['+int'],
                             bi  => BIfloatToInt,
                             ibi => BIfloatToIntInline,
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
                     ibi => BIfdivInline ,
                     native => false},

    '*'         => { in  => ['+number','+number'],
                     out => ['+number'],
                     bi  => BImult,
                     ibi => BImultInline,
                     native => false},

    'div'       => { in  => ['+int','+int'],
                     out => ['+int'],
                     bi  => BIdiv,
                     ibi => BIdivInline,
                     native => false},

    'mod'       => { in  => ['+int','+int'],
                     out => ['+int'],
                     bi  => BImod,
                     ibi => BImodInline,
                     native => false},

    '-'         => { in  => ['+number','+number'],
                     out => ['+number'],
                     bi  => BIminus,
                     ibi => BIminusInline,
                     native => false},

    '+'         => { in  => ['+number','+number'],
                     out => ['+number'],
                     bi  => BIplus,
                     ibi => BIplusInline,
                     native => false},

    'Max'       => { in  => ['+comparable','+comparable'],
                     out => ['+comparable'],
                     bi  => BImax,
                     ibi => BImaxInline,
                     native => false},

    'Min'       => { in  => ['+comparable','+comparable'],
                     out => ['+comparable'],
                     bi  => BImin,
                     ibi => BIminInline,
                     native => false},

    '<'         => { in  => ['+comparable','+comparable'],
                     out => ['+bool'],
                     bi  => BIlessFun,
                     ibi => BIlessInlineFun,
                     shallow => '<Rel' ,
                     native => false},

    '=<'        => { in  => ['+comparable','+comparable'],
                     out => ['+bool'],
                     bi  => BIleFun,
                     ibi => BIleInlineFun,
                     shallow => '=<Rel' ,
                     native => false},

    '>'         => { in  => ['+comparable','+comparable'],
                     out => ['+bool'],
                     bi  => BIgreatFun,
                     ibi => BIgreatInlineFun,
                     shallow => '>Rel' ,
                     native => false},

    '>='        => { in  => ['+comparable','+comparable'],
                     out => ['+bool'],
                     bi  => BIgeFun,
                     ibi => BIgeInlineFun,
                     shallow => '>=Rel' ,
                     native => false},

    '=<Rel'     => { in  => ['+comparable','+comparable'],
                     out => [],
                     bi  => BIle,
                     ibi => BIleInline,
                     native => false},

    '<Rel'      => { in  => ['+comparable','+comparable'],
                     out => [],
                     bi  => BIless,
                     ibi => BIlessInline,
                     native => false},

    '>=Rel'     => { in  => ['+comparable','+comparable'],
                     out => [],
                     bi  => BIge,
                     ibi => BIgeInline,
                     native => false},

    '>Rel'      => { in  => ['+comparable','+comparable'],
                     out => [],
                     bi  => BIgreat,
                     ibi => BIgreatInline,
                     native => false},

    '~'         => { in  => ['+number'],
                     out => ['+number'],
                     bi  => BIuminus,
                     ibi => BIuminusInline,
                     native => false},

    '+1'        => { in  => ['+int'],
                     out => ['+int'],
                     bi  => BIadd1,
                     ibi => BIadd1Inline,
                     native => false},

    '-1'        => { in  => ['+int'],
                     out => ['+int'],
                     bi  => BIsub1,
                     ibi => BIsub1Inline,
                     native => false},

    'Exp'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIexp,
                     ibi => BIinlineExp,
                     native => false},

    'Log'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIlog,
                     ibi => BIinlineLog,
                     native => false},

    'Sqrt'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIsqrt,
                     ibi => BIinlineSqrt,
                     native => false},

    'Sin'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIsin,
                     ibi => BIinlineSin,
                     native => false},

    'Asin'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIasin,
                     ibi => BIinlineAsin,
                     native => false},

    'Cos'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIcos,
                     ibi => BIinlineCos,
                     native => false},

    'Acos'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIacos,
                     ibi => BIinlineAcos,
                     native => false},

    'Tan'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BItan,
                     ibi => BIinlineTan,
                     native => false},

    'Atan'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIatan,
                     ibi => BIinlineAtan,
                     native => false},

    'Ceil'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIceil,
                     ibi => BIinlineCeil,
                     native => false},

    'Floor'     => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIfloor,
                     ibi => BIinlineFloor,
                     native => false},

    'Abs'       => { in  => ['+number'],
                     out => ['+number'],
                     bi  => BIabs,
                     ibi => BIabsInline,
                     native => false},

    'Round'     => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIround,
                     ibi => BIinlineRound,
                     native => false},

    'Atan2'     => { in  => ['+float','+float'],
                     out => ['+float'],
                     bi  => BIatan2,
                     ibi => BIatan2Inline,
                     native => false},

    'fPow'      => { in  => ['+float','+float'],
                     out => ['+float'],
                     bi  => BIfPow,
                     ibi => BIfPowInline,
                     native => false},

    ###* Array/Dictionaries

    'NewArray'          => { in  => ['+int','+int','value'],
                             out => ['+array'],
                             BI  => BIarrayNew,
                             native => false},

    'Array.high'        => { in  => ['+array'],
                             out => ['+int'],
                             bi  => BIarrayHigh,
                             ibi => arrayHighInline,
                             native => false},

    'Array.low'         => { in  => ['+array'],
                             out => ['+int'],
                             bi  => BIarrayLow,
                             ibi => arrayLowInline,
                             native => false},

    'Get'               => { in  => ['+array','+int'],
                             out => ['value'],
                             bi  => BIarrayGet,
                             ibi => arrayGetInline,
                             native => false},

    'Put'               => { in  => ['+array','+int','value'],
                             out => [],
                             bi  => BIarrayPut,
                             ibi => arrayPutInline,
                             native => false},


    'NewDictionary'     => { in  => [],
                             out => ['+dictionary'],
                             BI  => BIdictionaryNew,
                             native => false},

    'Dictionary.isEmpty'=> { in  => ['+dictionary'],
                             out => ['+bool'],
                             bi  => BIdictionaryIsMt,
                             ibi => dictionaryIsMtInline,
                             native => false},

    'Dictionary.get'    => { in  => ['+dictionary','+feature'],
                             out => ['value'],
                             bi  => BIdictionaryGet,
                             ibi => dictionaryGetInline,
                             native => false},

    'Dictionary.condGet'=> { in  => ['+dictionary','+feature','value'],
                             out => ['value'],
                             bi  => BIdictionaryCondGet,
                             ibi => dictionaryCondGetInline,
                             native => false},

    'Dictionary.put'    => { in  => ['+dictionary','+feature','value'],
                             out => [],
                             bi  => BIdictionaryPut,
                             ibi => dictionaryPutInline,
                             native => false},

    'Dictionary.condPut'=> { in  => ['+dictionary','+feature','value'],
                             out => [],
                             bi  => BIdictionaryCondPut,
                             ibi => dictionaryCondPutInline,
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
                             ibi => dictionaryRemoveInline,
                             native => false},

    'Dictionary.removeAll'=> { in  => ['+dictionary'],
                               out => [],
                               BI  => BIdictionaryRemoveAll,
                               native => false},

    'Dictionary.member' => { in  => ['+dictionary','+feature'],
                             out => ['+bool'],
                             bi  => BIdictionaryMember,
                             ibi => dictionaryMemberInline,
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
                             ibi => BIexchangeCellInline,
                             native => false},

    'Access'            => { in  => ['+cell'],
                             out => ['value'],
                             bi  => BIaccessCell,
                             ibi => BIaccessCellInline,
                             native => false},

    'Assign'            => { in  => ['+cell','value'],
                             out => [],
                             bi  => BIassignCell,
                             ibi => BIassignCellInline,
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
                             ibi => BIadjoinInline,
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
                             ibi => BIarityInline,
                             native => false},

    'AdjoinAt'          => { in  => ['+record','+feature','value'],
                             out => ['+record'],
                             BI  => BIadjoinAt,
                             native => false},

    'MakeTuple'         => { in  => ['+literal','+int'],
                             out => ['+tuple'],
                             bi  => BItuple,
                             ibi => tupleInline,
                             native => false},

    'Label'             => { in  => ['*recordC'],
                             out => ['+literal'],
                             bi  => BIlabel,
                             ibi => labelInline,
                             native => false},

    'hasLabel'          => { in  => ['value'],
                             out => ['+bool'],
                             bi  => BIhasLabel,
                             ibi => hasLabelInline,
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
                             native => true},

    'tellRecordSize'    => { in  => ['+literal','+int','record'],
                             out => [],
                             BI  => BIsystemTellSize,
                             native => false},

    ###* Records and Chunks

    '.'                 => { in  => ['*recordCOrChunk','+feature'],
                             out => ['value'],
                             bi  => BIdot,
                             ibi => dotInline,
                             native => false},

    '^'                 => { in  => ['*recordCOrChunk','+feature'],
                             out => ['value'],
                             bi  => BIuparrowBlocking,
                             ibi => uparrowInlineBlocking,
                             native => false},

    'HasFeature'        => { in  => ['*recordCOrChunk','+feature'],
                             out => ['+bool'],
                             bi  => BIhasFeatureB,
                             ibi => hasFeatureBInline,
                             native => false},

    'CondSelect'        => { in  => ['*recordCOrChunk','+feature','value'],
                             out => ['value'],
                             bi  => BImatchDefault,
                             ibi => matchDefaultInline,
                             native => false},

    'Width'             => { in  => ['+record'],
                             out => ['+int'],
                             bi  => BIwidth,
                             ibi => widthInline,
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
                             ibi => procedureArityInline,
                             native => false},

    ###* Object-Oriented Primitives

    '@'                 => { in  => ['+feature'],
                             out => ['value'],
                             bi  => BIat,
                             ibi => atInline,
                             native => false},

    '<-'                => { in  => ['+feature','value'],
                             out => [],
                             bi  => BIassign,
                             ibi => assignInline,
                             native => false},

    'copyRecord'        => { in  => ['+record'],
                             out => ['+record'],
                             BI  => BIcopyRecord,
                             native => false},

    'makeClass'         => { in  => ['+dictionary','+record','+record',
                                     '+dictionary','+bool'],
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
                             ibi => getClassInline,
                             native => false},

    'ooGetLock'         => { in  => ['lock'],
                             out => [],
                             bi  => BIooGetLock,
                             ibi => ooGetLockInline,
                             native => false},

    'newObject'         => { in  => ['+class'],
                             out => ['+object'],
                             bi  => BInewObject,
                             ibi => newObjectInline,
                             native => false},

    'New'               => { in  => ['+class','+record','+object'],
                             out => [],
                             bi  => BINew,
                             native => false},

    'setSelf'           => { in  => ['+object'],
                             out => [],
                             BI  => BIsetSelf,
                             native => false},

    'ooExch'            => { in  => ['+feature','value'],
                             out => ['value'],
                             bi  => BIooExch,
                             ibi => ooExchInline,
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

    'Thread.frameVariables'=> { in  => ['+thread','+int'],
                                out => ['+record'],
                                BI  => BIthreadFrameVariables,
                                native => false},

    'Thread.location'   => { in  => ['+thread'],
                             out => ['+[atom]'],
                             BI  => BIthreadLocation,
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


    ##* Misc Operations
    ###* Equalities

    '=='                => { in  => ['*value','*value'],
                             out => ['+bool'],
                             bi  => BIeqB,
                             ibi => eqeqInline,
                             eqeq => 1,
                             native => false},

    '\\\\='             => { in  => ['*value','*value'],
                             out => ['+bool'],
                             bi  => BIneqB,
                             ibi => neqInline,
                             eqeq => 1,
                             native => false},

    '==Rel'             => { in  => ['*value','*value'],
                             out => [],
                             BI  => BIeq,
                             native => false},

    '\\\\=Rel'          => { in  => ['*value','*value'],
                             out => [],
                             BI  => BIneq,
                             native => false},

    ###* Other Misc Operations

    'Wait'              => { in  => ['+value'],
                             out => [],
                             bi  => BIisValue,
                             ibi => isValueInline,
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
                             ibi => notInline,
                             native => false},

    'And'               => { in  => ['+bool','+bool'],
                             out => ['+bool'],
                             bi  => BIand,
                             ibi => andInline,
                             native => false},

    'Or'                => { in  => ['+bool','+bool'],
                             out => ['+bool'],
                             bi  => BIor,
                             ibi => orInline,
                             native => false},

    'Value.status'      => { in  => ['value'],
                             out => ['+tuple'],
                             bi  => BIstatus,
                             ibi => BIstatusInline,
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


    ##* Finalization

    'Finalize.register' => { in  => ['+value','+value'],
                             out => [],
                             BI  => BIfinalize_register,
                             native => true},

    'Finalize.setHandler'=> { in  => ['+value'],
                              out => [],
                              BI  => BIfinalize_setHandler,
                              native => true},

    'GetCloneDiff'      => { in  => ['+space'],
                             out => ['+value'],
                             BI  => BIgetCloneDiff,
                             ifdef=>'CS_PROFILE',
                             native => true},



    ##* Diffent Kinds of Special Variables

    ###* Promise

    'Promise.new'       => { in  => [],
                             out => ['value'],
                             BI  => BIPromiseNew,
                             module=>'promise',
                             native => false},

    'Promise.is'        => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIPromiseIs,
                             module=>'promise',
                             native => false},
    'Promise.bind'      => { in  => ['value','value'],
                             out => [],
                             BI  => BIPromiseBind,
                             module=>'promise',
                             native => false},
    'Promise.waitRequest'=> { in  => ['value'],
                             out => [],
                             BI  => BIPromiseWaitRequest,
                             module=>'promise',
                             native => false},
    ###* Lazy

    'Lazy.new'          => { in  => ['value','value'],
                             out => [],
                             BI  => BILazyNew,
                             module=>'lazy',
                             native => false},

    'Lazy.is'           => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BILazyIs,
                             module=>'lazy',
                             native => false},

    ###* Metavar

    'metaIsVar'         => { in  => ['value'],
                             out => [],
                             BI  => BImetaIsVar,
                             module=>'metavar',
                             native => true},

    'metaIsVarB'        => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BImetaIsVarB,
                             module=>'metavar',
                             native => true},

    'metaWatchVar'      => { in  => ['value','value'],
                             out => [],
                             BI  => BImetaWatchVar,
                             module=>'metavar',
                             native => true},

    'metaWatchVarB'     => { in  => ['value','value','+bool'],
                             out => [],
                             BI  => BImetaWatchVarB,
                             module=>'metavar',
                             native => true},

    'metaGetDataAsAtom' => { in  => ['value','atom'],
                             out => [],
                             BI  => BImetaGetDataAsAtom,
                             module=>'metavar',
                             native => true},

    'metaGetNameAsAtom' => { in  => ['value','atom'],
                             out => [],
                             BI  => BImetaGetNameAsAtom,
                             module=>'metavar',
                             native => true},

    'metaGetStrength'   => { in  => ['value','value'],
                             out => [],
                             BI  => BImetaGetStrength,
                             module=>'metavar',
                             native => true},

    #* System Stuff

    ##* Virtual Properties

    'GetProperty'       => { in  => ['+literal'],
                             out => ['value'],
                             BI  => BIgetProperty,
                             module=>'vprop',
                             native => true},

    'CondGetProperty'   => { in  => ['+literal','value'],
                             out => ['value'],
                             BI  => BIcondGetProperty,
                             module=>'vprop',
                             native => true},

    'PutProperty'       => { in  => ['+literal','value'],
                             out => [],
                             BI  => BIputProperty,
                             module=>'vprop',
                             native => true},



    ##* Printing

    'showBuiltins'      => { in  => [],
                             out => [],
                             BI  => BIshowBuiltins,
                             native => true},

    'Print'             => { in  => ['value'],
                             out => [],
                             bi  => BIprint,
                             ibi => printInline,
                             native => true},

    'Show'              => { in  => ['value'],
                             out => [],
                             bi  => BIshow,
                             ibi => showInline,
                             native => true},

    ##* Statistics

    'statisticsReset'   => { in  => [],
                             out => [],
                             BI  => BIstatisticsReset,
                             native => true},

    'statisticsPrint'   => { in  => ['+virtualString'],
                             out => [],
                             BI  => BIstatisticsPrint,
                             native => true},

    'statisticsPrintProcs'=> { in  => [],
                               out => [],
                               BI  => BIstatisticsPrintProcs,
                               native => true},

    'statisticsGetProcs'=> { in  => [],
                             out => ['+value'],
                             BI  => BIstatisticsGetProcs,
                             native => true},

    'setProfileMode'    => { in  => ['+bool'],
                             out => [],
                             BI  => BIsetProfileMode,
                             native => true},

    'instructionsPrint' => { in  => [],
                             out => [],
                             BI  => BIinstructionsPrint,
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

    'System.printName'  => { in  => ['value'],
                             out => ['+atom'],
                             BI  => BIgetPrintName,
                             native => true},

    'System.printInfo'  => { in  => ['virtualString'],
                             out => [],
                             BI  => BIprintInfo,
                             native => true},

    'System.printError' => { in  => ['virtualString'],
                             out => [],
                             BI  => BIprintError,
                             native => true},

    'System.valueToVirtualString'=> { in  => ['value','+int','+int'],
                                      out => ['+string'],
                                      BI  => BItermToVS,
                                      native => false},

    'getTermSize'       => { in  => ['value','+int','+int'],
                             out => ['+int'],
                             BI  => BIgetTermSize,
                             native => false},

    ##* Browser Support

    'getsBoundB'        => { in  => ['value','value'],
                             out => [],
                             BI  => BIgetsBoundB,
                             native => true},

    'addr'              => { in  => ['value'],
                             out => ['+int'],
                             BI  => BIaddr,
                             native => true},

    'recordCIsVarB'     => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIisRecordCVarB,
                             native => true},

    'deepFeed'          => { in  => ['+cell','value'],
                             out => [],
                             BI  => BIdeepFeed,
                             native => true},

    'chunkWidth'        => { in  => ['+chunk'],
                             out => ['+int'],
                             BI  => BIchunkWidth,
                             native => true},

    ##* Misc. System Procs

    'shutdown'          => { in  => ['+int'],
                             out => [],
                             BI  => BIshutdown,
                             native => true},

    'Alarm'             => { in  => ['+int','unit'],
                             out => [],
                             BI  => BIalarm,
                             native => false},

    'Delay'             => { in  => ['!+int'],
                             out => [],
                             BI  => BIdelay,
                             native => false},

    'System.gcDo'       => { in  => [],
                             out => [],
                             BI  => BIgarbageCollection,
                             native => true},

    'System.apply'      => { in  => ['+procedureOrObject','+[value]'],
                             out => [],
                             BI  => BIapply,
                             native => false},

    'System.eq'         => { in  => ['value','value'],
                             out => ['+bool'],
                             BI  => BIsystemEq,
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

    'setProcNames'      => { in  => ['value', 'value'],
                             out => [],
                             BI  => BIsetProcNames,
                             native => false},

    'onToplevel'        => { in  => [],
                             out => ['+bool'],
                             BI  => BIonToplevel,
                             native => false},


    #* OS interface

    'OS.getDir'         => { in  => ['+virtualString'],
                             out => ['+[string]'],
                             BI  => unix_getDir,
                             module=>'os',
                             native => OK,
                             native => true},

    'OS.stat'           => { in  => ['+virtualString'],
                             out => ['+record'],
                             BI  => unix_stat,
                             module=>'os',
                             native => true},

    'OS.getCWD'         => { in  => [],
                             out => ['+atom'],
                             BI  => unix_getCWD,
                             module=>'os',
                             native => true},

    'OS.open'           => { in  => ['+virtualString','+[atom]','+[atom]'],
                             out => ['+int'],
                             BI  => unix_open,
                             module=>'os',
                             native => true},

    'OS.fileDesc'       => { in  => ['+atom'],
                             out => ['+int'],
                             BI  => unix_fileDesc,
                             module=>'os',
                             native => true},

    'OS.close'          => { in  => ['+int'],
                             out => [],
                             BI  => unix_close,
                             module=>'os',
                             native => true},

    'OS.write'          => { in  => ['+int','+virtualString'],
                             out => ['+value'],
                             BI  => unix_write,
                             module=>'os',
                             native => true},

    'OS.read'           => { in  => ['+int','+int','value','value','int'],
                             out => [],
                             BI  => unix_read,
                             module=>'os',
                             native => true},

    'OS.lSeek'          => { in  => ['+int','+int','+atom'],
                             out => ['+int'],
                             BI  => unix_lSeek,
                             module=>'os',
                             native => true},

    'OS.unlink'         => { in  => ['+virtualString'],
                             out => [],
                             BI  => unix_unlink,
                             module=>'os',
                             native => true},

    'OS.readSelect'     => { in  => ['+int'],
                             out => [],
                             BI  => unix_readSelect,
                             module=>'os',
                             native => true},

    'OS.writeSelect'    => { in  => ['+int'],
                             out => [],
                             BI  => unix_writeSelect,
                             module=>'os',
                             native => true},

    'OS.acceptSelect'   => { in  => ['+int'],
                             out => [],
                             BI  => unix_acceptSelect,
                             module=>'os',
                             native => true},

    'OS.deSelect'       => { in  => ['+int'],
                             out => [],
                             BI  => unix_deSelect,
                             module=>'os',
                             native => true},

    'OS.system'         => { in  => ['+virtualString'],
                             out => ['+int'],
                             BI  => unix_system,
                             module=>'os',
                             native => true},

    'OS.getEnv'         => { in  => ['+virtualString'],
                             out => ['+value'],
                             BI  => unix_getEnv,
                             module=>'os',
                             native => true},

    'OS.putEnv'         => { in  => ['+virtualString','+virtualString'],
                             out => [],
                             BI  => unix_putEnv,
                             module=>'os',
                             native => true},

    'OS.time'           => { in  => [],
                             out => ['+int'],
                             BI  => unix_time,
                             module=>'os',
                             native => true},

    'OS.gmTime'         => { in  => [],
                             out => ['+record'],
                             BI  => unix_gmTime,
                             module=>'os',
                             native => true},

    'OS.localTime'      => { in  => [],
                             out => ['+record'],
                             BI  => unix_localTime,
                             module=>'os',
                             native => true},

    'OS.srand'          => { in  => ['+int'],
                             out => [],
                             BI  => unix_srand,
                             module=>'os',
                             native => true},

    'OS.rand'           => { in  => [],
                             out => ['+int'],
                             BI  => unix_rand,
                             module=>'os',
                             native => true},

    'OS.randLimits'     => { in  => [],
                             out => ['+int','+int'],
                             BI  => unix_randLimits,
                             module=>'os',
                             native => true},

    'OS.socket'         => { in  => ['+atom','+atom','+virtualString'],
                             out => ['+int'],
                             BI  => unix_socket,
                             module=>'os',
                             native => true},

    'OS.bind'           => { in  => ['+int','+int'],
                             out => [],
                             BI  => unix_bindInet,
                             module=>'os',
                             native => true},

    'OS.listen'         => { in  => ['+int','+int'],
                             out => [],
                             BI  => unix_listen,
                             module=>'os',
                             native => true},

    'OS.connect'        => { in  => ['+int','+virtualString','+int'],
                             out => [],
                             BI  => unix_connectInet,
                             module=>'os',
                             native => true},

    'OS.accept'         => { in  => ['+int'],
                             out => ['+int','+string','+int'],
                             BI  => unix_acceptInet,
                             module=>'os',
                             native => true},

    'OS.shutDown'       => { in  => ['+int','+int'],
                             out => [],
                             BI  => unix_shutDown,
                             doesNotReturn=>1,
                             module=>'os',
                             native => true},

    'OS.send'           => { in  => ['+int','+virtualString','+[atom]'],
                             out => ['+value'],
                             BI  => unix_send,
                             module=>'os',
                             native => true},

    'OS.sendTo'         => { in  => ['+int','+virtualString','+[atom]',
                                     '+virtualString','+int'],
                             out => ['+value'],
                             BI  => unix_sendToInet,
                             module=>'os',
                             native => true},

    'OS.receiveFrom'    => { in  => ['+int','+int','+[atom]','value','value'],
                             out => ['+string','+int','+int'],
                             BI  => unix_receiveFromInet,
                             module=>'os',
                             native => true},

    'OS.getSockName'    => { in  => ['+int'],
                             out => ['+int'],
                             BI  => unix_getSockName,
                             module=>'os',
                             native => true},

    'OS.getHostByName'  => { in  => ['+virtualString'],
                             out => ['+record'],
                             BI  => unix_getHostByName,
                             module=>'os',
                             native => true},

    'OS.pipe'           => { in  => ['+virtualString','value'],
                             out => ['+int','+int#int'],
                             BI  => unix_pipe,
                             module=>'os',
                             native => true},

    'OS.tmpnam'         => { in  => [],
                             out => ['+string'],
                             BI  => unix_tmpnam,
                             module=>'os',
                             native => true},

    'OS.wait'           => { in  => [],
                             out => ['+int','+int'],
                             BI  => unix_wait,
                             module=>'os',
                             native => true},

    'OS.getServByName'  => { in  => ['+virtualString','+virtualString'],
                             out => ['+int'],
                             BI  => unix_getServByName,
                             module=>'os',
                             native => true},

    'OS.uName'          => { in  => [],
                             out => ['+record'],
                             BI  => unix_uName,
                             module=>'os',
                             native => true},

    'OS.getpwnam'       => { in  => ['+virtualString'],
                             out => ['+record'],
                             BI  => unix_getpwnam,
                             module=>'os',
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

    'PerdioVar.is'      => { in  => ['value'],
                             out => ['+bool'],
                             BI  => PerdioVar_is,
                             module=>'perdiovar',
                             native => true},

    'probe'             => { in  => ['value'],
                             out => [],
                             BI  => BIprobe,
                             native => true},

    'crash'             => { in  => [],
                             out => [],
                             BI  => BIcrash,
                             doesNotReturn=>1,
                             native => true},

    'installHW'         => { in  => ['+value','+value','value'],
                             out => [],
                             BI  => BIhwInstall,
                             native => true},

    'deInstallHW'       =>  { in  => ['+value','+value','value'],
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
                             ibi => atInlineRedo,
                             native => true},

    #* Pickles

    'smartSave'         => { in  => ['value','value','+virtualString'],
                             out => [],
                             BI  => BIsmartSave,
                             module=>components,
                             native => true},

    'load'              => { in  => ['value','value'],
                             out => [],
                             BI  => BIload,
                             module=>components,
                             native => true},

    #* Connection

    'PID.get'           => { in  => [],
                             out => ['+record'],
                             BI  => BIGetPID,
                             module=>components,
                             native => true},

    'PID.received'      => { in  => ['value'],
                             out => [],
                             BI  => BIReceivedPID,
                             module=>components,
                             native => true},

    'PID.close'         => { in  => [],
                             out => [],
                             BI  => BIClosePID,
                             module=>components,
                             native => true},

    'PID.send'          => { in  => ['+virtualString','+int','+int','value'],
                             out => [],
                             BI  => BISendPID,
                             module=>components,
                             native => true},

    #* URL

    'URL.localize'      => { in  => ['+virtualString'],
                             out => ['+record'],
                             BI  => BIurl_localize,
                             module=>components,
                             native => true},

    'URL.open'          => { in  => ['+virtualString'],
                             out => ['+int'],
                             BI  => BIurl_open,
                             module=>components,
                             native => true},

    'URL.load'          => { in  => ['+virtualString'],
                             out => ['value'],
                             BI  => BIurl_load,
                             module=>components,
                             native => true},

    #* Tools

    ##* TCL_TK

    'getTclNames'       => { in  => [],
                             out => ['value','value','value'],
                             BI  => BIgetTclNames,
                             module=>'tcl_tk',
                             native => true},

    'initTclSession'    => { in  => ['value','value','value'],
                             out => ['value'],
                             BI  => BIinitTclSession,
                             module=>'tcl_tk',
                             native => true},

    'closeTclSession'   => { in  => ['value'],
                             out => [],
                             BI  => BIcloseTclSession,
                             module=>'tcl_tk',
                             native => true},

    'Tk.send'           => { in  => ['value','!value'],
                             out => [],
                             BI  => BItclWrite,
                             module=>'tcl_tk',
                             native => true},

    'tclWriteReturn'    => { in  => ['!value','value','value','value'],
                             out => [],
                             BI  => BItclWriteReturn,
                             module=>'tcl_tk',
                             native => true},

    'tclWriteReturnMess'=> { in  => ['!value','value','value','value','value'],
                             out => [],
                             BI  => BItclWriteReturnMess,
                             module=>'tcl_tk',
                             native => true},

    'Tk.batch'          => { in  => ['value','!value'],
                             out => [],
                             BI  => BItclWriteBatch,
                             module=>'tcl_tk',
                             native => true},

    'tclWriteTuple'     => { in  => ['value','!value','value'],
                             out => [],
                             BI  => BItclWriteTuple,
                             module=>'tcl_tk',
                             native => true},

    'tclWriteTagTuple'  => { in  => ['value','!value','value','value'],
                             out => [],
                             BI  => BItclWriteTagTuple,
                             module=>'tcl_tk',
                             native => true},

    'tclWriteFilter'    => { in  => ['value','!value','value','value',
                                     'value','value'],
                             out => [],
                             BI  => BItclWriteFilter,
                             module=>'tcl_tk',
                             native => true},

    'tclClose'          => { in  => ['value','!value','value'],
                             out => [],
                             BI  => BItclClose,
                             module=>'tcl_tk',
                             native => true},

    'tclCloseWeb'       => { in  => ['value','!value'],
                             out => [],
                             BI  => BItclCloseWeb,
                             module=>'tcl_tk',
                             native => true},

    'addFastGroup'      => { in  => ['+value','value'],
                             out => ['value'],
                             BI  => BIaddFastGroup,
                             module=>'tcl_tk',
                             native => true},

    'delFastGroup'      => { in  => ['value'],
                             out => [],
                             BI  => BIdelFastGroup,
                             module=>'tcl_tk',
                             native => true},

    'getFastGroup'      => { in  => ['+value'],
                             out => ['+value'],
                             BI  => BIgetFastGroup,
                             module=>'tcl_tk',
                             native => true},

    'delAllFastGroup'   => { in  => ['+value'],
                             out => ['+value'],
                             BI  => BIdelAllFastGroup,
                             module=>'tcl_tk',
                             native => true},

    'genTopName'        => { in  => ['value'],
                             out => ['value'],
                             BI  => BIgenTopName,
                             module=>'tcl_tk',
                             native => true},

    'genWidgetName'     => { in  => ['value','value'],
                             out => ['value'],
                             BI  => BIgenWidgetName,
                             module=>'tcl_tk',
                             native => true},

    'genTagName'        => { in  => ['value'],
                             out => ['value'],
                             BI  => BIgenTagName,
                             module=>'tcl_tk',
                             native => true},

    'genVarName'        => { in  => ['value'],
                             out => ['value'],
                             BI  => BIgenVarName,
                             module=>'tcl_tk',
                             native => true},

    'genImageName'      => { in  => ['value'],
                             out => ['value'],
                             BI  => BIgenImageName,
                             module=>'tcl_tk',
                             native => true},

    ##* Debugger


    ###* Debugger Internal

    'Debug.mode'        => { in  => [],
                             out => ['+bool'],
                             BI  => BIdebugmode,
                             native => true},

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

    'index2Tagged'      => { in  => ['int'],
                             out => ['value'],
                             BI  => BIindex2Tagged,
                             ifdef=>'UNUSED',
                             native => true},

    'time2localTime'    => { in  => ['int'],
                             out => ['value'],
                             BI  => BItime2localTime,
                             ifdef=>'UNUSED',
                             native => true},



    ##* Compiler
    ###* OPI

    'setOPICompiler'    => { in  => ['+object'],
                             out => [],
                             BI  => BIsetOPICompiler,
                             native => true},

    'getOPICompiler'    => { in  => [],
                             out => ['+value'],
                             BI  => BIgetOPICompiler,
                             native => true},

    ###* Misc

    'concatenateAtomAndInt' => { in  => ['+atom','+int'],
                                 out => ['+atom'],
                                 BI  => BIconcatenateAtomAndInt,
                                 native => true},

    'getProcInfo' => { in  => ['+procedure'],
                       out => ['value'],
                       BI  => BIgetProcInfo,
                       native => true},

    'setProcInfo' => { in  => ['+procedure','value'],
                       out => [],
                       BI  => BIsetProcInfo,
                       native => true},

    'isBuiltin' => { in  => ['+value'],
                     out => ['+bool'],
                     BI  => BIisBuiltin,
                     native => true},

    'getBuiltinName' => { in  => ['+value'],
                          out => ['+atom'],
                          BI  => BIgetBuiltinName,
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

    'generateCopies' => { in  => ['+[value]'],
                          out => ['+[value#value]'],
                          BI  => BIgenerateCopies,
                          native => true},

    # will expire soon:
    'generateAbstractionTableID'=> { in  => ['+bool'],
                                     out => ['+foreignPointer'],
                                     BI  => BIgenerateAbstractionTableID,
                                     native => true},

    ###* RegSets
    'RegSet.new'        => { in  => ['+int','+int'],
                             out => ['+chunk'],
                             BI  => BIregSet_new,
                             native => true},

    'RegSet.copy'       => { in  => ['+chunk'],
                             out => ['+chunk'],
                             BI  => BIregSet_copy,
                             native => true},

    'RegSet.adjoin'     => { in  => ['+chunk','+int'],
                             out => [],
                             BI  => BIregSet_adjoin,
                             native => true},

    'RegSet.remove'     => { in  => ['+chunk','+int'],
                             out => [],
                             BI  => BIregSet_remove,,
                             native => true},

    'RegSet.member'     => { in  => ['+int','+chunk'],
                             out => ['+bool'],
                             BI  => BIregSet_member,
                             native => true},

    'RegSet.union'      => { in  => ['+chunk','+chunk'],
                             out => [],
                             BI  => BIregSet_union,
                             native => true},

    'RegSet.intersect'  => { in  => ['+chunk','+chunk'],
                             out => [],
                             BI  => BIregSet_intersect,
                             native => true},

    'RegSet.subtract'   => { in  => ['+chunk','+chunk'],
                             out => [],
                             BI  => BIregSet_subtract,
                             native => true},

    'RegSet.toList'     => { in  => ['+chunk'],
                             out => ['+[int]'],
                             BI  => BIregSet_toList,
                             native => true},

    'RegSet.complementToList'   => { in  => ['+chunk'],
                                     out => ['+[int]'],
                                     BI  => BIregSet_complementToList,
                                     native => true},

    ###* Parser

    'ozparser_fileExists'       => { in  => ['+virtualString'],
                                     out => ['+bool'],
                                     bi  => ozparser_fileExists,
                                     native => true},

    'ozparser_parseFile'        => { in  => ['+virtualString','+record'],
                                     out => ['+value'],
                                     bi  => ozparser_parseFile,
                                     native => true},

    'ozparser_parseVirtualString'=> { in  => ['+virtualString','+record'],
                                      out => ['+value'],
                                      bi  => ozparser_parseVirtualString,
                                      native => true},

    ###* Assembler

    'newCodeBlock'      => { in  => ['+int'],
                             out => ['+int'],
                             BI  => BInewCodeBlock,
                             native => true},

    'getOpcode'         => { in  => ['+atom'],
                             out => ['+int'],
                             BI  => BIgetOpcode,
                             native => true},

    'getInstructionSize'=> { in  => ['+atom'],
                             out => ['+int'],
                             BI  => BIgetInstructionSize,
                             native => true},

    'makeProc'          => { in  => ['+int','+[value]'],
                             out => ['+procedure/0'],
                             BI  => BImakeProc,
                             native => true},

    'addDebugInfo'      => { in  => ['+int','+atom','+int'],
                             out => [],
                             BI  => BIaddDebugInfo,
                             native => true},

    'storeOpcode'       => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIstoreOpcode,
                             native => true},

    'storeNumber'       => { in  => ['+int','+number'],
                             out => [],
                             BI  => BIstoreNumber,
                             native => true},

    'storeLiteral'      => { in  => ['+int','+literal'],
                             out => [],
                             BI  => BIstoreLiteral,
                             native => true},

    'storeFeature'      => { in  => ['+int','+feature'],
                             out => [],
                             BI  => BIstoreFeature,
                             native => true},

    'storeConstant'     => { in  => ['+int','+value'],
                             out => [],
                             BI  => BIstoreConstant,
                             native => true},

    'storeBuiltinname'  => { in  => ['+int','+procedure'],
                             out => [],
                             BI  => BIstoreBuiltinname,
                             native => true},

    'storeVariablename' => { in  => ['+int','+atom'],
                             out => [],
                             BI  => BIstoreVariablename,
                             native => true},

    'storeRegisterIndex'=> { in  => ['+int','+int'],
                             out => [],
                             BI  => BIstoreRegisterIndex,
                             native => true},

    'storeInt'          => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIstoreInt,
                             native => true},

    'storeLabel'        => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIstoreLabel,
                             native => true},

    'storePredicateRef' => { in  => ['+int','+value'],
                             out => [],
                             BI  => BIstorePredicateRef,
                             native => true},

    'storePredId'       => { in  => ['+int','+atom','+value','+atom',
                                     '+int','+bool'],
                             out => [],
                             BI  => BIstorePredId,
                             native => true},

    'newHashTable'      => { in  => ['+int','+int','+int'],
                             out => ['+int'],
                             BI  => BInewHashTable,
                             native => true},

    'storeHTScalar'     => { in  => ['+int','+int','+value','+int'],
                             out => [],
                             BI  => BIstoreHTScalar,
                             native => true},

    'storeHTRecord'     => { in  => ['+int','+int','+literal','+value','+int'],
                             out => [],
                             BI  => BIstoreHTRecord,
                             native => true},

    'storeRecordArity'  => { in  => ['+int','+value'],
                             out => [],
                             BI  => BIstoreRecordArity,
                             native => true},

    'storeGenCallInfo'  => { in  => ['+int','+int','+bool','+literal',
                                     '+bool','+value'],
                             out => [],
                             BI  => BIstoreGenCallInfo,
                             native => true},

    'storeApplMethInfo' => { in  => ['+int','+literal','+value'],
                             out => [],
                             BI  => BIstoreApplMethInfo,
                             native => true},

    'storeGRegRef'      => { in  => ['+int','+[tuple]'],
                             out => [],
                             BI  => BIstoreGRegRef,
                             native => true},

    'storeLocation'     => { in  => ['+int','+list#list'],
                             out => [],
                             BI  => BIstoreLocation,
                             native => true},

    'storeCache'        => { in  => ['+int','+value'],
                             out => [],
                             BI  => BIstoreCache,
                             native => true},

    #* Finite Domains

    'foreignFDProps'    => { in  => [],
                             out => ['+bool'],
                             BI  => BIforeignFDProps,
                             native => true},

    'fdReset'           => { in  => [],
                             out => [],
                             bi  =>BIfdReset,
                             ifdef=>PROFILE_FD,
                             module=>fd,
                             native => true},

    'fdDiscard'         => { in  => [],
                             out => [],
                             bi  => BIfdDiscard,
                             ifdef=>PROFILE_FD,
                             module=>fd,
                             native => true},

    'fdGetNext'         => { in  => ['value'],
                             out => [],
                             bi  => BIfdGetNext,
                             ifdef=>PROFILE_FD,
                             module=>fd,
                             native => true},

    'fdPrint'           => { in  => [],
                             out => [],
                             bi  => BIfdPrint,
                             ifdef=>PROFILE_FD,
                             module=>fd,
                             native => true},

    'fdTotalAverage'    => { in  => [],
                             out => [],
                             bi  => BIfdTotalAverage,
                             ifdef=>PROFILE_FD,
                             module=>fd,
                             native => true},

    'fdIs'              => { in  => ['*value','bool'],
                             out => [],
                             bi  => BIfdIs,
                             module=>fd,
                             native => true},

    'fdIsVar'           => { in  => ['value'],
                             out => [],
                             BI  => BIisFdVar,
                             module=>fd,
                             native => true},

    'fdIsVarB'          => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIisFdVarB,
                             module=>fd,
                             native => true},

    'fdGetLimits'       => { in  => [],
                             out => ['+int','+int'],
                             BI  => BIgetFDLimits,
                             module=>fd,
                             native => true},

    'fdGetMin'          => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMin,
                             module=>fd,
                             native => true},

    'fdGetMid'          => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMid,
                             module=>fd,
                             native => true},

    'fdGetMax'          => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMax,
                             module=>fd,
                             native => true},

    'fdGetDom'          => { in  => ['*int','+[value]'],
                             out => [],
                             bi  => BIfdGetAsList,
                             module=>fd,
                             native => true},

    'fdGetCard'         => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdGetCardinality,
                             module=>fd,
                             native => true},

    'fdGetNextSmaller'  => { in  => ['+int','*int','int'],
                             out => [],
                             bi  => BIfdNextSmaller,
                             module=>fd,
                             native => true},

    'fdGetNextLarger'   => { in  => ['+int','*int','int'],
                             out => [],
                             bi  => BIfdNextLarger,
                             module=>fd,
                             native => true},

    'fdTellConstraint'  => { in  => ['int','+value'],
                             out => [],
                             bi  => BIfdTellConstraint,
                             module=>fd,
                             native => true},

    'fdWatchSize'       => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchSize,
                             module=>fd,
                             native => true},

    'fdWatchMin'        => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchMin,
                             module=>fd,
                             native => true},

    'fdWatchMax'        => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchMax,
                             module=>fd,
                             native => true},

    'fdConstrDisjSetUp' => { in  => ['+value','+value','+value','+value'],
                             out => [],
                             bi  => BIfdConstrDisjSetUp,
                             module=>fd,
                             native => true},

    'fdConstrDisj'      => { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => BIfdConstrDisj,
                             module=>fd,
                             native => true},

    'fdTellConstraintCD'=> { in  => ['value','value','value'],
                             out => [],
                             bi  => BIfdTellConstraintCD,
                             module=>fd,
                             native => true},

    'fdp_init'          => { in  => ['atom'],
                             out => [],
                             bi  => fdp_init,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_sum'           => { in  => ['+value','+atom','int'],
                             out => [],
                             bi  => fdp_sum,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_sumC'          => { in  => ['+value','+value','+atom','int'],
                             out => [],
                             bi  => fdp_sumC,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_sumCN'         => { in  => ['+value','+value','+atom','int'],
                             out => [],
                             bi  => fdp_sumCN,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_sumR'          => { in  => ['+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumR,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_sumCR'         => { in  => ['+value','+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCR,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_sumCNR'        => { in  => ['+value','+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCNR,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_sumCD'         => { in  => ['+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCD,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_sumCCD'        => { in  => ['+value','+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCCD,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_sumCNCD'       => { in  => ['+value','+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCNCD,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_plus_rel'      => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_plus_rel,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_plus'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_plus,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_minus'         => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_minus,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_times'         => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_times,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_times_rel'     => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_times_rel,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_power'         => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_power,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_divD'          => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_divD,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_divI'          => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_divI,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_modD'          => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_modD,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_modI'          => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_modI,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_conj'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_conj,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_disj'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_disj,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_exor'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_exor,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_impl'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_impl,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_equi'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_equi,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_nega'          => { in  => ['int','int'],
                             out => [],
                             bi  => fdp_nega,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_intR'          => { in  => ['int','+value','int'],
                             out => [],
                             bi  => fdp_intR,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_card'          => { in  => ['+value','int','int','int'],
                             out => [],
                             bi  => fdp_card,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_exactly'       => { in  => ['int','+value','+int'],
                             out => [],
                             bi  => fdp_exactly,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_atLeast'       => { in  => ['int','+value','+int'],
                             out => [],
                             bi  => fdp_atLeast,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_atMost'        => { in  => ['int','+value','+int'],
                             out => [],
                             bi  => fdp_atMost,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_element'       => { in  => ['int','+value','int'],
                             out => [],
                             bi  => fdp_element,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_notEqOff'      => { in  => ['int','int','+int'],
                             out => [],
                             bi  => fdp_notEqOff,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_lessEqOff'     => { in  => ['int','int','+int'],
                             out => [],
                             bi  => fdp_lessEqOff,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_minimum'       => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_minimum,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_maximum'       => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_maximum,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_inter' => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_inter,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                     native => true},

    'fdp_union' => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_union,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                     native => true},

    'fdp_distinct'      => { in  => ['+value'],
                             out => [],
                             bi  => fdp_distinct,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_distinctD'     => { in  => ['+value'],
                             out => [],
                             bi  => fdp_distinctD,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_distinctStream'=> { in  => ['+value','value'],
                             out => [],
                             bi  => fdp_distinctStream,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_distinctOffset'=> { in  => ['+value','+value'],
                             out => [],
                             bi  => fdp_distinctOffset,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_disjoint'=> { in  => ['int','+int','int','+int'],
                             out => [],
                             bi  => fdp_disjoint,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                       native => true},

    'sched_disjoint_card'=> { in  => ['int','+int','int','+int'],
                             out => [],
                             bi  => sched_disjoint_card,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                              native => true},

    'fdp_disjointC'=> { in  => ['int','+int','int','+int','int'],
                             out => [],
                             bi  => fdp_disjointC,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                        native => true},

    'fdp_distance'      => { in  => ['int','int','+atom','int'],
                             out => [],
                             bi  => fdp_distance,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_distinct2'     => { in  => ['+value','+value','+value','+value'],
                             out => [],
                             bi  => fdp_distinct2,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'sched_cpIterate'   => { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => sched_cpIterate,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'sched_cpIterateCap'=> { in  => ['+value','+value','+value',
                                     '+value','+value','+int'],
                             out => [],
                             bi  => sched_cpIterateCap,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'sched_cumulativeTI'=> { in  => ['+value','+value','+value',
                                     '+value','+value'],
                             out => [],
                             bi  => sched_cumulativeTI,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'sched_cpIterateCapUp'=> { in  => ['+value','+value','+value',
                                       '+value','+value'],
                             out => [],
                             bi  => sched_cpIterateCapUp,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                               native => true},

    'sched_taskIntervals'=> { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => sched_taskIntervals,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                              native => true},

    'sched_disjunctive' => { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => sched_disjunctive,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'sched_disjunctiveStream'=> { in  => ['+value','+value','value'],
                             out => [],
                             bi  => sched_disjunctiveStream,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                                  native => true},

    'fdp_twice'         => { in  => ['int','int'],
                             out => [],
                             bi  => fdp_twice,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_square'        => { in  => ['int','int'],
                             out => [],
                             bi  => fdp_square,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_subset'        => { in  => ['int','int'],
                             out => [],
                             bi  => fdp_subset,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_dsum'          => { in  => ['+value','+atom','int'],
                             out => [],
                             bi  => fdp_dsum,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_dsumC'         => { in  => ['+value','+value','+atom','int'],
                             out => [],
                             bi  => fdp_dsumC,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'fdp_sumAC'         => { in  => ['+value','+value','+atom','int'],
                             out => [],
                             bi  => fdp_sumAC,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'counter'           => { in  => ['int','value'],
                             out => [],
                             bi  => fdtest_counter,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'firstFail'         => { in  => ['+value','value'],
                             out => [],
                             bi  => fdtest_firstFail,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'sched_taskIntervalsProof'=> { in  => ['value','value','value','value',
                                           'value'],
                             out => [],
                             bi  => sched_taskIntervalsProof,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                                   native => true},

    'sched_firstsLasts' => { in  => ['value','value','value','value',
                                     'value'],
                             out => [],
                             bi  => sched_firstsLasts,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'spawnLess'         => { in  => ['int','int'],
                             out => [],
                             bi  => fdtest_spawnLess,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'dplus'             => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdtest_plus,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'sumac'             => { in  => ['value','value','int'],
                             out => [],
                             bi  => fdtest_sumac,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'testgensum'        => { in  => ['value','int'],
                             out => [],
                             bi  => fdtest_gensum,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'testsum'           => { in  => ['value','int'],
                             out => [],
                             bi  => fdtest_sum,
                             ifdef =>ALLDIFF,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'inqueens'          => { in  => ['value'],
                             out => [],
                             bi  => fdtest_inqueens,
                             ifdef =>INPROP,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd,
                             native => true},

    'debugStable'       => { in  => [],
                             out => [],
                             bi  => debugStable,
                             ifdef =>DEBUG_STABLE,
                             module=>fd,
                             native => true},

    'resetStable'       => { in  => [],
                             out => [],
                             bi  => resetStable,
                             ifdef =>DEBUG_STABLE,
                             module=>fd,
                             native => true},

    'fddistribute'      => { in  => ['value','value','value','value','value',],
                             out => [],
                             bi  => BIfdDistribute,
                             module=>fd,
                             native => true},

    #* Finite Sets

    'fsValueToString'   => { in  => ['+fset'],
                             out => ['+string'],
                             BI  => BIfsValueToString,
                             module=>fset ,
                             native => true},

    'fsIsVarB'          => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIfsIsVarB,
                             module=>fset ,
                             native => false},

    'fsIsValueB'        => { in  => ['+value','bool'],
                             out => [],
                             bi  => BIfsIsValueB,
                             module=>fset ,
                             native => true},

    'fsSetValue'        => { in  => ['+value','fset'],
                             out => [],
                             bi  => BIfsSetValue,
                             module=>fset ,
                             native => true},

    'fsSet'             => { in  => ['+value','+value','fset'],
                             out => [],
                             bi  => BIfsSet,
                             module=>fset ,
                             native => true},

    'fsSup'             => { in  => [],
                             out => ['+int'],
                             BI  => BIfsSup,
                             module=>fset ,
                             native => true},

    'fsGetKnownIn'      => { in  => ['fset','value'],
                             out => [],
                             bi  => BIfsGetKnownIn,
                             module=>fset ,
                             native => true},

    'fsGetKnownNotIn'   => { in  => ['fset','value'],
                             out => [],
                             bi  => BIfsGetKnownNotIn,
                             module=>fset ,
                             native => true},

    'fsGetUnknown'      => { in  => ['fset','value'],
                             out => [],
                             bi  => BIfsGetUnknown,
                             module=>fset ,
                             native => true},

    'fsGetGlb'          => { in  => ['fset','value'],
                             out => [],
                             bi  => BIfsGetKnownIn,
                             module=>fset ,
                             native => true},

    'fsGetLub'          => { in  => ['fset','value'],
                             out => [],
                             bi  => BIfsGetLub,
                             module=>fset ,
                             native => true},

    'fsGetCard'         => { in  => ['fset','value'],
                             out => [],
                             bi  => BIfsGetCard,
                             module=>fset ,
                             native => true},

    'fsCardRange'       => { in  => ['int','int','fset'],
                             out => [],
                             bi  => BIfsCardRange,
                             module=>fset ,
                             native => true},

    'fsGetNumOfKnownIn' => { in  => ['fset','int'],
                             out => [],
                             bi  => BIfsGetNumOfKnownIn,
                             module=>fset ,
                             native => true},

    'fsGetNumOfKnownNotIn'=> { in  => ['fset','int'],
                               out => [],
                               bi  => BIfsGetNumOfKnownNotIn,
                               module=>fset,
                               native => true},

    'fsGetNumOfUnknown' => { in  => ['fset','int'],
                             out => [],
                             bi  => BIfsGetNumOfUnknown,
                             module=>fset ,
                             native => true},

    'fsClone'           => { in  => ['fset','fset'],
                             out => [],
                             bi  => BIfsClone,
                             module=>fset ,
                             native => true},

    'fsp_init'          => { in  => [],
                             out => ['+atom'],
                             BI  => fsp_init,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS,
                             native => true},

    'fsp_isIn'          => { in  => ['int','fset','bool'],
                             out => [],
                             bi  => fsp_isIn,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_isInR'         => { in  => ['int','fset','int'],
                             out => [],
                             bi  => fsp_isInR,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_include'       => { in  => ['int','fset'],
                             out => [],
                             bi  => fsp_include,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_exclude'       => { in  => ['int','fset'],
                             out => [],
                             bi  => fsp_exclude,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_match'         => { in  => ['fset','+value'],
                             out => [],
                             bi  => fsp_match,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_seq'           => { in  => ['+value'],
                             out => [],
                             bi  => fsp_seq,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_minN'          => { in  => ['fset','+value'],
                             out => [],
                             bi  => fsp_minN,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_maxN'          => { in  => ['fset','+value'],
                             out => [],
                             bi  => fsp_maxN,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_card'          => { in  => ['fset','int'],
                             out => [],
                             bi  => fsp_card,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_union'         => { in  => ['fset','fset','fset'],
                             out => [],
                             bi  => fsp_union,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_intersection'  => { in  => ['fset','fset','fset'],
                             out => [],
                             bi  => fsp_intersection,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_subsume'       => { in  => ['fset','fset'],
                             out => [],
                             bi  => fsp_subsume,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_disjoint'      => { in  => ['fset','fset'],
                             out => [],
                             bi  => fsp_disjoint,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_distinct'      => { in  => ['fset','fset'],
                             out => [],
                             bi  => fsp_distinct,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_monitorIn'     => { in  => ['fset','value'],
                             out => [],
                             bi  => fsp_monitorIn,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_min'           => { in  => ['fset','int'],
                             out => [],
                             bi  => fsp_min,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_max'           => { in  => ['fset','int'],
                             out => [],
                             bi  => fsp_max,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_convex'        => { in  => ['fset'],
                             out => [],
                             bi  => fsp_convex,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_diff'          => { in  => ['fset','fset','fset'],
                             out => [],
                             bi  => fsp_diff,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_includeR'      => { in  => ['int','fset','int'],
                             out => [],
                             bi  => fsp_includeR,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_bounds'        => { in  => ['+fset','fset','int','int','int'],
                             out => [],
                             bi  => fsp_bounds,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_boundsN'       => { in  => ['+value','+value','+value',
                                     '+value','+value'],
                             out => [],
                             bi  => fsp_boundsN,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_disjointN'     => { in  => ['+value'],
                             out => [],
                             bi  => fsp_disjointN,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_unionN'        => { in  => ['+value','fset'],
                             out => [],
                             bi  => fsp_unionN,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_partition'     => { in  => ['+value','fset'],
                             out => [],
                             bi  => fsp_partition,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS ,
                             native => true},

    'fsp_partitionReified'=> { in  => ['+value','fset','+value'],
                             out => [],
                             bi  => fsp_partitionReified,
                             module=>fset,
                             ifndef=>FOREIGNFDPROPS,
                             native => true},

    'fsp_partitionProbing'=> { in  => ['+value','fset','+value'],
                               out => [],
                               bi  => fsp_partitionProbing,
                               module=>fset,
                               ifndef=>FOREIGNFDPROPS ,
                               native => true},

    'fsp_partitionReified1'=> { in  => ['+value','fset','+value','int'],
                                out => [],
                                bi  => fsp_partitionReified1,
                                module=>fset,
                                ifndef=>FOREIGNFDPROPS ,
                                native => true},



    #* Unclassified

    ##* Constraints

    'System.nbSusps'    => { in  => ['value'],
                             out => ['+int'],
                             BI  => BIconstraints,
                             native => true},

#    'SystemRegistry'   => { in  => [],
#                            out => ['+dictionary'],
#                            BI  => BIsystem_registry,
#                            native => true},
#
#    'ServiceRegistry'  => { in  => [],
#                            out => ['+dictionary'],
#                            BI  => BIsystem_registry,
#                            native => true},


};


# this is the function that converts these descriptions to
# an array of declarations appropriate for the emulator

sub CTABLE {
    my ($key,$info);
    while (($key,$info) = each %$builtins) {
        next unless &included($info);
        my $inArity = @{$info->{in}};
        my $outArity = @{$info->{out}};
        my $BI = $info->{BI};
        my @ifdef  = split(/\,/,$info->{ifdef});
        my @ifndef = split(/\,/,$info->{ifndef});
        my $macro;
        foreach $macro (@ifdef)  { print "#ifdef $macro\n"; }
        foreach $macro (@ifddef) { print "#ifndef $macro\n"; }
        my $native = $info->{native};
        if ( $native eq "true") {
            $native = "OK";
        } elsif ( $native eq "false") {
            $native = "NO";
        } else {
            die "*** native flag for $key must be 'true' or 'false'";
        }
        $BI = $info->{bi} unless $BI;
        print "{\"$key\",\t$inArity,\t$outArity,$BI,\t$native},\n";
        foreach $macro (@ifddef) { print "#endif\n"; }
        foreach $macro (@ifdef)  { print "#endif\n"; }
    }
}

sub argspec {
    my $spec = shift;
    my ($mod,$det,$typ,$own) = (0,'any','value',0);
    my $again = 1;

    # first we handle the various annotations

    while ($again) {
        # is the argument register side effected?
        if    ($spec =~ /^\!/) { $spec=$'; $mod=1; }
        # what is the determinacy condition on the argument?
        elsif ($spec =~ /^\+/) { $spec=$'; $det='det'; }
        elsif ($spec =~ /^\*/) { $spec=$'; $det='detOrKinded'; }
        # does it need its own register
        elsif ($spec =~ /^\^/) { $spec=$'; $own=1; }
        else { $again=0; }
    }

    # now parse the type of the argument

    if    ($spec =~ /^\[(.+)\#(.+)\]$/) { $typ="list(pair('$1' '$2'))"; }
    elsif ($spec =~ /^\[(.+)\]$/      ) { $typ="list('$1')"; }
    elsif ($spec =~ /^(.+)\#(.+)$/    ) { $typ="pair('$1' '$2')"; }
    else                                { $typ="'$spec'"; }

    return ($mod,$det,$typ,$own);
}

sub OZTABLE {
    my ($key,$info);
    while (($key,$info) = each %$builtins) {
        next unless &included($info);
        my (@imods,@idets,@ityps,$spec,$destroys,@oowns);
        foreach $spec (@{$info->{in}}) {
            my ($mod,$det,$typ,$own) = &argspec($spec);
            $destroys=1 if $mod;
            push @imods,($mod?'true':'false');
            push @idets,$det;
            push @ityps,$typ;
            die "found ^ annotation on input arg spec for builtin $key"
                if $own;
        }
        my (@odets,@otyps);
        foreach $spec (@{$info->{out}}) {
            my ($mod,$det,$typ,$own) = &argspec($spec);
            $det="any(det)" if $det eq 'det';
            push @odets,$det;
            push @otyps,$typ;
            push @oowns,($own?'true':'false');
        }
        print "'$key':\n\tbuiltin(\n";
        if ((@ityps+@otyps)>0) {
            print "\t\ttypes: [",join(' ',@ityps,@otyps),"]\n";
            print "\t\tdet: [",join(' ',@idets,@odets),"]\n";
        } else {
            print "\t\ttypes: nil\n";
            print "\t\tdet: nil\n";
        }

        if (@imods) {
            print "\t\timods: [",join(' ',@imods),"]\n";
        } else {
            print "\t\timods: nil\n";
        }
        if (@oowns) {
            print "\t\toowns: [",join(' ',@oowns),"]\n";
        } else {
            print "\t\toowns: nil\n";
        }
        if ($#otyps == 0 && $otyps[0] eq '\'bool\''
            && $odets[0] eq 'any(det)') {
            print "\t\ttest: true\n";
        }

        print "\t\tdoesNotReturn: true\n" if $info->{doesNotReturn};
        print "\t)\n";
    }
}

sub CDECL {
    my ($key,$info,$bi);
    while (($key,$info) = each %$builtins) {
        next unless &included($info);
        $bi = $info->{bi} || $info->{BI};
        print "OZ_C_proc_proto($bi);\n";
    }
}


sub checkNative {
    my $value = shift;
    printf "Builtins with nativeness: '$value'\n";
    printf "**************************************\n";
    my ($key,$info);
    foreach $key (sort keys %$builtins) {
        my $info = $builtins->{$key};
        if ( $info->{native} eq $value) {
            print "   $key\n";
        }
    }
    print "\n\n";
}

sub SORTNATIVENESS {
    &checkNative("true");
    &checkNative("false");
}

sub STRUCTURE {
    exec "grep '#\\*' $0 | sed -e 's/[ \t]*#/  /'g";
}

my %include = ();
my %exclude = ();
my $includedefault = 1;

sub included {
    my $info = shift;
    my $module = $info->{module} || 'oz';
    return 0 if $exclude{$module};
    return 1 if $include{$module};
    return $includedefault;
}

my ($option,$choice,@include,@exclude);

while (@ARGV) {
    $option = shift;
    if    ($option eq '-ctable' )    { $choice='ctable';  }
    elsif ($option eq '-cdecl'  )    { $choice='cdecl';   }
    elsif ($option eq '-oztable')    { $choice='oztable'; }
    elsif ($option eq '-sortnativeness') { $choice='sortnativeness'; }
    elsif ($option eq '-structure')   { $choice='structure'; }
    elsif ($option eq '-include')    { push @include,split(/\,/,shift); }
    elsif ($option eq '-exclude')    { push @exclude,split(/\,/,shift); }
    else { die "unrecognized option: $option"; }
}

if (@include!=0 && @exclude!=0) {
    die "cannot have both -include and -exclude";
}

foreach $option (@include) { $include{$option} = 1; }
foreach $option (@exclude) { $exclude{$option} = 1; }

$includedefault = 0 if @include!=0;

if    ($choice eq 'ctable' )    { &CTABLE;  }
elsif ($choice eq 'cdecl'  )    { &CDECL;   }
elsif ($choice eq 'oztable')    { &OZTABLE; }
elsif ($choice eq 'sortnativeness') { &SORTNATIVENESS; }
elsif ($choice eq 'structure')   { &STRUCTURE; }
else { die "must specify one of: -ctable -cdecl -oztable -structure -sortnativeness"; }
