#! /usr/local/bin/perl
###
### Here we declare all builtins in a general format which can be used
### to generate both the table of builtins used by the emulator and the
### information used by the Oz compiler.
###
###	bidecl.perl -ctable
###		generates the table of builtins for the emulator
###	bidecl.perl -oztable
###		generates the table of builtins for the Oz compiler
###
### Each entry has the form: 'NAME' => { ... }, where 'NAME' is the
### string by which the builtin is known to the emulator.  The associative
### array describing the builtin contains at least:
###	in  => [...],
###	out => [...],
### describing respectively the input and output arguments.  Each argument
### is described by a type, possibly annotated by a prefix indicating
### the determinacy condition.  Thus an integer argument might be specified
### in one of these ways:
###	 'int'		possibly non-determined
###	'+int'		determined
###	'*int'		kinded (e.g. an FD variable)
### '+int' (resp. '*int') indicates that the builtin will suspend until
### this argument is determined (resp. kinded).
###
### Furthermore, there are builtins that overwrite their input arguments.
### This should be indicated by the prefix `!' (which should come first).
### Thus '!+value' indicates an argument for which the builtin will suspend
### until it is determined and which may be overwriten by its execution.
###
### A type may be simple or complex:
###
### SIMPLE    ::= abstraction		(not yet known to compiler)
###		| atom
###		| array
###		| bool
###		| cell
###		| char
###		| chunk
###		| comparable
###		| class
###		| dictionary
###		| feature
###		| float
###		| foreignPointer	(not yet known to compiler)
###		| int
###		| literal
###		| lock
###		| name
###		| number
###		| object
###		| port
###		| procedure
###		| procedure/1
###		| procedureOrObject	(not yet known to compiler)
###		| record
###		| recordC
###		| recordCOrChunk
###		| space
###		| string
###		| thread
###		| tuple
###		| value
###		| virtualString
###
### COMPLEX   ::= [SIMPLE]		(list of SIMPLE)
###		| [SIMPLE#SIMPLE]	(list of pairs of SIMPLE)
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
###
### doesNotReturn => 1, indicates that the builtin does not return and
### therefore that the code following it will never be executed.  For
### example 'raise'.
###

