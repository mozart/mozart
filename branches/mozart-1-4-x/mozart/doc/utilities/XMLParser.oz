functor
export
   'class' : XMLParser
import
   TOK('class':TOKC) at 'XMLTokenizer.ozf'
prepare
   fun {ProcessAlist L}
      {Map L ProcessAttrib}
   end
   fun {ProcessAttrib A}
      case A
      of K|V then
	 attribute(
	    name  : {StringToAtom K}
	    kind  : unit
	    value : V)
      [] K#V then
	 attribute(
	    name  : {StringToAtom K}
	    kind  : unit
	    value : V)
      end
   end
   MakeBytes = ByteString.make
   fun {IsElement X}
      case X
      of element(tag:_ attributes:_ linkAttributes:_ children:_) then true
      else false end
   end
define
   class XMLParser
      attr
	 Tokenizer
	 Stack   : nil
	 Content : nil
	 Root
      meth init(F)
	 Tokenizer <- {New TOKC init(F)}
	 local
	    L
	 in
	    Root <- root(L)
	    Content <- L
	 end
      end
      meth getDoc(Doc)
	 case {Filter @Root.1 IsElement}
	 of [R] then
	    Doc = document(docElem:R emap:unit)
	 else
	    XMLParser,error(expectedExactlyOneTopElement)
	 end
      end
      meth error(R)
	 {Exception.raiseError xmlparser(R)}
      end
      meth process()
	 case {@Tokenizer scanToken($)}
	 of unit       then
	    case @Stack
	    of nil then @Content=nil
	    [] (T|_)|_ then
	       XMLParser,error(missingEndTag(T))
	    end
	 [] doctype    then XMLParser,process()
	 [] etag(S)    then
	    @Content = nil
	    case @Stack
	    of (T|C)|L then A={StringToAtom S} in
	       if A==T then
		  Content <- C
		  Stack   <- L
	       else
		  XMLParser,error(mismatchedEtag(T A))
	       end
	    end
	    XMLParser,process()
	 [] pi(S)      then L in
	    @Content = pi({StringToAtom S})|L
	    Content <- L
	    XMLParser,process()
	 [] comment(_) then XMLParser,process()
	 [] stag(Tag Alist Empty) then
	    A = {StringToAtom Tag}
	    Children
	    Elem = element(
		      tag            : A
		      attributes     : {ProcessAlist Alist}
		      linkAttributes : nil
		      children       : Children)
	    NewContent
	 in
	    @Content=(Elem|NewContent)
	    if Empty then
	       Children=nil
	       Content <- NewContent
	    else
	       Stack <- (A|NewContent)|@Stack
	       Content <- Children
	    end
	    XMLParser,process()
	 [] text(S)    then L in
	    @Content = data({MakeBytes S})|L
	    Content <- L
	    XMLParser,process()
	 [] Tok        then
	    XMLParser,error(unexpectedToken(Tok))
	 end
      end
   end
end
