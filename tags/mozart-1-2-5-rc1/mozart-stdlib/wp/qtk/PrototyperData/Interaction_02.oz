local
   C
   Desc=td(canvas(handle:C))
in
   {{QTk.build Desc} show}
   {C create(text 100 100 text:"Hello World" anchor:nw)}
end
