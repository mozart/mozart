declare
Data = [ belgium     # [france netherlands germany luxemburg]
	 germany     # [austria france luxemburg netherlands]
	 switzerland # [italy france germany austria]	 
	 austria     # [italy switzerland germany]
	 france      # [spain luxemburg italy]
	 spain       # [portugal] ]

fun {MapColoring Data}
   Countries = {FoldR Data fun {$ C#Cs A} {Append Cs C|A} end nil}
in
   proc {$ Color}
      NbColors  = {FD.decl}
   in
      % Color: Countries --> 1#NbColors
      {FD.distribute naive [NbColors]}
      {FD.record color Countries 1#NbColors Color}
      {ForAll Data
       proc {$ A#Bs}
	  {ForAll Bs proc {$ B} Color.A \=: Color.B end}
       end}
      {FD.distribute ff Color}
   end
end

{ExploreOne {MapColoring Data}}

