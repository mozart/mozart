functor
import
   Open(file)
export
   GetChunk ProcessSpecs
define
   class OzDocToCode
      attr Reporter Defs Refs
      meth init(MyReporter)
	 Reporter <- MyReporter
      end
      meth processNode(Node)
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
	    %{@Reporter startPhase('looking up target chunk')}
	    %OzDocToCode,get(Title Chunk)
	    if {@Reporter hasSeenError($)} then
	       {@Reporter endBatch(rejected)}
	    else
	       {@Reporter endBatch(accepted)}
	    end
	    {@Reporter tell(done())}
	 end
      end
      meth getChunk(Node Title Chunk)
	 OzDocToCode,processNode(Node)
	 OzDocToCode,getOneChunk(Title Chunk)
      end
      meth get(Title $)
	 {Dictionary.get @Defs {VirtualString.toAtom Title}}
      end
      meth getOneChunk(Title Chunk)
	 {@Reporter startPhase('getting chunk: <'#Title#'>')}
	 OzDocToCode,get(Title Chunk)
      end
      meth processSpec(Spec Sep)
	 Title File Code IndentedCode F
      in
	 {String.token Spec Sep Title File}
	 OzDocToCode,getOneChunk(Title Code)
	 {@Reporter startSubPhase('writing into file: '#File)}
	 {New Indentor init(Code IndentedCode) _}
	 F = {New Open.file init(name:File flags:[create write truncate])}
	 {F write(vs:IndentedCode)}
	 {F close}
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
	 Prev  = {Dictionary.condGet @Defs Key nil}
      in
	 {Dictionary.put @Defs Key
	  if Prev==nil then Code else Prev#("\n"#Code) end}
      end
      meth BatchCode(Node I $)
	 if {HasFeature Node I} then M=Node.I in
	    if {Label M}=='chunk.ref' then
	       Title = OzDocToCode,Content(M $)
	       Key   = {VirtualString.toAtom Title}
	    in
	       chunk(OzDocToCode,Get(Key $))
	    else
	       case M of _|_ then M
	       elsecase M of nil then nil
	       end
	    end
	    # OzDocToCode,BatchCode(Node I+1 $)
	 else nil end
      end
   end
   %%
   proc {GetChunk Reporter Node Title IndentedCode}
      O = {New OzDocToCode init(Reporter)}
      Code
   in
      {O getChunk(Node Title Code)}
      {New Indentor init(Code IndentedCode) _}
   end
   %%
   proc {ProcessSpecs Reporter Node Specs Sep}
      O = {New OzDocToCode init(Reporter)}
   in
      {O processNode(Node)}
      {ForAll Specs
       proc {$ Spec}
	  {O processSpec(Spec Sep)}
       end}
   end
   %%
   class Indentor
      attr head tail column margin flushed innercolumn
      meth init(Code Result)
	 head<-Result tail<-Result
	 column<-0
	 margin<-0
	 flushed<-false
	 innercolumn<-0
	 Indentor,entercode(Code)
	 @tail=nil
      end
      meth PUTC(C) L in
	 @tail=C|L
	 tail<-L
      end
      meth PUTMARGIN(N)
	 if N==0 then skip else
	    Indentor,PUTC(& )
	    Indentor,PUTMARGIN(N-1)
	 end
      end
      meth PUTTAB
	 Indentor,PUTC(& )
	 column<-@column+1
	 innercolumn<-@innercolumn+1
	 if @innercolumn mod 8 \= 0 then
	    Indentor,PUTTAB
	 end
      end      
      meth putc(C)
	 if C==&\n then
	    column<-@margin flushed<-false
	    innercolumn<-0
	    Indentor,PUTC(C)
	 else
	    if @flushed then skip
	    else
	       Indentor,PUTMARGIN(@margin)
	       flushed<-true
	    end
	    if C==&\t then
	       Indentor,PUTTAB
	    else
	       column<-@column+1
	       Indentor,PUTC(C)
	    end
	 end
      end
      meth entercode(Code)
	 case Code of X#Y then
	    Indentor,entercode(X)
	    Indentor,entercode(Y)
	 elseof chunk(Code) then
	    Margin = @margin
	    Inner  = @innercolumn
	 in
	    margin<-if @flushed then @column else @margin end
	    innercolumn<-0
	    Indentor,entercode(Code)
	    margin<-Margin
	    innercolumn<-Inner
	 elseof nil then skip
	 else
	    Indentor,enterstring(Code)
	 end
      end
      meth enterstring(Str)
	 case Str of H|T then
	    Indentor,putc(H)
	    Indentor,enterstring(T)
	 elseof nil then skip end
      end
   end
end
