functor
export
   NewFromString
   NewFromURL
   NewFromURLOld
import
   OldTokenizer at 'Tokenizer.ozf'
   Tokenizer at 'FastTokenizer.ozf'
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
   
   fun {NewParser GET Params NS}
      IgnoreComments = {CondSelect Params ignoreComments true}
      StripSpaces = {CondSelect Params stripSpaces unit}
      fun {ParseSeq MAP TAG}
	 {ParseSeqTok {GET} MAP TAG}
      end
      fun {ParseSeqTok T MAP TAG}
	 case T
	 of unit then
	    if TAG==unit then nil
	    else raise xml(parser(expectedEtagEOF(TAG))) end end
	 [] doctype(_) then {ParseSeq MAP TAG}
	 [] pi(_ _ _) then T|{ParseSeq MAP TAG}
	 [] comment(_ _) then
	    if IgnoreComments
	    then {ParseSeq MAP TAG}
	    else T|{ParseSeq MAP TAG} end
	 [] stag(Name Alist Empty Coord) then
	    Tag2 Alist2 Map2
	 in
	    {NS.processElement
	     Name Alist MAP
	     Tag2 Alist2 Map2}
	    element(
	       tag      : Tag2
	       alist    : Alist2
	       children : if Empty then nil else
			     {ParseSeq Map2 Tag2}
			  end
	       coord    : Coord
	       )
	    |{ParseSeq MAP TAG}
	 [] text(S C) then
	    {ParseSeqText S C MAP TAG}
	 [] etag(Name Coord) then
	    Tag={NS.processName Name MAP}
	 in
	    if Tag==TAG then nil else
	       raise xml(parser(want:TAG got:Tag coord:Coord)) end
	    end
	 else
	    raise xml(parser(unexpected(T))) end
	 end
      end
      fun {ParseSeqText S Coord MAP TAG}
	 case {GET}
	 of text(S2 _) then
	    {ParseSeqText S#S2 Coord MAP TAG}
	 [] T then
	    if {CondSelect StripSpaces TAG.key false} then
	       SS={VS2S S}
	    in
	       if {AllSpaces SS} then {ParseSeqTok T MAP TAG}
	       else text({MakeBS SS} Coord)|{ParseSeqTok T MAP TAG} end
	    else
	       text({MakeBS S} Coord)|{ParseSeqTok T MAP TAG}
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
   fun {NewFromURLOld S Params}
      {NewParser {OldTokenizer.newLazyFromURL S}.get Params NameSpaces}
   end
end
