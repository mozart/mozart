local
   T
   Desc=td(text(handle:T))
in
   {{QTk.build Desc} show}
   {T exec(insert "@1,0" "Hello")}
   {Show {T return(get "@1,0" 'end' type:atom $)}}
end
