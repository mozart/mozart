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
###             | bitArray
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
### Old style builtins have: bi => OLDBI, where OLDBI is the name
### of the C procedure that implements it (normally defined using
### OZ_C_proc_begin(OLDBI,...)).
###
### New style builtins have: BI => NEWBI, where NEWBI is the name
### of the C procedure that implements it (defined using
### OZ_BI_define(NEWBI,...,...)).
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
### negated => BI, indicates that the builtin BI returns the negated
### result of the builtin.  For example `<' is the negated version of `>='.
### This only makes sense for builtins with a single output argument which
### is of type bool.
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
                             native => true},

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
    'Promise.assign'    => { in  => ['value','value'],
                             out => [],
                             BI  => BIPromiseAssign,
                             module=>'promise',
                             native => false},
    'Promise.access'    => { in  => ['value'],
                             out => ['value'],
                             BI  => BIPromiseAccess,
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

    ###* GenCtVar

    'isCtVarB'          => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIIsGenCtVarB,
                             module =>  ct,
                             native => true},

    'getCtVarConstraintAsAtom' => { in  => ['value','atom'],
                             out => [],
                             BI  => BIGetCtVarConstraintAsAtom,
                             module => ct,
                             native => true},

    'getCtVarNameAsAtom'   => { in  => ['value','atom'],
                             out => [],
                             BI  => BIGetCtVarNameAsAtom,
                             module =>  ct,
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
                             native => true},

    'Show'              => { in  => ['value'],
                             out => [],
                             bi  => BIshow,
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

    ##* Misc. System Procs

    'shutdown'          => { in  => ['+int'],
                             out => [],
                             BI  => BIshutdown,
                             doesNotReturn => 1,
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

    'OS.chDir'          => { in  => ['+virtualString'],
                             out => [],
                             BI  => unix_chDir,
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

    'OS.getRUsage'      => { in  => ['+virtualString'],
                             out => ['+record'],
                             BI  => unix_getRUsage,
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

    'dlStaticLoad'      => { in  => ['+virtualString'],
                             out => ['+record'],
                             BI  => BIdlStaticLoad,
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

    #* Pickles

    'save'              => { in  => ['value','+virtualString'],
                             out => [],
                             BI  => BIsave,
                             module=>components,
                             native => false},

    'load'              => { in  => ['value','value'],
                             out => [],
                             BI  => BIload,
                             module=>components,
                             native => false},

    'export'            => { in  => ['value'],
                             out => [],
                             BI  => BIexport,
                             module=>components,
                             native => false},

    #* Connection

    'PID.get'           => { in  => [],
                             out => ['+record'],
                             BI  => BIGetPID,
                             module=>components,
                             native => false},

    'PID.received'      => { in  => ['value'],
                             out => [],
                             BI  => BIReceivedPID,
                             module=>components,
                             native => false},

    'PID.close'         => { in  => [],
                             out => [],
                             BI  => BIClosePID,
                             module=>components,
                             native => false},

    'PID.send'          => { in  => ['+virtualString','+int','+int','+int','+int','value'],
                             out => [],
                             BI  => BISendPID,
                             module=>components,
                             native => false},

    'PID.toPort'        => { in  => ['+virtualString','+int','+int','+int'],
                             out => ['+port'],
                             BI  => BITicket2Port,
                             module=>components,
                             native => false},

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

    ##* WIF (Wish InterFace)

    'wifInit'           => { in     => ['value','value','value'],
                             out    => [],
                             BI     => BIwif_init,
                             module => 'wif',
                             native => true},

    'wifWrite'          => { in     => ['!value'],
                             out    => [],
                             BI     => BIwif_write,
                             module => 'wif',
                             native => true},

    'wifWriteReturn'    => { in     => ['!value','value','value'],
                             out    => [],
                             BI     => BIwif_writeReturn,
                             module => 'wif',
                             native => true},

    'wifWriteReturnMess'=> { in     => ['!value','value','value','value'],
                             out    => [],
                             BI     => BIwif_writeReturnMess,
                             module => 'wif',
                             native => true},

    'wifWriteBatch'     => { in     => ['!value'],
                             out    => [],
                             BI     => BIwif_writeBatch,
                             module => 'wif',
                             native => true},

    'wifWriteTuple'     => { in     => ['!value','value'],
                             out    => [],
                             BI     => BIwif_writeTuple,
                             module => 'wif',
                             native => true},

    'wifWriteTagTuple'  => { in     => ['!value','value','value'],
                             out    => [],
                             BI     => BIwif_writeTagTuple,
                             module => 'wif',
                             native => true},

    'wifWriteFilter'    => { in     => ['!value','value','value',
                                        'value','value'],
                             out    => [],
                             BI     => BIwif_writeFilter,
                             module => 'wif',
                             native => true},

    'wifClose'          => { in     => ['!value','value'],
                             out    => [],
                             BI     => BIwif_close,
                             module => 'wif',
                             native => true},

    'wifGenTopName'     => { in     => [],
                             out    => ['value'],
                             BI     => BIwif_genTopName,
                             module => 'wif',
                             native => true},

    'wifGenWidgetName'  => { in     => ['value'],
                             out    => ['value'],
                             BI     => BIwif_genWidgetName,
                             module => 'wif',
                             native => true},

    'wifGenTagName'     => { in     => [],
                             out    => ['value'],
                             BI     => BIwif_genTagName,
                             module => 'wif',
                             native => true},

    'wifGenVarName'     => { in     => [],
                             out    => ['value'],
                             BI     => BIwif_genVarName,
                             module => 'wif',
                             native => true},

    'wifGenImageName'   => { in     => [],
                             out    => ['value'],
                             BI     => BIwif_genImageName,
                             module => 'wif',
                             native => true},

    'wifGetNames'       => { in     => [],
                             out    => ['value','value','value'],
                             BI     => BIwif_getNames,
                             module => 'wif',
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

    ###* Parser

    'parser_parseFile'          => { in     => ['+virtualString','+record'],
                                     out    => ['+value'],
                                     bi     => parser_parseFile,
                                     module => libparser,
                                     native => true},

    'parser_parseVirtualString' => { in     => ['+virtualString','+record'],
                                     out    => ['+value'],
                                     bi     => parser_parseVirtualString,
                                     module => libparser,
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

    'predIdFlags'       => { in  => [],
                             out => ['+int','+int'],
                             BI  => BIpredIdFlags,
                             native => true},

    'storePredId'       => { in  => ['+int','+atom','+value','+record',
                                     '+value','+int'],
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

    # Internal stuff (always included)

    'fdReset'           => { in     => [],
                             out    => [],
                             bi     => BIfdReset,
                             ifdef  => PROFILE_FD,
                             module => fd,
                             native => true},

    'fdDiscard'         => { in     => [],
                             out    => [],
                             bi     => BIfdDiscard,
                             ifdef  => PROFILE_FD,
                             module => fd,
                             native => true},

    'fdGetNext'         => { in  => ['value'],
                             out => [],
                             bi  => BIfdGetNext,
                             ifdef=>PROFILE_FD,
                             module => fd,
                             native => true},

    'fdPrint'           => { in  => [],
                             out => [],
                             bi  => BIfdPrint,
                             ifdef=>PROFILE_FD,
                             module => fd,
                             native => true},

    'fdTotalAverage'    => { in  => [],
                             out => [],
                             bi  => BIfdTotalAverage,
                             ifdef=>PROFILE_FD,
                             module => fd,
                             native => true},

    'fdIs'              => { in  => ['*value','bool'],
                             out => [],
                             bi  => BIfdIs,
                             module => fd,
                             native => false},

    'fdIsVar'           => { in  => ['value'],
                             out => [],
                             BI  => BIisFdVar,
                             module => fd,
                             native => false},

    'fdIsVarB'          => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIisFdVarB,
                             module => fd,
                             native => false},

    'fdGetLimits'       => { in  => [],
                             out => ['+int','+int'],
                             BI  => BIgetFDLimits,
                             module => fd,
                             native => true},

    'fdGetMin'          => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMin,
                             module => fd,
                             native => true},

    'fdGetMid'          => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMid,
                             module => fd,
                             native => true},

    'fdGetMax'          => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMax,
                             module => fd,
                             native => true},

    'fdGetDom'          => { in  => ['*int','+[value]'],
                             out => [],
                             bi  => BIfdGetAsList,
                             module => fd,
                             native => true},

    'fdGetCard'         => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdGetCardinality,
                             module => fd,
                             native => true},

    'fdGetNextSmaller'  => { in  => ['+int','*int','int'],
                             out => [],
                             bi  => BIfdNextSmaller,
                             module => fd,
                             native => true},

    'fdGetNextLarger'   => { in  => ['+int','*int','int'],
                             out => [],
                             bi  => BIfdNextLarger,
                             module => fd,
                             native => true},

    'fdTellConstraint'  => { in  => ['int','+value'],
                             out => [],
                             bi  => BIfdTellConstraint,
                             module => fd,
                             native => true},

    'fdWatchSize'       => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchSize,
                             module => fd,
                             native => true},

    'fdWatchMin'        => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchMin,
                             module => fd,
                             native => true},

    'fdWatchMax'        => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchMax,
                             module => fd,
                             native => true},

    'fdConstrDisjSetUp' => { in  => ['+value','+value','+value','+value'],
                             out => [],
                             bi  => BIfdConstrDisjSetUp,
                             module => fd,
                             native => true},

    'fdConstrDisj'      => { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => BIfdConstrDisj,
                             module => fd,
                             native => true},

    'fdTellConstraintCD'=> { in  => ['value','value','value'],
                             out => [],
                             bi  => BIfdTellConstraintCD,
                             module => fd,
                             native => true},

    'debugStable'       => { in  => [],
                             out => [],
                             bi  => debugStable,
                             ifdef =>DEBUG_STABLE,
                             module => fd,
                             native => true},


    'resetStable'       => { in  => [],
                             out => [],
                             bi  => resetStable,
                             ifdef =>DEBUG_STABLE,
                             module => fd,
                             native => true},

    # External stuff (might be loaded dynamically)
    'fdd_selVarNaive'   => { in     => ['+tuple'],
                             out    => ['*int'],
                             bi     => BIfdd_selVarNaive,
                             module => libfd,
                             native => true},

    'fdd_selVarSize'    => { in     => ['+tuple'],
                             out    => ['*int'],
                             bi     => BIfdd_selVarSize,
                             module => libfd,
                             native => true},

    'fdd_selVarMin'     => { in     => ['+tuple'],
                             out    => ['*int'],
                             bi     => BIfdd_selVarMin,
                             module => libfd,
                             native => true},

    'fdd_selVarMax'     => { in     => ['+tuple'],
                             out    => ['*int'],
                             bi     => BIfdd_selVarMax,
                             module => libfd,
                             native => true},

    'fdd_selVarNbSusps' => { in     => ['+tuple'],
                             out    => ['*int'],
                             bi     => BIfdd_selVarNbSusps,
                             module => libfd,
                             native => true},

    'fdp_sum'           => { in  => ['+value','+atom','int'],
                             out => [],
                             bi  => fdp_sum,
                             module => libfd,
                             native => true},

    'fdp_sumC'          => { in  => ['+value','+value','+atom','int'],
                             out => [],
                             bi  => fdp_sumC,
                             module => libfd,
                             native => true},

    'fdp_sumCN'         => { in  => ['+value','+value','+atom','int'],
                             out => [],
                             bi  => fdp_sumCN,
                             module => libfd,
                             native => true},

    'fdp_sumR'          => { in  => ['+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumR,
                             module => libfd,
                             native => true},

    'fdp_sumCR'         => { in  => ['+value','+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCR,
                             module => libfd,
                             native => true},

    'fdp_sumCNR'        => { in  => ['+value','+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCNR,
                             module => libfd,
                             native => true},

    'fdp_sumCD'         => { in  => ['+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCD,
                             module => libfd,
                             native => true},

    'fdp_sumCCD'        => { in  => ['+value','+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCCD,
                             module => libfd,
                             native => true},

    'fdp_sumCNCD'       => { in  => ['+value','+value','+atom','*int','*int'],
                             out => [],
                             bi  => fdp_sumCNCD,
                             module => libfd,
                             native => true},

    'fdp_plus'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_plus,
                             module => libfd,
                             native => true},

    'fdp_minus'         => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_minus,
                             module => libfd,
                             native => true},

    'fdp_times'         => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_times,
                             module => libfd,
                             native => true},

    'fdp_power'         => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_power,
                             module => libfd,
                             native => true},

    'fdp_divD'          => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_divD,
                             module => libfd,
                             native => true},

    'fdp_divI'          => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_divI,
                             module => libfd,
                             native => true},

    'fdp_modD'          => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_modD,
                             module => libfd,
                             native => true},

    'fdp_modI'          => { in  => ['int','+int','int'],
                             out => [],
                             bi  => fdp_modI,
                             module => libfd,
                             native => true},

    'fdp_conj'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_conj,
                             module => libfd,
                             native => true},

    'fdp_disj'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_disj,
                             module => libfd,
                             native => true},

    'fdp_exor'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_exor,
                             module => libfd,
                             native => true},

    'fdp_impl'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_impl,
                             module => libfd,
                             native => true},

    'fdp_equi'          => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_equi,
                             module => libfd,
                             native => true},

    'fdp_nega'          => { in  => ['int','int'],
                             out => [],
                             bi  => fdp_nega,
                             module => libfd,
                             native => true},

    'fdp_intR'          => { in  => ['int','+value','int'],
                             out => [],
                             bi  => fdp_intR,
                             module => libfd,
                             native => true},

    'fdp_card'          => { in  => ['+value','int','int','int'],
                             out => [],
                             bi  => fdp_card,
                             module => libfd,
                             native => true},

    'fdp_exactly'       => { in  => ['int','+value','+int'],
                             out => [],
                             bi  => fdp_exactly,
                             module => libfd,
                             native => true},

    'fdp_atLeast'       => { in  => ['int','+value','+int'],
                             out => [],
                             bi  => fdp_atLeast,
                             module => libfd,
                             native => true},

    'fdp_atMost'        => { in  => ['int','+value','+int'],
                             out => [],
                             bi  => fdp_atMost,
                             module => libfd,
                             native => true},

    'fdp_element'       => { in  => ['int','+value','int'],
                             out => [],
                             bi  => fdp_element,
                             module => libfd,
                             native => true},

    'fdp_lessEqOff'     => { in  => ['int','int','+int'],
                             out => [],
                             bi  => fdp_lessEqOff,
                             module => libfd,
                             native => true},

    'fdp_minimum'       => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_minimum,
                             module => libfd,
                             native => true},

    'fdp_maximum'       => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_maximum,
                             module => libfd,
                             native => true},

    'fdp_inter' => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_inter,
                             module => libfd,
                     native => true},

    'fdp_union' => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdp_union,
                             module => libfd,
                     native => true},

    'fdp_distinct'      => { in  => ['+value'],
                             out => [],
                             bi  => fdp_distinct,
                             module => libfd,
                             native => true},

    'fdp_distinctD'     => { in  => ['+value'],
                             out => [],
                             bi  => fdp_distinctD,
                             module => libfd,
                             native => true},

    'fdp_distinctStream'=> { in  => ['+value','value'],
                             out => [],
                             bi  => fdp_distinctStream,
                             module => libfd,
                             native => true},

    'fdp_distinctOffset'=> { in  => ['+value','+value'],
                             out => [],
                             bi  => fdp_distinctOffset,
                             module => libfd,
                             native => true},

    'fdp_disjoint'=> { in  => ['int','+int','int','+int'],
                             out => [],
                             bi  => fdp_disjoint,
                             module => libfd,
                       native => true},

    'sched_disjoint_card'=> { in  => ['int','+int','int','+int'],
                             out => [],
                             bi  => sched_disjoint_card,
                             module => libschedule,
                              native => true},

    'fdp_disjointC'=> { in  => ['int','+int','int','+int','int'],
                             out => [],
                             bi  => fdp_disjointC,
                             module => libfd,
                        native => true},

    'fdp_distance'      => { in  => ['int','int','+atom','int'],
                             out => [],
                             bi  => fdp_distance,
                             module => libfd,
                             native => true},

    'fdp_distinct2'     => { in  => ['+value','+value','+value','+value'],
                             out => [],
                             bi  => fdp_distinct2,
                             module => libfd,
                             native => true},

    'sched_cpIterate'   => { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => sched_cpIterate,
                             module => libschedule,
                             native => true},

    'sched_cpIterateCap'=> { in  => ['+value','+value','+value',
                                     '+value','+value','+int'],
                             out => [],
                             bi  => sched_cpIterateCap,
                             module => libschedule,
                             native => true},

    'sched_cumulativeTI'=> { in  => ['+value','+value','+value',
                                     '+value','+value'],
                             out => [],
                             bi  => sched_cumulativeTI,
                             module => libschedule,
                             native => true},

    'sched_cpIterateCapUp'=> { in  => ['+value','+value','+value',
                                       '+value','+value'],
                             out => [],
                             bi  => sched_cpIterateCapUp,
                             module => libschedule,
                               native => true},

    'sched_taskIntervals'=> { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => sched_taskIntervals,
                             module => libschedule,
                              native => true},

    'sched_disjunctive' => { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => sched_disjunctive,
                             module => libschedule,
                             native => true},

    'sched_disjunctiveStream'=> { in  => ['+value','+value','value'],
                             out => [],
                             bi  => sched_disjunctiveStream,
                             module => libschedule,
                                  native => true},

    'fdp_subset'        => { in  => ['int','int'],
                             out => [],
                             bi  => fdp_subset,
                             module => libfd,
                             native => true},

    'fdp_dsum'          => { in  => ['+value','+atom','int'],
                             out => [],
                             bi  => fdp_dsum,
                             module => libfd,
                             native => true},

    'fdp_dsumC'         => { in  => ['+value','+value','+atom','int'],
                             out => [],
                             bi  => fdp_dsumC,
                             module => libfd,
                             native => true},

    'fdp_sumAC'         => { in  => ['+value','+value','+atom','int'],
                             out => [],
                             bi  => fdp_sumAC,
                             module => libfd,
                             native => true},

    'counter'           => { in  => ['int','value'],
                             out => [],
                             bi  => fdtest_counter,
                             module => libfd,
                             native => true},

    'firstFail'         => { in  => ['+value','value'],
                             out => [],
                             bi  => fdtest_firstFail,
                             module => libfd,
                             native => true},

    'sched_taskIntervalsProof'=> { in  => ['value','value','value','value',
                                           'value'],
                             out => [],
                             bi  => sched_taskIntervalsProof,
                             module => libschedule,
                                   native => true},

    'sched_firstsLasts' => { in  => ['value','value','value','value',
                                     'value'],
                             out => [],
                             bi  => sched_firstsLasts,
                             module => libschedule,
                             native => true},

    'spawnLess'         => { in  => ['int','int'],
                             out => [],
                             bi  => fdtest_spawnLess,
                             module => libfd,
                             native => true},

    'dplus'             => { in  => ['int','int','int'],
                             out => [],
                             bi  => fdtest_plus,
                             module => libfd,
                             native => true},

    'sumac'             => { in  => ['value','value','int'],
                             out => [],
                             bi  => fdtest_sumac,
                             module => libfd,
                             native => true},

    'testgensum'        => { in  => ['value','int'],
                             out => [],
                             bi  => fdtest_gensum,
                             module => libfd,
                             native => true},

    'testsum'           => { in  => ['value','int'],
                             out => [],
                             bi  => fdtest_sum,
                             ifdef =>ALLDIFF,
                             module => libfd,
                             native => true},

    'inqueens'          => { in  => ['value'],
                             out => [],
                             bi  => fdtest_inqueens,
                             ifdef =>INPROP,
                             module => libfd,
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
                             native => false},

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
                             module => libfset,
                             native => true},

    'fsp_isIn'          => { in  => ['int','fset','bool'],
                             out => [],
                             bi  => fsp_isIn,
                             module => libfset,
                             native => true},

    'fsp_isInR'         => { in  => ['int','fset','int'],
                             out => [],
                             bi  => fsp_isInR,
                             module => libfset,
                             native => true},

    'fsp_include'       => { in  => ['int','fset'],
                             out => [],
                             bi  => fsp_include,
                             module => libfset,
                             native => true},

    'fsp_exclude'       => { in  => ['int','fset'],
                             out => [],
                             bi  => fsp_exclude,
                             module => libfset,
                             native => true},

    'fsp_match'         => { in  => ['fset','+value'],
                             out => [],
                             bi  => fsp_match,
                             module => libfset,
                             native => true},

    'fsp_seq'           => { in  => ['+value'],
                             out => [],
                             bi  => fsp_seq,
                             module => libfset,
                             native => true},

    'fsp_minN'          => { in  => ['fset','+value'],
                             out => [],
                             bi  => fsp_minN,
                             module => libfset,
                             native => true},

    'fsp_maxN'          => { in  => ['fset','+value'],
                             out => [],
                             bi  => fsp_maxN,
                             module => libfset,
                             native => true},

    'fsp_card'          => { in  => ['fset','int'],
                             out => [],
                             bi  => fsp_card,
                             module => libfset,
                             native => true},

    'fsp_union'         => { in  => ['fset','fset','fset'],
                             out => [],
                             bi  => fsp_union,
                             module => libfset,
                             native => true},

    'fsp_intersection'  => { in  => ['fset','fset','fset'],
                             out => [],
                             bi  => fsp_intersection,
                             module => libfset,
                             native => true},

    'fsp_subsume'       => { in  => ['fset','fset'],
                             out => [],
                             bi  => fsp_subsume,
                             module => libfset,
                             native => true},

    'fsp_disjoint'      => { in  => ['fset','fset'],
                             out => [],
                             bi  => fsp_disjoint,
                             module => libfset,
                             native => true},

    'fsp_distinct'      => { in  => ['fset','fset'],
                             out => [],
                             bi  => fsp_distinct,
                             module => libfset,
                             native => true},

    'fsp_monitorIn'     => { in  => ['fset','value'],
                             out => [],
                             bi  => fsp_monitorIn,
                             module => libfset,
                             native => true},

    'fsp_min'           => { in  => ['fset','int'],
                             out => [],
                             bi  => fsp_min,
                             module => libfset,
                             native => true},

    'fsp_max'           => { in  => ['fset','int'],
                             out => [],
                             bi  => fsp_max,
                             module => libfset,
                             native => true},

    'fsp_convex'        => { in  => ['fset'],
                             out => [],
                             bi  => fsp_convex,
                             module => libfset,
                             native => true},

    'fsp_diff'          => { in  => ['fset','fset','fset'],
                             out => [],
                             bi  => fsp_diff,
                             module => libfset,
                             native => true},

    'fsp_includeR'      => { in  => ['int','fset','int'],
                             out => [],
                             bi  => fsp_includeR,
                             module => libfset,
                             native => true},

    'fsp_bounds'        => { in  => ['+fset','fset','int','int','int'],
                             out => [],
                             bi  => fsp_bounds,
                             module => libfset,
                             native => true},

    'fsp_boundsN'       => { in  => ['+value','+value','+value',
                                     '+value','+value'],
                             out => [],
                             bi  => fsp_boundsN,
                             module => libfset,
                             native => true},

    'fsp_disjointN'     => { in  => ['+value'],
                             out => [],
                             bi  => fsp_disjointN,
                             module => libfset,
                             native => true},

    'fsp_unionN'        => { in  => ['+value','fset'],
                             out => [],
                             bi  => fsp_unionN,
                             module => libfset,
                             native => true},

    'fsp_partition'     => { in  => ['+value','fset'],
                             out => [],
                             bi  => fsp_partition,
                             module => libfset,
                             native => true},

    'fsp_partitionReified'=> { in  => ['+value','fset','+value'],
                             out => [],
                             bi  => fsp_partitionReified,
                             module => libfset,
                             native => true},

    'fsp_partitionProbing'=> { in  => ['+value','fset','+value'],
                               out => [],
                               bi  => fsp_partitionProbing,
                               module => libfset,
                               native => true},

    'fsp_partitionReified1'=> { in  => ['+value','fset','+value','int'],
                                out => [],
                                bi  => fsp_partitionReified1,
                                module => libfset,
                                native => true},



    #* Unclassified

    ##* Constraints

    'System.nbSusps'    => { in  => ['value'],
                             out => ['+int'],
                             BI  => BIconstraints,
                             native => true},

    'ozma_readProc'     => { in     => ['+virtualString'],
                             out    => ['+value'],
                             BI     => ozma_readProc,
                             ifdef  => STATIC_LIBOZMA,
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

# this is the function that converts these descriptions to
# an array of declarations appropriate for dynamic linking

sub CDYNTABLE {
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
        if ( !($native eq "true")) {
            die "*** native flag for $key must be 'true'";
        }
        $BI = $info->{bi} unless $BI;
        print "{\"$key\",\t$inArity,\t$outArity,$BI},\n";
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
        my $negated = $info->{negated};
        print "\t\tnegated: '$negated'\n" if $negated;
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
    elsif ($option eq '-cdyntable')  { $choice='cdyntable'; }
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
elsif ($choice eq 'cdyntable' ) { &CDYNTABLE;   }
elsif ($choice eq 'oztable')    { &OZTABLE; }
elsif ($choice eq 'sortnativeness') { &SORTNATIVENESS; }
elsif ($choice eq 'structure')   { &STRUCTURE; }
else { die "must specify one of: -ctable -cdecl -oztable -structure -sortnativeness"; }
