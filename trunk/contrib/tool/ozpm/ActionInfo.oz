functor
import
   Global(args             : Args
	  fileMftPkl       : FILEMFTPKL
	  fileMftTxt       : FILEMFTTXT
	  localDB          : LocalDB)
   Archive('class')
   OS(tmpnam unlink)
   Pickle(load)
   Application(exit)
   System(showInfo:Print)
   Resolve(localize)
export
   Run
   View %% get information on external file
   Info %% get information on an installed module
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
	  if R.path\=FILEMFTPKL andthen R.path\=FILEMFTTXT then 
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
      PackageResult A Tmp
   in
      try
	 PackageResult={Resolve.localize Package}
	 A   = {New Archive.'class' init(PackageResult.1)}
	 Tmp = {OS.tmpnam}
	 {A extract(FILEMFTPKL Tmp)}
	 {Pickle.load Tmp} # {A lsla($)}
      finally
	 if {IsDet PackageResult} then
	    case PackageResult of new(F) then {OS.unlink F}
	    else skip end
	 end
	 if {IsDet Tmp} then
	    try {OS.unlink Tmp} catch _ then skip end
	 end
      end
   end
   %%
   fun {Info Id1}
      Id = {ByteString.make Id1}
   in
      try
	 for Entry in LocalDB do
	    if Entry.id==Id then raise found(Entry) end end
	 end
	 notFound
      catch found(E) then E end
   end
end
