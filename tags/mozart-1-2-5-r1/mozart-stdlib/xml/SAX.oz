functor
export
   Processor
   Parser
import
   NameSpaces at 'NameSpaces.ozf'
   Tokenizer  at 'Tokenizer.ozf'
prepare
   MakeBS = ByteString.make
define
   class Processor
      attr
	 GET MAP STACK TAG ProcessElem ProcessName
	 %%
      meth initFromTokenizer(T)
	 Processor,init(get:T.get)
      end
      meth initFromString(S)
	 Processor,init(string:S)
      end
      meth initFromURL(U)
	 Processor,init(url:U)
      end
      meth init(...)=Init
	 TOK = if {CondSelect Init fast false}
	       then Tokenizer.fast
	       else Tokenizer end
	 NS  = if {CondSelect Init namespaces true}
	       then NameSpaces
	       else NameSpaces.fast end
	 FEA = if {HasFeature Init string}
	       then newFromString
	       else newFromURL end
	 ARG = if {HasFeature Init string} then Init.string
	       elseif {HasFeature Init url} then Init.url
	       else Init.file end
	 MA  = if {HasFeature Init prefixMap}
	       then Init.prefixMap
	       else {NS.newPrefixMap} end
	 GG  = if {HasFeature Init get}
	       then Init.get
	       else {TOK.FEA ARG}.get end
      in
	 GET   <- GG
	 MAP   <- MA
	 STACK <- nil
	 TAG   <- unit
	 ProcessElem <- NS.processElement
	 ProcessName <- NS.processName
	 {self startDocument()}
	 Processor,PARSE()
      end
      %%
      meth startDocument()              skip end
      meth endDocument()                skip end
      meth startElement(_ _ _)          skip end
      meth endElement(_ _)              skip end
      meth characters(_ _)              skip end
      meth processingInstruction(_ _ _) skip end
      meth comment(_ _)                 skip end
      %%
      meth ERROR(R)
	 raise xml(sax:R) end
      end
      %%
      meth PARSE()
	 case {@GET}
	 of unit then
	    {self endDocument()}
	    if @TAG\=unit then
	       Processor,ERROR(nonTerminatedElement(@TAG))
	    end
	 [] stag(Name Alist Empty Coord) then Tag2 Alist2 Map2 in
	    {@ProcessElem Name Alist @MAP Tag2 Alist2 Map2}
	    {self startElement(Tag2 Alist2 Coord)}
	    if Empty then {self endElement(Tag2 Coord)}
	    else
	       STACK<-(@TAG|@MAP)|@STACK
	       TAG<-Tag2
	       MAP<-Map2
	    end
	    Parser,PARSE()
	 [] etag(Name Coord) then
	    Tag={@ProcessName Name @MAP}
	 in
	    if @TAG\=Tag then
	       Processor,ERROR(mismatchedEtag(found:Tag wanted:@TAG coord:Coord))
	    else
	       {self endElement(Tag Coord)}
	    end
	    case @STACK
	    of (Tag|Map)|Stack then
	       TAG<-Tag MAP<-Map STACK<-Stack
	       Processor,PARSE()
	    else Processor,ERROR(unexpectedErrorAtEtag(Tag Coord)) end
	 [] pi(Target Args Coord) then
	    {self processingInstruction(Target Args Coord)}
	    Processor,PARSE()
	 [] text(Chars Coord) then
	    {self characters(Chars Coord)}
	    Processor,PARSE()
	 [] comment(Chars Coord) then
	    {self comment(Chars Coord)}
	    Processor,PARSE()
	 end
      end
   end
   %%
   class Parser from Processor
      feat root
      attr
	 STACK    : nil
	 CONTENTS : nil
	 PARENT   : unit

      meth getParent($) @PARENT end

      meth makeRoot(L $)
	 root(L)
      end
      meth startDocument() L in
	 CONTENTS <- L
	 self.root = {self makeRoot(L $)}
	 PARENT <- self.root
      end
      meth endDocument() @CONTENTS=nil end

      meth makeElement(Tag Alist Contents Coord $)
	 element(
	    tag      : Tag
	    alist    : Alist
	    contents : Contents
	    coord    : Coord
	    )
      end
      meth makeAttribute(Key Val $)
	 attribute(
	    name  : Key
	    value : Val)
      end
      meth makeAlist(Alist $)
	 {Map Alist
	  fun {$ Key|Val}
	     {self makeAttribute(Key Val $)}
	  end}
      end
      meth startElement(Tag Alist Coord)
	 Alist2 Contents2
	 H = {self makeElement(Tag Alist2 Contents2 Coord $)}
	 T
      in
	 @CONTENTS=H|T
	 CONTENTS<-Contents2
	 STACK<-(T|@PARENT)|@STACK
	 PARENT<-H
	 Alist2 = {self makeAlist(Alist $)}
      end
      meth endElement(_ _)
	 @CONTENTS=nil
	 case @STACK of (T|P)|L then
	    STACK<-L
	    CONTENTS<-T
	    PARENT<-P
	 end
      end

      meth makeText(Chars Coord $)
	 text({MakeBS Chars} Coord)
      end
      meth characters(Chars Coord) L in
	 @CONTENTS = {self makeText(Chars Coord $)}|L
	 CONTENTS<-L
      end

      meth makePI(Target Data Coord $)
	 pi({StringToAtom Target}
	    {StringToAtom Data} Coord)
      end
      meth processingInstruction(Target Data Coord) L in
	 @CONTENTS = {self makePI(Target Data Coord $)}|L
	 CONTENTS<-L
      end

      meth makeComment(Chars Coord $)
	 comment({MakeBS Chars} Coord)
      end
      meth comment(Chars Coord) L in
	 @CONTENTS = {self makeComment(Chars Coord $)}|L
	 CONTENTS<-L
      end
   end
end
