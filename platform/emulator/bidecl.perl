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
### SIMPLE    ::= abstraction           (not yet known to compiler)
###             | atom
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
###             | foreignPointer        (not yet known to compiler)
###             | int
###             | literal
###             | lock
###             | name
###             | number
###             | object
###             | port
###             | procedure
###             | procedure/1
###             | procedureOrObject     (not yet known to compiler)
###             | record
###             | recordC
###             | recordCOrChunk
###             | space
###             | string
###             | thread
###             | tuple
###             | value
###             | virtualString
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
### eqeq => 1, indicates that the builtin can be specially compiled using
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

$builtins = {
    'builtin'   => { in  => ['+virtualString','+int'],
                     out => ['+procedure'],
                     BI  => BIbuiltin},

    '/'         => { in  => ['+float','+float'],
                     out => ['+float'],
                     bi  => BIfdiv,
                     ibi => BIfdivInline },

    '*'         => { in  => ['+number','+number'],
                     out => ['+number'],
                     bi  => BImult,
                     ibi => BImultInline},

    'div'       => { in  => ['+int','+int'],
                     out => ['+int'],
                     bi  => BIdiv,
                     ibi => BIdivInline},

    'mod'       => { in  => ['+int','+int'],
                     out => ['+int'],
                     bi  => BImod,
                     ibi => BImodInline},

    '-'         => { in  => ['+number','+number'],
                     out => ['+number'],
                     bi  => BIminus,
                     ibi => BIminusInline},

    '+'         => { in  => ['+number','+number'],
                     out => ['+number'],
                     bi  => BIplus,
                     ibi => BIplusInline},

    'Max'       => { in  => ['+comparable','+comparable'],
                     out => ['+comparable'],
                     bi  => BImax,
                     ibi => BImaxInline},

    'Min'       => { in  => ['+comparable','+comparable'],
                     out => ['+comparable'],
                     bi  => BImin,
                     ibi => BIminInline},

    '<'         => { in  => ['+comparable','+comparable'],
                     out => ['+bool'],
                     bi  => BIlessFun,
                     ibi => BIlessInlineFun,
                     shallow => '<Rel' },

    '=<'        => { in  => ['+comparable','+comparable'],
                     out => ['+bool'],
                     bi  => BIleFun,
                     ibi => BIleInlineFun,
                     shallow => '=<Rel' },

    '>'         => { in  => ['+comparable','+comparable'],
                     out => ['+bool'],
                     bi  => BIgreatFun,
                     ibi => BIgreatInlineFun,
                     shallow => '>Rel' },

    '>='        => { in  => ['+comparable','+comparable'],
                     out => ['+bool'],
                     bi  => BIgeFun,
                     ibi => BIgeInlineFun,
                     shallow => '>=Rel' },

    '=<Rel'     => { in  => ['+comparable','+comparable'],
                     out => [],
                     bi  => BIle,
                     ibi => BIleInline},

    '<Rel'      => { in  => ['+comparable','+comparable'],
                     out => [],
                     bi  => BIless,
                     ibi => BIlessInline},

    '>=Rel'     => { in  => ['+comparable','+comparable'],
                     out => [],
                     bi  => BIge,
                     ibi => BIgeInline},

    '>Rel'      => { in  => ['+comparable','+comparable'],
                     out => [],
                     bi  => BIgreat,
                     ibi => BIgreatInline},

    '~'         => { in  => ['+number'],
                     out => ['+number'],
                     bi  => BIuminus,
                     ibi => BIuminusInline},

    '+1'        => { in  => ['+int'],
                     out => ['+int'],
                     bi  => BIadd1,
                     ibi => BIadd1Inline},

    '-1'        => { in  => ['+int'],
                     out => ['+int'],
                     bi  => BIsub1,
                     ibi => BIsub1Inline},

    'Exp'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIexp,
                     ibi => BIinlineExp},

    'Log'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIlog,
                     ibi => BIinlineLog},

    'Sqrt'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIsqrt,
                     ibi => BIinlineSqrt},

    'Sin'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIsin,
                     ibi => BIinlineSin},

    'Asin'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIasin,
                     ibi => BIinlineAsin},

    'Cos'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIcos,
                     ibi => BIinlineCos},

    'Acos'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIacos,
                     ibi => BIinlineAcos},

    'Tan'       => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BItan,
                     ibi => BIinlineTan},

    'Atan'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIatan,
                     ibi => BIinlineAtan},

    'Ceil'      => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIceil,
                     ibi => BIinlineCeil},

    'Floor'     => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIfloor,
                     ibi => BIinlineFloor},

    'Abs'       => { in  => ['+number'],
                     out => ['+number'],
                     bi  => BIabs,
                     ibi => BIabsInline},

    'Round'     => { in  => ['+float'],
                     out => ['+float'],
                     bi  => BIround,
                     ibi => BIinlineRound},

    'Atan2'     => { in  => ['+float','+float'],
                     out => ['+float'],
                     bi  => BIatan2,
                     ibi => BIatan2Inline},

    'fPow'      => { in  => ['+float','+float'],
                     out => ['+float'],
                     bi  => BIfPow,
                     ibi => BIfPowInline},

    # conversion: float <-> int <-> virtualStrings

    'IntToFloat'        => { in  => ['+int'],
                             out => ['+float'],
                             bi  => BIintToFloat,
                             ibi => BIintToFloatInline},

    'FloatToInt'        => { in  => ['+float'],
                             out => ['+int'],
                             bi  => BIfloatToInt,
                             ibi => BIfloatToIntInline},

    'IntToString'       => { in  => ['+int'],
                             out => ['+string'],
                             BI  => BIintToString}, # new style builtin

    'FloatToString'     => { in  => ['+float'],
                             out => ['+string'],
                             BI  => BIfloatToString},

    'StringToInt'       => { in  => ['+string'],
                             out => ['+int'],
                             BI  => BIstringToInt},

    'StringToFloat'     => { in  => ['+string'],
                             out => ['+float'],
                             BI  => BIstringToFloat},

    'String.isInt'      => { in  => ['+string'],
                             out => ['+bool'],
                             BI  => BIstringIsInt},

    'String.isFloat'    => { in  => ['+string'],
                             out => ['+bool'],
                             BI  => BIstringIsFloat},

    'String.isAtom'     => { in  => ['+string'],
                             out => ['+bool'],
                             BI  => BIstringIsAtom},

    'IsArray'           => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisArray,
                             ibi => isArrayInline},

    'NewArray'          => { in  => ['+int','+int','value'],
                             out => ['+array'],
                             BI  => BIarrayNew},

    'Array.high'        => { in  => ['+array'],
                             out => ['+int'],
                             bi  => BIarrayHigh,
                             ibi => arrayHighInline},

    'Array.low'         => { in  => ['+array'],
                             out => ['+int'],
                             bi  => BIarrayLow,
                             ibi => arrayLowInline},

    'Get'               => { in  => ['+array','+int'],
                             out => ['value'],
                             bi  => BIarrayGet,
                             ibi => arrayGetInline},

    'Put'               => { in  => ['+array','+int','value'],
                             out => [],
                             bi  => BIarrayPut,
                             ibi => arrayPutInline},


    'NewDictionary'     => { in  => [],
                             out => ['+dictionary'],
                             BI  => BIdictionaryNew},

    'IsDictionary'      => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisDictionary,
                             ibi => isDictionaryInline},

    'Dictionary.isEmpty'=> { in  => ['+dictionary'],
                             out => ['+bool'],
                             bi  => BIdictionaryIsMt,
                             ibi => dictionaryIsMtInline},

    'Dictionary.get'    => { in  => ['+dictionary','+feature'],
                             out => ['value'],
                             bi  => BIdictionaryGet,
                             ibi => dictionaryGetInline},

    'Dictionary.condGet'=> { in  => ['+dictionary','+feature','value'],
                             out => ['value'],
                             bi  => BIdictionaryCondGet,
                             ibi => dictionaryCondGetInline},

    'Dictionary.put'    => { in  => ['+dictionary','+feature','value'],
                             out => [],
                             bi  => BIdictionaryPut,
                             ibi => dictionaryPutInline},

    'Dictionary.condPut'=> { in  => ['+dictionary','+feature','value'],
                             out => [],
                             bi  => BIdictionaryCondPut,
                             ibi => dictionaryCondPutInline},

    'Dictionary.exchange'=> { in  => ['+dictionary','+feature','value',
                                      'value'],
                              out => [],
                              BI  => BIdictionaryExchange},

    'Dictionary.condExchange' => { in  => ['+dictionary','+feature','value',
                                           'value','value'],
                                   out => [],
                                   BI  => BIdictionaryCondExchange},

    'Dictionary.remove' => { in  => ['+dictionary','+feature'],
                             out => [],
                             bi  => BIdictionaryRemove,
                             ibi => dictionaryRemoveInline},

    'Dictionary.removeAll'=> { in  => ['+dictionary'],
                               out => [],
                               BI  => BIdictionaryRemoveAll},

    'Dictionary.member' => { in  => ['+dictionary','+feature'],
                             out => ['+bool'],
                             bi  => BIdictionaryMember,
                             ibi => dictionaryMemberInline},

    'Dictionary.keys' => { in  => ['+dictionary'],
                           out => ['+[feature]'],
                           BI  => BIdictionaryKeys},

    'Dictionary.entries' => { in  => ['+dictionary'],
                              out => ['+[feature#value]'],
                              BI  => BIdictionaryEntries},

    'Dictionary.items' => { in  => ['+dictionary'],
                            out => ['+[value]'],
                            BI  => BIdictionaryItems},

    'Dictionary.clone' => { in  => ['+dictionary'],
                            out => ['+dictionary'],
                            BI  => BIdictionaryClone},

    'Dictionary.markSafe' => { in  => ['+dictionary'],
                               out => [],
                               BI  => BIdictionaryMarkSafe},

    'NewLock'           => { in  => [],
                             out => ['+lock'],
                             BI  => BInewLock},

    'Lock'              => { in  => ['+lock'],
                             out => [],
                             BI  => BIlockLock},

    'Unlock'            => { in  => ['+lock'],
                             out => [],
                             BI  => BIunlockLock},


    'NewPort'           => { in  => ['value'],
                             out => ['+port'],
                             BI  => BInewPort},

    'Send'              => { in  => ['+port','value'],
                             out => [],
                             BI  => BIsendPort},

    'NewCell'           => { in  => ['value'],
                             out => ['+cell'],
                             BI  => BInewCell},

    'Exchange'          => { in  => ['+cell','value','value'],
                             out => [],
                             bi  => BIexchangeCell,
                             ibi => BIexchangeCellInline},

    'Access'            => { in  => ['+cell'],
                             out => ['value'],
                             bi  => BIaccessCell,
                             ibi => BIaccessCellInline},

    'Assign'            => { in  => ['+cell','value'],
                             out => [],
                             bi  => BIassignCell,
                             ibi => BIassignCellInline},

    # perdio

    'probe'             => { in  => ['value'],
                             out => [],
                             BI  => BIprobe},

    'perdioRestop'      => { in  => ['value'],
                             out => [],
                             BI  => BIrestop},

    'crash'             => { in  => [],
                             out => [],
                             BI  => BIcrash,
                             doesNotReturn=>1},

    'InstallHandler'    => { in  => ['+value','+value','value'],
                             out => [],
                             BI  => BIhandlerInstall},

    'InstallWatcher'    => { in  => ['+value','+value','value'],
                             out => [],
                             BI  => BIwatcherInstall},

    # characters

    'IsChar'            => { in  => ['+value'],
                             out => ['+bool'],
                             BI  => BIcharIs},

    'Char.isAlNum'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsAlNum},

    'Char.isAlpha'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsAlpha},

    'Char.isCntrl'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsCntrl},

    'Char.isDigit'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsDigit},

    'Char.isGraph'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsGraph},

    'Char.isLower'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsLower},

    'Char.isPrint'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsPrint},

    'Char.isPunct'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsPunct},

    'Char.isSpace'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsSpace},

    'Char.isUpper'      => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsUpper},

    'Char.isXDigit'     => { in  => ['+char'],
                             out => ['+bool'],
                             BI  => BIcharIsXDigit},

    'Char.toLower'      => { in  => ['+char'],
                             out => ['+char'],
                             BI  => BIcharToLower},

    'Char.toUpper'      => { in  => ['+char'],
                             out => ['+char'],
                             BI  => BIcharToUpper},

    'Char.toAtom'       => { in  => ['+char'],
                             out => ['+atom'],
                             BI  => BIcharToAtom},

    'Char.type'         => { in  => ['+char'],
                             out => ['+atom'],
                             BI  => BIcharType},

    # records

    'Adjoin'            => { in  => ['+record','+record'],
                             out => ['+record'],
                             bi  => BIadjoin,
                             ibi => BIadjoinInline},

    'AdjoinList'        => { in  => ['+record','+[feature#value]'],
                             out => ['+record'],
                             BI  => BIadjoinList},

    'record'            => { in  => ['+literal','+[feature#value]'],
                             out => ['+record'],
                             BI  => BImakeRecord},

    'Arity'             => { in  => ['+record'],
                             out => ['+[feature]'],
                             bi  => BIarity,
                             ibi => BIarityInline},

    'AdjoinAt'          => { in  => ['+record','+feature','value'],
                             out => ['+record'],
                             BI  => BIadjoinAt},

    # types tests

    'IsNumber'          => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisNumberB,
                             ibi => BIisNumberBInline,
                             shallow => isNumberRel},

    'IsInt'             => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisIntB,
                             ibi => BIisIntBInline,
                             shallow => isIntRel},

    'IsFloat'           => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisFloatB,
                             ibi => BIisFloatBInline,
                             shallow => isFloatRel},

    'IsRecord'          => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisRecordB,
                             ibi => isRecordBInline,
                             shallow => isRecordRel},

    'IsTuple'           => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisTupleB,
                             ibi => isTupleBInline,
                             shallow => isTupleRel},

    'IsLiteral'         => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisLiteralB,
                             ibi => isLiteralBInline,
                             shallow => isLiteralRel},

    'IsLock'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisLockB,
                             ibi => isLockBInline,
                             shallow => isLockRel},

    'IsCell'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisCellB,
                             ibi => isCellBInline,
                             shallow => isCellRel},

    'IsPort'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisPortB,
                             ibi => isPortBInline,
                             shallow => isPortRel},

    'IsProcedure'       => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisProcedureB,
                             ibi => isProcedureBInline,
                             shallow => isProcedureRel},

    'IsName'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisNameB,
                             ibi => isNameBInline,
                             shallow => isNameRel},

    'IsAtom'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisAtomB,
                             ibi => isAtomBInline,
                             shallow => isAtomRel},

    'IsBool'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisBoolB,
                             ibi => isBoolBInline,
                             shallow => isBoolRel},

    'IsUnit'            => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisUnitB,
                             ibi => isUnitBInline,
                             shallow => isUnitRel},

    'IsChunk'           => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisChunkB,
                             ibi => isChunkBInline,
                             shallow => isChunkRel},

    'IsRecordC'         => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisRecordCB,
                             ibi => isRecordCBInline,
                             shallow => isRecordCRel},

    'IsObject'          => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisObjectB,
                             ibi => BIisObjectBInline,
                             shallow => isObjectRel},

    'IsString'          => { in  => ['+value'],
                             out => ['+bool'],
                             BI  => BIisString},

    'IsVirtualString'   => { in  => ['!+value'],
                             out => ['+bool'],
                             BI  => BIvsIs},

    'IsFree'            => { in  => ['value'],
                             out => ['+bool'],
                             bi  => BIisFree,
                             ibi => isFreeInline,
                             shallow => IsFreeRel},

    'IsKinded'          => { in  => ['value'],
                             out => ['+bool'],
                             bi  => BIisKinded,
                             ibi => isKindedInline,
                             shallow => IsKindedRel},

    'IsDet'             => { in  => ['value'],
                             out => ['+bool'],
                             bi  => BIisDet,
                             ibi => isDetInline,
                             shallow => IsDetRel},

    'isNumberRel'       => { in  => ['+value'],
                             out => [],
                             bi  => BIisNumber,
                             ibi => BIisNumberInline},

    'isIntRel'          => { in  => ['+value'],
                             out => [],
                             bi  => BIisInt,
                             ibi => BIisIntInline},

    'isFloatRel'        => { in  => ['+value'],
                             out => [],
                             bi  => BIisFloat,
                             ibi => BIisFloatInline},

    'isRecordRel'       => { in  => ['+value'],
                             out => [],
                             bi  => BIisRecord,
                             ibi => isRecordInline},

    'isTupleRel'        => { in  => ['+value'],
                             out => [],
                             bi  => BIisTuple,
                             ibi => isTupleInline},

    'isLiteralRel'      => { in  => ['+value'],
                             out => [],
                             bi  => BIisLiteral,
                             ibi => isLiteralInline},

    'isCellRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisCell,
                             ibi => isCellInline},

    'isPortRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisPort,
                             ibi => isPortInline},

    'isProcedureRel'    => { in  => ['+value'],
                             out => [],
                             bi  => BIisProcedure,
                             ibi => isProcedureInline},

    'isNameRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisName,
                             ibi => isNameInline},

    'isAtomRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisAtom,
                             ibi => isAtomInline},

    'isLockRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisLock,
                             ibi => isLockInline},

    'isBoolRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisBool,
                             ibi => isBoolInline},

    'isUnitRel'         => { in  => ['+value'],
                             out => [],
                             bi  => BIisUnit,
                             ibi => isUnitInline},

    'isChunkRel'        => { in  => ['+value'],
                             out => [],
                             bi  => BIisChunk,
                             ibi => isChunkInline},

    'isRecordCRel'      => { in  => ['+value'],
                             out => [],
                             bi  => BIisRecordC,
                             ibi => isRecordCInline},

    'isObjectRel'       => { in  => ['+value'],
                             out => [],
                             bi  => BIisObject,
                             ibi => BIisObjectInline},

    'IsFreeRel'         => { in  => ['value'],
                             out => [],
                             bi  => BIisFreeRel,
                             ibi => isFreeRelInline},

    'IsKindedRel'       => { in  => ['value'],
                             out => [],
                             bi  => BIisKindedRel,
                             ibi => isKindedRelInline},

    'IsDetRel'          => { in  => ['value'],
                             out => [],
                             bi  => BIisDetRel,
                             ibi => isDetRelInline},

    'Wait'              => { in  => ['+value'],
                             out => [],
                             bi  => BIisValue,
                             ibi => isValueInline},

    'WaitOr'            => { in  => ['value','value'],
                             out => [],
                             BI  => BIwaitOr},

    'virtualStringLength'=> { in  => ['!virtualString','!+int'],
                              out => ['+int'],
                              BI  => BIvsLength},


    'Length'            => { in  => ['+[value]'],
                             out => ['+int'],
                             BI  => BIlength},

    'Not'               => { in  => ['+bool'],
                             out => ['+bool'],
                             bi  => BInot,
                             ibi => notInline},

    'And'               => { in  => ['+bool','+bool'],
                             out => ['+bool'],
                             bi  => BIand,
                             ibi => andInline},

    'Or'                => { in  => ['+bool','+bool'],
                             out => ['+bool'],
                             bi  => BIor,
                             ibi => orInline},

    'Type.ofValue'      => { in  => ['+value'],
                             out => ['+atom'],
                             bi  => BItermType,
                             ibi => BItermTypeInline},

    'Value.status'      => { in  => ['value'],
                             out => ['+tuple'],
                             bi  => BIstatus,
                             ibi => BIstatusInline},

    # deep magic for procedures

    'procedureEnvironment'=> { in  => ['+procedure'],
                               out => ['+tuple'],
                               BI  => BIprocedureEnvironment},

    'getProcInfo'       => { in  => ['+procedure'],
                             out => ['value'],
                             BI  => BIgetProcInfo},

    'setProcInfo'       => { in  => ['+procedure','value'],
                             out => [],
                             BI  => BIsetProcInfo},

    'getProcNames'      => { in  => ['+procedure'],
                             out => ['+[name]'],
                             BI  => BIgetProcNames},

    'setProcNames'      => { in  => ['+procedure','+[name]'],
                             out => [],
                             BI  => BIsetProcNames},

    'getProcPos'        => { in  => ['+procedure'],
                             out => ['+literal','+int'],
                             BI  => BIgetProcPos},

    # tuples and records and OFS

    'MakeTuple'         => { in  => ['+literal','+int'],
                             out => ['+tuple'],
                             bi  => BItuple,
                             ibi => tupleInline},

    'Label'             => { in  => ['*recordC'],
                             out => ['+literal'],
                             bi  => BIlabel,
                             ibi => labelInline},

    'hasLabel'          => { in  => ['value'],
                             out => ['+bool'],
                             bi  => BIhasLabel,
                             ibi => hasLabelInline},

    'ProcedureArity'    => { in  => ['+procedure'],
                             out => ['+int'],
                             bi  => BIprocedureArity,
                             ibi => procedureArityInline},

    'TellRecord'        => { in  => ['+literal','record'],
                             out => [],
                             BI  => BIrecordTell},

    'WidthC'            => { in  => ['*record','int'],
                             out => [],
                             BI  => BIwidthC},

    'monitorArity'      => { in  => ['*recordC','value','[feature]'],
                             out => [],
                             BI  => BImonitorArity},

    'tellRecordSize'    => { in  => ['+literal','+int','record'],
                             out => [],
                             BI  => BIsystemTellSize},

    'recordCIsVarB'     => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIisRecordCVarB},

    # records and chunks

    '.'                 => { in  => ['*recordCOrChunk','+feature'],
                             out => ['value'],
                             bi  => BIdot,
                             ibi => dotInline},

    '^'                 => { in  => ['*recordCOrChunk','+feature'],
                             out => ['value'],
                             bi  => BIuparrowBlocking,
                             ibi => uparrowInlineBlocking},

    'HasFeature'        => { in  => ['*recordCOrChunk','+feature'],
                             out => ['+bool'],
                             bi  => BIhasFeatureB,
                             ibi => hasFeatureBInline},

    'CondSelect'        => { in  => ['*recordCOrChunk','+feature','value'],
                             out => ['value'],
                             bi  => BImatchDefault,
                             ibi => matchDefaultInline},

    'Width'             => { in  => ['+record'],
                             out => ['+int'],
                             bi  => BIwidth,
                             ibi => widthInline},

    # atoms

    'AtomToString'      => { in  => ['+atom'],
                             out => ['+string'],
                             bi  => BIatomToString,
                             ibi => atomToStringInline},

    'StringToAtom'      => { in  => ['+string'],
                             out => ['+atom'],
                             BI  => BIstringToAtom},

    # chunks

    'NewChunk'          => { in  => ['+record'],
                             out => ['+chunk'],
                             BI  => BInewChunk},

    'chunkArity'        => { in  => ['+chunk'],
                             out => ['+[feature]'],
                             BI  => BIchunkArity},

    'chunkWidth'        => { in  => ['+chunk'],
                             out => ['+int'],
                             BI  => BIchunkWidth},

    'recordWidth'       => { in  => ['record'],
                             out => ['int'],
                             BI  => BIrecordWidth},

    # names

    'NewName'           => { in  => [],
                             out => ['+name'],
                             BI  => BInewName},

    'NewUniqueName'     => { in  => ['+atom'],
                             out => ['+name'],
                             BI  => BInewUniqueName},

    # equalities

    '=='                => { in  => ['*value','*value'],
                             out => ['+bool'],
                             bi  => BIeqB,
                             ibi => eqeqInline,
                             eqeq => 1},

    '\\\\='             => { in  => ['*value','*value'],
                             out => ['+bool'],
                             bi  => BIneqB,
                             ibi => neqInline,
                             eqeq => 1},

    '==Rel'             => { in  => ['*value','*value'],
                             out => [],
                             BI  => BIeq},

    '\\\\=Rel'          => { in  => ['*value','*value'],
                             out => [],
                             BI  => BIneq},

    # dynamic libraries

    'isForeignPointer'  => { in  => ['+value'],
                             out => ['+bool'],
                             BI  => BIisForeignPointer },

    'ForeignPointerToInt'=> { in  => ['+foreignPointer'],
                              out => ['+int'],
                              BI  => BIForeignPointerToInt},


    'dlOpen'            => { in  => ['+virtualString'],
                             out => ['+foreignPointer'],
                             BI  => BIdlOpen},

    'dlClose'           => { in  => ['+foreignPointer'],
                             out => [],
                             BI  => BIdlClose},

    'findFunction'      => { in  => ['+virtualString','+int',
                                     '+foreignPointer'],
                             out => [],
                             BI  => BIfindFunction},

    'dlLoad'            => { in  => ['+virtualString'],
                             out => ['+foreignPointer#record'],
                             BI  => BIdlLoad},

    # miscellaneous system things

    'shutdown'          => { in  => ['+int'],
                             out => [],
                             BI  => BIshutdown},

    'Alarm'             => { in  => ['+int','unit'],
                             out => [],
                             BI  => BIalarm},

    'Delay'             => { in  => ['!+int'],
                             out => [],
                             BI  => BIdelay},

    'System.gcDo'       => { in  => [],
                             out => [],
                             BI  => BIgarbageCollection},

    'System.apply'      => { in  => ['+procedureOrObject','+[value]'],
                             out => [],
                             BI  => BIapply},

    'System.eq'         => { in  => ['value','value'],
                             out => ['+bool'],
                             BI  => BIsystemEq},

    '='                 => { in  => ['value','value'],
                             out => [],
                             BI  => BIunify},

    'fail'              => { in  => [],
                             out => [],
                             BI  => BIfail},

    'nop'               => { in  => [],
                             out => [],
                             BI  => BInop},

    'deepFeed'          => { in  => ['+cell','value'],
                             out => [],
                             BI  => BIdeepFeed},

    # browser primitives

    'getsBoundB'        => { in  => ['value','value'],
                             out => [],
                             BI  => BIgetsBoundB},

    # useless

    'setAbstractionTabDefaultEntry' => { in  => ['value'],
      out => [],
      BI  => BIsetAbstractionTabDefaultEntry},

    # printing

    'showBuiltins'      => { in  => [],
                             out => [],
                             BI  => BIshowBuiltins},

    'Print'             => { in  => ['value'],
                             out => [],
                             bi  => BIprint,
                             ibi => printInline},

    'Show'              => { in  => ['value'],
                             out => [],
                             bi  => BIshow,
                             ibi => showInline},

    # constraints

    'System.nbSusps'    => { in  => ['value'],
                             out => ['+int'],
                             BI  => BIconstraints},

    # miscellaneous

    'onToplevel'        => { in  => [],
                             out => ['+bool'],
                             BI  => BIonToplevel},

    # browser

    'addr'              => { in  => ['value'],
                             out => ['+int'],
                             BI  => BIaddr},

    # source level debugger

    'Debug.mode'        => { in  => [],
                             out => ['+bool'],
                             BI  => BIdebugmode},

    'Debug.getStream'   => { in  => [],
                             out => ['value'],
                             BI  => BIgetDebugStream},

    'Debug.setStepFlag' => { in  => ['+thread','+bool'],
                             out => [],
                             BI  => BIsetStepFlag},

    'Debug.setTraceFlag'=> { in  => ['+thread','+bool'],
                             out => [],
                             BI  => BIsetTraceFlag},

    'Debug.checkStopped'=> { in  => ['+thread'],
                             out => ['+bool'],
                             BI  => BIcheckStopped},

    # Debug module

    'Debug.prepareDumpThreads'  => { in  => [],
                                     out => [],
                                     BI  => BIprepareDumpThreads},

    'Debug.dumpThreads' => { in  => [],
                             out => [],
                             BI  => BIdumpThreads},

    'Debug.listThreads' => { in  => [],
                             out => ['+[thread]'],
                             BI  => BIlistThreads},

    'Debug.breakpointAt'=> { in  => ['+atom','+int','+bool'],
                             out => ['+bool'],
                             BI  => BIbreakpointAt},

    'Debug.breakpoint'  => { in  => [],
                             out => [],
                             BI  => BIbreakpoint},

    'Debug.displayCode' => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIdisplayCode},

    'Debug.procedureCode'=> { in  => ['+procedure'],
                              out => ['+int'],
                              BI  => BIprocedureCode},

    'Debug.procedureCoord'=> { in  => ['+procedure'],
                               out => ['+record'],
                               BI  => BIprocedureCoord},

    'Debug.livenessX'   => { in  => ['+int'],
                             out => ['+int'],
                             BI  => BIlivenessX},

    'index2Tagged'      => { in  => ['int'],
                             out => ['value'],
                             BI  => BIindex2Tagged,
                             ifdef=>'UNUSED'},

    'time2localTime'    => { in  => ['int'],
                             out => ['value'],
                             BI  => BItime2localTime,
                             ifdef=>'UNUSED'},

    # Builtins for the Thread module

    'Thread.is'         => { in  => ['+value'],
                             out => ['+bool'],
                             BI  => BIthreadIs},

    'Thread.id'         => { in  => ['+thread'],
                             out => ['+int'],
                             BI  => BIthreadID},

    'Thread.setId'      => { in  => ['+thread','+int'],
                             out => [],
                             BI  => BIsetThreadID},

    'Thread.parentId'   => { in  => ['+thread'],
                             out => ['+int'],
                             BI  => BIparentThreadID},

    'Thread.this'       => { in  => [],
                             out => ['+thread'],
                             BI  => BIthreadThis},

    'Thread.suspend'    => { in  => ['+thread'],
                             out => [],
                             BI  => BIthreadSuspend},

    'Thread.unleash'    => { in  => ['+thread','+int'],
                             out => [],
                             BI  => BIthreadUnleash},

    'Thread.resume'     => { in  => ['+thread'],
                             out => [],
                             BI  => BIthreadResume},

    'Thread.injectException'=> { in  => ['+thread','+value'],
                                 out => [],
                                 BI  => BIthreadRaise},

    'Thread.preempt'    => { in  => ['+thread'],
                             out => [],
                             BI  => BIthreadPreempt},

    'Thread.setPriority'=> { in  => ['+thread','+atom'],
                             out => [],
                             BI  => BIthreadSetPriority},

    'Thread.getPriority'=> { in  => ['+thread'],
                             out => ['+atom'],
                             BI  => BIthreadGetPriority},

    'Thread.isSuspended'=> { in  => ['+thread'],
                             out => ['+bool'],
                             BI  => BIthreadIsSuspended},

    'Thread.state'      => { in  => ['+thread'],
                             out => ['+atom'],
                             BI  => BIthreadState},

    'Thread.setRaiseOnBlock'=> { in  => ['+thread','+bool'],
                                 out => [],
                                 BI  => BIthreadSetRaiseOnBlock},

    'Thread.getRaiseOnBlock'=> { in  => ['+thread'],
                                 out => ['+bool'],
                                 BI  => BIthreadGetRaiseOnBlock},

    'Thread.taskStack'  => { in  => ['+thread','+int','+bool'],
                             out => ['+[record]'],
                             BI  => BIthreadTaskStack},

    'Thread.frameVariables'=> { in  => ['+thread','+int'],
                                out => ['+record'],
                                BI  => BIthreadFrameVariables},

    'Thread.location'   => { in  => ['+thread'],
                             out => ['+[atom]'],
                             BI  => BIthreadLocation},

    # printing primitives for debugging

    'Debug.print'       => { in  => ['value','+int'],
                             out => [],
                             BI  => BIdebugPrint,
                             ifdef=>'DEBUG_PRINT'},

    'Debug.printLong'   => { in  => ['value','+int'],
                             out => [],
                             BI  => BIdebugPrintLong,
                             ifdef=>'DEBUG_PRINT'},

    # statistics

    'statisticsReset'   => { in  => [],
                             out => [],
                             BI  => BIstatisticsReset},

    'statisticsPrint'   => { in  => ['+virtualString'],
                             out => [],
                             BI  => BIstatisticsPrint},

    'statisticsPrintProcs'=> { in  => [],
                               out => [],
                               BI  => BIstatisticsPrintProcs},

    'statisticsGetProcs'=> { in  => [],
                             out => ['+value'],
                             BI  => BIstatisticsGetProcs},

    'setProfileMode'    => { in  => ['+bool'],
                             out => [],
                             BI  => BIsetProfileMode},

    'instructionsPrint' => { in  => [],
                             out => [],
                             BI  => BIinstructionsPrint,
                             ifdef=>'PROFILE_INSTR'},

    'biPrint'           => { in  => [],
                             out => [],
                             BI  => BIbiPrint,
                             ifdef=>'PROFILE_BI'},

    'halt'              => { in  => [],
                             out => [],
                             BI  => BIhalt,
                             ifdef=>'DEBUG_TRACE'},

    # system primitives

    'System.printName'  => { in  => ['value'],
                             out => ['+atom'],
                             BI  => BIgetPrintName},

    'System.printInfo'  => { in  => ['virtualString'],
                             out => [],
                             BI  => BIprintInfo},

    'System.printError' => { in  => ['virtualString'],
                             out => [],
                             BI  => BIprintError},

    'System.valueToVirtualString'=> { in  => ['value','+int','+int'],
                                      out => ['+string'],
                                      BI  => BItermToVS},

    'getTermSize'       => { in  => ['value','+int','+int'],
                             out => ['+int'],
                             BI  => BIgetTermSize},

    # FD linking info

    'foreignFDProps'    => { in  => [],
                             out => ['+bool'],
                             BI  => BIforeignFDProps},

    # object-oriented primitives

    '@'                 => { in  => ['+feature'],
                             out => ['value'],
                             bi  => BIat,
                             ibi => atInline},

    '<-'                => { in  => ['+feature','value'],
                             out => [],
                             bi  => BIassign,
                             ibi => assignInline},

    'copyRecord'        => { in  => ['+record'],
                             out => ['+record'],
                             BI  => BIcopyRecord},

    'makeClass'         => { in  => ['+dictionary','+record','+record',
                                     '+dictionary','+bool'],
                             out => ['+class'],
                             BI  => BImakeClass},

    ','                 => { in  => ['+class','+record'],
                             out => [],
                             bi  => BIcomma},

    'send'              => { in  => ['+record','+class','+object'],
                             out => [],
                             bi  => BIsend},

    'getClass'          => { in  => ['+object'],
                             out => ['+class'],
                             bi  => BIgetClass,
                             ibi => getClassInline},

    'ooGetLock'         => { in  => [],
                             out => ['+lock'],
                             bi  => BIooGetLock,
                             ibi => ooGetLockInline},

    'newObject'         => { in  => ['+class'],
                             out => ['+object'],
                             bi  => BInewObject,
                             ibi => newObjectInline},

    'New'               => { in  => ['+class','+record'],
                             out => ['+object'],
                             bi  => BINew},

    'setSelf'           => { in  => ['+object'],
                             out => [],
                             BI  => BIsetSelf},

    'ooExch'            => { in  => ['+feature','value'],
                             out => ['value'],
                             bi  => BIooExch,
                             ibi => ooExchInline},

    # spaces

    'Space.new'         => { in  => ['+procedure/1'],
                             out => ['+space'],
                             BI  => BInewSpace},

    'IsSpace'           => { in  => ['+value'],
                             out => ['+bool'],
                             BI  => BIisSpace},

    'Space.ask'         => { in  => ['+space'],
                             out => ['+tuple'],
                             BI  => BIaskSpace},

    'Space.askVerbose'  => { in  => ['+space','!value'],
                             out => [],
                             BI  => BIaskVerboseSpace},

    'Space.merge'       => { in  => ['+space'],
                             out => ['+value'],
                             BI  => BImergeSpace},

    'Space.clone'       => { in  => ['+space'],
                             out => ['+space'],
                             BI  => BIcloneSpace},

    'Space.commit'      => { in  => ['+space','+value'],
                             out => [],
                             BI  => BIcommitSpace},

    'Space.inject'      => { in  => ['+space','+procedure/1'],
                             out => [],
                             BI  => BIinjectSpace},

    # exceptions

    'biExceptionHandler'=> { in  => ['value'],
                             out => [],
                             BI  => BIbiExceptionHandler},

    'setDefaultExceptionHandler'=> { in  => ['+procedure/1'],
                                     out => [],
                                     BI  => BIsetDefaultExceptionHandler},

    'getDefaultExceptionHandler'=> { in  => [],
                                     out => ['+procedure/1'],
                                     BI  => BIgetDefaultExceptionHandler},

    'raise'             => { in  => ['value'],
                             out => [],
                             BI  => BIraise,
                             doesNotReturn => 1},

    'raiseError'        => { in  => ['value'],
                             out => [],
                             BI  => BIraiseError,
                             doesNotReturn => 1},

    'raiseDebug'        => { in  => ['value'],
                             out => [],
                             BI  => BIraiseDebug,
                             doesNotReturn => 1},

    # builtins for the new compiler
    # (OPI and environment handling):

    'setOPICompiler'    => { in  => ['+object'],
                             out => [],
                             BI  => BIsetOPICompiler},

    'getOPICompiler'    => { in  => [],
                             out => ['+value'],
                             BI  => BIgetOPICompiler},

    'isBuiltin'         => { in  => ['+value'],
                             out => ['+bool'],
                             BI  => BIisBuiltin},

    'getBuiltinName'    => { in  => ['+value'],
                             out => ['+atom'],
                             BI  => BIgetBuiltinName},

    'nameVariable'      => { in  => ['value','+atom'],
                             out => [],
                             BI  => BInameVariable},

    'newNamedName'      => { in  => ['+atom'],
                             out => ['+literal'],
                             BI  => BInewNamedName},

    'isUniqueName'      => { in  => ['+value'],
                             out => ['+bool'],
                             BI  => BIisUniqueName},

    'generateAbstractionTableID'=> { in  => ['+bool'],
                                     out => ['+foreignPointer'],
                                     BI  => BIgenerateAbstractionTableID},

    'concatenateAtomAndInt'     => { in  => ['+atom','+int'],
                                     out => ['+atom'],
                                     BI  => BIconcatenateAtomAndInt},

    'RegSet.new'        => { in  => ['+int','+int'],
                             out => ['+chunk'],
                             BI  => BIregSet_new},

    'RegSet.copy'       => { in  => ['+chunk'],
                             out => ['+chunk'],
                             BI  => BIregSet_copy},

    'RegSet.adjoin'     => { in  => ['+chunk','+int'],
                             out => [],
                             BI  => BIregSet_adjoin},

    'RegSet.remove'     => { in  => ['+chunk','+int'],
                             out => [],
                             BI  => BIregSet_remove,},

    'RegSet.member'     => { in  => ['+int','+chunk'],
                             out => ['+bool'],
                             BI  => BIregSet_member},

    'RegSet.union'      => { in  => ['+chunk','+chunk'],
                             out => [],
                             BI  => BIregSet_union},

    'RegSet.intersect'  => { in  => ['+chunk','+chunk'],
                             out => [],
                             BI  => BIregSet_intersect},

    'RegSet.subtract'   => { in  => ['+chunk','+chunk'],
                             out => [],
                             BI  => BIregSet_subtract},

    'RegSet.toList'     => { in  => ['+chunk'],
                             out => ['+[int]'],
                             BI  => BIregSet_toList},

    'RegSet.complementToList'   => { in  => ['+chunk'],
                                     out => ['+[int]'],
                                     BI  => BIregSet_complementToList},

    # Oz parser

    'ozparser_parseFile'        => { in  => ['+virtualString','+record'],
                                     out => ['+value'],
                                     bi  => ozparser_parseFile},

    'ozparser_parseVirtualString'=> { in  => ['+virtualString','+record'],
                                      out => ['+value'],
                                      bi  => ozparser_parseVirtualString},

    'ozparser_fileExists'       => { in  => ['+virtualString'],
                                     out => ['+bool'],
                                     bi  => ozparser_fileExists},

    'copyCode'          => { in  => ['+abstraction','+dictionary'],
                             out => [],
                             BI  => BIcopyCode},

    # Finalization

    'Finalize.register' => { in  => ['+value','+value'],
                             out => [],
                             BI  => BIfinalize_register},

    'Finalize.setHandler'=> { in  => ['+value'],
                              out => [],
                              BI  => BIfinalize_setHandler},

    'GetCloneDiff'      => { in  => ['+space'],
                             out => ['+value'],
                             BI  => BIgetCloneDiff,
                             ifdef=>'CS_PROFILE'},

