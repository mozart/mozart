functor
export
   'class' : MessageParser
prepare
   IsSpace = Char.isSpace

   fun {DropInitialSpaces L}
      case L
      of H|T andthen (H==&  orelse H==&\t) then
	 {DropInitialSpaces T}
      else L end
   end

   class MessageParser
      attr
	 AcceptCRLF : false
	 AcceptLF   : false
	 AcceptCR   : false

      meth acceptCRLF(B) AcceptCRLF<-B end
      meth acceptLF(  B) AcceptLF  <-B end
      meth acceptCR(  B) AcceptCR  <-B end

      meth messageStart() skip end
      meth messageEnd()   skip end
      meth messageHeader(Header Value) skip end
      meth messageBody()  skip end

      meth parse(L)
	 {self messageStart}
	 MessageParser,MaybeHeader(L)
	 {self messageEnd}
      end
      
      meth EOL(L $)
	 case L
	 of &\r|&\n|L andthen @AcceptCRLF then L
	 [] &\r    |L andthen @AcceptCR   then L
	 []     &\n|L andthen @AcceptLF   then L
	 else unit end
      end

      meth MaybeHeader(L)
	 if L==nil then skip
	 elsecase MessageParser,EOL(L $)
	 of unit then H in MessageParser,Header(L H H)
	 [] L then {self messageBody(L)} end
      end

      meth Header(L Head Tail)
	 case L
	 of H|T then
	    if H==&: then V in
	       Tail=nil
	       MessageParser,AfterSemiColon(
				{DropInitialSpaces T}
				Head V V)
	    elseif {IsSpace H} then
	       Tail=nil
	       MessageParser,AfterHeader(T Head)
	    else Tail2 in
	       Tail=(H|Tail2)
	       MessageParser,Header(T Head Tail2)
	    end
	 else
	    raise messageParser(unexpectedEndOfMessage) end
	 end
      end

      meth AfterHeader(L Header)
	 case L
	 of H|T then
	    if H==&: then H in
	       MessageParser,AfterSemiColon(
				{DropInitialSpaces T} Header H H)
	    elseif {IsSpace H} then
	       MessageParser,AfterHeader(T Header)
	    else
	       raise messageParser(expectedSpaceOrSemiColon:L) end
	    end
	 else
	    raise messageParser(unexpectedEndOfMessage) end
	 end
      end

      meth AfterSemiColon(L Header Value Tail)
	 case MessageParser,EOL(L $)
	 of unit then
	    case L
	    of H|T then Tail2 in
	       Tail=(H|Tail2)
	       MessageParser,AfterSemiColon(T Header Value Tail2)
	    else
	       Tail=nil
	       {self messageHeader(Header Value)}
	    end
	 [] L then
	    case L
	    of H|T andthen (H==&  orelse H==&\t) then Tail2 in
	       Tail=(H|Tail2)
	       MessageParser,AfterSemiColon(T Header Value Tail2)
	    else
	       Tail=nil
	       {self messageHeader(Header Value)}
	       MessageParser,MaybeHeader(L)
	    end
	 end
      end
   end
end