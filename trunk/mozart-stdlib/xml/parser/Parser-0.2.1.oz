%% -*-oz-*-
functor
export
   new : NewParser
prepare
   MakeBS = ByteString.make
   StringToken = String.token
   DictClone = Dictionary.clone
   RecToDict = Record.toDictionary
   VS2A = VirtualString.toAtom
   VS2S = VirtualString.toString

   CharIsAlNum = Char.isAlNum
   CharIsSpace = Char.isSpace
   CharUpcase  = Char.toUpper

   fun {Upcase S} {Map S CharUpcase} end

   %% ================================================================
   %% {VS2SFast S}
   %%     is like VS2S but is intended to be applied to values which
   %% are either strings or virtual strings built from strings by
   %% tupling.  It checks for the common case that the main constructor
   %% is a tupling constructor: if not, then the argument is already
   %% a string.
   %% ================================================================

   fun {VS2SFast S}
      case S of _#_ then {VS2S S} else S end
   end

   %% ================================================================
   %% {AllSpaces +S ?B}
   %%     returns true iff all chars in string S are white space
   %% ================================================================

   fun {AllSpaces S}
      case S of nil then true
      [] H|T then {CharIsSpace H} andthen {AllSpaces T}
      end
   end

   %% ================================================================
   %% {IsNameChar +C ?B}
   %%     returns true iff C may legally occur in an XML name
   %% ================================================================

   fun {IsNameChar C}
      {CharIsAlNum C} orelse
      C==&-           orelse
      C==&_           orelse
      C==&.           orelse
      C==&:
   end

   %% ================================================================
   %% tables mapping character digits to their numerical value.  They
   %% are also used to determine if a character is a legal digit.
   %% ================================================================

   DecToInt = o(&0:0 &1:1 &2:2 &3:3 &4:4 &5:5 &6:6 &7:7 &8:8 &9:9)
   HexToInt = o(&0:0 &1:1 &2:2 &3:3 &4:4 &5:5 &6:6 &7:7 &8:8 &9:9
		&a:10 &b:11 &c:12 &d:13 &e:14 &f:15
		&A:10 &B:11 &C:12 &D:13 &E:14 &F:15)

   %% ================================================================
   %% default entity definitions
   %% ================================================================

   DefaultEntityRecord =
   o(amp : string("\&")
     lt  : string("<")
     gt  : string(">")
     apos: string("'")
     quot: string("\""))

   %% ================================================================
   %% we use DevNull just to have a file object when actually taking
   %% input from a string
   %% ================================================================

   proc {DevNull Msg}
      Msg.list=Msg.tail Msg.len=0
   end

   %% ================================================================
   %% its extremely annoying, but XML says that attribute xmlns can be
   %% be spelled with any capitalization.  Here, we provide a table to
   %% quickly check whether a given atom is really one of the 32
   %% possible spellings of xmlns.  This way we can avoid doing case
   %% normalization just to check this.
   %% ================================================================

   XMLNS =
      o(
	 'xmlns' : unit
	 'Xmlns' : unit
	 'xMlns' : unit
	 'xmLns' : unit
	 'xmlNs' : unit
	 'xmlnS' : unit
	 'XMlns' : unit
	 'XmLns' : unit
	 'XmlNs' : unit
	 'XmlnS' : unit
	 'xMLns' : unit
	 'xMlNs' : unit
	 'xMlnS' : unit
	 'xmLNs' : unit
	 'xmLnS' : unit
	 'xmlNS' : unit
	 'XMLns' : unit
	 'XMlNs' : unit
	 'XMlnS' : unit
	 'XmLNs' : unit
	 'XmLnS' : unit
	 'XmlNS' : unit
	 'xMLNs' : unit
	 'xMlNS' : unit
	 'xMLnS' : unit
	 'xmLNS' : unit
	 'XMLNs' : unit
	 'XmLNS' : unit
	 'XMlNS' : unit
	 'XMLnS' : unit
	 'xMLNS' : unit
	 'XMLNS' : unit
	 )

   %% ================================================================
   %% the big tokenizer+parser combo
   %% ================================================================

   class Parser
      prop final
      attr
	 %%-----------------------------------------------------------
	 %% tokenizer
	 %%-----------------------------------------------------------
	 Filename    : unit	% URL of current input
	 File        : unit	% file object used for reading
	 Buffer      : nil	% buffered input
	 Line        : 1	% current line in current file
	 Stack       : nil	% stack of interrupted inputs
	 EntityTable : unit	% table mapping entity names to values
	 FileOpen    : unit	% function to open a URL for input
	 Coord       : unit	% coord(@Filename @Line) saved for reuse in same line
	 %%-----------------------------------------------------------
	 %% namespace support
	 %%-----------------------------------------------------------
	 PrefixTable : unit
	 NameTable   : unit
	 NeedCopy    : false
	 %%-----------------------------------------------------------
	 %% parser
	 %%-----------------------------------------------------------
	 STACK       : unit
	 PARENT      : unit
	 INDEX       : 1
	 STRIP       : unit
	 LOOK        : unit
	 TAG         : unit
	 CONTENTS    : nil
	 PSTACK      : nil

      meth init(
	      string      : UserBuffer      <= nil
	      url         : UserURL         <= unit
	      file        : UserFile        <= unit
	      prefixtable : UserPrefixTable <= unit
	      nametable   : UserNameTable   <= unit
	      striptable  : UserStripTable  <= unit
	      fileopen    : UserFileOpen    <= unit
	      root        : UserRoot        <= _)
	 if UserPrefixTable==unit then
	    PrefixTable <- {NewDictionary}
	 else
	    PrefixTable <- UserPrefixTable
	 end
	 if UserNameTable==unit then
	    NameTable <- {NewDictionary}
	 else
	    NameTable <- UserNameTable
	 end
	 PSTACK      <- nil
	 Line        <- 1
	 Stack       <- nil
	 Coord       <- unit
	 STRIP       <- UserStripTable
	 Buffer      <- UserBuffer
	 FileOpen    <- UserFileOpen
	 EntityTable <- {RecToDict DefaultEntityRecord}
	 if UserURL\=unit then
	    Filename <- UserURL
	    File     <- {UserFileOpen UserURL}
	 elseif UserFile\=unit then
	    Filename <- UserFile
	    File     <- {UserFileOpen UserFile}
	 else
	    File     <- DevNull
	 end
	 INDEX       <- 1
	 TAG         <- unit
	 local L in
	    PARENT   <- root(children:L index:0 parent:unit)
	    CONTENTS <- L
	    UserRoot = @PARENT
	 end
	 Parser,PARSE()
      end

      %% =============================================================
      %% TOKENIZER
      %% =============================================================

      meth GetCoord($) C=@Coord in
	 if C==unit
	 then C=coord(@Filename @Line) in Coord<-C C
	 else C end
      end

      meth FillBuffer($)
	 L
      in
	 {@File read(list:L tail:nil size:1024 len:_)}
	 if L==nil then
	    %% current input has been exhausted: we need
	    %% to return to the input that caused the
	    %% "include" that just ended.
	    case @Stack
	    of o(FN FO BU LI)|LL then
	       Filename         <- FN
	       File             <- FO
	       Buffer           <- BU
	       Line             <- LI
	       Stack            <- LL
	       Parser,FillBuffer($)
	    else false end
	 else Buffer<-L true end
      end

      %% get the next character of input
      
      meth GET($)
	 case @Buffer
	 of H|T then
	    Buffer<-T
	    if H==&\n then Line<-@Line+1 Coord<-unit end
	    H
	 elseif Parser,FillBuffer($)
	 then   Parser,GET($)
	 else false end
      end

      %% put back a character to be read again later
      %%
      %% there is no limit to how many characters may be
      %% put back. however, it is possible for the following
      %% situation to happen: we read reading from URL1, get C1,
      %% pop the stack, now we are reading again from URL2,
      %% get C2, put back C2, put back C1.  When C1 is read
      %% again, it will have the incorrect coordinate
      %% coord(URL2 -1).  This situation is extremely unlikely
      %% and it only affects error reporting by introducing
      %% some inaccuracy.  It is not worth fixing.

      meth UNGET(C)
	 if C\=false then
	    Buffer <- C|@Buffer
	    if C==&\n then Line<-@Line-1 Coord<-unit end
	 end
      end

      %% just peek at the next character without actually reading it

      meth PEEK($)
	 case @Buffer
	 of H|_ then H
	 elseif Parser,FillBuffer($)
	 then   Parser,PEEK($)
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

      meth TERROR(R)
	 raise xml(tokenizer(file : @Filename
			     line : @Line
			     info : R))
	 end
      end

      meth SkipSpaces C in
	 Parser,GET(C)
	 if C==false then skip
	 elseif {CharIsSpace C} then
	    Parser,SkipSpaces
	 else
	    Parser,UNGET(C)
	 end
      end

      %% we have just encountered a & and now we must recognize the
      %% entity reference

      meth ScanEntityRef(Value)
	 case Parser,PEEK($)
	 of false then
	    Parser,TERROR(entityRefEOF)
	 [] &# then C in
	    Parser,GET(_)
	    Parser,ScanCharRef(C)
	    case Parser,GET($)
	    of &; then Value=string([C])
	    else
	       Parser,TERROR(charRefMissingSemiColon)
	    end
	 else Name in
	    Parser,ScanName(Name)
	    if Name==nil then
	       Parser,TERROR(entityRefNoName)
	    else
	       A = {StringToAtom Name}
	       V = {CondSelect @EntityTable A unit}
	    in
	       if V==unit then
		  Parser,TERROR(entityRefUnknown(A))
	       else
		  Parser,SkipSpaces
		  case Parser,GET($)
		  of &; then Value=V
		  else
		     Parser,TERROR(entityRefMissingSemiColon)
		  end
	       end
	    end
	 end
      end

      meth ScanCharRef($)
	 case Parser,PEEK($)
	 of &x then
	    Parser,GET(_)
	    Parser,ScanCharRefHex(0 $)
	 else
	    Parser,ScanCharRefDec(0 $)
	 end
      end

      meth ScanCharRefDec(Accu $) D I in
	 Parser,GET(D)
	 {CondSelect DecToInt D unit I}
	 if I==unit then Parser,UNGET(D) Accu
	 else
	    Parser,ScanCharRefDec(Accu*10+I $)
	 end
      end

      meth ScanCharRefHex(Accu $) D I in
	 Parser,GET(D)
	 {CondSelect HexToInt D unit I}
	 if I==unit then Parser,UNGET(D) Accu
	 else
	    Parser,ScanCharRefHex(Accu*16+I $)
	 end
      end

      meth ScanValue(Value)
	 case Parser,GET($)
	 of &" then Parser,ScanDelim(&" Value)
	 [] &' then Parser,ScanDelim(&' Value)
	 else
	    Parser,TERROR(expectedQuotedValue)
	 end
      end

      meth ScanDelim(Q V)
	 case Parser,GET($)
	 of false then
	    Parser,TERROR(valueEOF)
	 [] && then S1 S2 in
	    Parser,ScanEntityRef(S1)
	    case S1 of string(S1) then
	       {Append S1 S2 V}
	    else
	       Parser,TERROR(illegalEntityTypeInValue(S1))
	    end
	    Parser,ScanDelim(Q S2)
	 [] C then
	    if C==Q then V=nil
	    else V2 in
	       V=C|V2
	       Parser,ScanDelim(Q V2)
	    end
	 end
      end

      meth ScanName(Name) C in
	 Parser,GET(C)
	 if C==false then Name=nil
	 elseif {IsNameChar C} then Name2 in
	    Name=C|Name2
	    Parser,ScanName(Name2)
	 else
	    Name=nil
	    Parser,UNGET(C)
	 end
      end

      meth ScanAttr(Name Value)
	 Parser,SkipSpaces
	 Parser,ScanName(Name)
	 Parser,SkipSpaces
	 case Parser,GET($)
	 of &= then
	    Parser,SkipSpaces
	    Parser,ScanValue(Value)
	 else
	    Parser,TERROR(expectedEqualSignInAttrib(Name))
	 end
      end

      %% we have just read </
      meth ScanEtag(Tag Coord) Name in
	 Parser,SkipSpaces
	 Parser,ScanName(Name)
	 Parser,SkipSpaces
	 case Parser,GET($)
	 of &> then
	    Tag=etag(Name Coord)
	 else
	    Parser,TERROR(expectedEtagRangle)
	 end
      end

      %% we have just read <
      meth ScanStag(Tag Coord) Name Alist Empty in
	 Tag=stag(Name Alist Empty Coord)
	 Parser,SkipSpaces
	 Parser,ScanName(Name)
	 Parser,ScanAlist(Alist Empty)
      end

      meth ScanAlist(Alist Empty)
	 Parser,SkipSpaces
	 case Parser,PEEK($)
	 of false then
	    Parser,TERROR(alistEOF)
	 [] &/ then
	    Parser,GET(_)
	    case Parser,GET($)
	    of false then
	       Parser,TERROR(alistEOF)
	    [] &> then Alist=nil Empty=true
	    else
	       Parser,TERROR(expectedEmptyTagRangle)
	    end
	 [] &> then
	    Parser,GET(_)
	    Alist=nil Empty=false
	 else Name Value Alist2 in
	    Alist=(Name|Value)|Alist2
	    Parser,ScanAttr(Name Value)
	    Parser,ScanAlist(Alist2 Empty)
	 end
      end

      %% the tokenizer will not guarantee the invariant that all
      %% all adjacent text tokens are collapsed.  Instead, we will
      %% leave that job to a higher abstraction.

      meth ScanText(Token Coord) Text in
	 Token=text(Text Coord)
	 Parser,ScanTEXT(Text)
      end

      meth ScanTEXT(Text) C in
	 Parser,GET(C)
	 if C==false then Text=nil
	 elseif C==&& orelse C==&< then
	    Text=nil
	    Parser,UNGET(C)
	 else Text2 in
	    Text=C|Text2
	    Parser,ScanTEXT(Text2)
	 end
      end

      %% we have just read <![
      meth ScanCDATA(Text Coord)
	 raise notImplemented(scanCDATA) end
      end

      %% we have just read <!--
      meth ScanComment(Comment Coord) Text in
	 Comment=comment(Text Coord)
	 Parser,ScanCOMMENT(Text)
      end

      meth ScanCOMMENT(Text) C1 in
	 Parser,GET(C1)
	 case C1
	 of false then
	    Parser,TERROR(commentEOF)
	 [] &- then C2 in
	    Parser,GET(C2)
	    case C2
	    of false then
	       Parser,TERROR(commentEOF)
	    [] &- then C3 in
	       Parser,GET(C3)
	       case C3
	       of false then
		  Parser,TERROR(commentEOF)
	       [] &> then Text=nil
	       else Text2 in
		  Text=C1|Text2
		  Parser,UNGET(C3)
		  Parser,UNGET(C2)
		  Parser,ScanCOMMENT(Text2)
	       end
	    else Text2 in
	       Text=C1|C2|Text2
	       Parser,ScanCOMMENT(Text2)
	    end
	 else Text2 in
	    Text=C1|Text2
	    Parser,ScanCOMMENT(Text2)
	 end
      end

      %% we have just read <?
      meth ScanPI(PI Coord) Text in
	 PI=pi(Parser,ScanName($) Text Coord)
	 Parser,ScanPIX(Text)
      end

      meth ScanPIX(Text) C1 in
	 Parser,GET(C1)
	 case C1
	 of false then
	    Parser,TERROR(piEOF)
	 [] &? then C2 in
	    Parser,GET(C2)
	    case C2
	    of false then
	       Parser,TERROR(piEOF)
	    [] &> then Text=nil
	    else Text2 in
	       Text=C1|Text2
	       Parser,UNGET(C2)
	       Parser,ScanPIX(Text2)
	    end
	 else Text2 in
	    Text=C1|Text2
	    Parser,ScanPIX(Text2)
	 end
      end

      meth GetToken(Token) C1 Coord in
	 Parser,GET(C1)
	 Parser,GetCoord(Coord)
	 case C1
	 of false then Token=unit
	 [] &< then C2 in
	    Parser,GET(C2)
	    case C2
	    of false then
	       Parser,TERROR(tokenEOF)
	    [] &/ then Parser,ScanEtag(Token Coord)
	    [] &? then Parser,ScanPI(Token Coord)
	    [] &! then C3 in
	       Parser,GET(C3)
	       case C3
	       of false then
		  Parser,TERROR(tokenEOF)
	       [] &- then C4 in
		  Parser,GET(C4)
		  case C4
		  of false then
		     Parser,TERROR(tokenEOF)
		  [] &- then
		     Parser,ScanComment(Token Coord)
		  else
		     Parser,TERROR(expectedComment)
		  end
	       [] &[ then
		  Parser,ScanCDATA(Token Coord)
	       else
		  Parser,UNGET(C3)
		  Token=doctype(Coord)
		  Parser,ScanDoctype
	       end
	    else
	       Parser,UNGET(C2)
	       Parser,ScanStag(Token Coord)
	    end
	 [] && then
	    case Parser,ScanEntityRef($)
	    of string(S) then Token=text(S Coord)
	    [] pi(S)     then Token=pi(S Coord)
	    [] system(S) then
	       Parser,PushFile(S)
	       Parser,GetToken(Token)
	    end
	 else
	    Parser,UNGET(C1)
	    Parser,ScanText(Token Coord)
	 end
      end

      %% we have just read <!
      meth ScanDoctype Name in
	 Parser,ScanName(Name)
	 case {Upcase Name}
	 of "DOCTYPE" then
	    Parser,ScanDOCTYPE
	 else
	    Parser,TERROR(expectedDoctype)
	 end
      end

      meth ScanDOCTYPE
	 Parser,SkipSpaces
	 case Parser,PEEK($)
	 of false then
	    Parser,TERROR(doctypeEOF)
	 [] &[ then
	    Parser,GET(_)
	    Parser,ScanDoctypeBody
	 [] &" then
	    Parser,ScanDelim(&" _)
	    Parser,ScanDOCTYPE
	 [] &' then
	    Parser,ScanDelim(&' _)
	    Parser,ScanDOCTYPE
	 else
	    Parser,ScanName(_)
	    Parser,ScanDOCTYPE
	 end
      end

      meth ScanDoctypeBody
	 Parser,SkipSpaces
	 case Parser,GET($)
	 of false then
	    Parser,TERROR(doctypeEOF)
	 [] &] then
	    Parser,SkipSpaces
	    case Parser,GET($)
	    of false then
	       Parser,TERROR(doctypeEOF)
	    [] &> then skip
	    else
	       Parser,TERROR(expectedDoctypeRangle)
	    end
	 [] &< then
	    case Parser,GET($)
	    of false then
	       Parser,TERROR(doctypeEOF)
	    [] &! then
	       if Parser,PEEK($)==&- then
		  Parser,GET(_)
		  case Parser,GET($)
		  of false then
		     Parser,TERROR(doctypeEOF)
		  [] &- then
		     Parser,ScanComment(_)
		     Parser,ScanDoctypeBody
		  [] C then
		     Parser,TERROR(unexpectCharInDoctype(C))
		  end
	       else S in
		  Parser,ScanName(S)
		  case {Upcase S}
		  of "ENTITY" then Name Type Value in
		     Parser,SkipSpaces
		     Parser,ScanName(Name)
		     Parser,SkipSpaces
		     if {Member Parser,PEEK($) [&' &"]} then
			Type = string
		     else TName in
			Parser,ScanName(TName)
			case {Upcase TName}
			of "SYSTEM" then Type=system
			[] "PI"     then Type=pi
			else
			   Parser,TERROR(unpexpectedEntityType(TName))
			end
		     end
		     Parser,SkipSpaces
		     Parser,ScanValue(Value)
		     @EntityTable.{StringToAtom Name} := Type(Value)
		     Parser,SkipSpaces
		     case Parser,GET($)
		     of false then
			Parser,TERROR(entityDeclEOF)
		     [] &> then
			Parser,ScanDoctypeBody
		     [] C then
			Parser,TERROR(unpextecCharInEntityDecl(C))
		     end
		  else
		     %% for anything else, we skip until the next >
		     Parser,SkipToRangle
		     Parser,ScanDoctypeBody
		  end
	       end
	    [] C then
	       Parser,TERROR(expectedBangInDoctype(C))
	    end
	 [] C then
	    Parser,TERROR(unexpectedCharInDoctype(C))
	 end
      end

      meth SkipToRangle
	 case Parser,GET($)
	 of &> then skip
	 else Parser,SkipToRangle end
      end

      %% =============================================================
      %% NAMESPACE MANAGER
      %% =============================================================

      %% =============================================================
      %% Intern(+Name ?QName)
      %%     takes a string Name and returns the unique record
      %% describing the fully qualified name:
      %%
      %%          qname( uri:URI name:LOC xname:XLOC )
      %%
      %% where URI is the uri bound to the prefix, LOC is the local
      %% part of the name, and XLOC is the atom obtained by concatenating
      %% LOC followed by ' @ ' followed by URI. XLOC can be used as
      %% a key uniquely identifying the name.
      %% =============================================================

      meth Intern(Name $)
	 Prefix Suffix
	 %% !!! IS IT REALLY MORE EFFICIENT TO TEST FIRST?
	 if {Member &: Name}
	 then {StringToken Name &: Prefix Suffix}
	 else Prefix=nil Suffix=Name end
	 PrefixA={StringToAtom Prefix}
	 SuffixA={StringToAtom Suffix}
      in
	 Parser,InternExplicit(PrefixA SuffixA $)
      end

      meth InternExplicit(PrefixA SuffixA $)
	 MaybeURI={CondSelect @PrefixTable PrefixA unit}
	 URI=if MaybeURI==unit then
		%% namespace prefix not declared, we let it default
		%% to denoting a uri that is string-equal to the prefix
		@PrefixTable.PrefixA := PrefixA
		PrefixA
	     else MaybeURI end
	 FullName = {VS2A SuffixA#' @ '#URI}
	 R1={CondSelect @NameTable FullName unit}
      in
	 if R1==unit then
	    R=qname(
		 uri   : URI
		 name  : SuffixA
		 xname : FullName)
	 in
	    @NameTable.FullName := R
	    R
	 else R1 end
      end

      %%--------------------------------------------------------------
      %% ProcessElement(+Tag1 +Alist1 ?Tag2 ?Alist2)
      %%     Tag1 is a string representing the tag as written, Alist1
      %% is the list of attributes where each attribute identifier is
      %% also a string representing it as written.
      %%
      %% Tag2 is the Intern'ed form of Tag1 and Alist2 is alist of
      %% attributes where each attribute identifier has been Intern'ed
      %% and local namespace declarations have been taken into account
      %% and removed from Alist2.
      %%--------------------------------------------------------------

      meth ProcessElement(Tag1 Alist1 Tag2 Alist2)
	 AlistMid
      in
	 NeedCopy <- true
	 Parser,ProcessAlist(Alist1 AlistMid)
	 Parser,Intern(Tag1 Tag2)
	 Parser,InternAlist(AlistMid Alist2)
      end

      meth InternAlist(Alist $)
	 case Alist
	 of nil then nil
	 [] (PrefA#NamA#ValA)|L then
	    ((Parser,InternExplicit(PrefA NamA $))|ValA)
	    |(Parser,InternAlist(L $))
	 end
      end

      %%--------------------------------------------------------------
      %% ProcessAlist(+AlistIN ?AlistOUT)
      %%     takes as input a list where each element is of the form
      %% (Attr|Value) where both Attr and Value are strings and returns
      %% a list where each element is of the form (PrefA#NamA#ValA)
      %% where all 3 components are atoms and PrefA is the namespace
      %% prefix, NamA is the local part of the attribute name, and
      %% ValA is the value.
      %%--------------------------------------------------------------

      meth ProcessAlist(AlistIN AlistOUT)
	 case AlistIN
	 of nil then
	    AlistOUT=nil
	    %% we keep the same prefix table for this scope
	    %% since there were no namespace declarations
	    PSTACK <- @PrefixTable|@PSTACK
	 [] (Attr|Value)|AlistIN then
	    Prefix Suffix
	    if {Member &: Attr}
	    then {StringToken Attr &: Prefix Suffix}
	    else Prefix=nil Suffix=Attr end
	    PrefixA={VS2A Prefix}
	    SuffixA={VS2A Suffix}
	    ValueA={VS2A Value}
	 in
	    %% check if this is a namespace declaration
	    if Prefix==nil then
	       if {HasFeature XMLNS SuffixA} then
		  %% default namespace declaration
		  %% we clone the prefix table and use
		  %% the updated clone for this scope
		  if @NeedCopy then
		     NeedCopy    <- false
		     PSTACK      <- @PrefixTable|@PSTACK
		     PrefixTable <- {DictClone @PrefixTable}
		  end
		  PrefixTable.'' := {VS2A Value}
		  Parser,ProcessAlist(AlistIN AlistOUT)
	       else
		  AlistOUT2
	       in
		  AlistOUT=(PrefixA#SuffixA#ValueA)|AlistOUT2
		  Parser,ProcessAlist(AlistIN AlistOUT2)
	       end
	    elseif {HasFeature XMLNS PrefixA} then
	       %% namespace declaration
	       %% if the declaration is really new, then
	       %% again we need to clone the prefix table
	       %% an use the updated clone for this scope
	       if {CondSelect @PrefixTable SuffixA unit}==ValueA then
		  Parser,ProcessAlist(AlistIN AlistOUT)
	       else
		  if @NeedCopy then
		     NeedCopy    <- false
		     PSTACK      <- @PrefixTable|@PSTACK
		     PrefixTable <- {DictClone @PrefixTable}
		  end
		  PrefixTable.SuffixA := ValueA
		  Parser,ProcessAlist(AlistIN AlistOUT)
	       end
	    else
	       AlistOUT2
	    in
	       AlistOUT=(PrefixA#SuffixA#ValueA)|AlistOUT2
	       Parser,ProcessAlist(AlistIN AlistOUT2)
	    end
	 end
      end

      %% =============================================================
      %% PARSER
      %% =============================================================

      meth ERROR(R) raise xml(parser:R) end end

      meth BuildAlist(L $)
	 case L
	 of nil then nil
	 [] (QAttr|ValA)|LL then I=@INDEX in
	    INDEX <- I+1
	    attribute(
	       name   : QAttr
	       value  : ValA
	       parent : @PARENT
	       index  : I)
	    | Parser,BuildAlist(LL $)
	 end
      end

      meth PARSE()
	 case (T=@LOOK in
		 if T==unit then Parser,GetToken($)
		 else LOOK<-unit T end)
	 of unit then
	    @CONTENTS=nil
	    if @TAG\=unit then
	       Parser,ERROR(nonTerminatedElement(@PARENT))
	    end
	 [] stag(Name Alist Empty Coord) then
	    Tag2 Alist2 Elem Children Tail Alist3 OParent
	 in
	    Parser,ProcessElement(Name Alist Tag2 Alist2)
	    Elem = element(
		      tag      : Tag2
		      alist    : Alist3
		      children : Children
		      coord    : Coord
		      index    : @INDEX
		      parent   : @PARENT)
	    OParent=@PARENT
	    PARENT <- Elem
	    %% this first
	    INDEX <- @INDEX+1
	    %% then this, else indices are wrong
	    Parser,BuildAlist(Alist2 Alist3)
	    @CONTENTS=Elem|Tail
	    if Empty then
	       Children=nil
	       CONTENTS <- Tail
	       PARENT   <- OParent
	    else
	       CONTENTS <- Children
	       STACK    <- (Tail|OParent)|@STACK
	       TAG      <- Tag2.xname
	    end
	    Parser,PARSE()
	 [] etag(Name Coord) then Tag in
	    Parser,Intern(Name Tag)
	    if @TAG\=Tag.xname then
	       Parser,ERROR(mismatchedEtag(found:Tag.xname wanted:@TAG coord:Coord))
	    else
	       @CONTENTS=nil
	       case @STACK
	       of (Tail|Par)|Stack then
		  CONTENTS <- Tail
		  PARENT   <- Par
		  TAG      <- {CondSelect {CondSelect Par tag unit} xname unit}
		  STACK    <- Stack
		  Parser,PARSE()
	       else Parser,ERROR(unexpectedErrorAtEtag(Tag Coord)) end
	       case @PSTACK
	       of H|T then PrefixTable<-H PSTACK<-T
	       else Parser,ERROR(unexpectedEndOfPSTACK) end
	    end
	 [] pi(Target Args Coord) then L in
	    @CONTENTS = pi(name   : {StringToAtom Target}
			   data   : {StringToAtom Args}
			   coord  : Coord
			   parent : @PARENT
			   index  : @INDEX)|L
	    INDEX    <- @INDEX+1
	    CONTENTS <- L
	    Parser,PARSE()
	 [] text(Chars Coord) then Parser,PARSETXT(Chars Coord)
	 [] comment(_ _) then
	    %% ignore
	    Parser,PARSE()
	 end
      end

      meth PARSETXT(VS Coord)
	 %% when invoking PARSETXT, there is never a saved @LOOK token
	 Tok = Parser,GetToken($)
      in
	 case Tok
	 of text(Chars _) then Parser,PARSETXT(VS#Chars Coord)
	 else
	    LOOK <- Tok
	    %% are we in an element where white space text nodes should
	    %% be stripped? the @STRIP table is supposed to map xnames
	    %% to true.
	    if {CondSelect @STRIP @TAG false} then
	       SS = {VS2SFast VS}
	    in
	       if {AllSpaces SS} then skip else L in
		  @CONTENTS = text(data   : {MakeBS SS}
				   coord  : Coord
				   parent : @PARENT
				   index  : @INDEX)|L
		  INDEX    <- @INDEX+1
		  CONTENTS <- L
	       end
	    else
	       L in
	       @CONTENTS = text(data   : {MakeBS VS}
				coord  : Coord
				parent : @PARENT
				index  : @INDEX)|L
	       INDEX    <- @INDEX+1
	       CONTENTS <- L
	    end
	    Parser,PARSE()
	 end
      end
   end
import
   Open(file:File)
define
   fun {FileOpen FName}
      {New File init(url:FName)}
   end
   proc {NewParser Init Root}
      {New Parser {Adjoin Init init(root:Root fileopen:FileOpen)} _}
   end
end