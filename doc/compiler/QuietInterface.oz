class Compiler.quietInterface from Compiler.genericInterface
   prop final
   meth init(CompilerObject DoVerbose <= false)
   meth reset()
   meth setVerbosity(B)
   meth hasErrors(?B)
   meth hasBeenTopped(?B)
   meth getVS(?VS)
   meth getMessages(?Ms)
   meth formatMessages(Ms ?VS)
   meth getInsertedFiles(?VSs)
   meth getSource(?VS)
end
