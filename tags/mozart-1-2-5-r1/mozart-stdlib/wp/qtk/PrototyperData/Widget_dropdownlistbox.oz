local
   L D
   Desc=lr(label(handle:L)
	   dropdownlistbox(init:[1 2 3 4 5 6]
			   handle:D
			   action:proc{$} {L set({List.nth
						  {D get($)}
						  {D get(firstselection:$)}})}
				  end))
in
   {{QTk.build Desc} show}
end
