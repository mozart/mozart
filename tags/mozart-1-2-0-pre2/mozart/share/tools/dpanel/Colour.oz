functor
export
   list:List
define
   Blues=[blue
	  dodgerblue
	  darkblue
	  blueviolet
	  mediumblue
	  royalblue
	  cadetblue
	  darkorchid
	  darkmagenta
	  darkslateblue
	  deepskyblue
	  purple
	  steelblue
	 ]

   Reds=[red
	 deeppink
	 magenta
	 coral
	 darkred
	 hotpink
	 indianred
	 sienna
	 tomato
	 violetred
	 orchid
	]

   Greens=[green
	   limegreen
	   darkcyan
	   darkgreen
	   darkseagreen
	   seagreen
	   turquoise
	   yellowgreen
	  ]

   Yellows=[gold
	    brown
	    chocolate
	    darkorange
	    darkgoldenrod
	    darkkhaki
	    firebrick
	    orange
	    orangered
	    rosybrown
	    saddlebrown
	    tan
	   ]

   Greys=[darkgrey
	  darkslategrey
	  dimgrey
	 ]

   fun{Comp _ _} true end

   MixedList=black|{Merge {Merge Blues {Append Greens Greys} Comp}
		    {Merge Reds Yellows Comp} Comp}
   E
   List={Append MixedList E}
   E=List
end
