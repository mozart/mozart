functor
export
   SpaceManager
require
   URL(make     : URLMake
       toString : URLToString
       resolve  : URLResolve)
prepare
   
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

   proc {SKIP} skip end

   MakeBS = ByteString.make
   DictClone = Dictionary.clone
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
   o(amp : string("\&")
     lt  : string("<")
     gt  : string(">")
     apos: string("'")
     quot: string("\""))

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
   %% namespace context
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

   fun {NewContext}
      D={NewDictionary}
   in
      D.unit := '' %% install the no-name-space for attributes
      {NewContextWrap D {NewDictionary}}
   end

   fun {NewContextWrap PrefixTable NameTable}
      
      %% =============================================================
      %% {Intern +Name ?QName}
      %%     takes a string Name and returns the unique record
      %% describing the fully qualified name:
      %%
      %%          qname( uri:URI name:LOC xname:XLOC )
      %%
      %% where URI is the uri bound to the prefix, LOC is the local
      %% part of the name, and XLOC is the atom obtained by concatenating
      %% LOC followed by ' @ ' followed by URI. XLOC can be used as
      %% a key uniquely identifying the name.  When URI is '' we just
      %% use LOC as XLOC.
      %% =============================================================

      fun {Intern Name}
	 PrefixA SuffixA
	 %% I expect the common case to be a name without prefix
	 %% i.e. in the default namespace.  Thus, I first check
	 %% if the name contains a `:' to avoid unnecessary consing
	 %% when splitting
	 if {HasColon Name}
	 then Prefix Suffix in
	    {SplitName Name Prefix Suffix}
	    if Prefix==nil then
	       PrefixA=''
	    else
	       {StringToAtom Prefix PrefixA}
	    end
	    {StringToAtom Suffix SuffixA}
	 else PrefixA='' SuffixA={StringToAtom Name} end
      in
	 {InternExplicit PrefixA SuffixA}
      end
      proc {PutPrefix Prefix URI}
	 PrefixTable.{VS2A Prefix} := {VS2A URI}
      end
      %% prefix -> uri is the table that needs to be cloned
      %% uri * name -> qname
      fun {InternExplicit PrefixA SuffixA}
	 MaybeURI={CondSelect PrefixTable PrefixA unit}
	 URI=if MaybeURI==unit then
		%% namespace prefix not declared, we let it default
		%% to denoting a uri that is string-equal to the prefix
		PrefixTable.PrefixA := PrefixA
		PrefixA
	     else MaybeURI end
	 MaybeSubTable={CondSelect NameTable URI unit}
	 SubTable=
	 if MaybeSubTable==unit then
	    T={NewDictionary}
	 in
	    NameTable.URI := T
	    T
	 else MaybeSubTable end
	 MaybeName = {CondSelect SubTable SuffixA unit}
      in
	 if MaybeName==unit then
	    XName = if URI=='' then SuffixA else
		       {VS2A SuffixA#' @ '#URI}
		    end
	    R=qname(
		 uri   : URI
		 name  : SuffixA
		 xname : XName)
	 in
	    SubTable.SuffixA := R
	    R
	 else MaybeName end
      end
      fun {Clone}
	 {NewContextWrap {DictClone PrefixTable} NameTable}
      end
      proc {Extend L}
	 case L
	 of nil then skip
	 [] (PrefixA|ValueA)|L then
	    PrefixTable.PrefixA := ValueA
	    {Extend L}
	 end
      end
      fun {InternAlist L}
	 case L
	 of nil then nil
	 [] (PrefixA#SuffixA#ValueA)|L then
	    ({InternExplicit PrefixA SuffixA}|ValueA)
	    |{InternAlist L}
	 end
      end
   in
      context(
	 intern         : Intern
	 internExplicit : InternExplicit
	 clone          : Clone
	 extend         : Extend
	 internAlist    : InternAlist
	 putPrefix      : PutPrefix
	 )
   end

   %% ================================================================
   %% {ProcessAlist +IN ?OUT ?NS}
   %%     takes as input a list where each element is of the form
   %% (Attr|Value) where both Attr and Value are strings and returns
   %% two lists, one for normal attributes (OUT) and one for namespace
   %% declarations (NS). OUT is a list of elements of the form
   %% (PrefA#NamA#ValA) where all 3 components are atoms, PrefA is the
   %% namespace prefix ('' if none), NamA is the local part of the
   %% attribute name, and ValA is the value.  NS is a list of elements
   %% of the form (PrefA|ValA) where both are also atoms and PrefA is
   %% the namespace prefix, and ValA the namespace URI.
   %%
   %% As a special case, PrefA for an attribute will be unit if the
   %% attribute had no explicit prefix.  The reason is that in this
   %% case the attribute's name is not in the current default namespace
   %% but rather in the no-name-space which we denote by unit as
   %% PrefixA.
   %% ================================================================

   proc {ProcessAlist IN OUT NS}
      case IN
      of nil then OUT=nil NS=nil
      [] (Attr|Value)|IN then
	 PrefixA SuffixA ValueA={StringToAtom Value}
      in
	 if {HasColon Attr}
	 then Prefix Suffix in
	    {SplitName Attr Prefix Suffix}
	    if Prefix==nil then
	       PrefixA=''
	    else
	       {StringToAtom Prefix PrefixA}
	    end
	    {StringToAtom Suffix SuffixA}
	 else PrefixA=unit SuffixA={StringToAtom Attr} end
	 if PrefixA==unit then
	    if {HasFeature XMLNS SuffixA} then NS2 in
	       %% default namespace declaration
	       NS=(''|ValueA)|NS2
	       {ProcessAlist IN OUT NS2}
	    else OUT2 in
	       %% attribute in no-namespace
	       OUT=(unit#SuffixA#ValueA)|OUT2
	       {ProcessAlist IN OUT2 NS}
	    end
	 elseif {HasFeature XMLNS PrefixA} then NS2 in
	    %% namespace declaration
	    NS=(SuffixA|ValueA)|NS2
	    {ProcessAlist IN OUT NS2}
	 else OUT2 in
	    %% qualified attribute
	    OUT=(PrefixA#SuffixA#ValueA)|OUT2
	    {ProcessAlist IN OUT2 NS}
	 end
      end
   end

   %% ================================================================
   %% the big tokenizer+parser combo
   %% ================================================================

   class Parser
      prop final
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
	 KeepComments : false
	 OpenWithTail : unit
	 %%-----------------------------------------------------------
	 %% namespace context
	 %%-----------------------------------------------------------
	 %%!!!CONTEXT     : unit
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

      meth init(OWT)
	 OpenWithTail <- OWT
      end
      
      meth setSpaceManager(M) STRIP<-M end
      meth setKeepComments(B<=true) KeepComments<-B end

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
	 Buffer <- {@OpenWithTail U nil SKIP}
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

      meth Parse($)
	 TAG <- ROOT_TAG
	 CONTENTS <- _
	 Parser,PARSE()
	 @CONTENTS
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
	 Buffer   <- {@OpenWithTail FullURL @Buffer
		      proc {$}
			 Filename <- CurrentFilename
			 Line     <- CurrentLine
		      end}
	 Line     <- 1
	 Filename <- FullFName
      end

      meth restoreFileInfo(FName LNum)
	 Filename <- FName
	 Line     <- LNum
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
	 else Name Value Alist2 in
	    Alist=(Name|Value)|Alist2
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

      %% =============================================================
      %% NAMESPACE CONTEXT
      %% =============================================================

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
	 AlistMid AlistNS
      in
	 {ProcessAlist Alist1 AlistMid AlistNS}
	 PSTACK <- @CONTEXT|@PSTACK
	 if AlistNS\=nil then C={@CONTEXT.clone} in
	    CONTEXT <- C
	    {C.extend AlistNS}
	 end
	 {@CONTEXT.intern Tag1 Tag2}
	 {@CONTEXT.internAlist AlistMid Alist2}
      end

      meth Intern(Name ?Prefix ?Localname ?URI)
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
	    %% in default namespace
	    Prefix    = unit
	    Localname = {StringToAtom Name}
	    URI       = {CondSelect @PREFIXTABLE unit unit}
	 end
      end

      meth ProcessAlist(IN $)
	 case IN
	 of nil then OUT=nil
	 [] (Attr|Value)|IN then
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
	       in
		  TRAIL<-(unit|{CondSelect @PREFIXTABLE unit unit})|@TRAIL
		  PREFIXTABLE.unit := ValueA
		  namespaceDeclaration(prefix:'' uri:ValueA)
		  |Parser,ProcessAlist(IN $)
	       else
		  %% attribute in no-namespace
		  attribute(
		     prefix : unit
		     uri    : unit
		     value  : ValueA)
		  |Parser,ProcessAlist(IN $)
	       end
	    elseif {HasFeature XMLNS PrefixA} then
	       %% namespace declaration
	       TRAIL<-(SuffixA|{CondSelect @PREFIXTABLE SuffixA unit})|@TRAIL
	       PREFIXTABLE.SuffixA := ValueA
	       namespaceDeclaration(prefix:SuffixA uri:ValueA)
	       |Parser,ProcessAlist(IN $)
	    else
	       %% qualified attribute
	       attribute(
		  name      : {StringToAtom Attr}
		  prefix    : PrefixA
		  localname : SuffixA
		  uri       : {CondSelect @PREFIXTABLE PrefixA unit}
		  value     : ValueA)
	       |Parser,ProcessAlist(IN $)
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
	 case Parser,GetToken($)
	 of unit then
	    @CONTENTS=nil
	    if @TAG.xname\=unit then
	       Parser,ERROR(nonTerminatedElement(@PARENT))
	    end
	 [] stag(Name Alist Empty Coord) then
	    Tag2 Alist2 Elem Children Tail Alist3 OParent
	 in
	    %% this will always push the CONTEXT on the PSTACK
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
	       %% we now need to pop the CONTEXT from the PSTACK
	       case @PSTACK of H|T then
		  CONTEXT <- H
		  PSTACK  <- T
	       end
	    else
	       CONTENTS <- Children
	       STACK    <- (Tail|OParent)|@STACK
	       TAG      <- Tag2
	    end
	    Parser,PARSE()
	 [] etag(Name Coord) then Tag in
	    {@CONTEXT.intern Name Tag}
	    if @TAG.xname\=Tag.xname then
	       Parser,ERROR(mismatchedEtag(found:Tag wanted:@TAG coord:Coord))
	    else
	       @CONTENTS=nil
	       case @STACK
	       of (Tail|Par)|Stack then
		  CONTENTS <- Tail
		  PARENT   <- Par
		  TAG      <- {CondSelect Par tag ROOT_TAG}
		  STACK    <- Stack
		  Parser,PARSE()
	       else Parser,ERROR(unexpectedErrorAtEtag(Tag Coord)) end
	       case @PSTACK
	       of H|T then CONTEXT<-H PSTACK<-T
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
	 [] text(Chars Coord) then
	    if {CondSelect @STRIP @TAG.xname false}
	       andthen {AllSpaces Chars}
	    then skip else L in
	       @CONTENTS = text(data   : {MakeBS Chars}
				coord  : Coord
				parent : @PARENT
				index  : @INDEX)|L
	       INDEX    <- @INDEX+1
	       CONTENTS <- L
	    end
	    Parser,PARSE()
	 [] comment(Data Coord) then
	    if @KeepComments then L in
	       @CONTENTS = comment(data   : {MakeBS Data}
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
   proc {Parse Init Root}
      {New Parser {Adjoin Init init(root:Root fileopen:FileOpenWithTail)} _}
   end
end
