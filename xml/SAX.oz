functor
export
   Processor
   Parser
import
   NameSpaces at 'NameSpaces.ozf'
   Tokenizer  at 'Tokenizer.ozf'
prepare
   MakeBS = ByteString.make
   CharIsSpace = Char.isSpace
   fun {AttrToAtom Name|Value}
      {StringToAtom Name}|{StringToAtom Value}
   end
define
   class Processor
      attr
	 GET MAP STACK TAG
	 %%
      meth INIT(Get)
	 GET   <- Get
	 MAP   <- {NameSpaces.newNameSpacePrefixMap}
	 STACK <- nil
	 TAG   <- unit
      end
      meth initFromTokenizer(T)
	 Processor,INIT(T.get)
	 {self startDocument()}
	 Processor,PARSE()
      end
      meth initFromString(S)
	 Processor,initFromTokenizer({Tokenizer.newFromString S})
      end
      meth initFromURL(U)
	 Processor,initFromTokenizer({Tokenizer.newFromURL U})
      end
      meth initLazyFromURL(U)
	 Processor,initFromTokenizer({Tokenizer.newLazyFromURL U})
      end
      %%
      meth startDocument()            skip end
      meth endDocument()              skip end
      meth startElement(_ _)          skip end
      meth endElement(_)              skip end
      meth characters(_)              skip end
      meth processingInstruction(_ _) skip end
      meth comment(_)                 skip end
      %%
      meth PARSE()
	 case {@GET}
	 of unit then
	    {self endDocument()}
	    if @TAG\=unit then
	       {Exception.raiseError
		xml(parse(nonTerminatedElement @TAG))}
	    end
	 [] stag(Name Alist Empty) then Tag2 Alist2 Map2 in
	    {NameSpaces.processElement
	     {StringToAtom Name}
	     {Map Alist AttrToAtom}
	     @MAP
	     Tag2 Alist2 Map2}
	    {self startElement(Tag2 Alist2)}
	    if Empty then {self endElement(Tag2)}
	    else
	       STACK<-(@TAG|@MAP)|@STACK
	       TAG<-Tag2
	       MAP<-Map2
	    end
	    Parser,PARSE()
	 [] etag(Name) then
	    Tag={NameSpaces.processName {StringToAtom Name} @MAP}
	 in
	    if @TAG\=Tag then
	       {Exception.raiseError
		xml(parse(mismatchedEtag(found:Tag wanted:@TAG)))}
	    else
	       {self endElement(Tag)}
	    end
	    case @STACK
	    of (Tag|Map)|Stack then
	       TAG<-Tag MAP<-Map STACK<-Stack
	       Processor,PARSE()
	    else {Exception.raiseError
		  xml(parse(unexpectedErrorAtEtag Tag))}
	    end
	 [] pi(Target Args) then
	    {self processingInstruction(Target Args)}
	    Processor,PARSE()
	 [] text(Chars) then
	    {self characters(Chars)}
	    Processor,PARSE()
	 [] comment(Chars) then
	    {self comment(Chars)}
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
	 N        : 1
	 PARENT   : unit

      meth makeRoot(L $)
	 root(contents : L
	      index    : 0
	      parent   : unit)
      end
      meth startDocument() L in
	 CONTENTS <- L
	 self.root = {self makeRoot(L $)}
	 PARENT <- self.root
      end
      meth endDocument() @CONTENTS=nil end

      meth makeElement(Tag Alist Contents $) I=@N in
	 N<-I+1
	 element(
	    tag      : Tag
	    alist    : Alist
	    index    : I
	    contents : Contents
	    parent   : @PARENT)
      end
      meth makeAttribute(Key Val $) I=@N in
	 N<-I+1
	 attribute(
	    name  : Key
	    value : Val
	    index : I
	    parent: @PARENT)
      end
      meth makeAlist(Alist $)
	 {Map Alist
	  fun {$ Key|Val}
	     {self makeAttribute(Key Val $)}
	  end}
      end
      meth startElement(Tag Alist)
	 Alist2 Contents2
	 H = {self makeElement(Tag Alist2 Contents2 $)}
	 T
      in
	 @CONTENTS=H|T
	 CONTENTS<-Contents2
	 STACK<-(T|@PARENT)|@STACK
	 PARENT<-H
	 Alist2 = {self makeAlist(Alist $)}
      end
      meth endElement(_)
	 @CONTENTS=nil
	 case @STACK of (T|P)|L then
	    STACK<-L
	    CONTENTS<-T
	    PARENT<-P
	 end
      end

      meth makeText(Chars $) I=@N in
	 N<-I+1
	 text({MakeBS Chars} index:I parent:@PARENT
	      whitespace : {All Chars CharIsSpace})
      end
      meth characters(Chars) L in
	 @CONTENTS = {self makeText(Chars $)}|L
	 CONTENTS<-L
      end

      meth makePI(Target Data $) I=@N in
	 N<-I+1
	 pi({StringToAtom Target}
	    {StringToAtom Data}
	    index:I parent:@PARENT)
      end
      meth processingInstruction(Target Data) L in
	 @CONTENTS = {self makePI(Target Data $)}|L
	 CONTENTS<-L
      end

      meth makeComment(Chars $) I=@N in
	 N<-I+1
	 comment({MakeBS Chars} index:I parent:@PARENT)
      end
      meth comment(Chars) L in
	 @CONTENTS = {self makeComment(Chars $)}|L
	 CONTENTS<-L
      end
   end
end
