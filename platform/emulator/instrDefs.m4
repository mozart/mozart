ifdef(`instructionsUnneededForNewCompiler',,
`define(`instructionsUnneededForNewCompiler',`')'
)

define(isReg,`ifelse($1,Register,1,0)')
define(numOfRegs,`eval(isReg($1)+isReg($2)+isReg($3))')

undefine(`incrop')
define(incrop,`define(`OPCODE',eval(OPCODE+$1))')


define(`TOUPPER',
       `translit($1,abcdefghijklmnopqrstuvwxyz,ABCDEFGHIJKLMNOPQRSTUVWXYZ)')

define(CacheSize,2)

dnl
dnl   Here come the instructions themselves
dnl

dnl   Pseudo-instruction to mark the end of a code segment
instruction(endOfFile)

instruction(skip)
instruction(failure)

dnl   Reg x ContAddr x (PrintName x Arity x FileName x LineNum)
dnl       x AbstractionEntry x AssRegArray
instruction(definition,writeArg(XRegisterIndex),Label,PredId,PredicateRef,GRegRef)
instruction(definitionCopy,writeArg(XRegisterIndex),Label,PredId,PredicateRef,GRegRef)
instruction(endDefinition,Label)

instruction(move,readArg(Register),writeArg(Register))

instruction(moveMoveXYXY,readArg(XRegisterIndex),writeArg(YRegisterIndex),
                         readArg(XRegisterIndex),writeArg(YRegisterIndex))
instruction(moveMoveYXYX,readArg(YRegisterIndex),writeArg(XRegisterIndex),
                         readArg(YRegisterIndex),writeArg(XRegisterIndex))
instruction(moveMoveXYYX,readArg(XRegisterIndex),writeArg(YRegisterIndex),
                         readArg(YRegisterIndex),writeArg(XRegisterIndex))
instruction(moveMoveYXXY,readArg(YRegisterIndex),writeArg(XRegisterIndex),
                         readArg(XRegisterIndex),writeArg(YRegisterIndex))

instruction(createVariable,writeArg(Register))

dnl   createVariableMove(R X) == createVariable(R) move(R X)
instruction(createVariableMove,writeArg(Register),writeArg(XRegisterIndex))

instruction(unify,readArg(Register),readArg(Register))

instruction(putRecord,Literal,RecordArity,writeArg(Register))
instruction(putList,writeArg(Register))
instruction(putConstant,Constant,writeArg(Register))

instruction(setVariable,writeArg(Register))
instruction(setValue,readArg(Register))
instruction(setConstant,Constant)
instruction(setPredicateRef,PredicateRef)
instruction(setVoid,Count)

instruction(getRecord,Literal,RecordArity,readArg(Register))
instruction(getList,readArg(Register))
instruction(getListValVar,readArg(XRegisterIndex),readArg(Register),writeArg(XRegisterIndex))
instruction(unifyVariable,writeArg(Register))
instruction(unifyValue,readArg(Register))
instruction(unifyValVar,readArg(Register),writeArg(Register))
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

instruction(genCall,GenCallInfo,Arity)
instruction(call,readArg(Register),Arity)
instruction(tailCall,readArg(Register),Arity)

dnl   first argument is a TaggedRef pointing to a procedure proxy;
dnl   second argument is 2 * arity + (is_tailcall? 1: 0)
instruction(marshalledFastCall,Constant,ArityAndIsTail)

dnl   second argument is a flag: non-zero iff tailcall
instruction(genFastCall,PredicateRef,IsTail)

dnl   second argument is dummy argument
instruction(fastCall,PredicateRef,Dummy)
instruction(fastTailCall,PredicateRef,Dummy)

dnl   instructions for objects
instruction(sendMsg,Literal,readArg(Register),RecordArity,Cache)
instruction(tailSendMsg,Literal,readArg(Register),RecordArity,Cache)
dnl   ApplMethInfo = MethName x Arity
instruction(applMeth,ApplMethInfo,readArg(Register))
instruction(tailApplMeth,ApplMethInfo,readArg(Register))

instruction(getSelf,readArg(XRegisterIndex))
instruction(lockThread,Label,readArg(XRegisterIndex),NLiveRegs)
instruction(inlineAt,Feature,writeArg(XRegisterIndex),NLiveRegs,Cache)
instruction(inlineAssign,Feature,readArg(XRegisterIndex),NLiveRegs,Cache)

instruction(branch,Label)

instruction(wait)
instruction(waitTop)
instruction(ask)

instruction(createCond,Label,NLiveRegs)
instruction(createOr)
instruction(createEnumOr)
instruction(createChoice)
instruction(clause)
instruction(emptyClause)

instruction(thread,Label)
instruction(threadX,Arity,Label)

instruction(exHandler,Label)
instruction(popEx)

instruction(return)

instruction(nextClause,Label)
instruction(lastClause)

dnl   conditionals
instruction(shallowGuard,Label,NLiveRegs)
instruction(shallowThen)

instruction(testLiteral,readArg(Register),Literal,Label,Label,NLiveRegs)
instruction(testNumber,readArg(Register),Number,Label,Label,NLiveRegs)

instruction(testRecord,readArg(Register),Literal,RecordArity,Label,Label,NLiveRegs)
instruction(testList,readArg(Register),Label,Label,NLiveRegs)

instruction(testBool,readArg(Register),Label,Label,Label,NLiveRegs)

instruction(match,readArg(Register),HashTableRef,NLiveRegs)
instruction(getVariable,writeArg(Register))
instruction(getVarVar,writeArg(Register),writeArg(Register))
instruction(getVoid,Count)

dnl   code annotations for the source-level debugger

dnl   debug args: file x line x column x comment x nliveregs
instruction(debugEntry,Literal,Number,Number,Literal,NLiveRegs)
instruction(debugExit,Literal,Number,Number,Literal,NLiveRegs)

instruction(globalVarname,Variablename)
instruction(localVarname,Variablename)

instruction(clearY,writeArg(YRegisterIndex))
instruction(profileProc)

dnl   builtin applications

instruction(callBI,Builtinname,Location,NLiveRegs)
instruction(inlinePlus1,readArg(XRegisterIndex),
                        writeArg(XRegisterIndex),NLiveRegs)
instruction(inlineMinus1,readArg(XRegisterIndex),
                         writeArg(XRegisterIndex),NLiveRegs)
instruction(inlinePlus, readArg(XRegisterIndex),
                        readArg(XRegisterIndex),
                        writeArg(XRegisterIndex),NLiveRegs)
instruction(inlineMinus,readArg(XRegisterIndex),
                        readArg(XRegisterIndex),
                        writeArg(XRegisterIndex),NLiveRegs)
instruction(inlineDot,readArg(XRegisterIndex),Feature,
                        writeArg(XRegisterIndex),NLiveRegs,Cache)
instruction(inlineUparrow,readArg(XRegisterIndex),readArg(XRegisterIndex),
                        writeArg(XRegisterIndex),NLiveRegs)

instruction(testBI,Builtinname,Location,Label,NLiveRegs)
instruction(testLT,readArg(XRegisterIndex),
                   readArg(XRegisterIndex),
                   writeArg(XRegisterIndex),
                   Label,NLiveRegs)
instruction(testLE,readArg(XRegisterIndex),
                   readArg(XRegisterIndex),
                   writeArg(XRegisterIndex),
                   Label,NLiveRegs)

dnl   dummy instructions to allow easy testing of new
dnl   instructions via assembler

instructionsUnneededForNewCompiler


instruction(test1,Dummy,Dummy,Dummy)
instruction(test2,Dummy,Dummy,Dummy)
instruction(test3,Dummy,Dummy,Dummy)
instruction(test4,Dummy,Dummy,Dummy)
instruction(testLabel1,Label,Dummy,Dummy)
instruction(testLabel2,Label,Dummy,Dummy)
instruction(testLabel3,Label,Dummy,Dummy)
instruction(testLabel4,Label,Dummy,Dummy)

dnl   Instruction for efficiently interpreting different tasks on the stack;
dnl   only used internally by the emulator

instruction(taskXCont)
instruction(taskCFunCont)
instruction(taskDebugCont)
instruction(taskCallCont)
instruction(taskLock)
instruction(taskSetSelf)
instruction(taskLTQ)
instruction(taskCatch)
instruction(taskEmptyStack)
instruction(taskProfileCall)
instruction(taskActor)

divert
