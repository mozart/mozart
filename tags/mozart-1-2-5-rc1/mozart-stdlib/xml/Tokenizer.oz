functor
export
   NewFromURL
   NewFromString
   Fast
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
   DefaultEntityRecord =
   o(amp : string("\&")
     lt  : string("<")
     gt  : string(">")
     apos: string("'")
     quot: string("\""))
   RecToDict = Record.toDictionary
   VS2A = VirtualString.toAtom

   class Tokenizer
      prop final
      attr
	 Filename         : unit
	 File             : unit
	 Buffer           : nil
	 Line             : 1
	 Stack            : nil
	 EntityTable      : unit
	 PreviousFilename : unit
	 FileOpen         : unit
	 Coord            : unit

      meth initFromString(TXT FNull FOpen)
	 Buffer      <- TXT
	 FileOpen    <- FOpen
	 File        <- FNull
	 EntityTable <- {RecToDict DefaultEntityRecord}
      end

      meth initFromURL(FName FOpen)
	 Filename    <- {VS2A FName}
	 FileOpen    <- FOpen
	 File        <- {FOpen FName}
	 EntityTable <- {RecToDict DefaultEntityRecord}
      end

      meth GetCoord($)
	 C = @Coord
      in
	 if C==unit then
	    C = coord(@Filename @Line)
	 in
	    Coord<-C C
	 else C end
      end

      meth FillBuffer($)
	 L
      in
	 {@File read(list:L tail:nil size:1024 len:_)}
	 if L==nil then
	    case @Stack
	    of o(FN FO BU LI)|LL then
	       PreviousFilename <- @Filename
	       Filename         <- FN
	       File             <- FO
	       Buffer           <- BU
	       Line             <- LI
	       Stack            <- LL
	       Tokenizer,FillBuffer($)
	    else false end
	 else Buffer<-L true end
      end

      meth GET($)
	 case @Buffer
	 of H|T then
	    Buffer<-T
	    if H==&\n then Line<-@Line+1 Coord<-unit end
	    H
	 elseif Tokenizer,FillBuffer($)
	 then   Tokenizer,GET($)
	 else false end
      end

      meth UNGET(C)
	 if C\=false then
	    Buffer <- C|@Buffer
	    if C==&\n then Line<-@Line-1 Coord<-unit end
	 end
      end

      meth PEEK($)
	 case @Buffer
	 of H|_ then H
	 elseif Tokenizer,FillBuffer($)
	 then   Tokenizer,PEEK($)
	 else false end
      end

      meth PushFile(FName)
	 if @File\=unit then
	    Stack<-o(@Filename @File @Buffer @Line)|@Stack
	 end
	 Filename <- {VS2A FName}
	 File     <- {@FileOpen FName}
	 Buffer   <- nil
	 Line     <- 1
      end

      meth ERROR(R)
	 raise xml(tokenizer(file : @Filename
			     line : @Line
			     info : R))
	 end
      end

      meth SkipSpaces C in
	 Tokenizer,GET(C)
	 if C==false then skip
	 elseif {CharIsSpace C} then
	    Tokenizer,SkipSpaces
	 else
	    Tokenizer,UNGET(C)
	 end
      end

      %% we have just encountered a & and now we must recognize the
      %% entity reference

      meth ScanEntityRef(Value)
	 case Tokenizer,PEEK($)
	 of false then
	    Tokenizer,ERROR(entityRefEOF)
	 [] &# then C in
	    Tokenizer,GET(_)
	    Tokenizer,ScanCharRef(C)
	    case Tokenizer,GET($)
	    of &; then Value=string([C])
	    else
	       Tokenizer,ERROR(charRefMissingSemiColon)
	    end
	 else Name in
	    Tokenizer,ScanName(Name)
	    if Name==nil then
	       Tokenizer,ERROR(entityRefNoName)
	    else
	       A = {StringToAtom Name}
	       V = {CondSelect @EntityTable A unit}
	    in
	       if V==unit then
		  Tokenizer,ERROR(entityRefUnknown(A))
	       else
		  Tokenizer,SkipSpaces
		  case Tokenizer,GET($)
		  of &; then Value=V
		  else
		     Tokenizer,ERROR(entityRefMissingSemiColon)
		  end
	       end
	    end
	 end
      end

      meth ScanCharRef($)
	 case Tokenizer,PEEK($)
	 of &x then
	    Tokenizer,GET(_)
	    Tokenizer,ScanCharRefHex(0 $)
	 else
	    Tokenizer,ScanCharRefDec(0 $)
	 end
      end

      meth ScanCharRefDec(Accu $) D in
	 Tokenizer,GET(D)
	 case {CondSelect DecToInt D unit}
	 of unit then
	    Tokenizer,UNGET(D) Accu
	 [] I then
	    Tokenizer,ScanCharRefDec(Accu*10+I $)
	 end
      end

      meth ScanCharRefHex(Accu $) D in
	 Tokenizer,GET(D)
	 case {CondSelect HexToInt D unit}
	 of unit then
	    Tokenizer,UNGET(D) Accu
	 [] I then
	    Tokenizer,ScanCharRefHex(Accu*16+I $)
	 end
      end

      meth ScanValue(Value)
	 case Tokenizer,GET($)
	 of &" then Tokenizer,ScanDelim(&" Value)
	 [] &' then Tokenizer,ScanDelim(&' Value)
	 else
	    Tokenizer,ERROR(expectedQuotedValue)
	 end
      end

      meth ScanDelim(Q V)
	 case Tokenizer,GET($)
	 of false then
	    Tokenizer,ERROR(valueEOF)
	 [] && then S1 S2 in
	    Tokenizer,ScanEntityRef(S1)
	    case S1 of string(S1) then
	       {Append S1 S2 V}
	    else
	       Tokenizer,ERROR(illegalEntityTypeInValue(S1))
	    end
	    Tokenizer,ScanDelim(Q S2)
	 [] C then
	    if C==Q then V=nil
	    else V2 in
	       V=C|V2
	       Tokenizer,ScanDelim(Q V2)
	    end
	 end
      end

      meth ScanName(Name) C in
	 Tokenizer,GET(C)
	 if C==false then Name=nil
	 elseif {IsNameChar C} then Name2 in
	    Name=C|Name2
	    Tokenizer,ScanName(Name2)
	 else
	    Name=nil
	    Tokenizer,UNGET(C)
	 end
      end

      meth ScanAttr(Name Value)
	 Tokenizer,SkipSpaces
	 Tokenizer,ScanName(Name)
	 Tokenizer,SkipSpaces
	 case Tokenizer,GET($)
	 of &= then
	    Tokenizer,SkipSpaces
	    Tokenizer,ScanValue(Value)
	 else
	    Tokenizer,ERROR(expectedEqualSignInAttrib(Name))
	 end
      end

      %% we have just read </
      meth ScanEtag(Tag Coord) Name in
	 Tokenizer,SkipSpaces
	 Tokenizer,ScanName(Name)
	 Tokenizer,SkipSpaces
	 case Tokenizer,GET($)
	 of &> then
	    Tag=etag(Name Coord)
	 else
	    Tokenizer,ERROR(expectedEtagRangle)
	 end
      end

      %% we have just read <
      meth ScanStag(Tag Coord) Name Alist Empty in
	 Tag=stag(Name Alist Empty Coord)
	 Tokenizer,SkipSpaces
	 Tokenizer,ScanName(Name)
	 Tokenizer,ScanAlist(Alist Empty)
      end

      meth ScanAlist(Alist Empty)
	 Tokenizer,SkipSpaces
	 case Tokenizer,PEEK($)
	 of false then
	    Tokenizer,ERROR(alistEOF)
	 [] &/ then
	    Tokenizer,GET(_)
	    case Tokenizer,GET($)
	    of false then
	       Tokenizer,ERROR(alistEOF)
	    [] &> then Alist=nil Empty=true
	    else
	       Tokenizer,ERROR(expectedEmptyTagRangle)
	    end
	 [] &> then
	    Tokenizer,GET(_)
	    Alist=nil Empty=false
	 else Name Value Alist2 in
	    Alist=(Name|Value)|Alist2
	    Tokenizer,ScanAttr(Name Value)
	    Tokenizer,ScanAlist(Alist2 Empty)
	 end
      end

      %% the tokenizer will not guarantee the invariant that all
      %% all adjacent text tokens are collapsed.  Instead, we will
      %% leave that job to a higher abstraction.

      meth ScanText(Token Coord) Text in
	 Token=text(Text Coord)
	 Tokenizer,ScanTEXT(Text)
      end

      meth ScanTEXT(Text) C in
	 Tokenizer,GET(C)
	 if C==false then Text=nil
	 elseif C==&& orelse C==&< then
	    Text=nil
	    Tokenizer,UNGET(C)
	 else Text2 in
	    Text=C|Text2
	    Tokenizer,ScanTEXT(Text2)
	 end
      end

      %% we have just read <![
      meth ScanCDATA(Text Coord)
	 raise notImplemented(scanCDATA) end
      end

      %% we have just read <!--
      meth ScanComment(Comment Coord) Text in
	 Comment=comment(Text Coord)
	 Tokenizer,ScanCOMMENT(Text)
      end

      meth ScanCOMMENT(Text) C1 in
	 Tokenizer,GET(C1)
	 case C1
	 of false then
	    Tokenizer,ERROR(commentEOF)
	 [] &- then C2 in
	    Tokenizer,GET(C2)
	    case C2
	    of false then
	       Tokenizer,ERROR(commentEOF)
	    [] &- then C3 in
	       Tokenizer,GET(C3)
	       case C3
	       of false then
		  Tokenizer,ERROR(commentEOF)
	       [] &> then Text=nil
	       else Text2 in
		  Text=C1|Text2
		  Tokenizer,UNGET(C3)
		  Tokenizer,UNGET(C2)
		  Tokenizer,ScanCOMMENT(Text2)
	       end
	    else Text2 in
	       Text=C1|C2|Text2
	       Tokenizer,ScanCOMMENT(Text2)
	    end
	 else Text2 in
	    Text=C1|Text2
	    Tokenizer,ScanCOMMENT(Text2)
	 end
      end

      %% we have just read <?
      meth ScanPI(PI Coord) Text in
	 PI=pi(Tokenizer,ScanName($) Text Coord)
	 Tokenizer,ScanPIX(Text)
      end

      meth ScanPIX(Text) C1 in
	 Tokenizer,GET(C1)
	 case C1
	 of false then
	    Tokenizer,ERROR(piEOF)
	 [] &? then C2 in
	    Tokenizer,GET(C2)
	    case C2
	    of false then
	       Tokenizer,ERROR(piEOF)
	    [] &> then Text=nil
	    else Text2 in
	       Text=C1|Text2
	       Tokenizer,UNGET(C2)
	       Tokenizer,ScanPIX(Text2)
	    end
	 else Text2 in
	    Text=C1|Text2
	    Tokenizer,ScanPIX(Text2)
	 end
      end

      meth ScanToken(Token) C1 Coord in
	 Tokenizer,GET(C1)
	 Tokenizer,GetCoord(Coord)
	 case C1
	 of false then Token=unit
	 [] &< then C2 in
	    Tokenizer,GET(C2)
	    case C2
	    of false then
	       Tokenizer,ERROR(tokenEOF)
	    [] &/ then Tokenizer,ScanEtag(Token Coord)
	    [] &? then Tokenizer,ScanPI(Token Coord)
	    [] &! then C3 in
	       Tokenizer,GET(C3)
	       case C3
	       of false then
		  Tokenizer,ERROR(tokenEOF)
	       [] &- then C4 in
		  Tokenizer,GET(C4)
		  case C4
		  of false then
		     Tokenizer,ERROR(tokenEOF)
		  [] &- then
		     Tokenizer,ScanComment(Token Coord)
		  else
		     Tokenizer,ERROR(expectedComment)
		  end
	       [] &[ then
		  Tokenizer,ScanCDATA(Token Coord)
	       else
		  Tokenizer,UNGET(C3)
		  Token=doctype(Coord)
		  Tokenizer,ScanDoctype
	       end
	    else
	       Tokenizer,UNGET(C2)
	       Tokenizer,ScanStag(Token Coord)
	    end
	 [] && then
	    case Tokenizer,ScanEntityRef($)
	    of string(S) then Token=text(S Coord)
	    [] pi(S)     then Token=pi(S Coord)
	    [] system(S) then
	       Tokenizer,PushFile(S)
	       Tokenizer,ScanToken(Token)
	    end
	 else
	    Tokenizer,UNGET(C1)
	    Tokenizer,ScanText(Token Coord)
	 end
      end

      %% we have just read <!
      meth ScanDoctype Name in
	 Tokenizer,ScanName(Name)
	 case {Upcase Name}
	 of "DOCTYPE" then
	    Tokenizer,ScanDOCTYPE
	 else
	    Tokenizer,ERROR(expectedDoctype)
	 end
      end

      meth ScanDOCTYPE
	 Tokenizer,SkipSpaces
	 case Tokenizer,PEEK($)
	 of false then
	    Tokenizer,ERROR(doctypeEOF)
	 [] &[ then
	    Tokenizer,GET(_)
	    Tokenizer,ScanDoctypeBody
	 [] &" then
	    Tokenizer,ScanDelim(&" _)
	    Tokenizer,ScanDOCTYPE
	 [] &' then
	    Tokenizer,ScanDelim(&' _)
	    Tokenizer,ScanDOCTYPE
	 else
	    Tokenizer,ScanName(_)
	    Tokenizer,ScanDOCTYPE
	 end
      end

      meth ScanDoctypeBody
	 Tokenizer,SkipSpaces
	 case Tokenizer,GET($)
	 of false then
	    Tokenizer,ERROR(doctypeEOF)
	 [] &] then
	    Tokenizer,SkipSpaces
	    case Tokenizer,GET($)
	    of false then
	       Tokenizer,ERROR(doctypeEOF)
	    [] &> then skip
	    else
	       Tokenizer,ERROR(expectedDoctypeRangle)
	    end
	 [] &< then
	    case Tokenizer,GET($)
	    of false then
	       Tokenizer,ERROR(doctypeEOF)
	    [] &! then
	       if Tokenizer,PEEK($)==&- then
		  Tokenizer,GET(_)
		  case Tokenizer,GET($)
		  of false then
		     Tokenizer,ERROR(doctypeEOF)
		  [] &- then
		     Tokenizer,ScanComment(_)
		     Tokenizer,ScanDoctypeBody
		  [] C then
		     Tokenizer,ERROR(unexpectCharInDoctype(C))
		  end
	       else S in
		  Tokenizer,ScanName(S)
		  case {Upcase S}
		  of "ENTITY" then Name Type Value in
		     Tokenizer,SkipSpaces
		     Tokenizer,ScanName(Name)
		     Tokenizer,SkipSpaces
		     if {Member Tokenizer,PEEK($) [&' &"]} then
			Type = string
		     else TName in
			Tokenizer,ScanName(TName)
			case {Upcase TName}
			of "SYSTEM" then Type=system
			[] "PI"     then Type=pi
			else
			   Tokenizer,ERROR(unpexpectedEntityType(TName))
			end
		     end
		     Tokenizer,SkipSpaces
		     Tokenizer,ScanValue(Value)
		     @EntityTable.{StringToAtom Name} := Type(Value)
		     Tokenizer,SkipSpaces
		     case Tokenizer,GET($)
		     of false then
			Tokenizer,ERROR(entityDeclEOF)
		     [] &> then
			Tokenizer,ScanDoctypeBody
		     [] C then
			Tokenizer,ERROR(unpextecCharInEntityDecl(C))
		     end
		  else
		     %% for anything else, we skip until the next >
		     Tokenizer,SkipToRangle
		     Tokenizer,ScanDoctypeBody
		  end
	       end
	    [] C then
	       Tokenizer,ERROR(expectedBangInDoctype(C))
	    end
	 [] C then
	    Tokenizer,ERROR(unexpectedCharInDoctype(C))
	 end
      end

      meth SkipToRangle
	 case Tokenizer,GET($)
	 of &> then skip
	 else Tokenizer,SkipToRangle end
      end

      meth getToken($)
	 Tokenizer,ScanToken($)
      end
   end

   class DevNull
      meth init skip end
      meth read(list:L tail:T size:_ len:N)
	 L=T N=0
      end
   end
import
   Open(file:OpenFileClass)
   Fast at 'FastTokenizer.ozf'
define
   local
      fun {OpenFile F}
	 {New OpenFileClass init(url:F)}
      end
   in
      fun {NewFromURL FName}
	 Tok={New Tokenizer initFromURL(FName OpenFile)}
	 fun {Get} {Tok getToken($)} end
      in
	 tokenizer(get:Get)
      end

      fun {NewFromString TXT}
	 Tok={New Tokenizer initFromString(TXT {New DevNull init} OpenFile)}
	 fun {Get} {Tok getToken($)} end
      in
	 tokenizer(get:Get)
      end
   end
end