$builtins = {
    '/'		=> { in  => ['+float','+float'],
		     out => ['+float'],
		     bi  => BIfdiv,
		     ibi => BIfdivInline },

    '*'		=> { in  => ['+number','+number'],
		     out => ['+number'],
		     bi  => BImult,
		     ibi => BImultInline},
	     
    'div'	=> { in  => ['+int','+int'],
		     out => ['+int'],
		     bi  => BIdiv,
		     ibi => BIdivInline},

    'mod'	=> { in  => ['+int','+int'],
		     out => ['+int'],
		     bi  => BImod,
		     ibi => BImodInline},

    '-'		=> { in  => ['+number','+number'],
		     out => ['+number'],
		     bi  => BIminus,
		     ibi => BIminusInline},

    '+'		=> { in  => ['+number','+number'],
		     out => ['+number'],
		     bi  => BIplus,
		     ibi => BIplusInline},

    'Max'	=> { in  => ['+comparable','+comparable'],
		     out => ['+comparable'],
		     bi  => BImax,
		     ibi => BImaxInline},

    'Min'	=> { in  => ['+comparable','+comparable'],
		     out => ['+comparable'],
		     bi  => BImin,
		     ibi => BIminInline},

    '<'		=> { in  => ['+comparable','+comparable'],
		     out => ['+bool'],
		     bi  => BIlessFun,
		     ibi => BIlessInlineFun,
		     shallow => '<Rel' },

    '=<'	=> { in  => ['+comparable','+comparable'],
		     out => ['+bool'],
		     bi  => BIleFun,
		     ibi => BIleInlineFun,
		     shallow => '=<Rel' },

    '>'		=> { in  => ['+comparable','+comparable'],
		     out => ['+bool'],
		     bi  => BIgreatFun,
		     ibi => BIgreatInlineFun,
		     shallow => '>Rel' },

    '>='	=> { in  => ['+comparable','+comparable'],
		     out => ['+bool'],
		     bi  => BIgeFun,
		     ibi => BIgeInlineFun,
		     shallow => '>=Rel' },

    '=<Rel'	=> { in  => ['+comparable','+comparable'],
		     out => [],
		     bi  => BIle,
		     ibi => BIleInline},

    '<Rel'	=> { in  => ['+comparable','+comparable'],
		     out => [],
		     bi  => BIless,
		     ibi => BIlessInline},

    '>=Rel'	=> { in  => ['+comparable','+comparable'],
		     out => [],
		     bi  => BIge,
		     ibi => BIgeInline},

    '>Rel'	=> { in  => ['+comparable','+comparable'],
		     out => [],
		     bi  => BIgreat,
		     ibi => BIgreatInline},

    '~'		=> { in  => ['+number'],
		     out => ['+number'],
		     bi  => BIuminus,
		     ibi => BIuminusInline},

    '+1'	=> { in  => ['+number'],
		     out => ['+number'],
		     bi  => BIadd1,
		     ibi => BIadd1Inline},

    '-1'	=> { in  => ['+number'],
		     out => ['+number'],
		     bi  => BIsub1,
		     ibi => BIsub1Inline},

    'Exp'	=> { in  => ['+float'],
		     out => ['+float'],
		     bi  => BIexp,
		     ibi => BIinlineExp},
  
    'Log'	=> { in  => ['+float'],
		     out => ['+float'],
		     bi  => BIlog,
		     ibi => BIinlineLog},
  
    'Sqrt'	=> { in  => ['+float'],
		     out => ['+float'],
		     bi  => BIsqrt,
		     ibi => BIinlineSqrt},
  
    'Sin'	=> { in  => ['+float'],
		     out => ['+float'],
		     bi  => BIsin,
		     ibi => BIinlineSin},
  
    'Asin'	=> { in  => ['+float'],
		     out => ['+float'],
		     bi  => BIasin,
		     ibi => BIinlineAsin},
  
    'Cos'	=> { in  => ['+float'],
		     out => ['+float'],
		     bi  => BIcos,
		     ibi => BIinlineCos},
  
    'Acos'	=> { in  => ['+float'],
		     out => ['+float'],
		     bi  => BIacos,
		     ibi => BIinlineAcos},
  
    'Tan'	=> { in  => ['+float'],
		     out => ['+float'],
		     bi  => BItan,
		     ibi => BIinlineTan},
  
    'Atan'	=> { in  => ['+float'],
		     out => ['+float'],
		     bi  => BIatan,
		     ibi => BIinlineAtan},
  
    'Ceil'	=> { in  => ['+float'],
		     out => ['+float'],
		     bi  => BIceil,
		     ibi => BIinlineCeil},
  
    'Floor'	=> { in  => ['+float'],
		     out => ['+float'],
		     bi  => BIfloor,
		     ibi => BIinlineFloor},
  
    'Abs'	=> { in  => ['+float'],
		     out => ['+float'],
		     bi  => BIabs,
		     ibi => BIabsInline},
  
    'Round'	=> { in  => ['+float'],
		     out => ['+float'],
		     bi  => BIround,
		     ibi => BIinlineRound},
  
    'Atan2'	=> { in  => ['+float','+float'],
		     out => ['+float'],
		     bi  => BIatan2,
		     ibi => BIatan2Inline},
  
    'fPow'	=> { in  => ['+float','+float'],
		     out => ['+float'],
		     bi  => BIfPow,
		     ibi => BIfPowInline},

    # conversion: float <-> int <-> virtualStrings

    'IntToFloat'	=> { in  => ['+int'],
			     out => ['+float'],
			     bi  => BIintToFloat,
			     ibi => BIintToFloatInline},

    'FloatToInt'	=> { in  => ['+float'],
			     out => ['+int'],
			     bi  => BIfloatToInt,
			     ibi => BIfloatToIntInline},

    'IntToString'	=> { in  => ['+int'],
			     out => ['+string'],
			     BI  => BIintToString}, # new style builtin

    'FloatToString'	=> { in  => ['+float'],
			     out => ['+string'],
			     BI  => BIfloatToString},

    'StringToInt'	=> { in  => ['+string'],
			     out => ['+int'],
			     BI  => BIstringToInt},

    'StringToFloat'	=> { in  => ['+string'],
			     out => ['+float'],
			     BI  => BIstringToFloat},

    'String.isInt'	=> { in  => ['+string'],
			     out => ['+bool'],
			     BI  => BIstringIsInt},

    'String.isFloat'	=> { in  => ['+string'],
			     out => ['+bool'],
			     BI  => BIstringIsFloat},

    'String.isAtom'	=> { in  => ['+string'],
			     out => ['+bool'],
			     BI  => BIstringIsAtom},

    'IsArray'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisArray,
			     ibi => isArrayInline},

    'NewArray'		=> { in  => ['+int','+int','value'],
			     out => ['+array'],
			     BI  => BIarrayNew},

    'Array.high'	=> { in  => ['+array'],
			     out => ['+int'],
			     bi  => BIarrayHigh,
			     ibi => arrayHighInline},

    'Array.low'		=> { in  => ['+array'],
			     out => ['+int'],
			     bi  => BIarrayLow,
			     ibi => arrayLowInline},

    'Get'		=> { in  => ['+array','+int'],
			     out => ['value'],
			     bi  => BIarrayGet,
			     ibi => arrayGetInline},

    'Put'		=> { in  => ['+array','+int','value'],
			     out => [],
			     bi  => BIarrayPut,
			     ibi => arrayPutInline},


    'NewDictionary' 	=> { in  => [],
			     out => ['+dictionary'],
			     BI  => BIdictionaryNew},

    'IsDictionary' 	=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisDictionary,
			     ibi => isDictionaryInline},

    'Dictionary.isEmpty'=> { in  => ['+dictionary'],
			     out => ['+bool'],
			     bi  => BIdictionaryIsMt,
			     ibi => dictionaryIsMtInline},

    'Dictionary.get'	=> { in  => ['+dictionary','+feature'],
			     out => ['value'],
			     bi  => BIdictionaryGet,
			     ibi => dictionaryGetInline},

    'Dictionary.condGet'=> { in  => ['+dictionary','+feature','value'],
			     out => ['value'],
			     bi  => BIdictionaryCondGet,
			     ibi => dictionaryCondGetInline},

    'Dictionary.put'	=> { in  => ['+dictionary','+feature','value'],
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

    'Dictionary.remove'	=> { in  => ['+dictionary','+feature'],
			     out => [],
			     bi  => BIdictionaryRemove,
			     ibi => dictionaryRemoveInline},

    'Dictionary.removeAll'=> { in  => ['+dictionary'],
			       out => [],
			       BI  => BIdictionaryRemoveAll},

    'Dictionary.member'	=> { in  => ['+dictionary','+feature'],
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

    'NewLock'		=> { in  => [],
			     out => ['+lock'],
			     BI  => BInewLock},

    'Lock'		=> { in  => ['+lock'],
			     out => [],
			     BI  => BIlockLock},

    'Unlock'		=> { in  => ['+lock'],
			     out => [],
			     BI  => BIunlockLock},


    'NewPort'		=> { in  => ['value'],
			     out => ['+port'],
			     BI  => BInewPort},

    'Send'		=> { in  => ['+port','value'],
			     out => [],
			     BI  => BIsendPort},

    'NewCell'		=> { in  => ['value'],
			     out => ['+cell'],
			     BI  => BInewCell},

    'Exchange'		=> { in  => ['+cell','value','value'],
			     out => [],
			     bi  => BIexchangeCell,
			     ibi => BIexchangeCellInline},

    'Access'		=> { in  => ['+cell'],
			     out => ['value'],
			     bi  => BIaccessCell,
			     ibi => BIaccessCellInline},

    'Assign'		=> { in  => ['+cell','value'],
			     out => [],
			     bi  => BIassignCell,
			     ibi => BIassignCellInline},

    # perdio

    'probe'		=> { in  => ['value'],
			     out => [],
			     BI  => BIprobe},

    'perdioRestop'	=> { in  => ['value'],
			     out => [],
			     BI  => BIrestop},

    'crash'		=> { in  => [],
			     out => [],
			     BI  => BIcrash},

    'InstallHandler'	=> { in  => ['+value','+value','value'],
			     out => [],
			     BI  => BIhandlerInstall},

    'InstallWatcher'	=> { in  => ['+value','+value','value'],
			     out => [],
			     BI  => BIwatcherInstall},

    # characters

    'IsChar'		=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => BIcharIs},

    'Char.isAlNum'	=> { in  => ['+char'],
			     out => ['+bool'],
			     BI  => BIcharIsAlNum},

    'Char.isAlpha'	=> { in  => ['+char'],
			     out => ['+bool'],
			     BI  => BIcharIsAlpha},

    'Char.isCntrl'	=> { in  => ['+char'],
			     out => ['+bool'],
			     BI  => BIcharIsCntrl},

    'Char.isDigit'	=> { in  => ['+char'],
			     out => ['+bool'],
			     BI  => BIcharIsDigit},

    'Char.isGraph'	=> { in  => ['+char'],
			     out => ['+bool'],
			     BI  => BIcharIsGraph},

    'Char.isLower'	=> { in  => ['+char'],
			     out => ['+bool'],
			     BI  => BIcharIsLower},

    'Char.isPrint'	=> { in  => ['+char'],
			     out => ['+bool'],
			     BI  => BIcharIsPrint},

    'Char.isPunct'	=> { in  => ['+char'],
			     out => ['+bool'],
			     BI  => BIcharIsPunct},

    'Char.isSpace'	=> { in  => ['+char'],
			     out => ['+bool'],
			     BI  => BIcharIsSpace},

    'Char.isUpper'	=> { in  => ['+char'],
			     out => ['+bool'],
			     BI  => BIcharIsUpper},

    'Char.isXDigit'	=> { in  => ['+char'],
			     out => ['+bool'],
			     BI  => BIcharIsXDigit},

    'Char.toLower'	=> { in  => ['+char'],
			     out => ['+char'],
			     BI  => BIcharToLower},

    'Char.toUpper'	=> { in  => ['+char'],
			     out => ['+char'],
			     BI  => BIcharToUpper},

    'Char.toAtom'	=> { in  => ['+char'],
			     out => ['+atom'],
			     BI  => BIcharToAtom},

    'Char.type'		=> { in  => ['+char'],
			     out => ['+atom'],
			     BI  => BIcharType},

    # records

    'Adjoin'		=> { in  => ['+record','+record'],
			     out => ['+record'],
			     bi  => BIadjoin,
			     ibi => BIadjoinInline},

    'AdjoinList'	=> { in  => ['+record','+[feature#value]'],
			     out => ['+record'],
			     BI  => BIadjoinList},

    'record'		=> { in  => ['+literal','+[feature#value]'],
			     out => ['+record'],
			     BI  => BImakeRecord},

    'Arity'		=> { in  => ['+record'],
			     out => ['+[feature]'],
			     bi  => BIarity,
			     ibi => BIarityInline},

    'AdjoinAt'		=> { in  => ['+record','+feature','value'],
			     out => ['+record'],
			     BI  => BIadjoinAt},

    # types tests

    'IsNumber'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisNumberB,
			     ibi => BIisNumberBInline,
			     shallow => isNumberRel},

    'IsInt'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisIntB,
			     ibi => BIisIntBInline,
			     shallow => isIntRel},

    'IsFloat'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisFloatB,
			     ibi => BIisFloatBInline,
			     shallow => isFloatRel},

    'IsRecord'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisRecordB,
			     ibi => isRecordBInline,
			     shallow => isRecordRel},

    'IsTuple'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisTupleB,
			     ibi => isTupleBInline,
			     shallow => isTupleRel},

    'IsLiteral'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisLiteralB,
			     ibi => isLiteralBInline,
			     shallow => isLiteralRel},

    'IsLock'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisLockB,
			     ibi => isLockBInline,
			     shallow => isLockRel},

    'IsCell'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisCellB,
			     ibi => isCellBInline,
			     shallow => isCellRel},

    'IsPort'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisPortB,
			     ibi => isPortBInline,
			     shallow => isPortRel},

    'IsProcedure'	=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisProcedureB,
			     ibi => isProcedureBInline,
			     shallow => isProcedureRel},

    'IsName'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisNameB,
			     ibi => isNameBInline,
			     shallow => isNameRel},

    'IsAtom'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisAtomB,
			     ibi => isAtomBInline,
			     shallow => isAtomRel},

    'IsBool'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisBoolB,
			     ibi => isBoolBInline,
			     shallow => isBoolRel},

    'IsUnit'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisUnitB,
			     ibi => isUnitBInline,
			     shallow => isUnitRel},

    'IsChunk'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisChunkB,
			     ibi => isChunkBInline,
			     shallow => isChunkRel},

    'IsRecordC'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisRecordCB,
			     ibi => isRecordCBInline,
			     shallow => isRecordCRel},

    'IsObject'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisObjectB,
			     ibi => BIisObjectBInline,
			     shallow => isObjectRel},

    'IsString'		=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => BIisString},

    'IsVirtualString'	=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => BIvsIs},

    'IsFree'		=> { in  => ['value'],
			     out => ['+bool'],
			     bi  => BIisFree,
			     ibi => isFreeInline,
			     shallow => IsFreeRel},

    'IsKinded'		=> { in  => ['value'],
			     out => ['+bool'],
			     bi  => BIisKinded,
			     ibi => isKindedInline,
			     shallow => IsKindedRel},

    'IsDet'		=> { in  => ['value'],
			     out => ['+bool'],
			     bi  => BIisDet,
			     ibi => isDetInline,
			     shallow => IsDetRel},

    'isNumberRel'	=> { in  => ['+value'],
			     out => [],
			     bi  => BIisNumber,
			     ibi => BIisNumberInline},

    'isIntRel'		=> { in  => ['+value'],
			     out => [],
			     bi  => BIisInt,
			     ibi => BIisIntInline},

    'isFloatRel'	=> { in  => ['+value'],
			     out => [],
			     bi  => BIisFloat,
			     ibi => BIisFloatInline},

    'isRecordRel'	=> { in  => ['+value'],
			     out => [],
			     bi  => BIisRecord,
			     ibi => isRecordInline},

    'isTupleRel'	=> { in  => ['+value'],
			     out => [],
			     bi  => BIisTuple,
			     ibi => isTupleInline},

    'isLiteralRel'	=> { in  => ['+value'],
			     out => [],
			     bi  => BIisLiteral,
			     ibi => isLiteralInline},

    'isCellRel'		=> { in  => ['+value'],
			     out => [],
			     bi  => BIisCell,
			     ibi => isCellInline},

    'isPortRel'		=> { in  => ['+value'],
			     out => [],
			     bi  => BIisPort,
			     ibi => isPortInline},

    'isProcedureRel'	=> { in  => ['+value'],
			     out => [],
			     bi  => BIisProcedure,
			     ibi => isProcedureInline},

    'isNameRel'		=> { in  => ['+value'],
			     out => [],
			     bi  => BIisName,
			     ibi => isNameInline},

    'isAtomRel'		=> { in  => ['+value'],
			     out => [],
			     bi  => BIisAtom,
			     ibi => isAtomInline},

    'isLockRel'		=> { in  => ['+value'],
			     out => [],
			     bi  => BIisLock,
			     ibi => isLockInline},

    'isBoolRel'		=> { in  => ['+value'],
			     out => [],
			     bi  => BIisBool,
			     ibi => isBoolInline},

    'isUnitRel'		=> { in  => ['+value'],
			     out => [],
			     bi  => BIisUnit,
			     ibi => isUnitInline},

    'isChunkRel'	=> { in  => ['+value'],
			     out => [],
			     bi  => BIisChunk,
			     ibi => isChunkInline},

    'isRecordCRel'	=> { in  => ['+value'],
			     out => [],
			     bi  => BIisRecordC,
			     ibi => isRecordCInline},

    'isObjectRel'	=> { in  => ['+value'],
			     out => [],
			     bi  => BIisObject,
			     ibi => BIisObjectInline},

    'IsFreeRel'		=> { in  => ['value'],
			     out => [],
			     bi  => BIisFreeRel,
			     ibi => isFreeRelInline},

    'IsKindedRel'	=> { in  => ['value'],
			     out => [],
			     bi  => BIisKindedRel,
			     ibi => isKindedRelInline},

    'IsDetRel'		=> { in  => ['value'],
			     out => [],
			     bi  => BIisDetRel,
			     ibi => isDetRelInline},

    'Wait'		=> { in  => ['+value'],
			     out => [],
			     bi  => BIisValue,
			     ibi => isValueInline},

    'WaitOr'		=> { in  => ['value','value'],
			     out => [],
			     BI  => BIwaitOr},

    'virtualStringLength'=> { in  => ['virtualString','+int'],
			      out => ['+int'],
			      BI  => BIvsLength},


    'Length'		=> { in  => ['+[value]'],
			     out => ['+int'],
			     BI  => BIlength},

    'Not'		=> { in  => ['+bool'],
			     out => ['+bool'],
			     bi  => BInot,
			     ibi => notInline},

    'And'		=> { in  => ['+bool','+bool'],
			     out => ['+bool'],
			     bi  => BIand,
			     ibi => andInline},

    'Or'		=> { in  => ['+bool','+bool'],
			     out => ['+bool'],
			     bi  => BIor,
			     ibi => orInline},

    'Type.ofValue'	=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BItermType,
			     ibi => BItermTypeInline},

    'Value.status'	=> { in  => ['value'],
			     out => ['+tuple'],
			     bi  => BIstatus,
			     ibi => BIstatusInline},

    # deep magic for procedures

    'procedureEnvironment'=> { in  => ['+procedure'],
			       out => ['+tuple'],
			       BI  => BIprocedureEnvironment},

    'getProcInfo'	=> { in  => ['+procedure'],
			     out => ['value'],
			     BI  => BIgetProcInfo},

    'setProcInfo'	=> { in  => ['+procedure','value'],
			     out => [],
			     BI  => BIsetProcInfo},

    'getProcNames'	=> { in  => ['+procedure'],
			     out => ['value'],
			     BI  => BIgetProcNames},

    'setProcNames'	=> { in  => ['+procedure','value'],
			     out => [],
			     BI  => BIsetProcNames},

    'getProcPos'	=> { in  => ['+procedure'],
			     out => ['+literal','+int'],
			     BI  => BIgetProcPos},

    # tuples and records and OFS

    'MakeTuple'		=> { in  => ['+literal','+int'],
			     out => ['+tuple'],
			     bi  => BItuple,
			     ibi => tupleInline},

    'Label'		=> { in  => ['*recordC'],
			     out => ['+literal'],
			     bi  => BIlabel,
			     ibi => labelInline},

    'hasLabel'		=> { in  => ['value'],
			     out => ['+bool'],
			     bi  => BIhasLabel,
			     ibi => hasLabelInline},

    'ProcedureArity'	=> { in  => ['+procedure'],
			     out => ['+int'],
			     bi  => BIprocedureArity,
			     ibi => procedureArityInline},

    'TellRecord'	=> { in  => ['+literal','record'],
			     out => [],
			     BI  => BIrecordTell},

    'WidthC'		=> { in  => ['*record','int'],
			     out => [],
			     BI  => BIwidthC},

    'monitorArity'	=> { in  => ['*recordC','value','[feature]'],
			     out => [],
			     BI  => BImonitorArity},

    'tellRecordSize'	=> { in  => ['+literal','+int','record'],
			     out => [],
			     BI  => BIsystemTellSize},

    'recordCIsVarB'	=> { in  => ['value'],
			     out => ['+bool'],
			     BI  => BIisRecordCVarB},

    # records and chunks

    '.'			=> { in  => ['*recordCOrChunk','+feature'],
			     out => ['value'],
			     bi  => BIdot,
			     ibi => dotInline},

    '^'			=> { in  => ['*recordCOrChunk','+feature'],
			     out => ['value'],
			     bi  => BIuparrowBlocking,
			     ibi => uparrowInlineBlocking},

    'HasFeature'	=> { in  => ['*recordCOrChunk','+feature'],
			     out => ['+bool'],
			     bi  => BIhasFeatureB,
			     ibi => hasFeatureBInline},

    'CondSelect'	=> { in  => ['*recordCOrChunk','+feature','value'],
			     out => ['value'],
			     bi  => BImatchDefault,
			     ibi => matchDefaultInline},

    'Width'		=> { in  => ['+record'],
			     out => ['+int'],
			     bi  => BIwidth,
			     ibi => widthInline},

    # atoms

    'AtomToString'	=> { in  => ['+atom'],
			     out => ['+string'],
			     bi  => BIatomToString,
			     ibi => atomToStringInline},

    'StringToAtom'	=> { in  => ['+string'],
			     out => ['+atom'],
			     BI  => BIstringToAtom},

    # chunks

    'NewChunk'		=> { in  => ['+record'],
			     out => ['+chunk'],
			     BI  => BInewChunk},

    'chunkArity'	=> { in  => ['+chunk'],
			     out => ['+int'],
			     BI  => BIchunkArity},

    'chunkWidth'	=> { in  => ['+chunk'],
			     out => ['+int'],
			     BI  => BIchunkWidth},

    'recordWidth'	=> { in  => ['record'],
			     out => ['int'],
			     BI  => BIrecordWidth},

    # names

    'NewName'		=> { in  => [],
			     out => ['+name'],
			     BI  => BInewName},

    'NewUniqueName'	=> { in  => ['+atom'],
			     out => ['+name'],
			     BI  => BInewUniqueName},

    # equalities

    '=='		=> { in  => ['*value','*value'],
			     out => ['+bool'],
			     bi  => BIeqB,
			     ibi => eqeqInline,
			     eqeq => 1},

    '\\\\='		=> { in  => ['*value','*value'],
			     out => ['+bool'],
			     bi  => BIneqB,
			     ibi => neqInline,
			     eqeq => 1},

    '==Rel'		=> { in  => ['*value','*value'],
			     out => [],
			     BI  => BIeq},

    '\\\\=Rel'		=> { in  => ['*value','*value'],
			     out => [],
			     BI  => BIneq},

    # dynamic libraries

    'isForeignPointer'	=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => BIisForeignPointer },

    'dlOpen'		=> { in  => ['+virtualString'],
			     out => ['+foreignPointer'],
			     BI  => BIdlOpen},

    'dlClose'		=> { in  => ['+foreignPointer'],
			     out => [],
			     BI  => BIdlClose},

    'findFunction'	=> { in  => ['+virtualString','+int',
				     '+foreignPointer'],
			     out => [],
			     BI  => BIfindFunction},

    'dlLoad'		=> { in  => ['+virtualString'],
			     out => ['+foreignPointer','+record'],
			     BI  => BIdlLoad},

    # miscellaneous system things

    'shutdown'		=> { in  => ['+int'],
			     out => [],
			     BI  => BIshutdown},

    'Alarm'		=> { in  => ['+int','value'],
			     out => [],
			     BI  => BIalarm},

    'Delay'		=> { in  => ['!+int'],
			     out => [],
			     BI  => BIdelay},

    'System.gcDo'	=> { in  => [],
			     out => [],
			     BI  => BIgarbageCollection},

    'System.apply'	=> { in  => ['+procedureOrObject','+[value]'],
			     out => [],
			     BI  => BIapply},

    'System.eq'		=> { in  => ['value','value'],
			     out => ['+bool'],
			     BI  => BIsystemEq},

    '='			=> { in  => ['value','value'],
			     out => [],
			     BI  => BIunify},

    'fail'		=> { in  => [],
			     out => [],
			     BI  => BIfail},

    'nop'		=> { in  => [],
			     out => [],
			     BI  => BInop},

    'deepFeed'		=> { in  => ['+cell','value'],
			     out => [],
			     BI  => BIdeepFeed},

    # browser primitives

    'getsBound'		=> { in  => ['value'],
			     out => [],
			     BI  => BIgetsBound},

    'getsBoundB'	=> { in  => ['value','value'],
			     out => [],
			     BI  => BIgetsBoundB},

    # useless

    'setAbstractionTabDefaultEntry' => { in  => ['value'],
      out => [],
      BI  => BIsetAbstractionTabDefaultEntry},

    # printing

    'showBuiltins'	=> { in  => [],
			     out => [],
			     BI  => BIshowBuiltins},

    'Print'		=> { in  => ['value'],
			     out => [],
			     bi  => BIprint,
			     ibi => printInline},

    'Show'		=> { in  => ['value'],
			     out => [],
			     bi  => BIshow,
			     ibi => showInline},

    # constraints

    'System.nbSusps'	=> { in  => ['value'],
			     out => ['+int'],
			     BI  => BIconstraints},

    # miscellaneous

    'onToplevel'	=> { in  => [],
			     out => ['+bool'],
			     BI  => BIonToplevel},

    # browser

    'addr'		=> { in  => ['value'],
			     out => ['+int'],
			     BI  => BIaddr},

    # source level debugger

    'Debug.mode'	=> { in  => [],
			     out => ['+bool'],
			     BI  => BIdebugmode},

    'Debug.getStream'	=> { in  => [],
			     out => ['value'],
			     BI  => BIgetDebugStream},

    'Debug.setStepFlag'	=> { in  => ['+thread','+bool'],
			     out => [],
			     BI  => BIsetStepFlag},

    'Debug.setTraceFlag'=> { in  => ['+thread','+bool'],
			     out => [],
			     BI  => BIsetTraceFlag},

    'Debug.checkStopped'=> { in  => ['+thread'],
			     out => ['+bool'],
			     BI  => BIcheckStopped},

    # Debug module

    'Debug.prepareDumpThreads'	=> { in  => [],
				     out => [],
				     BI  => BIprepareDumpThreads},
    
    'Debug.dumpThreads'	=> { in  => [],
			     out => [],
			     BI  => BIdumpThreads},

    'Debug.listThreads'	=> { in  => [],
			     out => ['value'],
			     BI  => BIlistThreads},

    'Debug.breakpointAt'=> { in  => ['value','+int','value'],
			     out => ['+bool'],
			     BI  => BIbreakpointAt},

    'Debug.breakpoint'	=> { in  => [],
			     out => [],
			     BI  => BIbreakpoint},

    'Debug.displayCode'	=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIdisplayCode},

    'Debug.procedureCode'=> { in  => ['+procedure'],
			      out => ['+int'],
			      BI  => BIprocedureCode},
    
    'Debug.procedureCoord'=> { in  => ['+procedure'],
			       out => ['+record'],
			       BI  => BIprocedureCoord},

    'Debug.livenessX'	=> { in  => ['+int'],
			     out => ['+int'],
			     BI  => BIlivenessX},

    'index2Tagged'	=> { in  => ['int'],
			     out => ['value'],
			     BI  => BIindex2Tagged,
			     ifdef=>'UNUSED'},

    'time2localTime'	=> { in  => ['int'],
			     out => ['value'],
			     BI  => BItime2localTime,
			     ifdef=>'UNUSED'},

    # Builtins for the Thread module

    'Thread.is'		=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => BIthreadIs},

    'Thread.id'		=> { in  => ['+thread'],
			     out => ['+int'],
			     BI  => BIthreadID},

    'Thread.setId'	=> { in  => ['+thread','+int'],
			     out => [],
			     BI  => BIsetThreadID},

    'Thread.parentId'	=> { in  => ['+thread'],
			     out => ['int'],
			     BI  => BIparentThreadID},

    'Thread.this'	=> { in  => [],
			     out => ['+thread'],
			     BI  => BIthreadThis},

    'Thread.suspend'	=> { in  => ['+thread'],
			     out => [],
			     BI  => BIthreadSuspend},

    'Thread.unleash'	=> { in  => ['+thread','+int'],
			     out => [],
			     BI  => BIthreadUnleash},

    'Thread.resume'	=> { in  => ['+thread'],
			     out => [],
			     BI  => BIthreadResume},

    'Thread.injectException'=> { in  => ['+thread','+value'],
				 out => [],
				 BI  => BIthreadRaise},

    'Thread.preempt'	=> { in  => ['+thread'],
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

    'Thread.state'	=> { in  => ['+thread'],
			     out => ['+atom'],
			     BI  => BIthreadState},

    'Thread.setRaiseOnBlock'=> { in  => ['+thread','+bool'],
				 out => [],
				 BI  => BIthreadSetRaiseOnBlock},

    'Thread.getRaiseOnBlock'=> { in  => ['+thread'],
				 out => ['+bool'],
				 BI  => BIthreadGetRaiseOnBlock},

    'Thread.taskStack'	=> { in  => ['+thread','+int','+bool'],
			     out => ['+value'],
			     BI  => BIthreadTaskStack},

    'Thread.frameVariables'=> { in  => ['+thread','+int'],
				out => ['+value'],
				BI  => BIthreadFrameVariables},

    'Thread.location'	=> { in  => ['+thread'],
			     out => ['+value'],
			     BI  => BIthreadLocation},

    # printing primitives for debugging

    'Debug.print'	=> { in  => ['value','+int'],
			     out => [],
			     BI  => BIdebugPrint,
			     ifdef=>'DEBUG_PRINT'},

    'Debug.printLong'	=> { in  => ['value','+int'],
			     out => [],
			     BI  => BIdebugPrintLong,
			     ifdef=>'DEBUG_PRINT'},

    # statistics

    'statisticsReset'	=> { in  => [],
			     out => [],
			     BI  => BIstatisticsReset},

    'statisticsPrint'	=> { in  => ['+virtualString'],
			     out => [],
			     BI  => BIstatisticsPrint},

    'statisticsPrintProcs'=> { in  => [],
			       out => [],
			       BI  => BIstatisticsPrintProcs},

    'statisticsGetProcs'=> { in  => [],
			     out => ['+value'],
			     BI  => BIstatisticsGetProcs},

    'setProfileMode'	=> { in  => ['+bool'],
			     out => [],
			     BI  => BIsetProfileMode},

    'instructionsPrint'	=> { in  => [],
			     out => [],
			     BI  => BIinstructionsPrint,
			     ifdef=>'PROFILE_INSTR'},

    'biPrint'		=> { in  => [],
			     out => [],
			     BI  => BIbiPrint,
			     ifdef=>'PROFILE_BI'},

    'halt'		=> { in  => [],
			     out => [],
			     BI  => BIhalt,
			     ifdef=>'DEBUG_TRACE'},

    # system primitives

    'System.printName'	=> { in  => ['value'],
			     out => ['+atom'],
			     BI  => BIgetPrintName},

    'System.printInfo'	=> { in  => ['virtualString'],
			     out => [],
			     BI  => BIprintInfo},

    'System.printError'	=> { in  => ['virtualString'],
			     out => [],
			     BI  => BIprintError},

    'System.valueToVirtualString'=> { in  => ['value','+int','+int'],
				      out => ['+string'],
				      BI  => BItermToVS},

    'getTermSize'	=> { in  => ['value','+int','+int'],
			     out => ['+int'],
			     BI  => BIgetTermSize},

    # FD linking info

    'foreignFDProps'	=> { in  => [],
			     out => ['+bool'],
			     BI  => BIforeignFDProps},

    # object-oriented primitives

    '@'			=> { in  => ['+feature'],
			     out => ['value'],
			     bi  => BIat,
			     ibi => atInline},

    '<-'		=> { in  => ['+feature','value'],
			     out => [],
			     bi  => BIassign,
			     ibi => assignInline},

    'copyRecord'	=> { in  => ['+record'],
			     out => ['+record'],
			     BI  => BIcopyRecord},

    'makeClass'		=> { in  => ['+dictionary','+record','+record',
				     '+dictionary','+bool'],
			     out => ['+class'],
			     BI  => BImakeClass},

    ','			=> { in  => ['+class','+record'],
			     out => [],
			     bi  => BIcomma},

    'send'		=> { in  => ['+record','+class','+object'],
			     out => [],
			     bi  => BIsend},

    'getClass'		=> { in  => ['+object'],
			     out => ['+class'],
			     bi  => BIgetClass,
			     ibi => getClassInline},

    'ooGetLock'		=> { in  => ['value'],
			     out => [],
			     bi  => BIooGetLock,
			     ibi => ooGetLockInline},

    'newObject'		=> { in  => ['+class'],
			     out => ['+object'],
			     bi  => BInewObject,
			     ibi => newObjectInline},

    'New'		=> { in  => ['+class','+record'],
			     out => ['+object'],
			     bi  => BINew},

    'setSelf'		=> { in  => ['+object'],
			     out => [],
			     BI  => BIsetSelf},

    'ooExch'		=> { in  => ['+feature','value'],
			     out => ['value'],
			     bi  => BIooExch,
			     ibi => ooExchInline},

    # spaces

    'Space.new'		=> { in  => ['+procedure/1'],
			     out => ['+space'],
			     BI  => BInewSpace},

    'IsSpace'		=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => BIisSpace},

    'Space.ask'		=> { in  => ['+space'],
			     out => ['+tuple'],
			     BI  => BIaskSpace},

    'Space.askVerbose'	=> { in  => ['+space','!value'],
			     out => [],
			     BI  => BIaskVerboseSpace},

    'Space.merge'	=> { in  => ['+space'],
			     out => ['+value'],
			     BI  => BImergeSpace},

    'Space.clone'	=> { in  => ['+space'],
			     out => ['+space'],
			     BI  => BIcloneSpace},

    'Space.commit'	=> { in  => ['+space','+value'],
			     out => [],
			     BI  => BIcommitSpace},

    'Space.inject'	=> { in  => ['+space','+procedure/1'],
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

    'raise'		=> { in  => ['value'],
			     out => [],
			     BI  => BIraise,
			     doesNotReturn => 1},

    'raiseError'	=> { in  => ['value'],
			     out => [],
			     BI  => BIraiseError,
			     doesNotReturn => 1},

    'raiseDebug'	=> { in  => ['value'],
			     out => [],
			     BI  => BIraiseDebug,
			     doesNotReturn => 1},

    # builtins for the new compiler
    # (OPI and environment handling):

    'setOPICompiler'	=> { in  => ['+object'],
			     out => [],
			     BI  => BIsetOPICompiler},

    'getOPICompiler'	=> { in  => [],
			     out => ['+value'],
			     BI  => BIgetOPICompiler},

    'isBuiltin'		=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => BIisBuiltin},

    'getBuiltinName'	=> { in  => ['+value'],
			     out => ['+atom'],
			     BI  => BIgetBuiltinName},

    'nameVariable'	=> { in  => ['value','+atom'],
			     out => [],
			     BI  => BInameVariable},

    'newNamedName'	=> { in  => ['+atom'],
			     out => ['+literal'],
			     BI  => BInewNamedName},

    'isUniqueName'	=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => BIisUniqueName},

    'generateAbstractionTableID'=> { in  => ['+value'],
				     out => ['+bool'],
				     BI  => BIgenerateAbstractionTableID},

    'concatenateAtomAndInt'	=> { in  => ['+atom','+int'],
				     out => ['+atom'],
				     BI  => BIconcatenateAtomAndInt},

    'RegSet.new'	=> { in  => ['+int','+int'],
			     out => ['+value'],
			     BI  => BIregSet_new},

    'RegSet.copy'	=> { in  => ['+value'],
			     out => ['+value'],
			     BI  => BIregSet_copy},

    'RegSet.adjoin'	=> { in  => ['+value','+int'],
			     out => [],
			     BI  => BIregSet_adjoin},

    'RegSet.remove'	=> { in  => ['+value','+int'],
			     out => [],
			     BI  => BIregSet_remove,},

    'RegSet.member'	=> { in  => ['+int','+value'],
			     out => ['+bool'],
			     BI  => BIregSet_member},

    'RegSet.union'	=> { in  => ['+value','+value'],
			     out => [],
			     BI  => BIregSet_union},

    'RegSet.intersect'	=> { in  => ['+value','+value'],
			     out => [],
			     BI  => BIregSet_intersect},

    'RegSet.subtract'	=> { in  => ['+value','+value'],
			     out => [],
			     BI  => BIregSet_subtract},

    'RegSet.toList'	=> { in  => ['+value'],
			     out => ['+[int]'],
			     BI  => BIregSet_toList},

    'RegSet.complementToList'	=> { in  => ['+value'],
				     out => ['+[int]'],
				     BI  => BIregSet_complementToList},

    # Oz parser

    'ozparser_parseFile'	=> { in  => ['+virtualString','+record'],
				     out => ['+value'],
				     bi  => ozparser_parseFile},

    'ozparser_parseVirtualString'=> { in  => ['+virtualString','+record'],
				      out => ['+value'],
				      bi  => ozparser_parseVirtualString},

    'ozparser_fileExists'	=> { in  => ['+virtualString','bool'],
				     out => [],
				     bi  => ozparser_fileExists},

    'copyCode'		=> { in  => ['+abstraction','+dictionary'],
			     out => [],
			     BI  => BIcopyCode},

    # Finalization

    'Finalize.register'	=> { in  => ['+value','+value'],
			     out => [],
			     BI  => BIfinalize_register},

    'Finalize.setHandler'=> { in  => ['+value'],
			      out => [],
			      BI  => BIfinalize_setHandler},

    'GetCloneDiff'	=> { in  => ['+space'],
			     out => ['+value'],
			     BI  => BIgetCloneDiff,
			     ifdef=>'CS_PROFILE'},

    'SystemRegistry'	=> { in  => [],
			     out => ['+dictionary'],
			     BI  => BIsystem_registry},

    'ServiceRegistry'	=> { in  => [],
			     out => ['+dictionary'],
			     BI  => BIsystem_registry},

    #-----------------------------------------------------------------
    # ASSEMBLE.CC
    #-----------------------------------------------------------------

    'getOpcode'		=> { in  => ['+atom'],
			     out => ['+int'],
			     BI  => BIgetOpcode},

    'getInstructionSize'=> { in  => ['+atom'],
			     out => ['+int'],
			     BI  => BIgetInstructionSize},

    'newCodeBlock'	=> { in  => ['+int'],
			     out => ['+int'],
			     BI  => BInewCodeBlock},

    'makeProc'		=> { in  => ['+int','+value'],
			     out => ['+abstraction'],
			     BI  => BImakeProc},

    'addDebugInfo'	=> { in  => ['+int','+atom','+int'],
			     out => [],
			     BI  => BIaddDebugInfo},

    'storeOpcode'	=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIstoreOpcode},

    'storeNumber'	=> { in  => ['+int','+number'],
			     out => [],
			     BI  => BIstoreNumber},

    'storeLiteral'	=> { in  => ['+int','+literal'],
			     out => [],
			     BI  => BIstoreLiteral},

    'storeFeature'	=> { in  => ['+int','+feature'],
			     out => [],
			     BI  => BIstoreFeature},

    'storeConstant'	=> { in  => ['+int','+value'],
			     out => [],
			     BI  => BIstoreConstant},

    'storeBuiltinname'	=> { in  => ['+int','+procedure'],
			     out => [],
			     BI  => BIstoreBuiltinname},

    'storeVariablename'	=> { in  => ['+int','+atom'],
			     out => [],
			     BI  => BIstoreVariablename},

    'storeRegisterIndex'=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIstoreRegisterIndex},

    'storeInt'		=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIstoreInt},

    'storeLabel'	=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIstoreLabel},

    'storePredicateRef'	=> { in  => ['+int','+value'],
			     out => [],
			     BI  => BIstorePredicateRef},

    'storePredId'	=> { in  => ['+int','+value','+value','+value',
				     '+int','+bool'],
			     out => [],
			     BI  => BIstorePredId},

    'newHashTable'	=> { in  => ['+int','+int','+int'],
			     out => ['+int'],
			     BI  => BInewHashTable},

    'storeHTVarLabel'	=> { in  => ['+int','+int','+int'],
			     out => [],
			     BI  => BIstoreHTVarLabel},

    'storeHTScalar'	=> { in  => ['+int','+int','+value','+int'],
			     out => [],
			     BI  => BIstoreHTScalar},

    'storeHTRecord'	=> { in  => ['+int','+int','+value','+value','+int'],
			     out => [],
			     BI  => BIstoreHTRecord},

    'storeRecordArity'	=> { in  => ['+int','+value'],
			     out => [],
			     BI  => BIstoreRecordArity},

    'storeGenCallInfo'	=> { in  => ['+int','+int','+value','+value',
				     '+value','+value'],
			     out => [],
			     BI  => BIstoreGenCallInfo},

    'storeApplMethInfo'	=> { in  => ['+int','+value','+value'],
			     out => [],
			     BI  => BIstoreApplMethInfo},

    'storeGRegRef'	=> { in  => ['+int','+value'],
			     out => [],
			     BI  => BIstoreGRegRef},

    'storeCache'	=> { in  => ['+int','+value'],
			     out => [],
			     BI  => BIstoreCache}

    #-----------------------------------------------------------------
    # FD
    #-----------------------------------------------------------------

