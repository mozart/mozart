ifdef(`instructionsUnneededForNewCompiler',,
`define(`instructionsUnneededForNewCompiler',`')'
)

define(`incrop',`define(`OPCODE',eval(OPCODE+$1))')


define(`TOUPPER',
       `translit($1,abcdefghijklmnopqrstuvwxyz,ABCDEFGHIJKLMNOPQRSTUVWXYZ)')

define(`CacheSize',2)

dnl
dnl   Here come the instructions themselves
dnl

dnl   Pseudo-instruction to mark the end of a code segment
instruction(endOfFile)

instruction(skip)

dnl   Reg x ContAddr
dnl       x (PrintName x Arity x FileName x LineNum x Flags x NLiveRegs)
dnl       x AbstractionEntry x AssRegArray
instruction(definition,writeArg(XRegisterIndex),Label,PredId,ProcedureRef,GRegRef)
instruction(definitionCopy,writeArg(XRegisterIndex),Label,PredId,ProcedureRef,GRegRef)
instruction(endDefinition,Label)

instruction(moveXX,readArg(XRegisterIndex),writeArg(XRegisterIndex))
instruction(moveXY,readArg(XRegisterIndex),writeArg(YRegisterIndex))
instruction(moveYX,readArg(YRegisterIndex),writeArg(XRegisterIndex))
instruction(moveYY,readArg(YRegisterIndex),writeArg(YRegisterIndex))
instruction(moveGX,readArg(GRegisterIndex),writeArg(XRegisterIndex))
instruction(moveGY,readArg(GRegisterIndex),writeArg(YRegisterIndex))

instruction(moveMoveXYXY,readArg(XRegisterIndex),writeArg(YRegisterIndex),
			 readArg(XRegisterIndex),writeArg(YRegisterIndex))
instruction(moveMoveYXYX,readArg(YRegisterIndex),writeArg(XRegisterIndex),
			 readArg(YRegisterIndex),writeArg(XRegisterIndex))
instruction(moveMoveXYYX,readArg(XRegisterIndex),writeArg(YRegisterIndex),
			 readArg(YRegisterIndex),writeArg(XRegisterIndex))
instruction(moveMoveYXXY,readArg(YRegisterIndex),writeArg(XRegisterIndex),
			 readArg(XRegisterIndex),writeArg(YRegisterIndex))

instruction(createVariableX,writeArg(XRegisterIndex))
instruction(createVariableY,writeArg(YRegisterIndex))

dnl   createVariableMove(R X) == createVariable(R) move(R X)
instruction(createVariableMoveX,writeArg(XRegisterIndex),writeArg(XRegisterIndex))
instruction(createVariableMoveY,writeArg(YRegisterIndex),writeArg(XRegisterIndex))

instruction(unifyXX,readArg(XRegisterIndex),readArg(XRegisterIndex))
instruction(unifyXY,readArg(XRegisterIndex),readArg(YRegisterIndex))
instruction(unifyXG,readArg(XRegisterIndex),readArg(GRegisterIndex))


instruction(putRecordX,Literal,RecordArity,writeArg(XRegisterIndex))
instruction(putRecordY,Literal,RecordArity,writeArg(YRegisterIndex))
instruction(putListX,writeArg(XRegisterIndex))
instruction(putListY,writeArg(YRegisterIndex))
instruction(putConstantX,Constant,writeArg(XRegisterIndex))
instruction(putConstantY,Constant,writeArg(YRegisterIndex))

instruction(setVariableX,writeArg(XRegisterIndex))
instruction(setVariableY,writeArg(YRegisterIndex))
instruction(setValueX,readArg(XRegisterIndex))
instruction(setValueY,readArg(YRegisterIndex))
instruction(setValueG,readArg(GRegisterIndex))
instruction(setConstant,Constant)
instruction(setProcedureRef,ProcedureRef)
instruction(setVoid,Count)

instruction(getRecordX,Literal,RecordArity,readArg(XRegisterIndex))
instruction(getRecordY,Literal,RecordArity,readArg(YRegisterIndex))
instruction(getRecordG,Literal,RecordArity,readArg(GRegisterIndex))
instruction(getListX,readArg(XRegisterIndex))
instruction(getListY,readArg(YRegisterIndex))
instruction(getListG,readArg(GRegisterIndex))
instruction(getListValVarX,readArg(XRegisterIndex),readArg(XRegisterIndex),writeArg(XRegisterIndex))
instruction(unifyVariableX,writeArg(XRegisterIndex))
instruction(unifyVariableY,writeArg(YRegisterIndex))
instruction(unifyValueX,readArg(XRegisterIndex))
instruction(unifyValueY,readArg(YRegisterIndex))
instruction(unifyValueG,readArg(GRegisterIndex))
instruction(unifyValVarXX,readArg(XRegisterIndex),writeArg(XRegisterIndex))
instruction(unifyValVarXY,readArg(XRegisterIndex),writeArg(YRegisterIndex))
instruction(unifyValVarYX,readArg(YRegisterIndex),writeArg(XRegisterIndex))
instruction(unifyValVarYY,readArg(YRegisterIndex),writeArg(YRegisterIndex))
instruction(unifyValVarGX,readArg(GRegisterIndex),writeArg(XRegisterIndex))
instruction(unifyValVarGY,readArg(GRegisterIndex),writeArg(YRegisterIndex))
instruction(unifyNumber,Number)
instruction(unifyLiteral,Literal)
instruction(unifyVoid,Count)

instruction(getLiteralX,Literal,readArg(XRegisterIndex))
instruction(getLiteralY,Literal,readArg(YRegisterIndex))
instruction(getLiteralG,Literal,readArg(GRegisterIndex))
instruction(getNumberX,Number,readArg(XRegisterIndex))
instruction(getNumberY,Number,readArg(YRegisterIndex))
instruction(getNumberG,Number,readArg(GRegisterIndex))

instruction(allocateL,Count)
dnl   allocateL1 == allocateL(1)
instruction(allocateL1)
instruction(allocateL2)
instruction(allocateL3)
instruction(allocateL4)
instruction(allocateL5)
instruction(allocateL6)
instruction(allocateL7)
instruction(allocateL8)
instruction(allocateL9)
instruction(allocateL10)

instruction(deAllocateL)
instruction(deAllocateL1)
instruction(deAllocateL2)
instruction(deAllocateL3)
instruction(deAllocateL4)
instruction(deAllocateL5)
instruction(deAllocateL6)
instruction(deAllocateL7)
instruction(deAllocateL8)
instruction(deAllocateL9)
instruction(deAllocateL10)

dnl   NOTE: The instructions callMethod, callGlobal, call{X,Y,Z},
dnl   tailCall{X,G}, callConstant, callProcedureRef, fastCall and
dnl   fastTailCall must all have the same size due to self-modifying code.

instruction(callMethod,CallMethodInfo,Arity)

dnl   second argument is 2 * arity + (is_tailcall? 1: 0)
instruction(callGlobal,readArg(GRegisterIndex),ArityAndIsTail)
instruction(callX,readArg(XRegisterIndex),Arity)
instruction(callY,readArg(YRegisterIndex),Arity)
instruction(callG,readArg(GRegisterIndex),Arity)
instruction(tailCallX,readArg(XRegisterIndex),Arity)
instruction(tailCallG,readArg(GRegisterIndex),Arity)

dnl   first argument is a TaggedRef eventually pointing to a procedure;
dnl   second argument is 2 * arity + (is_tailcall? 1: 0)
instruction(callConstant,Constant,ArityAndIsTail)

dnl   second argument is 2 * arity + (is_tailcall? 1: 0)
instruction(callProcedureRef,ProcedureRef,ArityAndIsTail)

dnl   second argument is dummy argument
instruction(fastCall,ProcedureRef,Dummy)
instruction(fastTailCall,ProcedureRef,Dummy)

dnl   instructions for objects
instruction(sendMsgX,Literal,readArg(XRegisterIndex),RecordArity,Cache)
instruction(sendMsgY,Literal,readArg(YRegisterIndex),RecordArity,Cache)
instruction(sendMsgG,Literal,readArg(GRegisterIndex),RecordArity,Cache)
instruction(tailSendMsgX,Literal,readArg(XRegisterIndex),RecordArity,Cache)
instruction(tailSendMsgY,Literal,readArg(YRegisterIndex),RecordArity,Cache)
instruction(tailSendMsgG,Literal,readArg(GRegisterIndex),RecordArity,Cache)

instruction(getSelf,writeArg(XRegisterIndex))
instruction(setSelfG,readArg(GRegisterIndex))
instruction(lockThread,Label,readArg(XRegisterIndex))
instruction(inlineAt,Feature,writeArg(XRegisterIndex),Cache)
instruction(inlineAssign,Feature,readArg(XRegisterIndex),Cache)

instruction(branch,Label)

instruction(exHandler,Label)
instruction(popEx)

instruction(return)
instruction(getReturnX,writeArg(XRegisterIndex))
instruction(getReturnY,writeArg(YRegisterIndex))
instruction(getReturnG,writeArg(GRegisterIndex))
instruction(funReturnX,readArg(XRegisterIndex))
instruction(funReturnY,readArg(YRegisterIndex))
instruction(funReturnG,readArg(GRegisterIndex))

dnl   conditionals
instruction(testLiteralX,readArg(XRegisterIndex),Literal,Label)
instruction(testLiteralY,readArg(YRegisterIndex),Literal,Label)
instruction(testLiteralG,readArg(GRegisterIndex),Literal,Label)
instruction(testNumberX,readArg(XRegisterIndex),Number,Label)
instruction(testNumberY,readArg(YRegisterIndex),Number,Label)
instruction(testNumberG,readArg(GRegisterIndex),Number,Label)

instruction(testRecordX,readArg(XRegisterIndex),Literal,RecordArity,Label)
instruction(testRecordY,readArg(YRegisterIndex),Literal,RecordArity,Label)
instruction(testRecordG,readArg(GRegisterIndex),Literal,RecordArity,Label)
instruction(testListX,readArg(XRegisterIndex),Label)
instruction(testListY,readArg(YRegisterIndex),Label)
instruction(testListG,readArg(GRegisterIndex),Label)

instruction(testBoolX,readArg(XRegisterIndex),Label,Label)
instruction(testBoolY,readArg(YRegisterIndex),Label,Label)
instruction(testBoolG,readArg(GRegisterIndex),Label,Label)

instruction(matchX,readArg(XRegisterIndex),HashTableRef)
instruction(matchY,readArg(YRegisterIndex),HashTableRef)
instruction(matchG,readArg(GRegisterIndex),HashTableRef)

instruction(getVariableX,writeArg(XRegisterIndex))
instruction(getVariableY,writeArg(YRegisterIndex))
instruction(getVarVarXX,writeArg(XRegisterIndex),writeArg(XRegisterIndex))
instruction(getVarVarXY,writeArg(XRegisterIndex),writeArg(YRegisterIndex))
instruction(getVarVarYX,writeArg(YRegisterIndex),writeArg(XRegisterIndex))
instruction(getVarVarYY,writeArg(YRegisterIndex),writeArg(YRegisterIndex))
instruction(getVoid,Count)

dnl   code annotations for the source-level debugger

dnl   debug args: file x line x column x comment
instruction(debugEntry,Literal,Number,Number,Literal)
instruction(debugExit,Literal,Number,Number,Literal)

instruction(globalVarname,Constant)
instruction(localVarname,Constant)

instruction(clearY,writeArg(YRegisterIndex))
instruction(profileProc)

dnl   builtin applications

instruction(callBI,Builtinname,Location)
instruction(inlinePlus1,readArg(XRegisterIndex),
		        writeArg(XRegisterIndex))
instruction(inlineMinus1,readArg(XRegisterIndex),
		         writeArg(XRegisterIndex))
instruction(inlinePlus, readArg(XRegisterIndex),
			readArg(XRegisterIndex),
			writeArg(XRegisterIndex))
instruction(inlineMinus,readArg(XRegisterIndex),
			readArg(XRegisterIndex),
			writeArg(XRegisterIndex))
instruction(inlineDot,readArg(XRegisterIndex),Feature,
			writeArg(XRegisterIndex),Cache)

instruction(testBI,Builtinname,Location,Label)
instruction(testLT,readArg(XRegisterIndex),
		   readArg(XRegisterIndex),
		   writeArg(XRegisterIndex),
		   Label)
instruction(testLE,readArg(XRegisterIndex),
		   readArg(XRegisterIndex),
		   writeArg(XRegisterIndex),
		   Label)

dnl   instructions to support SML-style cartesian procedures

instruction(deconsCallX,readArg(XRegisterIndex))
instruction(deconsCallY,readArg(YRegisterIndex))
instruction(deconsCallG,readArg(GRegisterIndex))
instruction(tailDeconsCallX,readArg(XRegisterIndex))
instruction(tailDeconsCallG,readArg(GRegisterIndex))

instruction(consCallX,readArg(XRegisterIndex),Arity)
instruction(consCallY,readArg(YRegisterIndex),Arity)
instruction(consCallG,readArg(GRegisterIndex),Arity)
instruction(tailConsCallX,readArg(XRegisterIndex),Arity)
instruction(tailConsCallG,readArg(GRegisterIndex),Arity)

instructionsUnneededForNewCompiler

dnl   Instructions for efficiently interpreting different tasks on the stack;
dnl   only used internally by the emulator

instruction(taskXCont)
instruction(taskDebugCont)
instruction(taskCallCont)
instruction(taskLock)
instruction(taskSetSelf)
instruction(taskCatch)
instruction(taskEmptyStack)
instruction(taskProfileCall)

dnl '!' to move! (will require complete bootstrapping)
dnl   Pseudo-instruction to mark the end of a code chunk
instruction(endOfChunk)

divert
