functor
export
   'class' : Message
import
   MessageParser at 'MessageParser.ozf'
prepare
   ToLower = Char.toLower
   S2A = String.toAtom
define
   class Message from MessageParser.'class'
      attr
	 headers
	 HeadersTail
	 body

      meth init(L eol:EOL<=[crlf lf])
	 {self acceptCRLF({Member crlf EOL})}
	 {self acceptLF(  {Member   lf EOL})}
	 {self acceptCR(  {Member cr   EOL})}
	 local L in
	    headers     <- L
	    HeadersTail <- L
	    body        <- nil
	 end
	 {self parse(L)}
      end

      meth messageHeader(Header Value) L in
	 ((
	   {self messageHeaderTag(Header  $)} #
	   {self messageHeaderValue(Value $)}
	  )
	  |L) = (HeadersTail<-L)
      end

      meth messageBody(L) body<-L end

      meth messageEnd() @HeadersTail=nil end

      meth messageHeaderTag(Header $)
	 {S2A {Map Header ToLower}}
      end

      meth messageHeaderValue(Value $)
	 Value
      end
   end
end
