functor
export
   SpaceManager parser:XMLParser
require
   URL(make     : URLMake
       toString : URLToString
       resolve  : URLResolve)
prepare

   %% ================================================================
   %% a SpaceManager can answer the question: "should isolated text
   %% nodes consisting of only whitespace be discarded?"  The question
   %% must be parametrized by a URI (for the namespace) and a Tag (for
   %% the name of the element in which the whitespace text node
   %% occurs).
   %%
   %% stripSpace(+URI +Tag)
   %% preserveSpace(+URI +Tag)
   %%
   %%     are methods used to state rules that the manager will use in
   %% answering questions. stripSpace(URI Tag) says that whitespace
   %% must be discarded when occurring in element Tag of namespace URI.
   %% For an element not in any namespace (this is different from an
   %% element in the default namespace, if any) URI=unit.
   %%     it is possible to use wildcards '*' for either or both of
   %% the arguments.  stripSpace(URI '*') says that whitespace nodes
   %% should be discarded for all elements in namespace URI.  It is
   %% possible to state additional rules to overrule in more specific
   %% cases.  For example:
   %%
   %%     stripSpace(URI '*')
   %%     preserveSpace(URI 'code')
   %%
   %% state that whitespace text nodes should be ignored in all elements
   %% of namespace URI, except for element "code" in which they should
   %% be preserved.  Which rule takes precedence?  Here is the hierarchy:
   %%
   %%     Low    ('*' '*')
   %%     Medium (URI *) ('*' Tag)
   %%     High   (URI Tag)
   %%
   %% there can be an ambiguity on Medium, in which case an error is
   %% raised when the question is asked.
   %%
   %% askStripSpace(+URI +Tag ?Bool)
   %% askPreserveSpace(+URI +Tag ?Bool)
   %%
   %%     are methods used to ask questions. No wild card can be used
   %% here of course.
   %% ================================================================
   
   class SpaceManager
      attr table
      meth init table<-{NewDictionary} end
      meth Put(URI Tag Bool)
	 SubTable = {CondSelect @table URI unit}
      in
	 if SubTable==unit then
	    SubTable={NewDictionary}
	 in
	    SubTable.Tag := Bool
	    @table.URI := SubTable
	 else
	    SubTable.Tag := Bool
	 end
      end
      meth Get(URI Tag $)
	 {CondSelect {CondSelect @table URI unit} Tag unit}
      end
      %% should we strip spaces?
      meth Ask(URI Tag $)
	 B1 = SpaceManager,Get(URI Tag $)
      in
	 if B1\=unit then B1 else
	    B2=SpaceManager,Get('*' Tag $)
	    B3=SpaceManager,Get(URI '*' $)
	 in
	    if B2\=unit then
	       if B3\=unit andthen B2\=B3 then
		  raise spaceManager(ambiguity(URI Tag)) end
	       else B2 end
	    elseif B3\=unit then B3
	    else B4=SpaceManager,Get('*' '*' $) in
	       if B4\=unit then B4 else false end
	    end
	 end
      end
      meth stripSpace(URI Tag)
	 SpaceManager,Put(URI Tag true)
      end
      meth preserveSpace(URI Tag)
	 SpaceManager,Put(URI Tag false)
      end
      meth askStripSpace(URI Tag $)
	 SpaceManager,Ask(URI Tag $)
      end
      meth askPreserveSpace(URI Tag $)
	 {Not SpaceManager,Ask(URI Tag $)}
      end
   end

   %%

   proc {NOOP} skip end

   MakeBS = ByteString.make
   DictRemove = Dictionary.remove
   RecToDict = Record.toDictionary
   VS2A = VirtualString.toAtom
   VS2S = VirtualString.toString
   CharIsAlNum = Char.isAlNum
   CharIsSpace = Char.isSpace

   %% ================================================================
   %% used to split the target of a PI from its arguments: split at,
   %% and remove, the first sequence of whitespaces
   %% ================================================================

   fun {Split S L}
      case S
      of nil then L=nil nil
      [] H|T then
	 if {CharIsSpace H} then L=nil {DropSpaces T}
	 else L2 in L=H|L2 {Split T L2} end
      end
   end
   fun {DropSpaces L}
      case L
      of nil then nil
      [] H|T then
	 if {CharIsSpace H}
	 then {DropSpaces T}
	 else L end
      end
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
   o(amp : text("\&")
     lt  : text("<")
     gt  : text(">")
     apos: text("'")
     quot: text("\""))

   %% ================================================================
   %% it's extremely annoying, but XML says that attribute xmlns can
   %% be be spelled with any capitalization.  Here, we provide a table
   %% to quickly check whether a given atom is really one of the 32
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
   %% namespace utils
   %% ================================================================

   %% like {Member &: S} but specialized for speed
   fun {HasColon S}
      case S of nil then false
      [] H|T then H==&: orelse {HasColon T} end
   end

   %% line {StringToken S &: Prefix Suffix} but specialized for speed
   proc {SplitName S Prefix Suffix}
      case S of H|T then
	 if H==&: then Prefix=nil Suffix=T
	 else Prefix2 in
	    Prefix=H|Prefix2
	    {SplitName T Prefix2 Suffix}
	 end
      end
   end

   %% ================================================================
   %% the big tokenizer+parser combo
   %% ================================================================

   class Parser
      attr
	 %%-----------------------------------------------------------
	 %% tokenizer
	 %%-----------------------------------------------------------
	 Filename     : unit	% URL of current input
	 Buffer       : nil	% buffered input
	 Line         : 1	% current line in current file
	 Stack        : nil	% stack of interrupted inputs
	 EntityTable  : unit	% table mapping entity names to values
	 Coord        : unit	% coord(@Filename @Line) saved for reuse in same line
	 SavedCoord   : unit
	 SavedToken   : unit
	 %%-----------------------------------------------------------
	 %% parser
	 %%-----------------------------------------------------------
	 STACK       : unit
	 STRIP       : unit
	 LOOK        : unit
	 TAG         : unit
	 CONTENTS    : nil
	 COORD       : unit
	 TRAIL       : nil
	 PREFIXTABLE : unit
	 ALIST       : nil
	 TAGS        : nil
	 keepComments : false
	 keepNamespaceDeclarations : false

      meth init() skip end
      
      meth setSpaceManager(M) STRIP<-M end
      meth setKeepComments(B<=true) keepComments<-B end
      meth setKeepNamespaceDeclarations(B<=true) keepNamespaceDeclarations<-B end

      meth parseVS(VS $)
	 Parser,Reset()
	 Filename <- unit
	 Buffer <- {VS2S VS}
	 Parser,Parse($)
      end
      meth parseFile(F $)
	 {self parseURL(F $)}
      end
      meth parseURL(UserURL $)
	 U = {URLMake UserURL}
      in
	 Parser,Reset()
	 Filename <- {VS2A {URLToString U}}
	 Buffer <- {self openWithTail(U nil NOOP $)}
	 Parser,Parse($)
      end
      
      meth Reset()
	 PREFIXTABLE  <- {NewDictionary}
	 TRAIL        <- nil
	 Line         <- 1
	 Stack        <- nil
	 Coord        <- unit
	 SavedCoord   <- unit
	 SavedToken   <- unit
	 EntityTable  <- {RecToDict DefaultEntityRecord}
      end

      meth Parse(?L)
	 if @STRIP==unit then
	    STRIP<-{New SpaceManager init}
	 end
	 TAG <- unit
	 CONTENTS <- L
	 {self onStartDocument}
	 Parser,PARSE()
	 {self onEndDocument}
	 @CONTENTS=nil
      end

      %% =============================================================
      %% TOKENIZER
      %% =============================================================

      meth GetCoord($) C=@Coord in
	 if C==unit
	 then C=coord(@Filename @Line) in Coord<-C C
	 else C end
      end

      meth PushFile(FName)
	 CurrentFilename = @Filename
	 CurrentLine     = @Line
	 FullURL         = {URLResolve CurrentFilename FName}
	 FullFName       = {VS2A {URLToString FullURL}}
      in
	 Buffer   <- {self openWithTail(
			      FullURL @Buffer
			      proc {$}
				 Filename <- CurrentFilename
				 Line     <- CurrentLine
			      end
			      $)}
	 Line     <- 1
	 Filename <- FullFName
      end

      meth TERROR(R)
	 raise xml(tokenizer(file : @Filename
			     line : @Line
			     info : R))
	 end
      end

      %% When skipping spaces we need to keep track of lines

      meth SkipSpaces
	 case @Buffer
	 of nil then skip
	 [] H|T then
	    if {CharIsSpace H} then
	       if H==&\n then Line<-@Line+1 Coord<-unit end
	       Buffer<-T Parser,SkipSpaces
	    end
	 end
      end

      %% we have just encountered a & and now we must recognize the
      %% entity reference.  returns:
      %% charref(C) for character references
      %% entityref(A) for named entity references
      %% someone else must do the interpretation

      meth ScanEntityRef($)
	 case @Buffer
	 of &#|&x|L then Buffer<-L Parser,ScanCharRefHex(0 $)
	 [] &#|L    then Buffer<-L Parser,ScanCharRefDec(0 $)
	 else Name in
	    Parser,ScanName(Name)
	    Parser,SkipSpaces
	    case @Buffer
	    of &;|L then Buffer<-L Key={VS2A Name}
	       Def={CondSelect @EntityTable Key unit}
	    in
	       if Def==unit then
		  Parser,TERROR(unknownEntityRef(Key)) unit
	       else Def end
	    else
	       Parser,TERROR(expectedRefSemiColon) unit
	    end
	 end
      end

      meth ScanCharRefHex(Accu $)
	 case @Buffer
	 of H|T then
	    Buffer<-T
	    if H==&; then Buffer<-T charref(Accu)
	    else I={CondSelect HexToInt H unit} in
	       if I==unit then
		  Parser,TERROR(badCharInCharRef(H)) unit
	       else
		  Parser,ScanCharRefHex(Accu*16+I $)
	       end
	    end
	 else
	    Parser,TERROR(charrefEOF) unit
	 end
      end

      meth ScanCharRefDec(Accu $)
	 case @Buffer
	 of H|T then
	    Buffer<-T
	    if H==&; then charref(Accu)
	    else I={CondSelect DecToInt H unit} in
	       if I==unit then
		  Parser,TERROR(badCharInCharRef(H)) unit
	       else
		  Parser,ScanCharRefDec(Accu*10+I $)
	       end
	    end
	 else
	    Parser,TERROR(charrefEOF) unit
	 end
      end

      meth ScanSQ($)
	 case @Buffer
	 of nil then Parser,TERROR(quotedValueEOF) unit
	 [] H|T then Buffer<-T
	    if H==&' then nil else
	       if H==&\n then Line<-@Line+1 Coord<-unit end
	       H|(Parser,ScanSQ($))
	    end
	 end
      end
      
      meth ScanDQ($)
	 case @Buffer
	 of nil then Parser,TERROR(quotedValueEOF) unit
	 [] H|T then Buffer<-T
	    if H==&" then nil else
	       if H==&\n then Line<-@Line+1 Coord<-unit end
	       H|(Parser,ScanDQ($))
	    end
	 end
      end
      
      meth ScanSQExpand($)
	 case @Buffer
	 of nil then Parser,TERROR(quotedValueEOF) unit
	 [] H|T then Buffer<-T
	    case H
	    of &' then nil
	    [] && then
	       case Parser,ScanEntityRef($)
	       of charref(C) then C|(Parser,ScanSQExpand($))
	       [] text(S) then {Append S Parser,ScanSQExpand($)}
	       [] Tok then Parser,TERROR(illegalEntityRefInQuotedValue(Tok)) unit
	       end
	    else
	       if H==&\n then Line<-@Line+1 Coord<-unit end
	       H|(Parser,ScanSQExpand($))
	    end
	 end
      end

      meth ScanDQExpand($)
	 case @Buffer
	 of nil then Parser,TERROR(quotedValueEOF) unit
	 [] H|T then Buffer<-T
	    case H
	    of &" then nil
	    [] && then
	       case Parser,ScanEntityRef($)
	       of charref(C) then C|(Parser,ScanDQExpand($))
	       [] text(S) then {Append S Parser,ScanDQExpand($)}
	       [] Tok then Parser,TERROR(illegalEntityRefInQuotedValue(Tok)) unit
	       end
	    else
	       if H==&\n then Line<-@Line+1 Coord<-unit end
	       H|(Parser,ScanDQExpand($))
	    end
	 end
      end

      meth ScanValue($)
	 case @Buffer
	 of &"|L then Buffer<-L Parser,ScanDQ($)
	 [] &'|L then Buffer<-L Parser,ScanSQ($)
	 else Parser,TERROR(expectedQuotedValue) unit end
      end

      meth ScanValueExpand($)
	 case @Buffer
	 of &"|L then Buffer<-L Parser,ScanDQExpand($)
	 [] &'|L then Buffer<-L Parser,ScanSQExpand($)
	 else Parser,TERROR(expectedQuotedValue) unit end
      end

      meth ScanName($)
	 case @Buffer
	 of H|T andthen {IsNameChar H} then
	    Buffer<-T
	    H|(Parser,ScanName($))
	 else nil end
      end

      meth ScanAttr(Name Value)
	 Parser,SkipSpaces
	 Parser,ScanName(Name)
	 Parser,SkipSpaces
	 case @Buffer
	 of &=|L then Buffer<-L
	    Parser,SkipSpaces
	    Parser,ScanValueExpand(Value)
	 else
	    Parser,TERROR(expectedEqualSignInAttrib(Name))
	 end
      end

      %% we have just read </
      meth ScanEtag($) Name in
	 %% Parser,SkipSpaces
	 Parser,ScanName(Name)
	 Parser,SkipSpaces
	 case @Buffer
	 of &>|L then Buffer<-L etag(Name @SavedCoord)
	 else Parser,TERROR(expectedEtagRangle) unit end
      end

      %% we have just read <
      meth ScanStag(Tag) Name Alist Empty in
	 Tag=stag(Name Alist Empty @SavedCoord)
	 %% Parser,SkipSpaces
	 Parser,ScanName(Name)
	 Parser,ScanAlist(Alist Empty)
      end

      meth ScanAlist(Alist Empty)
	 Parser,SkipSpaces
	 case @Buffer
	 of nil     then Parser,TERROR(alistEOF)
	 [] &/|&>|L then Buffer<-L Alist=nil Empty=true
	 [] &>|L    then Buffer<-L Alist=nil Empty=false
	 else Name Value Alist2 Coord=Parser,GetCoord($) in
	    Alist=(Name#Value#Coord)|Alist2
	    Parser,ScanAttr(Name Value)
	    Parser,ScanAlist(Alist2 Empty)
	 end
      end

      %% the tokenizer guarantees the invariant that there are no
      %% consecutive text tokens.  They are automatically collapsed
      %% on the fly.

      meth ScanText(Token) Text in
	 Token=text(Text @SavedCoord)
	 Parser,ScanTEXT(Text)
      end

      meth ScanTEXT($)
	 case @Buffer
	 of nil then nil
	 [] H|T then
	    case H
	    of &\n then Buffer<-T Line<-@Line+1 Coord<-unit
	       H|(Parser,ScanTEXT($))
	    [] &&  then
	       Buffer<-T
	       case Parser,ScanEntityRef($)
	       of charref(C) then C|(Parser,ScanTEXT($))
	       [] text(S) then {Append S Parser,ScanTEXT($)}
	       [] system(S) then
		  Parser,PushFile(S)
		  Parser,ScanTEXT($)
	       [] pi(Name Args) then
		  SavedToken <- pi(Name Args Parser,GetCoord($))
		  nil
	       [] Def then
		  Parser,TERROR(bizzareEntityDef(Def)) unit
	       end
	    [] &< then nil
	    else Buffer<-T H|(Parser,ScanTEXT($)) end
	 end
      end

      %% we have just read <![
      meth ScanCDATA(Text)
	 raise notImplemented(scanCDATA) end
      end

      %% we have just read <!--
      meth ScanComment(Comment) Text in
	 Comment=comment(Text @SavedCoord)
	 Parser,ScanCOMMENT(Text)
      end

      meth ScanCOMMENT($)
	 case @Buffer
	 of nil then Parser,TERROR(commentEOF) unit
	 [] &-|&-|&>|L then Buffer<-L nil
	 [] H|L then
	    Buffer<-L
	    if H==&\n then Line<-@Line+1 Coord<-unit end
	    H|(Parser,ScanCOMMENT($))
	 end
      end

      %% we have just read <?
      meth ScanPI(Tok) Data in
	 Tok=pi(Parser,ScanName($) Data @SavedCoord)
	 Parser,SkipSpaces
	 Parser,ScanPIX(Data)
      end

      meth ScanPIX($)
	 case @Buffer
	 of nil then Parser,TERROR(piEOF) unit
	 [] &?|&>|L then Buffer<-L nil
	 [] H|L then
	    Buffer<-L
	    if H==&\n then Line<-@Line+1 Coord<-unit end
	    H|(Parser,ScanPIX($))
	 end
      end

      meth ScanEntityRefToken(Tok)
	 case Parser,ScanEntityRef($)
	 of charref(C) then Data in
	    Tok=text(C|Data @SavedCoord)
	    Parser,ScanTEXT(Data)
	 [] text(S) then Data in
	    Tok=text({Append S Data} @SavedCoord)
	    Parser,ScanTEXT(Data)
	 [] pi(Name Data) then
	    Tok=pi(Name Data @SavedCoord)
	 [] system(S) then
	    Parser,PushFile(S)
	    Parser,ScanToken(Tok)
	 [] Def then
	    Parser,TERROR(bizzareEntityDef(Def))
	 end
      end

      meth ScanToken($)
	 SavedCoord <- Parser,GetCoord($)
	 case @Buffer
	 of nil           then unit
	 [] &<|&/|L       then Buffer<-L Parser,ScanEtag($)
	 [] &<|&?|L       then Buffer<-L Parser,ScanPI($)
	 [] &<|&!|&-|&-|L then Buffer<-L Parser,ScanComment($)
	 [] &<|&!|&[|L    then Buffer<-L Parser,ScanCDATA($)
	 [] &<|&!|L       then Buffer<-L Parser,ScanDoctype($)
	 [] &<|L          then Buffer<-L Parser,ScanStag($)
	 [] &&|L          then Buffer<-L Parser,ScanEntityRefToken($)
	 else                            Parser,ScanText($) end
      end

      meth GetToken($)
	 %% @SavedToken is normally unit, it is pi(...) when
	 %% the last token was text(...) and an entity ref
	 %% was scanned and then discovered to be a PI.
	 case @SavedToken
	 of unit then Parser,ScanToken($)
	 [] Tok  then SavedToken<-unit Tok
	 end
      end

      %% we have just read <!
      meth ScanDoctype($)
	 case @Buffer
	 of &D|&O|&C|&T|&Y|&P|&E|L then Buffer<-L
	    Parser,ScanDoctypeHead($)
	 else Parser,TERROR(expectedDoctype) unit end
      end

      meth ScanDoctypeHead($)
	 Parser,SkipSpaces
	 case @Buffer
	 of nil then Parser,TERROR(doctypeEOF) unit
	 [] H|T then
	    Buffer<-T
	    case H
	    of &[ then Parser,ScanDoctypeBody($)
	    [] &" then Parser,ScanDQ(_) Parser,ScanDoctypeHead($)
	    [] &' then Parser,ScanSQ(_) Parser,ScanDoctypeHead($)
	    [] &> then Parser,ScanToken($)
	    else Parser,ScanName(_) Parser,ScanDoctypeHead($)
	    end
	 end
      end

      meth ScanDoctypeBody($)
	 Parser,SkipSpaces
	 case @Buffer
	 of nil then Parser,TERROR(doctypeEOF) unit
	 [] &]|L then
	    Buffer<-L
	    Parser,SkipSpaces
	    case @Buffer of &>|L then
	       Buffer<-L
	       Parser,ScanToken($)
	    end
	 %%[] &]|&>|L then Buffer<-L Parser,ScanToken($)
	 [] &<|&!|&E|&N|&T|&I|&T|&Y|L then Name in Buffer<-L
	    Parser,SkipSpaces
	    Parser,ScanName(Name)
	    Parser,SkipSpaces
	    case @Buffer
	    of &P|&I|L then Data Fun Arg in Buffer<-L
	       Parser,SkipSpaces
	       Parser,ScanValue(Data)
	       {Split Data Fun Arg}
	       @EntityTable.{VS2A Name} := pi(Fun Arg)
	    [] &S|&Y|&S|&T|&E|&M|L then Data in Buffer<-L
	       Parser,SkipSpaces
	       Parser,ScanValue(Data)
	       @EntityTable.{VS2A Name} := system(Data)
	    else Data in
	       Parser,ScanValue(Data)
	       @EntityTable.{VS2A Name} := text(Data)
	    end
	    Parser,SkipSpaces
	    case @Buffer
	    of nil then Parser,TERROR(entityDeclEOF) unit
	    [] &>|L then Buffer<-L Parser,ScanDoctypeBody($)
	    [] H|_ then Parser,TERROR(unexpectedEntityDeclRangle(H)) unit
	    end
	 [] &<|&!|&-|&-|L then Buffer<-L
	    Parser,ScanCOMMENT(_)
	    Parser,ScanDoctypeBody($)
	 [] &<|L then Buffer<-L
	    Parser,SkipToRangle
	    Parser,ScanDoctypeBody($)
	 [] C|_ then Parser,TERROR(unexpectedCharInDoctype(C)) unit
	 end
      end

      meth SkipToRangle
	 case @Buffer
	 of nil then Parser,TERROR(doctypeEOF)
	 [] H|T then
	    Buffer<-T
	    case H
	    of &>  then skip
	    [] &\n then Line<-@Line+1 Coord<-unit Parser,SkipToRangle
	    else Parser,SkipToRangle end
	 end
      end

      %%--------------------------------------------------------------
      %% Intern(+Name ?Fullname ?Prefix ?Localname ?URI)
      %%
      %%     Name is a string representing e.g. a tag as written is
      %% the document.  Fullname is the atom corresponding to Name.
      %% Prefix is an atom for the namespace prefix if any, unit
      %% otherwise.  Localname is an atom for the non-namespace part
      %% of the name.  URI is the uri corresponding to Prefix if any,
      %% or unit otherwise.
      %%--------------------------------------------------------------

      meth Intern(Name ?Fullname ?Prefix ?Localname ?URI)
	 {StringToAtom Name Fullname}
	 %% I expect the common case to be a name without prefix
	 %% i.e. in the default namespace.  Thus, I first check if
	 %% the name contains a `:' to avoid unnecessary consing
	 %% when splitting
	 if {HasColon Name}
	 then PrefixS SuffixS in
	    {SplitName Name PrefixS SuffixS}
	    Prefix    = {StringToAtom PrefixS}
	    Localname = {StringToAtom SuffixS}
	    URI       = {CondSelect @PREFIXTABLE Prefix unit}
	 else
	    %% in default (or no) namespace
	    Prefix    = unit
	    Localname = Fullname
	    URI       = {CondSelect @PREFIXTABLE unit unit}
	 end
      end

      %%--------------------------------------------------------------
      %% ProcessAlist(+IN ?OUT)
      %%
      %%     IN is a list of elements of the form (Attr#Value#Coord)
      %% where both are strings, and corresponds to the attributes of
      %% an element.  OUT is a list of attribute(s) and
      %% namespaceDeclaration(s).  Additionally, a namespaceDeclaration
      %% causes an update to the PREFIXTABLE for the duration of the
      %% element.  In order to back the PREFIXTABLE to its original
      %% state when we leave the scope of the element, we also update
      %% a TRAIL.
      %%--------------------------------------------------------------

      meth ProcessAlist(IN ?OUT)
	 ALIST<-OUT
	 TAGS<-nil
	 Parser,PreProcessAlist(IN)
	 @ALIST=nil
	 Parser,PostProcessAlist(@TAGS)
	 TAGS<-nil
      end

      meth PreProcessAlist(IN)
	 case IN
	 of nil then skip
	 [] (Attr#Value#Coord)|IN then
	    PrefixA SuffixA ValueA={StringToAtom Value}
	 in
	    if {HasColon Attr}
	    then Prefix Suffix in
	       {SplitName Attr Prefix Suffix}
	       {StringToAtom Prefix PrefixA}
	       {StringToAtom Suffix SuffixA}
	    else PrefixA=unit SuffixA={StringToAtom Attr} end
	    if PrefixA==unit then
	       if {HasFeature XMLNS SuffixA} then
		  %% default namespace declaration
		  TRAIL<-(unit|{CondSelect @PREFIXTABLE unit unit})|@TRAIL
		  @PREFIXTABLE.unit := ValueA
		  {self onNamespaceDeclaration('' ValueA Coord)}
	       else
		  %% attribute in no-namespace
		  {self onAttribute(
			   tag(
			      qname  : SuffixA
			      prefix : unit
			      name   : SuffixA
			      uri    : unit
			      coord  : Coord)
			   ValueA)}
		  Parser,PreProcessAlist(IN)
	       end
	    elseif {HasFeature XMLNS PrefixA} then
	       %% namespace declaration
	       TRAIL<-(SuffixA|{CondSelect @PREFIXTABLE SuffixA unit})|@TRAIL
	       @PREFIXTABLE.SuffixA := ValueA
	       {self onNamespaceDeclaration(SuffixA ValueA Coord)}
	    else Tag = tag(
			  qname  : {StringToAtom Attr}
			  prefix : PrefixA
			  name   : SuffixA
			  uri    : _  % need to delay lookup until all local ns decls have been seen
			  coord  : Coord)
	    in
	       TAGS<-Tag|@TAGS
	       %% qualified attribute
	       {self onAttribute(Tag ValueA)}
	       Parser,PreProcessAlist(IN)
	    end
	 end
      end

      meth PostProcessAlist(L)
	 %% now that all local ns decls have been seen, we fill in the missing uris
	 case L
	 of nil then skip
	 [] H|T then
	    H.uri = {CondSelect @PREFIXTABLE H.prefix unit}
	    Parser,PostProcessAlist(T)
	 end
      end   

      %% =============================================================
      %% PARSER
      %% =============================================================

      meth ERROR(R) raise xml(parser:R) end end

      meth UnTrail()
	 case @TRAIL
	 of H|T then
	    TRAIL<-T
	    case H
	    of unit then skip
	    [] Key|Val then
	       if Val==unit then
		  {DictRemove @PREFIXTABLE Key}
	       else
		  @PREFIXTABLE.Key := Val
	       end
	       Parser,UnTrail()
	    end
	 end
      end

      meth AskStripSpace($)
	 case @TAG
	 of unit then true
	 [] Tag then
	    {@STRIP askStripSpace(Tag.uri Tag.name $)}
	 end
      end

      %%--------------------------------------------------------------
      %% methods onXXX(...) are meant to be overriden to build more
      %% specialized document representations
      %%
      %% onStartDocument()
      %% onEndDocument()
      %% onStartElement(Tag Alist Children)
      %% onEndElement(Tag)
      %% onAttribute(Tag Value)
      %% onNamespaceDeclaration(Prefix URI Coord)
      %% onProcessingInstruction(Name Data Coord)
      %% onCharacters(Chars Coord)
      %% onComment(Data Coord)
      %%
      %% append(_)
      %% attributeAppend(_)
      %%--------------------------------------------------------------

      %% invoked respectively at the start and end of the document
      
      meth onStartDocument() skip end
      meth onEndDocument() skip end

      %% onStartElement(Tag Alist Children)
      %%
      %% invoked on the start tag of an element.  It is its
      %% responsibility to construct a representation of the element
      %% and to contribute it to the list of items currently being
      %% accummulated (by invoking the append(_) method)
      %%
      %% Tag is a record that describes the start tag and has the
      %% following feature:
      %%
      %% - qname    : the tag's name as it appears in the document
      %% - prefix   : just the prefix of the name (unit if none)
      %% - uri      : the uri bound to the prefix (unit if none)
      %% - name     : the localname of the tag (i.e. minus prefix)
      %% - coord    : the debug coordinates where the tag occurred
      %% - endCoord : the, as yet uninstantiated, debug coordinates
      %%              where the corresponding tag occurs
      %%
      %% qname, prefix, uri and name are all atoms
      %% debug coordinates are records of the form:
      %%     coord(Filename LineNumber)
      %%
      %% Alist is the list of accummulated attributes and possibly
      %% namespaceDeclarations.
      %%
      %% Children is the, as yet uninstantiated, list of accummulated
      %% children of this elements
      
      meth onStartElement(Tag Alist Children)
	 {self append(
		  element(
		     uri        : Tag.uri
		     name       : Tag.name
		     attributes : Alist
		     children   : Children))}
      end

      %% contributes a new item to the contents of the current element

      meth append(X) L in
	 @CONTENTS=X|L
	 CONTENTS<-L
      end

      %% invoked on an end tag
      
      meth onEndElement(Tag) skip end

      %% onAttribute(Tag Value)
      %%
      %% invoked for each attribute of an element.  It is its
      %% responsibility to construct a representation of the attribute
      %% and to contribute it to the list of attributes currently being
      %% accumulated (by invoking the attributeAppend(_) method).  Of
      %% course, attributes can be ignored by not contributing them.
      %%
      %% Tag is a record describing the attribute's name and has the
      %% following features:
      %%
      %% - qname
      %% - prefix
      %% - name
      %% - uri
      %% - coord
      %%
      %% with the same interpretation as for elements.  Note that
      %% attributes without an explicit prefix are always considered to
      %% be in NO namespace, and not in the default namespace if any.
      %%
      %% It should be noted that the attributes of an element are
      %% processed before its onStartElement method is called.  The
      %% reason is that it is necessary to process all namespace
      %% declarations before attempting to interpret the tag.

      meth onAttribute(Tag Value)
	 {self attributeAppend(
		  attribute(
		     uri   : Tag.uri
		     name  : Tag.name
		     value : Value))}
      end

      %% contributes a new item to the list of attributes and
      %% namespaceDeclarations currently being accummulated
      
      meth attributeAppend(X) L in
	 @ALIST=X|L
	 ALIST<-L
      end

      %% onNamespaceDeclaration(Prefix URI Coord)
      %%
      %% some attributes are actually namespace declarations.
      %% This is identified by their "xmlns" prefix (using any
      %% possibly mixed capitalization as desired).
      %%
      %% Prefix and URI are both atoms
      %% Coord is a debug coordinates record
      %%
      %% By default, namespace declarations are contributed to
      %% the list of attributes only if @keepNamespaceDeclarations
      %% is true

      meth onNamespaceDeclaration(Prefix URI Coord)
	 if @keepNamespaceDeclarations then
	    {self attributeAppend(
		     namespaceDeclaration(
			prefix : Prefix
			uri    : URI))}
	 end
      end

      %% onProcessingInstructions(Name Data Coord)
      %%
      %% Name is an atom, Data is a string, Coord is a debug
      %% coordinates record
	 
      meth onProcessingInstruction(Name Data Coord)
	 {self append(
		  pi(name : Name
		     data : {StringToAtom Data}))}
      end

      %% onCharacters(Chars Coord)
      %%
      %% invoked for text nodes. Chars is a string, Coord is a
      %% debug coordinates record
      
      meth onCharacters(Chars Coord)
	 {self append(
		  text(data : {MakeBS Chars}))}
      end

      %% onComment(Data Coord)
      %%
      %% invoked for comment nodes. Data is a string, Coord is a
      %% debug coordinates record.  Note that comment nodes are
      %% automatically discarded if @keepComments is false
      
      meth onComment(Data Coord)
	 {self append(
		  comment(data:{MakeBS Data}))}
      end


      %% the main recursive loop for parsing the document

      meth PARSE()
	 case Parser,GetToken($)
	 of unit then
	    @CONTENTS=nil
	    if @TAG\=unit then
	       Parser,ERROR(nonTerminatedElement(
			       name : @TAG.name
			       uri  : @TAG.uri))
	    end
	 [] stag(Name Alist Empty Coord) then
	    Alist2 Fullname Prefix Localname URI
	    %% mark the trail on entry
	    TRAIL <- unit|@TRAIL
	    %% process the attributes, which also causes
	    %% the prefix table to be updated
	    Parser,ProcessAlist(Alist Alist2)
	    %% now that we have all the namespaces, process
	    %% the tag itself
	    Parser,Intern(Name Fullname Prefix Localname URI)
	    Children
	    Tag = tag(qname    : Fullname
		      name     : Localname
		      prefix   : Prefix
		      uri      : URI
		      coord    : Coord
		      endCoord : _)
	 in
	    {self onStartElement(Tag Alist2 Children)}
	    if Empty then
	       Children = nil
	       Tag.endCoord=Coord
	       Parser,UnTrail()
	    else
	       STACK <- (@CONTENTS|@TAG)|@STACK
	       CONTENTS <- Children
	       TAG <- Tag
	    end
	    Parser,PARSE()
	 [] etag(Name Coord) then
	    Fullname Localname URI
	 in
	    Parser,Intern(Name Fullname _ Localname URI)
	    case @STACK of (Tail|Tag)|L then
	       STACK <- L
	       if URI\=@TAG.uri orelse Localname\=@TAG.name orelse Fullname\=@TAG.qname then
		  Parser,ERROR(mismatchedEtag(
				  wanted:Tag.qname
				  found :Fullname
				  coord :Coord))
	       else
		  @CONTENTS = nil
		  CONTENTS <- Tail
		  @TAG.endCoord=Coord
		  {self onEndElement(@TAG)}
		  TAG <- Tag
		  Parser,UnTrail()
		  Parser,PARSE()
	       end
	    else
	       Parser,ERROR(unexpectedEndOfSTACK)
	    end
	 [] pi(Target Args Coord) then
	    {self onProcessingInstruction({StringToAtom Target} Args Coord)}
	    Parser,PARSE()
	 [] text(Chars Coord) then
	    if Parser,AskStripSpace($) andthen {AllSpaces Chars}
	    then skip else
	       {self onCharacters(Chars Coord)}
	    end
	    Parser,PARSE()
	 [] comment(Data Coord) then
	    if @keepComments then
	       {self onComment(Data Coord)}
	    end
	    Parser,PARSE()
	 end
      end
   end
import
   Open(file:OpenFile)
define
   %% ================================================================
   %% this function is the trick to take care of "includes". It is
   %% invoked when we want to start including from url U.  Tail is the
   %% (lazy) string of current input and Restore is a procedure to be
   %% invoked when we run out of input from U to restore the state in
   %% the parser that identifies filename and line number; these need
   %% to be restored to what they were before starting reading from U.
   %%
   %% CAUTION: there is the possibility that lookahead occasioned by
   %% pattern matching may cause the previous state to be restored a
   %% bit too soon (before that last few characters of the include
   %% have been effectively consumed).  I have not thought of a case
   %% where this would matter, i.e. where parsing would continue but
   %% line numbers would be slightly off.  I think the simpler, faster
   %% design is worth the unlikely risk that coordinates might be
   %% slightly off: after all they are only relevant to error reports
   %% and need only be precise enough to help the user quickly locate
   %% the problem.
   %% ================================================================
   
   fun {FileOpenWithTail U Tail Restore}
      O={New OpenFile init(url:U)}
      %% the string on input is lazily computed to reduce memory
      %% requirements
      fun lazy {More} L T N in
	 {O read(list:L tail:T size:1024 len:N)}
	 T = if N==0
		%% when we run out of data in this file, we restore
		%% the previous state (filename+line) and continue
		%% with the previous lazy input string
	     then {O close} {Restore} Tail
		%% otherwise, the tail is computed by a lazy recursive
		%% call
	     else {More} end
	 L
      end
   in
      {More}
   end

   class XMLParser from Parser
      meth openWithTail(Url Tail Restore $)
	 {FileOpenWithTail Url Tail Restore}
      end
   end

end
