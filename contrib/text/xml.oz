%%% -*-oz-gump-*-
%%% XML parser
\switch +gump
functor
import
   Resolve OS
   GumpScanner('class':GS)
   GumpParser('class':GP)
export
   'class': XML_Parser
define
   \gumpscannerprefix 'xml_'
   scanner XML_Scanner from GS
      meth scanVs(VS)
	 GS,setMode(Element)
	 GS,scanVirtualString(VS)
      end
      meth scanFile(F)
	 GS,setMode(Element)
	 GS,scanFile(F)
      end
      lex id2 = <[a-zA-Z0-9:._-]> end
      lex id  = <[a-zA-Z]{id2}*> end
      lex <<EOF>> GS,putToken1('EOF') end
      mode Element
	 lex <"<"{id}> S in
	    GS,getString(_|S)
	    GS,putToken('<' {String.toAtom S})
	    GS,setMode(Attribute)
	 end
	 lex <"</"{id}> S in
	    GS,getString(_|_|S)
	    GS,putToken('</' {String.toAtom S})
	    GS,setMode(EndTag)
	 end
	 lex <"<?"{id}> S in
	    GS,getString(_|_|S)
	    GS,putToken('<?' {String.toAtom S})
	    GS,setMode(EndPI)
	 end
	 lex <[^<]+> S in
	    GS,getString(S)
	    GS,putToken('cdata' S)
	 end
	 lex <.|\n> raise bad(element) end end
      end
      mode Attribute
	 lex <"/>">
	    GS,putToken1('/>')
	    GS,setMode(Element)
	 end
	 lex <">">
	    GS,putToken1('>')
	    GS,setMode(Element)
	 end
	 lex <[ \t\n]+> skip end
	 lex <{id}> A in
	    GS,getAtom(A)
	    GS,putToken('attribute' A)
	    GS,setMode(Equal)
	 end
	 lex <.> raise bad(attribute) end end
      end
      mode Equal
	 lex <[ \t\n]+> skip end
	 lex <=[ \t\n]*> GS,setMode(Value) end
	 lex <.|\n> raise bad(equal) end end
      end
      mode Value
	 lex <\"([^\"]|\\.)*\"> S in
	    GS,getString(_|S)
	    GS,putToken('value' {Reverse {Reverse S}.2})
	    GS,setMode(Attribute)
	 end
	 lex <\'([^\']|\\.)*\'> S in
	    GS,getString(_|S)
	    GS,putToken('value' {Reverse {Reverse S}.2})
	    GS,setMode(Attribute)
	 end
	 lex <{id2}+>
	    GS,putToken('value' GS,getString($))
	    GS,setMode(Attribute)
	 end
	 lex <.|\n> raise bad(value) end end
      end
      mode EndPI
	 lex <[ \t\n]*"?>"> GS,setMode(Element) end
	 lex <.|\n> raise bad(endPI) end end
      end
      mode EndTag
	 lex <[ \t\n]*">"> GS,setMode(Element) end
	 lex <.|\n> raise bad(endTag) end end
      end
   end
   parser XML_Parser from GP
      prop locking
      attr theScanner
      meth init
	 theScanner <- {New XML_Scanner init}
	 GP,init(@theScanner)
      end
      meth parseVs(VS L)
	 lock
	    {@theScanner scanVs(VS)}
	    if GP,parse(document(L) $) then skip
	    else raise xml(parseFailed) end end
	 end
      end
      meth parseFile(F L)
	  R = {Resolve.pickle.localize F}
      in
	 try
	    lock
	       {@theScanner scanFile(R.1)}
	       if GP,parse(document(L) $) then skip
	       else raise xml(parseFailed) end end
	    end
	 finally
	    case R of new(X) then
	       try {OS.unlink X} catch _ then skip end
	    else skip end
	 end
      end
      token '<' '</' '<?' cdata '/>' '>' attribute value
      syn document($)
	 L = { Element($) }* => root(L)
      end
      syn Element($)
	 '<'(ID) Attributes(ALIST) '/>'
	 => element(type:ID attribute:ALIST content:nil)
      [] L in '<'(ID) Attributes(ALIST) '>' !L = { Element($) }* '</'(ID2)
	 => ID=ID2
	    element(type:ID attribute:ALIST content:L)
      [] cdata(S) => cdata(S)
      [] '<?'(ID) => pi(ID)
      end
      syn Attribute($)
	 attribute(KEY) value(VAL)
	 => KEY#attribute(name:KEY value:VAL)
      end
      syn Attributes($)
	 L={ Attribute($) }* => {List.toRecord o L}
      end
   end
end
