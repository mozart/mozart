functor
export
   IsExtension
   ExtractExtension
define
   fun{ExtractExtension Name}
      Flag
      L={List.takeWhile {Reverse {VirtualString.toString Name}}
	 fun{$ C}
	    if C\=&. then true else Flag=unit false end
	 end}
   in
      if {IsFree Flag} then "" else {Reverse L} end
   end
   %%
   %%
   fun{IsExtension Ext Name}
      {VirtualString.toString Ext}=={ExtractExtension Name}
   end
end
