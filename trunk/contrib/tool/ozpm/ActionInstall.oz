functor
export Run Install
import
   Resolve(localize)
   OS(unlink tmpnam)
   Archive('class')
   Global(fileMftPkl    : FILEMFTPKL
	  localDB       : LOCALDB
	  args          : Args
	  dirPrefix     : DirPrefix
	  pathLocalDB   : PATHLOCALDB)
   FileUtils(createPath : CreatePath
	     addToPath  : AddToPath
	     dirname    : Dirname)
   Application(exit)
   System(showInfo:Print)
   Pickle(load save)
define
   fun {Install Package Force}
      PackageResult = {Resolve.localize Package}
      A
   in
      try
	 PackageFile=PackageResult.1
	 {New Archive.'class' init(PackageFile) A}
	 PLS={A lsla($)}
	 PInfo Tmp={OS.tmpnam}
      in
	 try
	    {A extract(FILEMFTPKL Tmp)}
	    {Pickle.load Tmp PInfo}
	 finally
	    try {OS.unlink Tmp} catch _ then skip end
	 end
	 %% now we have the information and file names of the package
	 %%
	 %% cross checks with the informations of the local installation is done here
	 %%
	 if {Not Force} then
	    for Entry in LOCALDB do
	       if Entry.id==PInfo.id then
		  raise ok(alreadyinstalled(loc:Entry pkg:PInfo)) end
	       else
		  for F in Entry.filelist do
		     for E in PInfo.filelist do
			if E==F then
			   raise ok(nameclash(name:E loc:Entry pkg:PInfo)) end
			end
		     end
		  end
	       end
	    end
	 end
	 %%
	 %% getting this far means the package can be installed
	 %%
	 for File in PInfo.filelist do
	    {CreatePath {Dirname {AddToPath DirPrefix File}}}
	    {A extract(File {AddToPath DirPrefix File})}
	 end
	 %% update ozpminfo
	 {Pickle.save {Record.adjoin
		       {Record.adjoinAt PInfo lsla PLS}
		       ipackage}
	  |{List.filter LOCALDB
	    fun{$ Entry}
	       Entry.id\=PInfo.id %% forcing an install keeps only one version
	    end}
	  PATHLOCALDB}
	 %% result
	 success(pkg:PInfo)
      catch ok(E) then E
      finally
	 if {IsDet A} then try {A close} catch _ then skip end end
	 case PackageResult of new(F) then {OS.unlink F}
	 else skip end
      end
   end
   %%
   proc {Run}
      case {Install Args.'in' Args.'force'}
      of success(pkg:P) then
	 {Print "Package "#P.id#" was successfully installed"}
	 {Application.exit 0}
      [] nameclash(name:N loc:L pkg:P) then
	 {Print "Unable to install package '"#P.id#"'"}
	 {Print "The file '"#N#"' is conflicting with the installed package '"#L.id#"'"}
	 {Application.exit 1}
      [] alreadyinstalled(loc:L pkg:P) then
	 if {HasFeature P version} then
	    {Print "Unable to install package '"#P.id#"', version "#P.version}
	 else
	    {Print "Unable to install package '"#P.id#"'"}
	 end
	 if {HasFeature L version} then
	    {Print "This package is already installed in version "#L.version}
	 else
	    {Print "A package of the same id is already installed"}
	 end
	 {Application.exit 1}
      end
   end
end
