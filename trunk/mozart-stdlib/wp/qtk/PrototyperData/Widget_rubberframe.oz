local
   Def=tdrubberframe(glue:nswe
		     lrrubberframe(glue:nswe
				   label(text:"Left frame")
				   label(text:"Right frame"))
		     label(text:"Bottom frame"))
in
   {{QTk.build td(Def)} show}
end
