local
   T
   Desc=td(text(handle:T))
in
   {{QTk.build Desc} show}
   {T tk(insert "@1,0" "Hello")}
   local
      R={T tkReturnAtom(get("@1,0" 'end') $)}
   in
      {Wait R}
      {Show R}
   end
end
