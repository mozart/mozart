local
   L R
   Desc=listbox(init:[a b c d e f g h i j k l m n o p]
		handle:L
		return:R
		action:proc{$}
			  {Show {L get(firstselection:$)}}
			  {Show {L get(selection:$)}}
		       in
			  {L set({List.filterInd {L get($)}
				  fun{$ J _}
				     J\={L get(firstselection:$)}
				  end})}
		       end
		tdscrollbar:true)
in
   {{QTk.build td(Desc)} show}
   {Wait R}
   {Show R}
end
