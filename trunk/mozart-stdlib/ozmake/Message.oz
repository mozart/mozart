functor
prepare
   fun {Normalize S}
      case S
      of nil then nil
      [] &\r|L then {Normalize L}
      [] H|L then H|{Normalize L}
      end
   end

   fun {ParseNextHeader S}
      case S
      of nil then nil
      [] &\n|L then [unit#L]
      else {ParseHeader nil S} end
   end

   fun {SplitEOL L S1 S2}
      {String.token L &\n S1 S2} 
   end

   fun {SkipSpaces L}
      case L
      of nil then nil
      [] H|T andthen {Char.isSpace H} then {SkipSpaces T}
      else L end
   end

   fun {SkipSpacesEnd L}
      {Reverse {SkipSpaces {Reverse L}}}
   end

   fun {ParseHeader S L}
      case L
      of &:|L then S1 S2 in
	 {SplitEOL {SkipSpaces L} S1 S2}
	 ({StringToAtom {Reverse S}}#{SkipSpacesEnd S1})
	 |{ParseNextHeader S2}
      [] H|T then {ParseNextHeader H|S T} end
   end

   fun {PreParse S}
      case S
      of nil then nil
      [] 
   end


   fun {ParseFromString S}
      
   end