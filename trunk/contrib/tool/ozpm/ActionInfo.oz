functor
import
   Global(args             : Args
	  ozpmmanifest     : OZPMMANIFEST
	  ozpmmanifesttext : OZPMMANIFESTTEXT
	  ozpmInfo         : OzpmInfo)
   Archive('class')
   OS(tmpnam unlink)
   Pickle(load)
   Application(exit)
   System(showInfo:Print)
export
   Run
define
   proc {Run}
      I L
   in
      try I#L = {View Args.'in'}
      catch _ then
	 case {Info Args.'in'}
	 of notFound then
	    {Print "Package '"#Args.'in'#"' is not found and not installed."}
	    {Application.exit 1}
	 [] V then I=V L=I.lsla end
      end
      {PrintInfo I L}
      {Application.exit 0}
   end
   %%
   proc {PrintInfo I L}
      {Print "Package id   : "#I.id}
      {Record.forAllInd I
       proc{$ I V}
	  if I\=filelist andthen I\=id andthen {VirtualString.is V} then
	     {Print {LAlign I 13}#": "#V}
	  end
       end}
      {Print " "}
      {Print "Contains the following files :"}
      {ForAll L
       proc{$ R}
	  if R.path\=OZPMMANIFEST andthen R.path\=OZPMMANIFESTTEXT then 
	     {Print {RAlign R.size 10}#" "#R.path}
	  end
       end}
   end
   %%
   fun {RAlign VS I}
      {List.map
       {List.make {Max I-{Length {VirtualString.toString VS}} 0}}
       fun{$ C} C=&  end}#VS
   end
   %%
   fun {LAlign VS I}
      VS#{List.map
	  {List.make {Max I-{Length {VirtualString.toString VS}} 0}}
	  fun{$ C} C=&  end}
   end
   %%
   fun {View Package}
      A   = {New Archive.'class' init(Package)}
      Tmp = {OS.tmpnam}
   in
      try
	 {A extract(OZPMMANIFEST Tmp)}
	 {Pickle.load Tmp} # {A lsla($)}
      finally
	 try {OS.unlink Tmp} catch _ then skip end
      end
   end
   %%
   fun {Info Id1}
      Id = {ByteString.make Id1}
   in
      try
	 for Entry in OzpmInfo do
	    if Entry.id==id then raise found(Entry) end end
	 end
	 notFound
      catch found(E) then E end
   end
end
