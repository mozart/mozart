functor
export
   'class' : XMLTokenizer
prepare
   %% ================================================================
   %% {IsNameChar C}
   %%     returns true iff C may legally occur in an XML name
   %% ================================================================
   
   CharIsAlNum = Char.isAlNum
   fun {IsNameChar C}
      {CharIsAlNum C} orelse
      C==&-           orelse
      C==&_           orelse
      C==&.           orelse
      C==&:
   end
   CharIsSpace = Char.isSpace
   DecToInt = o(&0:0 &1:1 &2:2 &3:3 &4:4 &5:5 &6:6 &7:7 &8:8 &9:9)
   HexToInt = o(&0:0 &1:1 &2:2 &3:3 &4:4 &5:5 &6:6 &7:7 &8:8 &9:9
		&a:10 &b:11 &c:12 &d:13 &e:14 &f:15
		&A:10 &B:11 &C:12 &D:13 &E:14 &F:15)
   CharUpcase = Char.toUpper
   fun {Upcase S} {Map S CharUpcase} end
import
   Open
define
   class TextFile from Open.file Open.text prop final end
   class XMLTokenizer
      prop final
      attr
	 Filename         : unit
	 File             : unit
	 Peeks            : nil
	 LineNumber       : 1
	 Stack            : nil
	 EntityTable      : unit
	 PreviousFilename : unit

      meth init(FName)
	 Filename    <- FName
	 File        <- {New TextFile init(url:FName)}
	 EntityTable <- {NewDictionary}
	 @EntityTable.'amp' := string("\&")
	 @EntityTable.'lt'  := string("<")
	 @EntityTable.'gt'  := string(">")
	 @EntityTable.'apos':= string("'")
	 @EntityTable.'quot':= string("\"")
      end

      meth getC($)
	 case @Peeks
	 of C|T then
	    Peeks<-T
	    if C==&\n then LineNumber<-@LineNumber+1 end
	    C
	 else C in
	    {@File getC(C)}
	    case C
	    of false then
	       if   XMLTokenizer,PopFile($)
	       then XMLTokenizer,getC($)
	       else false end
	    [] &\n then
	       LineNumber<-@LineNumber+1
	       C
	    else C end
	 end
      end
      
      meth PopFile($)
	 case @Stack
	 of o(filename:FN file:F linenumber:N)|L then
	    try {@File close} catch _ then skip end
	    PreviousFilename <- @Filename
	    Filename         <- FN
	    File             <- F
	    LineNumber       <- N
	    Stack            <- L
	    true
	 [] nil then false end
      end
      
      meth unGetC(C)
	 if C\=false then
	    Peeks <- C|@Peeks
	    if C==&\n then LineNumber<-@LineNumber-1 end
	 end
      end
      
      meth peekC($)
	 case @Peeks
	 of C|_ then C
	 else C in
	    {@File getC(C)}
	    if C==false then
	       if XMLTokenizer,PopFile($) then
		  XMLTokenizer,peekC($)
	       else false end
	    else
	       XMLTokenizer,unGetC(C)
	       C
	    end
	 end
      end
      
      meth OpenFile(F)
	 if @File\=unit then
	    Stack <- o(filename:@Filename file:@File linenumber:@LineNumber)|@Stack
	 end
	 Filename   <- F
	 LineNumber <- 1
	 File       <- {New TextFile init(name:F)}
      end
      meth getCs(N S)
	 if N==0 then S=nil else C in
	    XMLTokenizer,getC(C)
	    if C==false then S=nil else S2 in
	       S=C|S2 XMLTokenizer,getCs(N-1 S2)
	    end
	 end
      end
      meth unGetCs(S)
	 if @Peeks==nil then
	    Peeks <- S
	 else
	    Peeks <- {Append S @Peeks}
	 end
      end

      %% =============================================================
      %% raise an informative error
      %% =============================================================

      meth error(R)
	 {Exception.raiseError
	  xmltokenizer(filename   : @Filename
		       linenumber : @LineNumber
		       problem    : R)}
      end
      
      meth skipSpaces() C in
	 XMLTokenizer,getC(C)
	 if C==false then skip
	 elseif {CharIsSpace C} then
	    XMLTokenizer,skipSpaces()
	 else
	    XMLTokenizer,unGetC(C)
	 end
      end

      %% we have just encountered a & and now we must recognize the
      %% entity reference.

      meth scanEntityRef(Value)
	 case XMLTokenizer,peekC($)
	 of false then
	    XMLTokenizer,error(unterminatedEntityRef)
	 [] &# then C in
	    XMLTokenizer,getC(_)
	    XMLTokenizer,scanCharRef(C)
	    case XMLTokenizer,getC($)
	    of &; then Value=string([C])
	    else
	       XMLTokenizer,error('expectedCharRefSemiColon')
	    end
	 else Name in
	    XMLTokenizer,scanName(Name)
	    if Name==nil then
	       XMLTokenizer,error('expectedEntityName')
	    else
	       A = {StringToAtom Name}
	       V = {CondSelect @EntityTable A unit}
	    in
	       if V==unit then
		  XMLTokenizer,error('unknownEntityRef'(A))
	       else
		  XMLTokenizer,skipSpaces()
		  case XMLTokenizer,getC($)
		  of &; then Value=V
		  else XMLTokenizer,error('expectedEntityColon')
		  end
	       end
	    end
	 end
      end

      meth scanCharRef($)
	 case XMLTokenizer,peekC($)
	 of &x then
	    XMLTokenizer,getC(_)
	    XMLTokenizer,scanCharRefHex(0 $)
	 else
	    XMLTokenizer,scanCharRefDec(0 $)
	 end
      end

      meth scanCharRefDec(Accu $) D in
	 XMLTokenizer,getC(D)
	 case {CondSelect DecToInt D unit}
	 of unit then
	    XMLTokenizer,unGetC(D) Accu
	 [] I then
	    XMLTokenizer,scanCharRefDec(Accu*10+I $)
	 end
      end

      meth scanCharRefHex(Accu $) D in
	 XMLTokenizer,getC(D)
	 case {CondSelect HexToInt D unit}
	 of unit then
	    XMLTokenizer,unGetC(D) Accu
	 [] I then
	    XMLTokenizer,scanCharRefHex(Accu*16+I $)
	 end
      end

      meth scanValue(Value)
	 case XMLTokenizer,getC($)
	 of &" then XMLTokenizer,ScanValue(&" Value)
	 [] &' then XMLTokenizer,ScanValue(&' Value)
	 else XMLTokenizer,error(expectedQuotedValue) end
      end

      meth ScanValue(C V)
	 case XMLTokenizer,getC($)
	 of false then
	    XMLTokenizer,error(unterminatedValue)
	 [] && then S1 S2 in
	    XMLTokenizer,scanEntityRef(S1)
	    case S1 of string(S1) then
	       {Append S1 S2 V}
	    else
	       XMLTokenizer,error(illegalEntityTypeInValue(S1))
	    end
	    XMLTokenizer,ScanValue(C S2)
	 [] CC then
	    if CC==C then V=nil
	    else V2 in
	       V=(CC|V2)
	       XMLTokenizer,ScanValue(C V2)
	    end
	 end
      end

      meth scanName(Name) C in
	 XMLTokenizer,getC(C)
	 if C==false then Name=nil
	 elseif {IsNameChar C} then Name2 in
	    Name=(C|Name2)
	    XMLTokenizer,scanName(Name2)
	 else
	    Name=nil
	    XMLTokenizer,unGetC(C)
	 end
      end

      meth scanAttr(Name Value)
	 XMLTokenizer,skipSpaces()
	 XMLTokenizer,scanName(Name)
	 XMLTokenizer,skipSpaces()
	 case XMLTokenizer,getC($)
	 of &= then
	    XMLTokenizer,skipSpaces()
	    XMLTokenizer,scanValue(Value)
	 else
	    XMLTokenizer,error(expectedEqualSignInAttrib(Name))
	 end
      end

      %% we have just read />
      meth scanEtag(Tag) Name in
	 XMLTokenizer,skipSpaces()
	 XMLTokenizer,scanName(Name)
	 XMLTokenizer,skipSpaces()
	 case XMLTokenizer,getC($) of &> then
	    Tag=etag(Name)
	 else
	    XMLTokenizer,error(expectedEtagRangle)
	 end
      end

      %% we have just read <
      meth scanStag(Tag) Name Alist Empty in
	 Tag=stag(Name Alist Empty)
	 XMLTokenizer,skipSpaces()
	 XMLTokenizer,scanName(Name)
	 XMLTokenizer,scanAlist(Alist Empty)
      end

      meth scanAlist(Alist Empty)
	 XMLTokenizer,skipSpaces()
	 case XMLTokenizer,peekC($)
	 of false then
	    XMLTokenizer,error(unterminatedAlist)
	 [] &/ then XMLTokenizer,getC(_)
	    case XMLTokenizer,getC($)
	    of false then
	       XMLTokenizer,error(unterminatedAlist)
	    [] &> then Alist=nil Empty=true
	    else
	       XMLTokenizer,error(expectedEmptyTagRangle)
	    end
	 [] &> then XMLTokenizer,getC(_)
	    Alist=nil Empty=false
	 else Name Value Alist2 in
	    %% # for debugging, change to | for production
	    Alist=(Name|Value)|Alist2
	    XMLTokenizer,scanAttr(Name Value)
	    XMLTokenizer,scanAlist(Alist2 Empty)
	 end
      end

      %% I should not try to do so much in scanText.  leave it to
      %% another level to put consecutive text fragments together
      %% if desired.

      meth scanText(Token) Text in
	 Token=text(Text)
	 XMLTokenizer,ScanText(Text)
      end
      
      meth ScanText(Text) C in
	 XMLTokenizer,getC(C)
	 if C==false then Text=nil
	 elseif C==&& orelse C==&< then
	    XMLTokenizer,unGetC(C)
	    Text=nil
	 else Text2 in
	    Text=C|Text2
	    XMLTokenizer,ScanText(Text2)
	 end
      end

      %% we have just read <![
      meth scanCDATA(Text)
	 raise notimplemented(scanCDATA) end
      end

      %% we have just read <!--
      meth scanComment(Comment) Text in
	 Comment=comment(Text)
	 XMLTokenizer,ScanComment(Text)
      end

      meth ScanComment(Text) C in
	 XMLTokenizer,getC(C)
	 case C
	 of false then
	    XMLTokenizer,error(unterminatedComment)
	 [] &- then C2 in
	    XMLTokenizer,getC(C2)
	    case C2
	    of false then
	       XMLTokenizer,error(unterminatedComment)
	    [] &- then C3 in
	       XMLTokenizer,getC(C3)
	       case C3
	       of false then
		  XMLTokenizer,error(unterminatedComment)
	       [] &> then Text=nil
	       else Text2 in
		  Text=C|C2|C3|Text2
		  XMLTokenizer,ScanComment(Text2)
	       end
	    else Text2 in
	       Text=C|C2|Text2
	       XMLTokenizer,ScanComment(Text2)
	    end
	 else Text2 in
	    Text=C|Text2
	    XMLTokenizer,ScanComment(Text2)
	 end
      end

      %% we have just read <?
      meth scanPI(PI) Text in
	 PI=pi(Text)
	 XMLTokenizer,ScanPI(Text)
      end

      meth ScanPI(Text) C in
	 XMLTokenizer,getC(C)
	 case C
	 of false then
	    XMLTokenizer,error(unterminatedPI)
	 [] &? then C2 in
	    XMLTokenizer,getC(C2)
	    case C2
	    of false then
	       XMLTokenizer,error(unterminatedPI)
	    [] &> then Text=nil
	    else Text2 in
	       XMLTokenizer,unGetC(C2)
	       Text=C|Text2
	       XMLTokenizer,ScanPI(Text2)
	    end
	 else Text2 in
	    Text=C|Text2
	    XMLTokenizer,ScanPI(Text2)
	 end
      end

      meth scanToken(Token) C in
	 XMLTokenizer,getC(C)
	 case C
	 of false then Token=unit
	 [] &< then C2 in
	    XMLTokenizer,getC(C2)
	    case C2
	    of false then
	       XMLTokenizer,error(unterminatedToken)
	    [] &/ then XMLTokenizer,scanEtag(Token)
	    [] &? then XMLTokenizer,scanPI(Token)
	    [] &! then C3 in
	       XMLTokenizer,getC(C3)
	       case C3
	       of false then
		  XMLTokenizer,error(unterminatedToken)
	       [] &- then C4 in
		  XMLTokenizer,getC(C4)
		  case C4
		  of false then
		     XMLTokenizer,error(unterminatedToken)
		  [] &- then XMLTokenizer,scanComment(Token)
		  else
		     XMLTokenizer,error(expectedComment)
		  end
	       [] &[ then
		  XMLTokenizer,scanCDATA(Token)
	       else
		  XMLTokenizer,unGetC(C3)
		  Token=doctype
		  XMLTokenizer,scanDoctype()
	       end
	    else
	       XMLTokenizer,unGetC(C2)
	       XMLTokenizer,scanStag(Token)
	    end
	 [] && then
	    case XMLTokenizer,scanEntityRef($)
	    of string(S) then Token=text(S)
	    [] pi(S)     then Token=pi(S)
	    [] system(S) then
	       XMLTokenizer,OpenFile(S)
	       XMLTokenizer,scanToken(Token)
	    end
	 else
	    XMLTokenizer,unGetC(C)
	    XMLTokenizer,scanText(Token)
	 end
      end

      %% we have just read <!
      meth scanDoctype() Name in
	 XMLTokenizer,scanName(Name)
	 case {Upcase Name}
	 of "DOCTYPE" then
	    XMLTokenizer,ScanDoctypeStart()
	 else
	    XMLTokenizer,error(expectedDoctype)
	 end
      end

      meth ScanDoctypeStart()
	 XMLTokenizer,skipSpaces()
	 case XMLTokenizer,peekC($)
	 of false then
	    XMLTokenizer,error(unterminatedDoctype)
	 [] &[ then
	    XMLTokenizer,getC(_)
	    XMLTokenizer,ScanDoctypeBody()
	 [] &" then
	    XMLTokenizer,ScanValue(&" _)
	    XMLTokenizer,ScanDoctypeStart()
	 [] &' then
	    XMLTokenizer,ScanValue(&" _)
	    XMLTokenizer,ScanDoctypeStart()
	 else
	    XMLTokenizer,scanName(_)
	    XMLTokenizer,ScanDoctypeStart()
	 end
      end

      meth ScanDoctypeBody()
	 XMLTokenizer,skipSpaces()
	 case XMLTokenizer,getC($)
	 of false then
	    XMLTokenizer,error(unterminatedDoctype)
	 [] &] then
	    XMLTokenizer,skipSpaces()
	    case XMLTokenizer,getC($)
	    of false then
	       XMLTokenizer,error(unterminatedDoctype)
	    [] &> then skip
	    else
	       XMLTokenizer,error(expectedDoctypeRangle)
	    end
	 [] &< then
	    case XMLTokenizer,getC($)
	    of false then
	       XMLTokenizer,error(unterminatedDoctype)
	    [] &! then
	       if XMLTokenizer,peekC($)==&- then
		  XMLTokenizer,getC(_)
		  case XMLTokenizer,getC($)
		  of false then
		     XMLTokenizer,error(unterminatedDoctype)
		  [] &- then
		     XMLTokenizer,scanComment(_)
		     XMLTokenizer,ScanDoctypeBody()
		  [] C then
		     XMLTokenizer,error(unexpectedCharInDoctype(C))
		  end
	       else S in
		  XMLTokenizer,scanName(S)
		  case {Upcase S}
		  of "ENTITY" then Name Type Value in
		     XMLTokenizer,skipSpaces()
		     XMLTokenizer,scanName(Name)
		     XMLTokenizer,skipSpaces()
		     if {Member XMLTokenizer,peekC($) [&' &"]} then
			Type = 'string'
		     else TName in
			XMLTokenizer,scanName(TName)
			case {Upcase TName}
			of "SYSTEM" then Type='system'
			[] "PI"     then Type='pi'
			else
			   XMLTokenizer,error(unexpectedEntityType(TName))
			end
		     end
		     XMLTokenizer,skipSpaces()
		     XMLTokenizer,scanValue(Value)
		     @EntityTable.{StringToAtom Name} := Type(Value)
		     XMLTokenizer,skipSpaces()
		     case XMLTokenizer,getC($)
		     of false then
			XMLTokenizer,error(unterminatedEntityDecl)
		     [] &> then
			XMLTokenizer,ScanDoctypeBody()
		     else
			XMLTokenizer,error(unexpectedCharInEntityDecl)
		     end
		  else
		     XMLTokenizer,error(expectedEntityDecl)
		  end
	       end
	    else
	       XMLTokenizer,error(expectedBangInDoctype)
	    end
	 [] C then
	    XMLTokenizer,error(unexpectedCharInDoctype(C))
	 end
      end
   end
end
