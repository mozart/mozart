functor
export
   MakeFromTokenizer
   MakeFromString
   MakeFromURL
   MakeLazyFromURL
import
   Tokenizer  at 'Tokenizer.ozf'
   NameSpaces at 'NameSpaces.ozf'
prepare

   MakeBS = ByteString.make

   fun {AttrToAtom Name|Value}
      {StringToAtom Name}|{StringToAtom Value}
   end

define

   proc {Loop Get Stack Siblings CurrentTag PrefixMap}
      case {Get}

      of unit then
	 Siblings=nil
	 if CurrentTag\=unit then
	    {Exception.raiseError
	     xml(parse(nonTerminatedElement CurrentTag))}
	 end

      [] stag(Name Alist Empty) then
	 Tag2 Alist2 PrefixMap2 Siblings2 Contents
      in
	 Siblings=element(
		     tag      : Tag2
		     alist    : Alist2
		     contents : Contents)|Siblings2
	 {NameSpaces.processElement
	  {StringToAtom Name}
	  {Map Alist AttrToAtom}
	  PrefixMap
	  Tag2 Alist2 PrefixMap2}
	 if Empty then Contents=nil
	    {Loop Get Stack Siblings2 CurrentTag PrefixMap}
	 else
	    {Loop Get (CurrentTag#Siblings2#PrefixMap)|Stack
	     Contents Tag2 PrefixMap2}
	 end

      [] etag(Name) then
	 Tag={NameSpaces.processName
	      {StringToAtom Name} PrefixMap}
      in
	 Siblings=nil
	 if CurrentTag\=Tag then
	    {Exception.raiseError
	     xml(parse(mismatchedEtag(found:Tag wanted:CurrentTag)))}
	 end
	 case Stack
	 of (Tag#Siblings#PrefixMap)|Stack then
	    {Loop Get Stack Siblings Tag PrefixMap}
	 else {Exception.raiseError
	       xml(parse(unexpectedErrorAtEtag Tag))}
	 end

      [] pi(Target Args) then Siblings2 in
	 Siblings=pi({StringToAtom Target}
		     {StringToAtom Args})|Siblings2
	 {Loop Get Stack Siblings2 CurrentTag PrefixMap}

      [] text(Chars) then Siblings2 in
	 Siblings=text({MakeBS Chars})|Siblings2
	 {Loop Get Stack Siblings2 CurrentTag PrefixMap}

      [] comment(Chars) then Siblings2 in
	 Siblings=comment({MakeBS Chars})|Siblings2
	 {Loop Get Stack Siblings2 CurrentTag PrefixMap}
      end
   end

   fun {MakeFromTokenizer Tok}
      root(
	 {Loop Tok.get nil $ unit {NameSpaces.newPrefixMap}})
   end

   fun {MakeFromString String}
      {MakeFromTokenizer {Tokenizer.newFromString String}}
   end

   fun {MakeFromURL U}
      {MakeFromTokenizer {Tokenizer.newFromURL U}}
   end

   fun {MakeLazyFromURL U}
      {MakeFromTokenizer {Tokenizer.newLazyFromURL U}}
   end
end