###    'fdReset'		=> { in  => [],
###			     out => [],
###			     bi  =>BIfdReset,
###			     ifdef=>PROFILE_FD},
###
###    'fdDiscard'		=> { in  => [],
###			     out => [],
###			     bi  => BIfdDiscard,
###			     ifdef=>PROFILE_FD},
###
###    'fdGetNext'		=> { in  => ['value'],
###			     out => [],
###			     bi  => BIfdGetNext,
###			     ifdef=>PROFILE_FD},
###
###    'fdPrint'		=> { in  => [],
###			     out => [],
###			     bi  => BIfdPrint,
###			     ifdef=>PROFILE_FD},
###
###    'fdTotalAverage'	=> { in  => [],
###			     out => [],
###			     bi  => BIfdTotalAverage,
###			     ifdef=>PROFILE_FD},
###
###    'fdIs'		=> { in  => ['*value','+bool'],
###			     out => [],
###			     bi  => BIfdIs},
###
###    'fdIsVar'		=> { in  => ['value'],
###			     out => [],
###			     BI  => BIisFdVar},
###
###    'fdIsVarB'		=> { in  => ['value'],
###			     out => ['+bool'],
###			     BI  => BIisFdVarB},
###
###    'fdGetLimits'	=> { in  => [],
###			     out => ['+int','+int'],
###			     BI  => BIgetFDLimits},
###
###    'fdGetMin'		=> { in  => ['value','int'],
###			     out => [],
###			     bi  => BIfdMin},
###
###    'fdGetMid'		=> { in  => ['value','int'],
###			     out => [],
###			     bi  => BIfdMid},
###
###    'fdGetMax'		=> { in  => ['value','int'],
###			     out => [],
###			     bi  => BIfdMax},
###
###    'fdGetDom'		=> { in  => ['*int','+[value]'],
###			     out => [],
###			     bi  => BIfdGetAsList},
###
###    'fdGetCard'		=> { in  => ['*int','int'],
###			     out => [],
###			     bi  => BIfdGetCardinality},
###
###    'fdGetNextSmaller'	=> { in  => ['+int','*int','int'],
###			     out => [],
###			     bi  => BIfdNextSmaller},
###
###    'fdGetNextLarger'	=> { in  => ['+int','*int','int'],
###			     out => [],
###			     bi  => BIfdNextLarger},
###
###    'fdTellConstraint'	=> { in  => ['+value','int'],
###			     out => [],
###			     bi  => BIfdTellConstraint},
###
###    'fdWatchSize'	=> { in  => ['*int','+int','bool'],
###			     out => [],
###			     bi  => BIfdWatchSize},
###
###    'fdWatchMin'	=> { in  => ['*int','+int','bool'],
###			     out => [],
###			     bi  => BIfdWatchMin},
###
###    'fdWatchMax'	=> { in  => ['*int','+int','bool'],
###			     out => [],
###			     bi  => BIfdWatchMax},
###
###    'fdConstrDisjSetUp'	=> { in  => ['+value','+value','+value','+value'],
###			     out => [],
###			     bi  => BIfdConstrDisjSetUp},
###
###    'fdConstrDisj'	=> { in  => ['+value','+value','+value'],
###			     out => [],
###			     bi  => BIfdConstrDisj},
###
###    'fdTellConstraintCD'=> { in  => ['value','value','value'],
###			     out => [],
###			     bi  => BIfdTellConstraintCD},
###
###    'fdp_init'		=> { in  => ['atom'],
###			     out => [],
###			     bi  => fdp_init,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_sum'		=> { in  => ['+value','+atom','int'],
###			     out => [],
###			     bi  => fdp_sum,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_sumC'		=> { in  => ['+value','+value','+atom','int'],
###			     out => [],
###			     bi  => fdp_sumC,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_sumCN'		=> { in  => ['+value','+value','+atom','int'],
###			     out => [],
###			     bi  => fdp_sumCN,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_sumR'		=> { in  => ['+value','+atom','*int','*int'],
###			     out => [],
###			     bi  => fdp_sumR,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_sumCR'		=> { in  => ['+value','+value','+atom','*int','*int'],
###			     out => [],
###			     bi  => fdp_sumCR,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_sumCNR'	=> { in  => ['+value','+value','+atom','*int','*int'],
###			     out => [],
###			     bi  => fdp_sumCNR,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_sumCD'		=> { in  => ['+value','+atom','*int','*int'],
###			     out => [],
###			     bi  => fdp_sumCD,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_sumCCD'	=> { in  => ['+value','+value','+atom','*int','*int'],
###			     out => [],
###			     bi  => fdp_sumCCD,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_sumCNCD'	=> { in  => ['+value','+value','+atom','*int','*int'],
###			     out => [],
###			     bi  => fdp_sumCNCD,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_plus_rel'	=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_plus_rel,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_plus'		=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_plus,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_minus'		=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_minus,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_times'		=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_times,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_times_rel'	=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_times_rel,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_power'		=> { in  => ['int','+int','int'],
###			     out => [],
###			     bi  => fdp_power,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_divD'		=> { in  => ['int','+int','int'],
###			     out => [],
###			     bi  => fdp_divD,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_divI'		=> { in  => ['int','+int','int'],
###			     out => [],
###			     bi  => fdp_divI,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_modD'		=> { in  => ['int','+int','int'],
###			     out => [],
###			     bi  => fdp_modD,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_modI'		=> { in  => ['int','+int','int'],
###			     out => [],
###			     bi  => fdp_modI,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_conj'		=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_conj,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_disj'		=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_disj,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_exor'		=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_exor,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_impl'		=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_impl,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_equi'		=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_equi,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_nega'		=> { in  => ['int','int'],
###			     out => [],
###			     bi  => fdp_nega,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_intR'		=> { in  => ['int','+value','int'],
###			     out => [],
###			     bi  => fdp_intR,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_card'		=> { in  => ['+value','int','int','int'],
###			     out => [],
###			     bi  => fdp_card,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_exactly'	=> { in  => ['int','+value','+int'],
###			     out => [],
###			     bi  => fdp_exactly,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_atleast'	=> { in  => ['int','+value','+int'],
###			     out => [],
###			     bi  => fdp_atleast,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_atmost'	=> { in  => ['int','+value','+int'],
###			     out => [],
###			     bi  => fdp_atmost,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_element'	=> { in  => ['int','+value','int'],
###			     out => [],
###			     bi  => fdp_element,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_notEqOff'	=> { in  => ['int','int','+int'],
###			     out => [],
###			     bi  => fdp_notEqOff,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_lessEqOff'	=> { in  => ['int','int','+int'],
###			     out => [],
###			     bi  => fdp_lessEqOff,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_minimum'	=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_minimum,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_maximum'	=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_maximum,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_inter'	=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_inter,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_union'	=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdp_union,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_distinct'	=> { in  => ['+value'],
###			     out => [],
###			     bi  => fdp_distinct,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_distinctD'	=> { in  => ['+value'],
###			     out => [],
###			     bi  => fdp_distinctD,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_distinctStream'=> { in  => ['+value','value'],
###			     out => [],
###			     bi  => fdp_distinctStream,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_distinctOffset'=> { in  => ['+value','+value'],
###			     out => [],
###			     bi  => fdp_distinctOffset,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_disjoint'=> { in  => ['int','+int','int','+int'],
###			     out => [],
###			     bi  => fdp_disjoint,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'sched_disjoint_card'=> { in  => ['int','+int','int','+int'],
###			     out => [],
###			     bi  => sched_disjoint_card,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_disjointC'=> { in  => ['int','+int','int','+int','int'],
###			     out => [],
###			     bi  => fdp_disjointC,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_distance'	=> { in  => ['int','int','+atom','int'],
###			     out => [],
###			     bi  => fdp_distance,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_distinct2'	=> { in  => ['+value','+value','+value','+value'],
###			     out => [],
###			     bi  => fdp_distinct2,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'sched_cpIterate'	=> { in  => ['+value','+value','+value'],
###			     out => [],
###			     bi  => sched_cpIterate,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'sched_cpIterateCap'=> { in  => ['+value','+value','+value',
###				     '+value','+value','+int'],
###			     out => [],
###			     bi  => sched_cpIterateCap,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'sched_cumulativeTI'=> { in  => ['+value','+value','+value',
###				     '+value','+value'],
###			     out => [],
###			     bi  => sched_cumulativeTI,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'sched_cpIterateCapUp'=> { in  => ['+value','+value','+value',
###				       '+value','+value'],
###			     out => [],
###			     bi  => sched_cpIterateCapUp,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'sched_taskIntervals'=> { in  => ['+value','+value','+value'],
###			     out => [],
###			     bi  => sched_taskIntervals,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'sched_disjunctive'	=> { in  => ['+value','+value','+value'],
###			     out => [],
###			     bi  => sched_disjunctive,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'sched_disjunctiveStream'=> { in  => ['+value','+value','value'],
###			     out => [],
###			     bi  => sched_disjunctiveStream,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_twice'		=> { in  => ['value','value'],
###			     out => [],
###			     bi  => fdp_twice,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_square'	=> { in  => ['value','value'],
###			     out => [],
###			     bi  => fdp_square,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_subset'	=> { in  => ['value','value'],
###			     out => [],
###			     bi  => fdp_subset,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_dsum'		=> { in  => ['+value','+atom','int'],
###			     out => [],
###			     bi  => fdp_dsum,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_dsumC'		=> { in  => ['+value','+value','+atom','int'],
###			     out => [],
###			     bi  => fdp_dsumC,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'fdp_sumAC'		=> { in  => ['+value','+value','+atom','int'],
###			     out => [],
###			     bi  => fdp_sumAC,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'counter'		=> { in  => ['int','value'],
###			     out => [],
###			     bi  => fdtest_counter,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'firstFail'		=> { in  => ['+value','value'],
###			     out => [],
###			     bi  => fdtest_firstFail,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'sched_taskIntervalsProof'=> { in  => ['value','value','value','value',
###					   'value'],
###			     out => [],
###			     bi  => sched_taskIntervalsProof,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'sched_firstsLasts'	=> { in  => ['value','value','value','value',
###				     'value'],
###			     out => [],
###			     bi  => sched_firstsLasts,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'spawnLess'		=> { in  => ['int','int'],
###			     out => [],
###			     bi  => fdtest_spawnLess,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'dplus'		=> { in  => ['int','int','int'],
###			     out => [],
###			     bi  => fdtest_plus,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'sumac'		=> { in  => ['value','value','int'],
###			     out => [],
###			     bi  => fdtest_sumac,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'testgensum'	=> { in  => ['value','int'],
###			     out => [],
###			     bi  => fdtest_gensum,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'testsum'		=> { in  => ['value','int'],
###			     out => [],
###			     bi  => fdtest_sum,
###			     ifdef =>ALLDIFF,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'inqueens'		=> { in  => ['value'],
###			     out => [],
###			     bi  => fdtest_inqueens,
###			     ifdef =>INPROP,
###			     ifndef=>FOREIGNFDPROPS},
###
###    'debugStable'	=> { in  => [],
###			     out => [],
###			     bi  => debugStable,
###			     ifdef =>DEBUG_STABLE},
###
###    'resetStable'	=> { in  => [],
###			     out => [],
###			     bi  => resetStable,
###			     ifdef =>DEBUG_STABLE},
###
###    'fddistribute'	=> { in  => ['value','value','value','value','value',],
###			     out => [],
###			     bi  => BIfdDistribute},

};

