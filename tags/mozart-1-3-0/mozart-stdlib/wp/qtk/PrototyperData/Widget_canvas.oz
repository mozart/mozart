local
   C
   Desc=canvas(handle:C width:200 height:200)
in
   {{QTk.build td(Desc)} show}
   {C create(rect 10 10 190 190 fill:blue outline:red)}
   {C create(text 100 100 text:"Canvas" fill:yellow)}
end

   
