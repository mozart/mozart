# -*- perl -*-

%builtins_all =
(
 ### 
 ### Bootstrapping stuff, should eventually go way
 ###
 
 'builtin'	=> { in  => ['+virtualString','+int'],
		     out => ['+procedure'],
		     BI  => BIbuiltin},
 
 
 'BootManager' => { in     => ['+virtualString'],
		    out    => ['+record'],
		    BI     => BIBootManager},


 ##
 ## Module: Array
 ## 

 'IsArray'    => { in  => ['+value'],
		   out => ['+bool'],
		   bi  => BIisArray},

 'NewArray'   => { in  => ['+int','+int','value'],
		   out => ['+array'],
		   BI  => BIarrayNew},

 'Array.high' => { in  => ['+array'],
		   out => ['+int'],
		   bi  => BIarrayHigh},
 
 'Array.low'  => { in  => ['+array'],
		   out => ['+int'],
		   bi  => BIarrayLow},

 'Get'	      => { in  => ['+array','+int'],
		   out => ['value'],
		   bi  => BIarrayGet},

 'Put'	      => { in  => ['+array','+int','value'],
		   out => [],
		   bi  => BIarrayPut},
 


 ##
 ## Module: Atom
 ## 

 'IsAtom'	=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisAtomB},

 'AtomToString'	=> { in  => ['+atom'],
		     out => ['+string'],
		     bi  => BIatomToString},



 ##
 ## Module: BitArray
 ## 

 'BitArray.new'	                => { in  => ['+int','+int'],
				     out => ['+bitArray'],
				     BI  => BIbitArray_new},

 'BitArray.is'	                => { in  => ['+value'],
				     out => ['+bool'],
				     BI  => BIbitArray_is},

 'BitArray.set'	                => { in  => ['+bitArray','+int'],
				     out => [],
				     BI  => BIbitArray_set},

 'BitArray.clear'	        => { in  => ['+bitArray','+int'],
				     out => [],
				     BI  => BIbitArray_clear},

 'BitArray.test'	        => { in  => ['+bitArray','+int'],
				     out => ['+bool'],
				     BI  => BIbitArray_test},

 'BitArray.low'	                => { in  => ['+bitArray'],
				     out => ['+int'],
				     BI  => BIbitArray_low},

 'BitArray.high'	        => { in  => ['+bitArray'],
				     out => ['+int'],
				     BI  => BIbitArray_high},

 'BitArray.clone'	        => { in  => ['+bitArray'],
				     out => ['+bitArray'],
				     BI  => BIbitArray_clone},

 'BitArray.or'	                => { in  => ['+bitArray','+bitArray'],
				     out => [],
				     BI  => BIbitArray_or},

 'BitArray.and'	                => { in  => ['+bitArray','+bitArray'],
				     out => [],
				     BI  => BIbitArray_and},

 'BitArray.card'	        => { in  => ['+bitArray'],
				     out => ['+int'],
				     BI  => BIbitArray_card},

 'BitArray.disjoint'	        => { in  => ['+bitArray','+bitArray'],
				     out => ['+bool'],
				     BI  => BIbitArray_disjoint},

 'BitArray.nimpl'	        => { in  => ['+bitArray','+bitArray'],
				     out => [],
				     BI  => BIbitArray_nimpl},

 'BitArray.toList'	        => { in  => ['+bitArray'],
				     out => ['+[int]'],
				     BI  => BIbitArray_toList},

 'BitArray.complementToList'	=> { in  => ['+bitArray'],
				     out => ['+[int]'],
				     BI  => BIbitArray_complementToList},



 ##
 ## Module: Bool
 ## 

 'IsBool'	=> { in  => ['+value'],
		     out => ['+bool'],
		     bi  => BIisBoolB},

 'Not'		=> { in  => ['+bool'],
		     out => ['+bool'],
		     bi  => BInot},

 'And'		=> { in  => ['+bool','+bool'],
		     out => ['+bool'],
		     bi  => BIand},

 'Or'		=> { in  => ['+bool','+bool'],
		     out => ['+bool'],
		     bi  => BIor},


 ##
 ## Module: Cell
 ## 

 'IsCell'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisCellB},

 'NewCell'		=> { in  => ['value'],
			     out => ['+cell'],
			     BI  => BInewCell},

 'Exchange'		=> { in  => ['+cell','value','value'],
			     out => [],
			     bi  => BIexchangeCell},

 'Access'		=> { in  => ['+cell'],
			     out => ['value'],
			     bi  => BIaccessCell},

 'Assign'		=> { in  => ['+cell','value'],
			     out => [],
			     bi  => BIassignCell},


 ##
 ## Module: Char
 ## 

 'IsChar'	=> { in  => ['+value'],
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

 'Char.isXDigit'=> { in  => ['+char'],
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

 'Char.type'	=> { in  => ['+char'],
		     out => ['+atom'],
		     BI  => BIcharType},



 ##
 ## Module: Chunk
 ## 

 'IsChunk'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisChunkB},

 'NewChunk'		=> { in  => ['+record'],
			     out => ['+chunk'],
			     BI  => BInewChunk},


 ##
 ## Module: Class
 ## 

 'getClass'		=> { in  => ['+object'],
			     out => ['+class'],
			     bi  => BIgetClass},



 ##
 ## Module: Dictionary
 ## 

 'IsDictionary' 	=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisDictionary},

 'NewDictionary' 	=> { in  => [],
			     out => ['+dictionary'],
			     BI  => BIdictionaryNew},
 
 'Dictionary.get'	=> { in  => ['+dictionary','+feature'],
			     out => ['value'],
			     bi  => BIdictionaryGet},

 'Dictionary.condGet'   => { in  => ['+dictionary','+feature','value'],
			     out => ['value'],
			     bi  => BIdictionaryCondGet},

 'Dictionary.put'	=> { in  => ['+dictionary','+feature','value'],
			     out => [],
			     bi  => BIdictionaryPut},

 'Dictionary.remove'	=> { in  => ['+dictionary','+feature'],
			     out => [],
			     bi  => BIdictionaryRemove},
 
 'Dictionary.removeAll' => { in  => ['+dictionary'],
			     out => [],
			     BI  => BIdictionaryRemoveAll},

 'Dictionary.member'	=> { in  => ['+dictionary','+feature'],
			     out => ['+bool'],
			     bi  => BIdictionaryMember},

 'Dictionary.keys'      => { in  => ['+dictionary'],
			     out => ['+[feature]'],
			     BI  => BIdictionaryKeys},

 'Dictionary.entries'   => { in  => ['+dictionary'],
			     out => ['+[feature#value]'],
			     BI  => BIdictionaryEntries},

 'Dictionary.items'     => { in  => ['+dictionary'],
			     out => ['+[value]'],
			     BI  => BIdictionaryItems},

 'Dictionary.clone'     => { in  => ['+dictionary'],
			     out => ['+dictionary'],
			     BI  => BIdictionaryClone},

 'Dictionary.markSafe'  => { in  => ['+dictionary'],
			     out => [],
			     BI  => BIdictionaryMarkSafe},



 ##
 ## Module: Exception
 ## 

 'raise'		     => { in  => ['value'],
				  out => [],
				  BI  => BIraise,
				  doesNotReturn => 1},

 'raiseError'	             => { in  => ['value'],
				  out => [],
				  BI  => BIraiseError,
				  doesNotReturn => 1},

 'raiseDebug'	             => { in  => ['value'],
				  out => [],
				  BI  => BIraiseDebug,
				  doesNotReturn => 1},

 'Exception.raiseDebugCheck' => { in  => ['value'],
				  out => ['+bool'],
				  BI  => BIraiseDebugCheck},

 'Thread.taskStackError'     => { in  => ['+thread','+bool'],
				  out => ['+[record]'],
				  BI  => BIthreadTaskStackError},



 ##
 ## Module: Float
 ## 

 'IsFloat'	 => { in  => ['+value'],
		      out => ['+bool'],
		      bi  => BIisFloatB},

 'Exp'	         => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIexp},
 
 'Log'	         => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIlog},
 
 'Sqrt'	         => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIsqrt},
 
 'Sin'	         => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIsin},
 
 'Asin'	         => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIasin},
 
 'Cos'	         => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIcos},
 
 'Acos'	         => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIacos},
 
 'Tan'	         => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BItan},
 
 'Atan'	         => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIatan},
 
 'Ceil'	         => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIceil},
 
 'Floor'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIfloor},
 
 'Round'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIround},
 
 'Atan2'	 => { in  => ['+float','+float'],
		      out => ['+float'],
		      bi  => BIatan2},
 
 'fPow'	         => { in  => ['+float','+float'],
		      out => ['+float'],
		      bi  => BIfPow},

 'FloatToString' => { in  => ['+float'],
		      out => ['+string'],
		      BI  => BIfloatToString},


 'FloatToInt'	 => { in  => ['+float'],
		      out => ['+int'],
		      bi  => BIfloatToInt},


 ##
 ## Module: Foreign Pointer
 ## 

 'IsForeignPointer'	=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => BIisForeignPointer},

 'ForeignPointer.toInt' => { in  => ['+foreignPointer'],
			     out => ['+int'],
			     BI  => BIForeignPointerToInt},



 ##
 ## Module: Int
 ## 

 'IsInt'	=> { in  => ['+value'],
		     out => ['+bool'],
		     bi  => BIisIntB},

 'IntToFloat'	=> { in     => ['+int'],
		     out    => ['+float'],
		     bi     => BIintToFloat},

 'IntToString'	=> { in     => ['+int'],
		     out    => ['+string'],
		     BI     => BIintToString},

 'div'	        => { in  => ['+int','+int'],
		     out => ['+int'],
		     bi  => BIdiv},

 'mod'	        => { in  => ['+int','+int'],
		     out => ['+int'],
		     bi  => BImod},

 '+1'	        => { in  => ['+int'],
		     out => ['+int'],
		     bi  => BIadd1},

 '-1'	        => { in  => ['+int'],
		     out => ['+int'],
		     bi  => BIsub1},



 ##
 ## Module: Literal
 ## 

 'IsLiteral' => { in  => ['+value'],
		  out => ['+bool'],
		  bi  => BIisLiteralB},



 ##
 ## Module: Lock
 ## 

 'IsLock'	=> { in  => ['+value'],
		     out => ['+bool'],
		     bi  => BIisLockB},

 'NewLock'	=> { in  => [],
		     out => ['+lock'],
		     BI  => BInewLock},

 'Lock'		=> { in  => ['+lock'],
		     out => [],
		     BI  => BIlockLock},

 'Unlock'       => { in  => ['+lock'],
		     out => [],
		     BI  => BIunlockLock},



 ##
 ## Module: Name
 ## 

 'IsName'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisNameB},

 'NewName'		=> { in  => [],
			     out => ['+name'],
			     BI  => BInewName},

 'NewUniqueName'	=> { in  => ['+atom'],
			     out => ['+name'],
			     BI  => BInewUniqueName},


 ##
 ## Module: Number
 ## 

 'IsNumber'	=> { in  => ['+value'],
		     out => ['+bool'],
		     bi  => BIisNumberB},

 'Abs'	        => { in  => ['+number'],
		     out => ['+number'],
		     bi  => BIabs},
 
 '/'		=> { in  => ['+float','+float'],
		     out => ['+float'],
		     bi  => BIfdiv},

 '*'		=> { in  => ['+number','+number'],
		     out => ['+number'],
		     bi  => BImult},
 
 '-'		=> { in  => ['+number','+number'],
		     out => ['+number'],
		     bi  => BIminus},

 '+'		=> { in  => ['+number','+number'],
		     out => ['+number'],
		     bi  => BIplus},

 '~'		=> { in  => ['+number'],
		     out => ['+number'],
		     bi  => BIuminus},



 ##
 ## Module: Object
 ## 

 'IsObject'	=> { in  => ['+value'],
		     out => ['+bool'],
		     bi  => BIisObjectB},

 '@'		=> { in  => ['value'],
		     out => ['value'],
		     bi  => BIat},

 '<-'		=> { in  => ['value','value'],
		     out => [],
		     bi  => BIassign},

 'ooExch'	=> { in  => ['value','value'],
		     out => ['value'],
		     bi  => BIexchange},

 'copyRecord'	=> { in  => ['+record'],
		     out => ['+record'],
		     BI  => BIcopyRecord},

 'makeClass'	=> { in  => ['+dictionary','+record','+record',
			     '+dictionary','+bool','+bool'],
		     out => ['+class'],
		     BI  => BImakeClass},

 ','		=> { in  => ['+class','+record'],
		     out => [],
		     bi  => BIcomma},

 'send'		=> { in  => ['+record','+class','+object'],
		     out => [],
		     bi  => BIsend},

 'ooGetLock'	=> { in  => ['lock'],
		     out => [],
		     bi  => BIooGetLock},

 'newObject'	=> { in  => ['+class'],
		     out => ['+object'],
		     bi  => BInewObject},
 
 'New'		=> { in  => ['+class','+record','value'],
		     out => [],
		     bi  => BINew},



 ##
 ## Module: Port
 ## 

 'IsPort'	=> { in  => ['+value'],
		     out => ['+bool'],
		     bi  => BIisPortB},

 'NewPort'	=> { in  => ['value'],
		     out => ['+port'],
		     BI  => BInewPort},

 'Send'		=> { in  => ['+port','value'],
		     out => [],
		     BI  => BIsendPort},


 ##
 ## Module: Procedure
 ## 

 'IsProcedure'	  => { in  => ['+value'],
		       out => ['+bool'],
		       bi  => BIisProcedureB},

 'ProcedureArity' => { in  => ['+procedure'],
		       out => ['+int'],
		       bi  => BIprocedureArity},



 ##
 ## Module: Record
 ## 

 'IsRecord'	  => { in  => ['+value'],
		       out => ['+bool'],
		       bi  => BIisRecordB},

 'IsRecordC'	  => { in  => ['+value'],
		       out => ['+bool'],
		       bi  => BIisRecordCB},

 'Adjoin'	  => { in  => ['+record','+record'],
		       out => ['+record'],
		       bi  => BIadjoin},

 'AdjoinList'	  => { in  => ['+record','+[feature#value]'],
		       out => ['+record'],
		       BI  => BIadjoinList},

 'record'	  => { in  => ['+literal','+[feature#value]'],
		       out => ['+record'],
		       BI  => BImakeRecord},

 'Arity'	  => { in  => ['+record'],
		       out => ['+[feature]'],
		       bi  => BIarity},
 
 'AdjoinAt'	  => { in  => ['+record','+feature','value'],
		       out => ['+record'],
		       BI  => BIadjoinAt},

 'Label'	  => { in  => ['*recordC'],
		       out => ['+literal'],
		       bi  => BIlabel},

 'hasLabel'	  => { in  => ['value'],
		       out => ['+bool'],
		       bi  => BIhasLabel},
 
 'TellRecord'	  => { in  => ['+literal','record'],
		       out => [],
		       BI  => BIrecordTell},

 'WidthC'	  => { in  => ['*record','int'],
		       out => [],
		       BI  => BIwidthC},

 'monitorArity'	  => { in  => ['*recordC','value','[feature]'],
		       out => [],
		       BI  => BImonitorArity},

 'tellRecordSize' => { in  => ['+literal','+int','record'],
		       out => [],
		       BI  => BIsystemTellSize},

 '.'		  => { in  => ['*recordCOrChunk','+feature'],
		       out => ['value'],
		       bi  => BIdot},
 
 '^'		  => { in  => ['*recordCOrChunk','+feature'],
		       out => ['value'],
		       bi  => BIuparrowBlocking},
 
 'Width'	  => { in  => ['+record'],
		       out => ['+int'],
		       bi  => BIwidth},


 ##
 ## Module: Space
 ## 

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

 'Space.merge'	        => { in  => ['+space'],
			     out => ['+value'],
			     BI  => BImergeSpace},
 
 'Space.clone'	        => { in  => ['+space'],
			     out => ['+space'],
			     BI  => BIcloneSpace},
 
 'Space.commit'	        => { in  => ['+space','+value'],
			     out => [],
			     BI  => BIcommitSpace},
 
 'Space.inject'	        => { in  => ['+space','+procedure/1'],
			     out => [],
			     BI  => BIinjectSpace},



 ##
 ## Module: String
 ## 

 'IsString'		=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => BIisString},

 'StringToAtom'	        => { in  => ['+string'],
			     out => ['+atom'],
			     BI  => BIstringToAtom},

 'StringToInt'	        => { in  => ['+string'],
			     out => ['+int'],
			     BI  => BIstringToInt},

 'StringToFloat'	=> { in  => ['+string'],
			     out => ['+float'],
			     BI  => BIstringToFloat},



 ##
 ## Module: Thread
 ## 

 'Thread.is'		  => { in  => ['+value'],
			       out => ['+bool'],
			       BI  => BIthreadIs},

 'Thread.id'		  => { in  => ['+thread'],
			       out => ['+int'],
			       BI  => BIthreadID},

 'Thread.setId'	          => { in  => ['+thread','+int'],
			       out => [],
			       BI  => BIsetThreadID},

 'Thread.parentId'	  => { in  => ['+thread'],
			       out => ['+int'],
			       BI  => BIparentThreadID},

 'Thread.this'	          => { in  => [],
			       out => ['+thread'],
			       BI  => BIthreadThis},

 'Thread.suspend'	  => { in  => ['+thread'],
			       out => [],
			       BI  => BIthreadSuspend},

 'Thread.resume'	  => { in  => ['+thread'],
			       out => [],
			       BI  => BIthreadResume},

 'Thread.injectException' => { in  => ['+thread','+value'],
			       out => [],
			       BI  => BIthreadRaise},

 'Thread.preempt'	  => { in  => ['+thread'],
			       out => [],
			       BI  => BIthreadPreempt},

 'Thread.setPriority'     => { in  => ['+thread','+atom'],
			       out => [],
			       BI  => BIthreadSetPriority},

 'Thread.getPriority'     => { in  => ['+thread'],
			       out => ['+atom'],
			       BI  => BIthreadGetPriority},

 'Thread.isSuspended'     => { in  => ['+thread'],
			       out => ['+bool'],
			       BI  => BIthreadIsSuspended},

 'Thread.state'	          => { in  => ['+thread'],
			       out => ['+atom'],
			       BI  => BIthreadState},

 'Thread.setRaiseOnBlock' => { in  => ['+thread','+bool'],
			       out => [],
			       BI  => BIthreadSetRaiseOnBlock},

 'Thread.getRaiseOnBlock' => { in  => ['+thread'],
			       out => ['+bool'],
			       BI  => BIthreadGetRaiseOnBlock},

 'Thread.taskStack'	 => { in  => ['+thread','+int','+bool'],
			      out => ['+[record]'],
			      BI  => BIthreadTaskStack},

 'Thread.frameVariables' => { in  => ['+thread','+int'],
			      out => ['+record'],
			      BI  => BIthreadFrameVariables},

 'Thread.location'	 => { in  => ['+thread'],
			      out => ['+[atom]'],
			      BI  => BIthreadLocation},

 'Thread.create'         => { in  => ['+procedure'],
			      out => [],
			      BI  => BIthreadCreate},



 ##
 ## Module: Time
 ## 

 'Alarm'		=> { in  => ['+int','unit'],
			     out => [],
			     BI  => BIalarm},

 'Delay'		=> { in  => ['!+int'],
			     out => [],
			     BI  => BIdelay},

 'Time.time'		=> { in  => [],
			     out => ['+int'],
			     BI  => BItimeTime},


 ##
 ## Module: Tuple
 ## 

 'IsTuple'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisTupleB},

 'MakeTuple'		=> { in  => ['+literal','+int'],
			     out => ['+tuple'],
			     bi  => BItuple},


 ##
 ## Module: Unit
 ## 

 'IsUnit'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisUnitB},


 ##
 ## Module: Value
 ## 

 'Wait'		         => { in  => ['+value'],
			      out => [],
			      bi  => BIisValue},

 'WaitOr'		=> { in  => ['value','value'],
			     out => [],
			     BI  => BIwaitOr},

 'IsFree'		=> { in  => ['value'],
			     out => ['+bool'],
			     bi  => BIisFree},

 'IsKinded'		=> { in  => ['value'],
			     out => ['+bool'],
			     bi  => BIisKinded},

 'IsDet'		=> { in  => ['value'],
			     out => ['+bool'],
			     bi  => BIisDet},

 'Max'	                => { in  => ['+comparable','+comparable'],
			     out => ['+comparable'],
			     bi  => BImax},

  'Min'	                => { in  => ['+comparable','+comparable'],
			     out => ['+comparable'],
			     bi  => BImin},

 'HasFeature'	        => { in  => ['*recordCOrChunk','+feature'],
			     out => ['+bool'],
			     bi  => BIhasFeatureB},

 'CondSelect'	        => { in  => ['*recordCOrChunk','+feature','value'],
			     out => ['value'],
			     bi  => BImatchDefault},

 'ByNeed'		=> { in  => ['value'],
			     out => ['value'],
			     BI  => BIbyNeed},

 'Future'		=> { in  => ['value'],
			     out => ['value'],
			     BI  => BIfuture},

 '=='		        => { in  => ['*value','*value'],
			     out => ['+bool'],
			     bi  => BIeqB,
			     negated => '\\\\='},

 '\\\\='		=> { in  => ['*value','*value'],
			     out => ['+bool'],
			     bi  => BIneqB,
			     negated => '=='},

 '<'		        => { in  => ['+comparable','+comparable'],
			     out => ['+bool'],
			     bi  => BIlessFun,
			     negated => '>='},

 '=<'	                => { in  => ['+comparable','+comparable'],
			     out => ['+bool'],
			     bi  => BIleFun,
			     negated => '>'},

 '>'		        => { in  => ['+comparable','+comparable'],
			     out => ['+bool'],
			     bi  => BIgreatFun,
			     negated => '=<'},

 '>='	                => { in  => ['+comparable','+comparable'],
			     out => ['+bool'],
			     bi  => BIgeFun,
			     negated => '<'},

 '='			=> { in  => ['value','value'],
			     out => [],
			     BI  => BIunify},

 'Value.status'	        => { in  => ['value'],
			     out => ['+tuple'],
			     bi  => BIstatus},

 'Value.type'	        => { in  => ['+value'],
			     out => ['+atom'],
			     bi  => BItermType},
    


 ##
 ## Module: VirtualString
 ## 

 'IsVirtualString'	=> { in  => ['!+value'],
			     out => ['+bool'],
			     BI  => BIvsIs},

 'virtualStringLength'  => { in  => ['!virtualString','!+int'],
			     out => ['+int'],
			     BI  => BIvsLength},

 );