### USED THIS TO VERIFY THAT I WAS PRODUCING THE SAME TABLE AS WAS
### ORIGINALLY IN BUILTINS.CC
### @foo = (
### "/",
### "*",
### "div",
### "mod",
### "-",
### "+",
### "Max",
### "Min",
### "<",
### "=<",
### ">",
### ">=",
### "=<Rel",
### "<Rel",
### ">=Rel",
### ">Rel",
### "~",
### "+1",
### "-1",
### "Exp",
### "Log",
### "Sqrt",
### "Sin",
### "Asin",
### "Cos",
### "Acos",
### "Tan",
### "Atan",
### "Ceil",
### "Floor",
### "Abs",
### "Round",
### "Atan2",
### "fPow",
### "IntToFloat",
### "FloatToInt",
### "IntToString",
### "FloatToString",
### "StringToInt",
### "StringToFloat",
### "String.isInt",
### "String.isFloat",
### "String.isAtom",
### "IsArray",
### "NewArray",
### "Array.high",
### "Array.low",
### "Get",
### "Put",
### "NewDictionary",
### "IsDictionary",
### "Dictionary.isEmpty",
### "Dictionary.get",
### "Dictionary.condGet",
### "Dictionary.put",
### "Dictionary.condPut",
### "Dictionary.exchange",
### "Dictionary.condExchange",
### "Dictionary.remove",
### "Dictionary.removeAll",
### "Dictionary.member",
### "Dictionary.keys",
### "Dictionary.entries",
### "Dictionary.items",
### "Dictionary.clone",
### "Dictionary.markSafe",
### "NewLock",
### "Lock",
### "Unlock",
### "NewPort",
### "Send",
### "NewCell",
### "Exchange",
### "Access",
### "Assign",
### "probe",
### "perdioRestop",
### "crash",
### "InstallHandler",
### "InstallWatcher",
### "IsChar",
### "Char.isAlNum",
### "Char.isAlpha",
### "Char.isCntrl",
### "Char.isDigit",
### "Char.isGraph",
### "Char.isLower",
### "Char.isPrint",
### "Char.isPunct",
### "Char.isSpace",
### "Char.isUpper",
### "Char.isXDigit",
### "Char.toLower",
### "Char.toUpper",
### "Char.toAtom",
### "Char.type",
### "Adjoin",
### "AdjoinList",
### "record",
### "Arity",
### "AdjoinAt",
### "IsNumber",
### "IsInt"   ,
### "IsFloat" ,
### "IsRecord",
### "IsTuple",
### "IsLiteral",
### "IsLock",
### "IsCell",
### "IsPort",
### "IsProcedure",
### "IsName",
### "IsAtom",
### "IsBool",
### "IsUnit",
### "IsChunk",
### "IsRecordC",
### "IsObject",
### "IsString",
### "IsVirtualString",
### "IsFree",
### "IsKinded",
### "IsDet",
### "isNumberRel",
### "isIntRel"   ,
### "isFloatRel" ,
### "isRecordRel",
### "isTupleRel",
### "isLiteralRel",
### "isCellRel",
### "isPortRel",
### "isProcedureRel",
### "isNameRel",
### "isAtomRel",
### "isLockRel",
### "isBoolRel",
### "isUnitRel",
### "isChunkRel",
### "isRecordCRel",
### "isObjectRel",
### "IsFreeRel",
### "IsKindedRel",
### "IsDetRel",
### "Wait",
### "WaitOr",
### "virtualStringLength",
### "Length",
### "Not",
### "And",
### "Or",
### "Type.ofValue",
### "Value.status",
### "procedureEnvironment",
### "getProcInfo",
### "setProcInfo",
### "getProcNames",
### "setProcNames",
### "getProcPos",
### "MakeTuple",
### "Label",
### "hasLabel",
### "ProcedureArity",
### "TellRecord",
### "WidthC",
### "monitorArity",
### "tellRecordSize",
### "recordCIsVarB",
### ".",
### "^",
### "HasFeature",
### "CondSelect",
### "Width",
### "AtomToString",
### "StringToAtom",
### "NewChunk",
### "chunkArity",
### "chunkWidth",
### "recordWidth",
### "NewName",
### "NewUniqueName",
### "==",
### "\\\\=",
### "==Rel",
### "\\\\=Rel",
### "dlOpen",
### "dlClose",
### "findFunction",
### "dlLoad",
### "shutdown",
### "Alarm",
### "Delay",
### "System.gcDo",
### "System.apply",
### "System.eq",
### "=",
### "fail",
### "nop",
### "deepFeed",
### "getsBound",
### "getsBoundB",
### "setAbstractionTabDefaultEntry",
### "showBuiltins",
### "Print",
### "Show",
### "System.nbSusps",
### "onToplevel",
### "addr",
### "Debug.mode",
### "Debug.getStream",
### "Debug.setStepFlag",
### "Debug.setTraceFlag",
### "Debug.checkStopped",
### "Debug.prepareDumpThreads",
### "Debug.dumpThreads",
### "Debug.listThreads",
### "Debug.breakpointAt",
### "Debug.breakpoint",
### "Debug.displayCode",
### "Debug.procedureCode",
### "Debug.procedureCoord",
### "Debug.livenessX",
### "index2Tagged",
### "time2localTime",
### "Thread.is",
### "Thread.id",
### "Thread.setId",
### "Thread.parentId",
### "Thread.this",
### "Thread.suspend",
### "Thread.unleash",
### "Thread.resume",
### "Thread.injectException",
### "Thread.preempt",
### "Thread.setPriority",
### "Thread.getPriority",
### "Thread.isSuspended",
### "Thread.state",
### "Thread.setRaiseOnBlock",
### "Thread.getRaiseOnBlock",
### "Thread.taskStack",
### "Thread.frameVariables",
### "Thread.location",
### "Debug.print",
### "Debug.printLong",
### "statisticsReset",
### "statisticsPrint",
### "statisticsPrintProcs",
### "statisticsGetProcs",
### "setProfileMode",
### "instructionsPrint",
### "biPrint",
### "halt",
### "System.printName",
### "System.printInfo",
### "System.printError",
### "System.valueToVirtualString",
### "getTermSize",
### "foreignFDProps",
### "@",
### "<-",
### "copyRecord",
### "makeClass",
### ",",
### "send",
### "getClass",
### "ooGetLock",
### "newObject",
### "New",
### "setSelf",
### "ooExch",
### "Space.new",
### "IsSpace",
### "Space.ask",
### "Space.askVerbose",
### "Space.merge",
### "Space.clone",
### "Space.commit",
### "Space.inject",
### "biExceptionHandler",
### "setDefaultExceptionHandler",
### "getDefaultExceptionHandler",
### "raise",
### "raiseError",
### "raiseDebug",
### "setOPICompiler",
### "getOPICompiler",
### "isBuiltin",
### "getBuiltinName",
### "nameVariable",
### "newNamedName",
### "isUniqueName",
### "generateAbstractionTableID",
### "concatenateAtomAndInt",
### "RegSet.new",
### "RegSet.copy",
### "RegSet.adjoin",
### "RegSet.remove",
### "RegSet.member",
### "RegSet.union",
### "RegSet.intersect",
### "RegSet.subtract",
### "RegSet.toList",
### "RegSet.complementToList",
### "ozparser_parseFile",
### "ozparser_parseVirtualString",
### "ozparser_fileExists",
### "copyCode",
### "Finalize.register",
### "Finalize.setHandler",
### "GetCloneDiff",
### "SystemRegistry",
### "ServiceRegistry");

