functor
export
   NewFromString
   NewFromURL
   Fast
   new : NewParserInit
import
   Tokenizer  at 'Tokenizer.ozf'
   NameSpaces at 'NameSpaces.ozf'
prepare
   MakeBS = ByteString.make
   BIVS2S = VirtualString.toString
   fun {VS2S S}
      %% for efficiency since the common case is that there
      %% is an isolated text node and it already contains
      %% a string
      case S of _#_ then {BIVS2S S} else S end
   end
   CharIsSpace = Char.isSpace
   fun {AllSpaces S}
      case S of nil then true
      [] H|T then {CharIsSpace H} andthen {AllSpaces T}
      end
   end
   fun {MakeElementMin Tag Alist Children Coord Parent}
      element(
	 tag      : Tag
	 alist    : Alist
	 children : Children)
   end
   fun {MakeElementMed Tag Alist Children Coord Parent}
      element(
	 tag      : Tag
	 alist    : Alist
	 children : Children
	 coord    : Coord)
   end
   fun {MakeElementMax Tag Alist Children Coord Parent}
      element(
	 tag      : Tag
	 alist    : Alist
	 children : Children
	 parent   : Parent
	 coord    : Coord)
   end
   fun {MakePiMin Name Args Coord Parent}
      pi(name:Name data:Args)
   end
   fun {MakePiMed Name Args Coord Parent}
      pi(name:Name data:Args coord:Coord)
   end
   fun {MakePiMax Name Args Coord Parent}
      pi(name:Name data:Args coord:Coord parent:Parent)
   end
   fun {MakeCommentMin Text Coord Parent}
      comment(data:Text)
   end
   fun {MakeCommentMed Text Coord Parent}
      comment(data:Text coord:Coord)
   end
   fun {MakeCommentMax Text Coord Parent}
      comment(data:Text coord:Coord parent:Parent)
   end
   fun {MakeTextMin Text Coord Parent}
      text(data:Text)
   end
   fun {MakeTextMed Text Coord Parent}
      text(data:Text coord:Coord)
   end
   fun {MakeTextMax Text Coord Parent}
      text(data:Text coord:Coord parent:Parent)
   end
   MakeNode =
   o(
      min : o(
	       element : MakeElementMin
	       pi      : MakePiMin
	       comment : MakeCommentMin
	       text    : MakeTextMin)
      med : o(
	       element : MakeElementMed
	       pi      : MakePiMed
	       comment : MakeCommentMed
	       text    : MakeTextMed)
      max : o(
	       element : MakeElementMax
	       pi      : MakePiMax
	       comment : MakeCommentMax
	       text    : MakeTextMax))
	       
	       
   
   fun {NewParser GET Params NS}
      IgnoreComments = {CondSelect Params ignoreComments true}
      StripSpaces = {CondSelect Params stripSpaces unit}
      UserMakeNode = case {CondSelect Params makeNode med}
		     of min then MakeNode.min
		     [] med then MakeNode.med
		     [] max then MakeNode.max
		     [] M   then M end
      MakeElement = UserMakeNode.element
      MakePI      = UserMakeNode.pi
      MakeComment = UserMakeNode.comment
      MakeText    = UserMakeNode.text
      ProcessElem = NS.processElement
      ProcessName = NS.processName
      fun {ParseSeq MAP PAR}
	 {ParseSeqTok {GET} MAP PAR}
      end
      fun {ParseSeqTok T MAP PAR}
	 case T
	 of unit then TAG={CondSelect PAR tag unit} in
	    if TAG==unit then nil
	    else raise xml(parser(expectedEtagEOF(TAG))) end end
	 [] doctype(_) then {ParseSeq MAP PAR}
	 [] pi(Name Args Coord) then {MakePI Name Args Coord PAR}|{ParseSeq MAP PAR}
	 [] comment(Text Coord) then
	    if IgnoreComments
	    then {ParseSeq MAP PAR}
	    else {MakeComment Text Coord PAR}|{ParseSeq MAP PAR} end
	 [] stag(Name Alist Empty Coord) then
	    Tag2 Alist2 Map2 Children Elem
	 in
	    {ProcessElem Name Alist MAP Tag2 Alist2 Map2}
	    {MakeElement Tag2 Alist2 Children Coord PAR Elem}
	    if Empty then Children=nil else {ParseSeq Map2 Elem Children} end
	    Elem|{ParseSeq MAP PAR}
	 [] text(S C) then
	    {ParseSeqText S C MAP PAR}
	 [] etag(Name Coord) then
	    Tag={ProcessName Name MAP}
	    TAG={CondSelect PAR tag unit}
	 in
	    if Tag==TAG then nil else
	       raise xml(parser(want:TAG got:Tag coord:Coord)) end
	    end
	 else
	    raise xml(parser(unexpected(T))) end
	 end
      end
      fun {ParseSeqText S Coord MAP PAR}
	 case {GET}
	 of text(S2 _) then
	    {ParseSeqText S#S2 Coord MAP PAR}
	 [] T then TAG={CondSelect PAR tag unit} in
	    if {CondSelect StripSpaces {CondSelect TAG key TAG} false} then
	       SS={VS2S S}
	    in
	       if {AllSpaces SS} then {ParseSeqTok T MAP PAR}
	       else {MakeText {MakeBS SS} Coord PAR}|{ParseSeqTok T MAP PAR} end
	    else
	       {MakeText {MakeBS S} Coord PAR}|{ParseSeqTok T MAP PAR}
	    end
	 end
      end
   in
      document(
	 children : {ParseSeq if {HasFeature Params prefixMap}
			      then Params.prefixMap
			      else {NS.newPrefixMap}
			      end
		     unit})
   end
define
   fun {NewFromString S Params}
      {NewParser {Tokenizer.newFromString S}.get Params NameSpaces}
   end
   fun {NewFromURL S Params}
      {NewParser {Tokenizer.newFromURL S}.get Params NameSpaces}
   end
   fun {FastNewFromString S Params}
      {NewParser {Tokenizer.fast.newFromString S}.get Params NameSpaces.fast}
   end
   fun {FastNewFromURL S Params}
      {NewParser {Tokenizer.fast.newFromURL S}.get Params NameSpaces.fast}
   end
   Fast = fast(newFromString : FastNewFromString
	       newFromURL    : FastNewFromURL)
   fun {NewParserInit Init}
      TOK = if {CondSelect Init fast false} then Tokenizer.fast else Tokenizer end
      NS  = if {CondSelect Init namespaces true} then NameSpaces else NameSpaces.fast end
      FEA = if {HasFeature Init string} then newFromString else newFromURL end
      ARG = if {HasFeature Init string} then Init.string
	    elseif {HasFeature Init url} then Init.url
	    else Init.file end
   in
      {NewParser {TOK.FEA ARG}.get Init NS}
   end
end