#    'SystemRegistry'   => { in  => [],
#                            out => ['+dictionary'],
#                            BI  => BIsystem_registry},
#
#    'ServiceRegistry'  => { in  => [],
#                            out => ['+dictionary'],
#                            BI  => BIsystem_registry},

    #-----------------------------------------------------------------
    # ASSEMBLE.CC
    #-----------------------------------------------------------------

    'getOpcode'         => { in  => ['+atom'],
                             out => ['+int'],
                             BI  => BIgetOpcode},

    'getInstructionSize'=> { in  => ['+atom'],
                             out => ['+int'],
                             BI  => BIgetInstructionSize},

    'newCodeBlock'      => { in  => ['+int'],
                             out => ['+int'],
                             BI  => BInewCodeBlock},

    'makeProc'          => { in  => ['+int','+[value]'],
                             out => ['+procedure/0'],
                             BI  => BImakeProc},

    'addDebugInfo'      => { in  => ['+int','+atom','+int'],
                             out => [],
                             BI  => BIaddDebugInfo},

    'storeOpcode'       => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIstoreOpcode},

    'storeNumber'       => { in  => ['+int','+number'],
                             out => [],
                             BI  => BIstoreNumber},

    'storeLiteral'      => { in  => ['+int','+literal'],
                             out => [],
                             BI  => BIstoreLiteral},

    'storeFeature'      => { in  => ['+int','+feature'],
                             out => [],
                             BI  => BIstoreFeature},

    'storeConstant'     => { in  => ['+int','+value'],
                             out => [],
                             BI  => BIstoreConstant},

    'storeBuiltinname'  => { in  => ['+int','+procedure'],
                             out => [],
                             BI  => BIstoreBuiltinname},

    'storeVariablename' => { in  => ['+int','+atom'],
                             out => [],
                             BI  => BIstoreVariablename},

    'storeRegisterIndex'=> { in  => ['+int','+int'],
                             out => [],
                             BI  => BIstoreRegisterIndex},

    'storeInt'          => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIstoreInt},

    'storeLabel'        => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIstoreLabel},

    'storePredicateRef' => { in  => ['+int','+value'],
                             out => [],
                             BI  => BIstorePredicateRef},

    'storePredId'       => { in  => ['+int','+atom','+value','+atom',
                                     '+int','+bool'],
                             out => [],
                             BI  => BIstorePredId},

    'newHashTable'      => { in  => ['+int','+int','+int'],
                             out => ['+int'],
                             BI  => BInewHashTable},

    'storeHTVarLabel'   => { in  => ['+int','+int','+int'],
                             out => [],
                             BI  => BIstoreHTVarLabel},

    'storeHTScalar'     => { in  => ['+int','+int','+value','+int'],
                             out => [],
                             BI  => BIstoreHTScalar},

    'storeHTRecord'     => { in  => ['+int','+int','+literal','+value','+int'],
                             out => [],
                             BI  => BIstoreHTRecord},

    'storeRecordArity'  => { in  => ['+int','+value'],
                             out => [],
                             BI  => BIstoreRecordArity},

    'storeGenCallInfo'  => { in  => ['+int','+int','+bool','+literal',
                                     '+bool','+value'],
                             out => [],
                             BI  => BIstoreGenCallInfo},

    'storeApplMethInfo' => { in  => ['+int','+literal','+value'],
                             out => [],
                             BI  => BIstoreApplMethInfo},

    'storeGRegRef'      => { in  => ['+int','+[tuple]'],
                             out => [],
                             BI  => BIstoreGRegRef},

    'storeCache'        => { in  => ['+int','+value'],
                             out => [],
                             BI  => BIstoreCache},

    #-----------------------------------------------------------------
    # FD
    #-----------------------------------------------------------------

    'fdReset'           => { in  => [],
                             out => [],
                             bi  =>BIfdReset,
                             ifdef=>PROFILE_FD,
                             module=>fd},

    'fdDiscard'         => { in  => [],
                             out => [],
                             bi  => BIfdDiscard,
                             ifdef=>PROFILE_FD,
                             module=>fd},

    'fdGetNext'         => { in  => ['value'],
                             out => [],
                             bi  => BIfdGetNext,
                             ifdef=>PROFILE_FD,
                             module=>fd},

    'fdPrint'           => { in  => [],
                             out => [],
                             bi  => BIfdPrint,
                             ifdef=>PROFILE_FD,
                             module=>fd},

    'fdTotalAverage'    => { in  => [],
                             out => [],
                             bi  => BIfdTotalAverage,
                             ifdef=>PROFILE_FD,
                             module=>fd},

    'fdIs'              => { in  => ['*value','bool'],
                             out => [],
                             bi  => BIfdIs,
                             module=>fd},

    'fdIsVar'           => { in  => ['value'],
                             out => [],
                             BI  => BIisFdVar,
                             module=>fd},

    'fdIsVarB'          => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIisFdVarB,
                             module=>fd},

    'fdGetLimits'       => { in  => [],
                             out => ['+int','+int'],
                             BI  => BIgetFDLimits,
                             module=>fd},

    'fdGetMin'          => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMin,
                             module=>fd},

    'fdGetMid'          => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMid,
                             module=>fd},

    'fdGetMax'          => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMax,
                             module=>fd},

    'fdGetDom'          => { in  => ['*int','+[value]'],
                             out => [],
                             bi  => BIfdGetAsList,
                             module=>fd},

    'fdGetCard'         => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdGetCardinality,
                             module=>fd},

    'fdGetNextSmaller'  => { in  => ['+int','*int','int'],
                             out => [],
                             bi  => BIfdNextSmaller,
                             module=>fd},

    'fdGetNextLarger'   => { in  => ['+int','*int','int'],
                             out => [],
                             bi  => BIfdNextLarger,
                             module=>fd},

    'fdTellConstraint'  => { in  => ['int','+value'],
                             out => [],
                             bi  => BIfdTellConstraint,
                             module=>fd},

    'fdWatchSize'       => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchSize,
                             module=>fd},

    'fdWatchMin'        => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchMin,
                             module=>fd},

    'fdWatchMax'        => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchMax,
                             module=>fd},

    'fdConstrDisjSetUp' => { in  => ['+value','+value','+value','+value'],
                             out => [],
                             bi  => BIfdConstrDisjSetUp,
                             module=>fd},

    'fdConstrDisj'      => { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => BIfdConstrDisj,
                             module=>fd},

    'fdTellConstraintCD'=> { in  => ['value','value','value'],
                             out => [],
                             bi  => BIfdTellConstraintCD,
                             module=>fd},

    'fdp_init'          => { in  => ['atom'],
                             out => [],
                             bi  => fdp_init,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_sum'           => { in  => ['+value','+atom','int'],
                             out => [],
                             bi  => fdp_sum,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_sumC'          => { in  => ['+value','+value','+atom','int'],
                             out => [],
                             bi  => fdp_sumC,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_sumCN'         => { in  => ['+value','+value','+atom','int'],
                             out => [],
                             bi  => fdp_sumCN,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_sumR'          => { in  => ['+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumR,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_sumCR'         => { in  => ['+value','+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCR,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_sumCNR'        => { in  => ['+value','+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCNR,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_sumCD'         => { in  => ['+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCD,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_sumCCD'        => { in  => ['+value','+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCCD,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_sumCNCD'       => { in  => ['+value','+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCNCD,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_plus_rel'      => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_plus_rel,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_plus'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_plus,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_minus'         => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_minus,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_times'         => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_times,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_times_rel'     => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_times_rel,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_power'         => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_power,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_divD'          => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_divD,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_divI'          => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_divI,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_modD'          => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_modD,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_modI'          => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_modI,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_conj'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_conj,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_disj'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_disj,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_exor'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_exor,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_impl'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_impl,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_equi'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_equi,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_nega'          => { in  => ['int','int'],
                             out => [],
                             bi  => fdp_nega,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_intR'          => { in  => ['int','+value','int'],
                             out => [],
                             bi  => fdp_intR,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_card'          => { in  => ['+value','int','int','int'],
                             out => [],
                             bi  => fdp_card,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_exactly'       => { in  => ['int','+value','+int'],
                             out => [],
                             bi  => fdp_exactly,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_atLeast'       => { in  => ['int','+value','+int'],
                             out => [],
                             bi  => fdp_atLeast,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_atMost'        => { in  => ['int','+value','+int'],
                             out => [],
                             bi  => fdp_atMost,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_element'       => { in  => ['int','+value','int'],
                             out => [],
                             bi  => fdp_element,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_notEqOff'      => { in  => ['int','int','+int'],
                             out => [],
                             bi  => fdp_notEqOff,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_lessEqOff'     => { in  => ['int','int','+int'],
                             out => [],
                             bi  => fdp_lessEqOff,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_minimum'       => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_minimum,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_maximum'       => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_maximum,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_inter' => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_inter,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_union' => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_union,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_distinct'      => { in  => ['+value'],
                             out => [],
                             bi  => fdp_distinct,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_distinctD'     => { in  => ['+value'],
                             out => [],
                             bi  => fdp_distinctD,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_distinctStream'=> { in  => ['+value','value'],
                             out => [],
                             bi  => fdp_distinctStream,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_distinctOffset'=> { in  => ['+value','+value'],
                             out => [],
                             bi  => fdp_distinctOffset,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_disjoint'=> { in  => ['int','+int','int','+int'],
                             out => [],
                             bi  => fdp_disjoint,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'sched_disjoint_card'=> { in  => ['int','+int','int','+int'],
                             out => [],
                             bi  => sched_disjoint_card,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_disjointC'=> { in  => ['int','+int','int','+int','int'],
                             out => [],
                             bi  => fdp_disjointC,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_distance'      => { in  => ['int','int','+atom','int'],
                             out => [],
                             bi  => fdp_distance,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_distinct2'     => { in  => ['+value','+value','+value','+value'],
                             out => [],
                             bi  => fdp_distinct2,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'sched_cpIterate'   => { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => sched_cpIterate,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'sched_cpIterateCap'=> { in  => ['+value','+value','+value',
                                     '+value','+value','+int'],
                             out => [],
                             bi  => sched_cpIterateCap,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'sched_cumulativeTI'=> { in  => ['+value','+value','+value',
                                     '+value','+value'],
                             out => [],
                             bi  => sched_cumulativeTI,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'sched_cpIterateCapUp'=> { in  => ['+value','+value','+value',
                                       '+value','+value'],
                             out => [],
                             bi  => sched_cpIterateCapUp,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'sched_taskIntervals'=> { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => sched_taskIntervals,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'sched_disjunctive' => { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => sched_disjunctive,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'sched_disjunctiveStream'=> { in  => ['+value','+value','value'],
                             out => [],
                             bi  => sched_disjunctiveStream,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_twice'         => { in  => ['int','int'],
                             out => [],
                             bi  => fdp_twice,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_square'        => { in  => ['int','int'],
                             out => [],
                             bi  => fdp_square,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_subset'        => { in  => ['int','int'],
                             out => [],
                             bi  => fdp_subset,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_dsum'          => { in  => ['+value','+atom','int'],
                             out => [],
                             bi  => fdp_dsum,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_dsumC'         => { in  => ['+value','+value','+atom','int'],
                             out => [],
                             bi  => fdp_dsumC,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'fdp_sumAC'         => { in  => ['+value','+value','+atom','int'],
                             out => [],
                             bi  => fdp_sumAC,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'counter'           => { in  => ['int','value'],
                             out => [],
                             bi  => fdtest_counter,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'firstFail'         => { in  => ['+value','value'],
                             out => [],
                             bi  => fdtest_firstFail,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'sched_taskIntervalsProof'=> { in  => ['value','value','value','value',
                                           'value'],
                             out => [],
                             bi  => sched_taskIntervalsProof,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'sched_firstsLasts' => { in  => ['value','value','value','value',
                                     'value'],
                             out => [],
                             bi  => sched_firstsLasts,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'spawnLess'         => { in  => ['int','int'],
                             out => [],
                             bi  => fdtest_spawnLess,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'dplus'             => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdtest_plus,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'sumac'             => { in  => ['value','value','int'],
                             out => [],
                             bi  => fdtest_sumac,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'testgensum'        => { in  => ['value','int'],
                             out => [],
                             bi  => fdtest_gensum,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'testsum'           => { in  => ['value','int'],
                             out => [],
                             bi  => fdtest_sum,
                             ifdef =>ALLDIFF,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'inqueens'          => { in  => ['value'],
                             out => [],
                             bi  => fdtest_inqueens,
                             ifdef =>INPROP,
                             ifndef=>FOREIGNFDPROPS,
                             module=>fd},

    'debugStable'       => { in  => [],
                             out => [],
                             bi  => debugStable,
                             ifdef =>DEBUG_STABLE,
                             module=>fd},

    'resetStable'       => { in  => [],
                             out => [],
                             bi  => resetStable,
                             ifdef =>DEBUG_STABLE,
                             module=>fd},

    'fddistribute'      => { in  => ['value','value','value','value','value',],
                             out => [],
                             bi  => BIfdDistribute,
                             module=>fd},

    #-----------------------------------------------------------------
    # METAVAR
    #-----------------------------------------------------------------

    'metaIsVar'         => { in  => ['value'],
                             out => [],
                             BI  => BImetaIsVar,
                             module=>'metavar'},

    'metaIsVarB'        => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BImetaIsVarB,
                             module=>'metavar'},

    'metaWatchVar'      => { in  => ['value','value'],
                             out => [],
                             BI  => BImetaWatchVar,
                             module=>'metavar'},

    'metaWatchVarB'     => { in  => ['value','value','+bool'],
                             out => [],
                             BI  => BImetaWatchVarB,
                             module=>'metavar'},

    'metaGetDataAsAtom' => { in  => ['value','atom'],
                             out => [],
                             BI  => BImetaGetDataAsAtom,
                             module=>'metavar'},

    'metaGetNameAsAtom' => { in  => ['value','atom'],
                             out => [],
                             BI  => BImetaGetNameAsAtom,
                             module=>'metavar'},

    'metaGetStrength'   => { in  => ['value','value'],
                             out => [],
                             BI  => BImetaGetStrength,
                             module=>'metavar'},

    #-----------------------------------------------------------------
    # AVAR
    #-----------------------------------------------------------------

    'isAVarB'           => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIisAVarB,
                             module=>'avar'},

    'newAVar'           => { in  => ['value'],
                             out => ['value'],
                             BI  => BInewAVar,
                             module=>'avar'},

    'readAVar'          => { in  => ['value'],
                             out => ['value'],
                             BI  => BIreadAVar,
                             module=>'avar'},

    'aVarHandler'       => { in  => ['+procedure','+procedure'],
                             out => [],
                             BI  => BIaVarHandler,
                             module=>'avar'},

    '==='               => { in  => ['value','value'],
                             out => [],
                             BI  => BIaVarBind,
                             module=>'avar'},

    #-----------------------------------------------------------------
    # PERDIOVAR
    #-----------------------------------------------------------------

    'PerdioVar.is'      => { in  => ['value'],
                             out => ['+bool'],
                             BI  => PerdioVar_is,
                             module=>'perdiovar'},

    #-----------------------------------------------------------------
    # UNIX
    #-----------------------------------------------------------------

    'OS.getDir'         => { in  => ['+virtualString'],
                             out => ['+[string]'],
                             BI  => unix_getDir,
                             module=>'os'},

    'OS.stat'           => { in  => ['+virtualString'],
                             out => ['+record'],
                             BI  => unix_stat,
                             module=>'os'},

    'OS.getCWD'         => { in  => [],
                             out => ['+atom'],
                             BI  => unix_getCWD,
                             module=>'os'},

    'OS.open'           => { in  => ['+virtualString','+[atom]','+[atom]'],
                             out => ['+int'],
                             BI  => unix_open,
                             module=>'os'},

    'OS.fileDesc'       => { in  => ['+atom'],
                             out => ['+int'],
                             BI  => unix_fileDesc,
                             module=>'os'},

    'OS.close'          => { in  => ['+int'],
                             out => [],
                             BI  => unix_close,
                             module=>'os'},

    'OS.write'          => { in  => ['+int','+virtualString'],
                             out => ['+value'],
                             BI  => unix_write,
                             module=>'os'},

    'OS.read'           => { in  => ['+int','+int','value','value','int'],
                             out => [],
                             BI  => unix_read,
                             module=>'os'},

    'OS.lSeek'          => { in  => ['+int','+int','+atom'],
                             out => ['+int'],
                             BI  => unix_lSeek,
                             module=>'os'},

    'OS.unlink'         => { in  => ['+virtualString'],
                             out => [],
                             BI  => unix_unlink,
                             module=>'os'},

    'OS.readSelect'     => { in  => ['+int'],
                             out => [],
                             BI  => unix_readSelect,
                             module=>'os'},

    'OS.writeSelect'    => { in  => ['+int'],
                             out => [],
                             BI  => unix_writeSelect,
                             module=>'os'},

    'OS.acceptSelect'   => { in  => ['+int'],
                             out => [],
                             BI  => unix_acceptSelect,
                             module=>'os'},

    'OS.deSelect'       => { in  => ['+int'],
                             out => [],
                             BI  => unix_deSelect,
                             module=>'os'},

    'OS.system'         => { in  => ['+virtualString'],
                             out => ['+int'],
                             BI  => unix_system,
                             module=>'os'},

    'OS.getEnv'         => { in  => ['+virtualString'],
                             out => ['+string'],
                             BI  => unix_getEnv,
                             module=>'os'},

    'OS.putEnv'         => { in  => ['+virtualString','+virtualString'],
                             out => [],
                             BI  => unix_putEnv,
                             module=>'os'},

    'OS.time'           => { in  => [],
                             out => ['+int'],
                             BI  => unix_time,
                             module=>'os'},

    'OS.gmTime'         => { in  => [],
                             out => ['+record'],
                             BI  => unix_gmTime,
                             module=>'os'},

    'OS.localTime'      => { in  => [],
                             out => ['+record'],
                             BI  => unix_localTime,
                             module=>'os'},

    'OS.srand'          => { in  => ['+int'],
                             out => [],
                             BI  => unix_srand,
                             module=>'os'},

    'OS.rand'           => { in  => [],
                             out => ['+int'],
                             BI  => unix_rand,
                             module=>'os'},

    'OS.randLimits'     => { in  => [],
                             out => ['+int','+int'],
                             BI  => unix_randLimits,
                             module=>'os'},

    'OS.socket'         => { in  => ['+atom','+atom','+virtualString'],
                             out => ['+int'],
                             BI  => unix_socket,
                             module=>'os'},

    'OS.bind'           => { in  => ['+int','+int'],
                             out => [],
                             BI  => unix_bindInet,
                             module=>'os'},

    'OS.listen'         => { in  => ['+int','+int'],
                             out => [],
                             BI  => unix_listen,
                             module=>'os'},

    'OS.connect'        => { in  => ['+int','+virtualString','+int'],
                             out => [],
                             BI  => unix_connectInet,
                             module=>'os'},

    'OS.accept'         => { in  => ['+int'],
                             out => ['+int','+string','+int'],
                             BI  => unix_acceptInet,
                             module=>'os'},

    'OS.shutDown'       => { in  => ['+int','+int'],
                             out => [],
                             BI  => unix_shutDown,
                             doesNotReturn=>1,
                             module=>'os'},

    'OS.send'           => { in  => ['+int','+virtualString','+[atom]'],
                             out => ['+value'],
                             BI  => unix_send,
                             module=>'os'},

    'OS.sendTo'         => { in  => ['+int','+virtualString','+[atom]',
                                     '+virtualString','+int'],
                             out => ['+value'],
                             BI  => unix_sendToInet,
                             module=>'os'},

    'OS.receiveFrom'    => { in  => ['+int','+int','+[atom]','value','value'],
                             out => ['+string','+int','+int'],
                             BI  => unix_receiveFromInet,
                             module=>'os'},

    'OS.getSockName'    => { in  => ['+int'],
                             out => ['+int'],
                             BI  => unix_getSockName,
                             module=>'os'},

    'OS.getHostByName'  => { in  => ['+virtualString'],
                             out => ['+record'],
                             BI  => unix_getHostByName,
                             module=>'os'},

    'OS.pipe'           => { in  => ['+virtualString','value'],
                             out => ['+int','+int#int'],
                             BI  => unix_pipe,
                             module=>'os'},

    'OS.tmpnam'         => { in  => [],
                             out => ['+string'],
                             BI  => unix_tmpnam,
                             module=>'os'},

    'OS.wait'           => { in  => [],
                             out => ['+int','+int'],
                             BI  => unix_wait,
                             module=>'os'},

    'OS.getServByName'  => { in  => ['+virtualString','+virtualString'],
                             out => ['+int'],
                             BI  => unix_getServByName,
                             module=>'os'},

    'OS.uName'          => { in  => [],
                             out => ['+record'],
                             BI  => unix_uName,
                             module=>'os'},

    'OS.getpwnam'       => { in  => ['+virtualString'],
                             out => ['+record'],
                             BI  => unix_getpwnam,
                             module=>'os'},

    #-----------------------------------------------------------------
    # TCL_TK
    #-----------------------------------------------------------------

    'getTclNames'       => { in  => [],
                             out => ['value','value','value'],
                             BI  => BIgetTclNames,
                             module=>'tcl_tk'},

    'initTclSession'    => { in  => ['value','value','value'],
                             out => ['value'],
                             BI  => BIinitTclSession,
                             module=>'tcl_tk'},

    'closeTclSession'   => { in  => ['value'],
                             out => [],
                             BI  => BIcloseTclSession,
                             module=>'tcl_tk'},

    'Tk.send'           => { in  => ['value','!value'],
                             out => [],
                             BI  => BItclWrite,
                             module=>'tcl_tk'},

    'tclWriteReturn'    => { in  => ['!value','value','value','value'],
                             out => [],
                             BI  => BItclWriteReturn,
                             module=>'tcl_tk'},

    'tclWriteReturnMess'=> { in  => ['!value','value','value','value','value'],
                             out => [],
                             BI  => BItclWriteReturnMess,
                             module=>'tcl_tk'},

    'Tk.batch'          => { in  => ['value','!value'],
                             out => [],
                             BI  => BItclWriteBatch,
                             module=>'tcl_tk'},

    'tclWriteTuple'     => { in  => ['value','!value','value'],
                             out => [],
                             BI  => BItclWriteTuple,
                             module=>'tcl_tk'},

    'tclWriteTagTuple'  => { in  => ['value','!value','value','value'],
                             out => [],
                             BI  => BItclWriteTagTuple,
                             module=>'tcl_tk'},

    'tclWriteFilter'    => { in  => ['value','!value','value','value',
                                     'value','value'],
                             out => [],
                             BI  => BItclWriteFilter,
                             module=>'tcl_tk'},

    'tclClose'          => { in  => ['value','!value','value'],
                             out => [],
                             BI  => BItclClose,
                             module=>'tcl_tk'},

    'tclCloseWeb'       => { in  => ['value','!value'],
                             out => [],
                             BI  => BItclCloseWeb,
                             module=>'tcl_tk'},

    'addFastGroup'      => { in  => ['+value','value'],
                             out => ['value'],
                             BI  => BIaddFastGroup,
                             module=>'tcl_tk'},

    'delFastGroup'      => { in  => ['value'],
                             out => [],
                             BI  => BIdelFastGroup,
                             module=>'tcl_tk'},

    'getFastGroup'      => { in  => ['+value'],
                             out => ['+value'],
                             BI  => BIgetFastGroup,
                             module=>'tcl_tk'},

    'delAllFastGroup'   => { in  => ['+value'],
                             out => ['+value'],
                             BI  => BIdelAllFastGroup,
                             module=>'tcl_tk'},

    'genTopName'        => { in  => ['value'],
                             out => ['value'],
                             BI  => BIgenTopName,
                             module=>'tcl_tk'},

    'genWidgetName'     => { in  => ['value','value'],
                             out => ['value'],
                             BI  => BIgenWidgetName,
                             module=>'tcl_tk'},

    'genTagName'        => { in  => ['value'],
                             out => ['value'],
                             BI  => BIgenTagName,
                             module=>'tcl_tk'},

    'genVarName'        => { in  => ['value'],
                             out => ['value'],
                             BI  => BIgenVarName,
                             module=>'tcl_tk'},

    'genImageName'      => { in  => ['value'],
                             out => ['value'],
                             BI  => BIgenImageName,
                             module=>'tcl_tk'},

    #-----------------------------------------------------------------
    # PERDIO
    #-----------------------------------------------------------------

    'dvset'             => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIdvset,
                             ifdef=>DEBUG_PERDIO,
                             module=>'perdio'},

    'NetCloseCon'       => { in  => ['+int'],
                             out => [],
                             BI  => BIcloseCon,
                             module=>'perdio'},

    'startTmp'          => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIstartTmp,
                             module=>'perdio'},

    'siteStatistics'    => { in  => [],
                             out => ['+[value]'],
                             BI  => BIsiteStatistics,
                             module=>'perdio'},

    'printBorrowTable'  => { in  => [],
                             out => [],
                             BI  => BIprintBorrowTable,
                             module=>'perdio'},

    'printOwnerTable'   => { in  => [],
                             out => [],
                             BI  => BIprintOwnerTable,
                             module=>'perdio'},

    #-----------------------------------------------------------------
    # LAZY
    #-----------------------------------------------------------------

    'Lazy.new'          => { in  => ['value','value'],
                             out => [],
                             BI  => BILazyNew,
                             module=>'lazy'},

    'Lazy.is'           => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BILazyIs,
                             module=>'lazy'},

    #-----------------------------------------------------------------
    # VPROPS
    #-----------------------------------------------------------------

    'GetProperty'       => { in  => ['+literal'],
                             out => ['value'],
                             BI  => BIgetProperty,
                             module=>'vprop'},

    'CondGetProperty'   => { in  => ['+literal','value'],
                             out => ['value'],
                             BI  => BIcondGetProperty,
                             module=>'vprop'},

    'PutProperty'       => { in  => ['+literal','value'],
                             out => [],
                             BI  => BIputProperty,
                             module=>'vprop'},

    #-----------------------------------------------------------------
    # COMPONENTS
    #-----------------------------------------------------------------

    'smartSave'         => { in  => ['value','value','+virtualString'],
                             out => [],
                             BI  => BIsmartSave,
                             module=>components},

    'load'              => { in  => ['value','value'],
                             out => [],
                             BI  => BIload,
                             module=>components},

    'PID.get'           => { in  => [],
                             out => ['+record'],
                             BI  => BIGetPID,
                             module=>components},

    'PID.received'      => { in  => ['value'],
                             out => [],
                             BI  => BIReceivedPID,
                             module=>components},

    'PID.close'         => { in  => [],
                             out => [],
                             BI  => BIClosePID,
                             module=>components},

    'PID.send'          => { in  => ['+virtualString','+int','+int','value'],
                             out => [],
                             BI  => BISendPID,
                             module=>components},

    'URL.localize'      => { in  => ['+virtualString'],
                             out => ['+record'],
                             BI  => BIurl_localize,
                             module=>components},

    'URL.open'          => { in  => ['+virtualString'],
                             out => ['+int'],
                             BI  => BIurl_open,
                             module=>components},

    'URL.load'          => { in  => ['+virtualString'],
                             out => ['value'],
                             BI  => BIurl_load,
                             module=>components},

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
        if ($BI) {
            # new style
            print "{\"$key\",\t$inArity,\t$outArity,$BI,\t0},\n";
        } else {
            # old style
            my $bi  = $info->{bi};
            my $ibi = $info->{ibi};
            if ($ibi) { $ibi = "(IFOR) $ibi"; }
            else      { $ibi = "0"; }
            print "{\"$key\",\t$inArity,\t$outArity,$bi,\t$ibi},\n";
        }
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

# $style==0     old style
# $style==1     both
# $style==2     new style

my $style = 0;

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
        if ($style>0) {
            if (@ityps) {
                print "\t\titypes:[",join(' ',@ityps),"]\n";
            } else {
                print "\t\titypes:nil\n";
            }
            if (@otyps) {
                print "\t\totypes:[",join(' ',@otyps),"]\n";
            } else {
                print "\t\totypes:nil\n";
            }
            if (@idets) {
                print "\t\tidets:[",join(' ',@idets),"]\n";
            } else {
                print "\t\tidets:nil\n";
            }
            if (@odets) {
                print "\t\todets:[",join(' ',@odets),"]\n";
            } else {
                print "\t\todets:nil\n";
            }
            if (@imods) {
                print "\t\timods:[",join(' ',@imods),"]\n";
            } else {
                print "\t\timods:nil\n";
            }
            if (@oowns) {
                print "\t\toowns:[",join(' ',@oowns),"]\n";
            } else {
                print "\t\toowns:nil\n";
            }
        }
        if ($style<2) {
            if ((@ityps+@otyps)>0) {
                print "\t\ttypes:[",join(' ',@ityps,@otyps),"]\n";
                print "\t\tdet:[",join(' ',@idets,@odets),"]\n";
            } else {
                print "\t\ttypes:nil\n";
                print "\t\tdet:nil\n";
            }
        }
        print "\t\teqeq:true\n" if $info->{eqeq};
        print "\t\tdestroysArguments:true\n" if $destroys;
        print "\t\tdoesNotReturn:true\n" if $info->{doesNotReturn};
        print "\t\tinlineFun:true\n" if $info->{ibi} && (@{$info->{out}}==1);
        print "\t\tinlineRel:true\n" if $info->{ibi} && (@{$info->{out}}==0);
        my $shallow = $info->{shallow};
        print "\t\trel:'$shallow'\n" if $shallow;
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
    if    ($option eq '-ctable' ) { $choice='ctable';  }
    elsif ($option eq '-cdecl'  ) { $choice='cdecl';   }
    elsif ($option eq '-oztable') { $choice='oztable'; }
    elsif ($option eq '-include') { push @include,split(/\,/,shift); }
    elsif ($option eq '-exclude') { push @exclude,split(/\,/,shift); }
    elsif ($option eq '-style'  ) { $style=int(shift); }
    else { die "unrecognized option: $option"; }
}

if (@include!=0 && @exclude!=0) {
    die "cannot have both -include and -exclude";
}

foreach $option (@include) { $include{$option} = 1; }
foreach $option (@exclude) { $exclude{$option} = 1; }

$includedefault = 0 if @include!=0;

if    ($choice eq 'ctable' ) { &CTABLE;  }
elsif ($choice eq 'cdecl'  ) { &CDECL;   }
elsif ($choice eq 'oztable') { &OZTABLE; }
else { die "must specify one of: -ctable -cdecl -oztable"; }