# this is the function that converts these descriptions to
# an array of declarations appropriate for the emulator

sub CTABLE {
    my ($key,$info);
    while (($key,$info) = each %$builtins) {
	my $arity = @{$info->{in}} + @{$info->{out}};
	my $BI = $info->{BI};
	my @ifdef  = split(/\,/,$info->{ifdef});
	my @ifndef = split(/\,/,$info->{ifndef});
	my $macro;
	foreach $macro (@ifdef)  { print "#ifdef $macro\n"; }
	foreach $macro (@ifddef) { print "#ifndef $macro\n"; }
	if ($BI) {
	    # new style
	    print "{\"$key\",\t$arity,$BI,\t0},\n";
	} else {
	    # old style
	    my $bi  = $info->{bi};
	    my $ibi = $info->{ibi};
	    if ($ibi) { $ibi = "(IFOR) $ibi"; }
	    else      { $ibi = "0"; }
	    print "{\"$key\",\t$arity,$bi,\t$ibi},\n";
	}
	foreach $macro (@ifddef) { print "#endif\n"; }
	foreach $macro (@ifdef)  { print "#endif\n"; }
    }
}

sub argspec {
    my $spec = shift;
    my ($mod,$det,$typ) = (0,'any','value');

    # is the argument register side effected?

    if ($spec =~ /^\!/) { $spec=$'; $mod=1; }

    # what is the determinacy condition on the argument?

    if    ($spec =~ /^\+/) { $spec=$'; $det='det'; }
    elsif ($spec =~ /^\*/) { $spec=$'; $det='detOrKinded'; }

    # now parse the type of the argument

    if    ($spec =~ /^\[(.+)\#(.+)\]$/) { $typ="list(pair($1 $2))"; }
    elsif ($spec =~ /^\[(.+)\]$/      ) { $typ="list($1)"; }
    else                                { $typ=$spec; }

    return ($mod,$det,$typ);
}

# $style==0	old style
# $style==1	both
# $style==2	new style

my $style = 0;

sub OZTABLE {
    my ($key,$info);
    while (($key,$info) = each %$builtins) {
	my (@imods,@idets,@ityps,$spec,$destroys);
	foreach $spec (@{$info->{in}}) {
	    my ($mod,$det,$typ) = &argspec($spec);
	    $destroys=1 if $mod;
	    push @imods,($mod?'true':'false');
	    push @idets,$det;
	    push @ityps,$typ;
	}
	my (@odets,@otyps);
	foreach $spec (@{$info->{out}}) {
	    my ($mod,$det,$typ) = &argspec($spec);
	    $det="any(det)" if $det eq 'det';
	    push @odets,$det;
	    push @otyps,$typ;
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
	$bi = $info->{bi} || $info->{BI};
	print "OZ_C_proc_proto($bi);\n";
    }
}

my ($option) = @ARGV;

if    ($option eq '-ctable' ) { &CTABLE; }
elsif ($option eq '-cdecl'  ) { &CDECL; }
elsif ($option eq '-oztable') { &OZTABLE; }
else { die "unrecognized options: $option"; }

