functor
import
   Path(make) at 'x-ozlib://duchier/sp/Path.ozf'
   System(showInfo:Print)
   Application(exit)
   Global(localDB       : LocalDB
	  args          : Args
	  dirPrefix     : DirPrefix)
export
   run    : Run
   remove : Remove
define
   fun{Remove PackageId Force Leave}
      try
	 Package
	 Conflicts
      in
	 try
	    Package={LocalDB get(PackageId $)}
	 catch _ then raise return(notFound) end
	 end
	 local
	    L={NewCell nil}
	 in
	    for Entry in {LocalDB items($)} do
	       if Entry\=Package then
		  for F in Entry.filelist do
		     for E in Package.filelist do
			if E==F then
			   {Assign L used(name:E pkg:Package loc:Entry)|{Access L}}
			end
		     end
		  end
	       end
	    end
	    if {Access L}\=nil andthen Force==false andthen Leave==false then
	       raise return(conflict({Access L})) end
	    end
	    Conflicts={Access L}
	 end
	 %%
	 %% we know the conflicting filenames and a conflict resolution is in Switch
	 %%
	 for E in Package.filelist do
	    if {List.some Conflicts fun{$ R} R.name==E end}
	       andthen Leave==true then skip
	    else
	       F = {DirPrefix resolve(E $)}
	    in
	       if {F isFile($)} then
		  {F rmtree}
	       else skip end
	    end
	 end
	 {LocalDB remove(Package.id)}
	 {LocalDB save}
	 success(pkg:Package)
      catch return(E) then E end
   end
   %%
   proc {Run}
      case {Remove Args.'in' Args.'force' Args.'leave'}
      of success(pkg:P) then
	 {Print "Package "#P.id#" was successfully uninstalled"}
	 {Application.exit 0}
      [] notFound then
	 {Print "No package "#Args.'in'#" currently installed"}
	 {Application.exit 1}
      [] conflict(L) then
	 {Print "The following file(s) from the package "#L.pkg.id#" are used :"}
	 {ForAll L
	  proc{$ R}
	     {Print R.name#"\tby package "#R.loc.id}
	  end}
	 {Print ""}
	 {Print "Try using the --leave or --force options"}
	 {Application.exit 1}
      end
   end
end
