ifdef(`instructionsUnneededForNewCompiler',,
`define(`instructionsUnneededForNewCompiler',`')'
)

define(`isReg',`ifelse($1,Register,1,0)')
define(`numOfRegs',`eval(isReg($1)+isReg($2)+isReg($3))')

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
instruction(definition,writeArg(XRegisterIndex),Label,PredId,PredicateRef,GRegRef)
instruction(definitionCopy,writeArg(XRegisterIndex),Label,PredId,PredicateRef,GRegRef)
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
instruction(setValue,readArg(Register))
instruction(setConstant,Constant)
instruction(setPredicateRef,PredicateRef)
instruction(setProcedureRef,PredicateRef)
instruction(setVoid,Count)

instruction(getRecord,Literal,RecordArity,readArg(Register))
instruction(getList,readArg(Register))
instruction(getListValVarX,readArg(XRegisterIndex),readArg(XRegisterIndex),writeArg(XRegisterIndex))
instruction(unifyVariableX,writeArg(XRegisterIndex))
instruction(unifyVariableY,writeArg(YRegisterIndex))
instruction(unifyValue,readArg(Register))
instruction(unifyValVarXX,readArg(XRegisterIndex),writeArg(XRegisterIndex))
instruction(unifyValVarXY,readArg(XRegisterIndex),writeArg(YRegisterIndex))
instruction(unifyValVarYX,readArg(YRegisterIndex),writeArg(XRegisterIndex))
instruction(unifyValVarYY,readArg(YRegisterIndex),writeArg(YRegisterIndex))
instruction(unifyValVarGX,readArg(GRegisterIndex),writeArg(XRegisterIndex))
instruction(unifyValVarGY,readArg(GRegisterIndex),writeArg(YRegisterIndex))
instruction(unifyNumber,Number)
instruction(unifyLiteral,Literal)
instruction(unifyVoid,Count)

instruction(getLiteral,Literal,readArg(Register))
instruction(getNumber,Number,readArg(Register))

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

dnl   NOTE: The instructions genCall, call, tailCall, marshalledFastCall,
dnl   genFastCall, fastCall and fastTailCall must all have the same size
dnl   due to self-modifying code.

instruction(callMethod,GenCallInfo,Arity)
instruction(genCall,GenCallInfo,Arity)
instruction(callGlobal,readArg(GRegisterIndex),ArityAndIsTail)
instruction(call,readArg(Register),Arity)
instruction(tailCallX,readArg(XRegisterIndex),Arity)
instruction(tailCallG,readArg(GRegisterIndex),Arity)

dnl   first argument is a TaggedRef pointing to a procedure proxy;
dnl   second argument is 2 * arity + (is_tailcall? 1: 0)
instruction(callConstant,Constant,ArityAndIsTail)
instruction(marshalledFastCall,Constant,ArityAndIsTail)

dnl   second argument is a flag: non-zero iff tailcall
instruction(callProcedureRef,PredicateRef,ArityAndIsTail)
instruction(genFastCall,PredicateRef,ArityAndIsTail)

dnl   second argument is dummy argument
instruction(fastCall,PredicateRef,Dummy)
instruction(fastTailCall,PredicateRef,Dummy)

dnl   instructions for objects
instruction(sendMsg,Literal,readArg(Register),RecordArity,Cache)
instruction(tailSendMsg,Literal,readArg(Register),RecordArity,Cache)

instruction(getSelf,writeArg(XRegisterIndex))
instruction(setSelf,readArg(XRegisterIndex))
instruction(setSelfG,readArg(GRegisterIndex))
instruction(lockThread,Label,readArg(XRegisterIndex))
instruction(inlineAt,Feature,writeArg(XRegisterIndex),Cache)
instruction(inlineAssign,Feature,readArg(XRegisterIndex),Cache)

instruction(branch,Label)

instruction(exHandler,Label)
instruction(popEx)

instruction(return)
instruction(getReturn,writeArg(Register))
instruction(funReturn,readArg(Register))

dnl   conditionals
instruction(testLiteral,readArg(Register),Literal,Label)
instruction(testNumber,readArg(Register),Number,Label)

instruction(testRecord,readArg(Register),Literal,RecordArity,Label)
instruction(testList,readArg(Register),Label)

instruction(testBool,readArg(Register),Label,Label)

instruction(match,readArg(Register),HashTableRef)
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

dnl   dummy instructions to allow easy testing of new
dnl   instructions via assembler

instructionsUnneededForNewCompiler

dnl   Instruction for efficiently interpreting different tasks on the stack;
dnl   only used internally by the emulator

instruction(taskXCont)
instruction(taskCFunCont)
instruction(taskDebugCont)
instruction(taskCallCont)
instruction(taskLock)
instruction(taskSetSelf)
instruction(taskCatch)
instruction(taskEmptyStack)
instruction(taskProfileCall)

divert
