functor
import
   Narrator SGML ErrorListener
export
   GetChunk
define
   class OzDocToCode from Narrator.'class'
      attr Reporter Defs Refs
      meth init
	 Reporter <- Narrator.'class',init($)
	 {@Reporter setLogPhases(true)}
      end
      meth getChunk(File Title Chunk)
	 {@Reporter startBatch()}
	 {@Reporter startPhase('parsing SGML input')}
	 Node = {SGML.parse File @Reporter}
      in
	 if {@Reporter hasSeenError($)} then skip else
	    Defs <- {Dictionary.new}
	    Refs <- {Dictionary.new}
	    {@Reporter startPhase('processing chunks')}
	    OzDocToCode,Process(Node)
	    %% instantiate all refs
	    {@Reporter startPhase('resolving chunk references')}
	    {ForAll {Dictionary.entries @Refs}
	     proc {$ Key#Code}
		{Dictionary.get @Defs Key Code}
	     end}
	    {@Reporter startPhase('looking up target chunk')}
	    OzDocToCode,get(Title Chunk)
	    if {@Reporter hasSeenError($)} then
	       {@Reporter endBatch(rejected)}
	    else
	       {@Reporter endBatch(accepted)}
	    end
	    {@Reporter tell(done())}
	 end
      end
      meth get(Title $)
	 {Dictionary.get @Defs {VirtualString.toAtom Title}}
      end
      %%
      %% obtain a (uninstantiated) reference to a code chunk
      %%
      meth Get(Key $)
	 if {Dictionary.member @Refs Key} then
	    {Dictionary.get @Refs Key}
	 else X in
	    {Dictionary.put @Refs Key X}
	    X
	 end
      end
      %%
      %% extract textual data from tree
      %%
      meth Content(Node $)
	 case Node of _|_ then Node
	 elseof nil then nil
	 elseof pi(S) then '\&'#S#';'
	 else OzDocToCode,BatchContent(Node 1 $) end
      end
      meth BatchContent(Node I $)
	 if {HasFeature Node I} then
	    OzDocToCode,Content(Node.I $) #
	    OzDocToCode,BatchContent(Node I+1 $)
	 else nil end
      end
      %%
      %% process all chunks
      %%
      meth Process(Node)
	 case Node of _|_ then skip
	 elseof pi(_) then skip
	 elsecase {Label Node} of 'chunk' then
	    OzDocToCode,ProcessChunk(Node)
	 else
	    OzDocToCode,BatchProcess(Node 1)
	 end
      end
      meth BatchProcess(Node I)
	 if {HasFeature Node I} then
	    OzDocToCode,Process(Node.I)
	    OzDocToCode,BatchProcess(Node I+1)
	 end
      end
      meth ProcessChunk(Node)
	 Title = OzDocToCode,Content(Node.1 $)
	 Code  = OzDocToCode,BatchCode(Node.2 1 $)
	 Key   = {VirtualString.toAtom Title}
      in
	 {Dictionary.put @Defs Key
	  {Dictionary.condGet @Defs Key nil}#Code}
      end
      meth BatchCode(Node I $)
	 if {HasFeature Node I} then M=Node.I in
	    if {Label M}=='chunk.ref' then
	       Title = OzDocToCode,Content(M $)
	       Key   = {VirtualString.toAtom Title}
	    in
	       OzDocToCode,Get(Key $)
	    elsecase M of _|_ then M
	    elsecase M of nil then nil end
	    # OzDocToCode,BatchCode(Node I+1 $)
	 else nil end
      end
   end
   %%
   class MyListener from ErrorListener.'class'
      attr Sync: unit
      meth init(O X)
	 Sync <- X
	 ErrorListener.'class', init(O ServeOne true)
      end
      meth ServeOne(M)
	 case M of done() then @Sync = unit
	 else skip
	 end
      end
   end
   %%
   proc {GetChunk File Title Code}
      O = {New OzDocToCode init}
      Sync
      L = {New MyListener init(O Sync)}
   in
      {O getChunk(File Title Code)}
      {Wait Sync}
      if {L hasErrors($)} then raise error end end
   end
end
