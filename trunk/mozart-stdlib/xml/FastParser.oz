functor
export
   NewFromString
   NewFromURL
import
   Tokenizer at 'FastTokenizer.ozf'
   NameSpaces at 'NameSpaces.ozf'
prepare
   MakeBS = ByteString.make
   
   fun {NewParser GET Params NS}
      IgnoreComments = {CondSelect Params ignoreComments true}
      fun {ParseSeq MAP TAG}
	 {ParseSeqTok {GET} MAP TAG}
      end
      fun {ParseSeqTok T MAP TAG}
	 case T
	 of unit then
	    if TAG==unit then nil
	    else raise xml(parser(expectedEtagEOF(TAG))) end end
	 [] doctype(_) then {ParseSeq MAP TAG}
	 [] pi(_ _) then T|{ParseSeq MAP TAG}
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
	    text({MakeBS S} Coord)|{ParseSeqTok T MAP TAG}
	 end
      end
   in
      document(
	 children : {ParseSeq {NS.newPrefixMap} unit})
   end
define
   fun {NewFromString S Params}
      {NewParser {Tokenizer.newFromString S}.get Params NameSpaces}
   end
   fun {NewFromURL S Params}
      {NewParser {Tokenizer.newFromURL S}.get Params NameSpaces}
   end
end
