functor

import
   Archive
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
   System(show:Show
	  showInfo:ShowInfo)
   Application
   Property
   OS
   Pickle
   Open
   Browser(browse:Browse)
   FileUtils(expand:Expand withSlash:WithSlash fullName:FullName
	    dirname:Dirname)
   Resolve
   Message(parse:Parse slurp:Slurp)
   
define

   PlatformWindows={Property.get 'platform.os'}==win32
   
   OZPMINFO={Expand "~/.oz/ozpm/ozpm.info"}
   OZPMMANIFEST='OZPMMFT.PKL'
   OZPMMANIFESTTEXT='OZPMMFT.TXT'
   OZPMPKG={Expand "~/.oz/"}
   MOGUL = "http://www.mozart-oz.org/mogul/" %"./"
   INFO  = "ozpm.info"

   proc{CreatePath P}
      Stat
   in
      {ShowInfo P}
      try
	 Stat={OS.stat P}
      catch system(os(...) ...) then % inexistant directory
	 if {OS.system "mkdir "#P}\=0 then
	    raise error(unableToCreateDirectory P) end
	 end
      end
      if {IsFree Stat} then
	 Stat={OS.stat P}
      end
      if Stat.type\=dir then % not a directory
	 raise error(unableToCreateDirectory P) end
      end
   end
   
   class ArchiveManagerClass

      feat mogul
      
      meth init
	 try
	    self.mogul={Pickle.load MOGUL#INFO}
	 catch _ then self.mogul=nil end
	    
			
	 skip
      end

      meth list(L)
	 L={List.map OzpmInfo fun{$ R} R.name end}
      end

      meth install(Package switch:Switch<=none result:Result<=_)
	 PackageFile={Resolve.localize Package}.1
	 A = {New Archive.'class' init(PackageFile)}
	 PLS={A lsla($)}
	 PInfo
      in
	 local
	    Tmp={OS.tmpnam}
	 in
	    {A extract(OZPMMANIFEST Tmp)}
	    {Pickle.load Tmp PInfo}
	    {OS.unlink Tmp}
	 end
	 %% now we have the information and file names of the package
	 %%
	 %% cross checks with the informations of the local installation is done here
	 %%
	 if {HasFeature Switch force} then %% always install, ignore all tests
	    skip
	 else
	    _={List.takeWhile OzpmInfo
	       fun{$ Entry}
		  Name
	       in
		  if Entry.name==PInfo.name then %% Same package
		     Result=alreadyinstalled(loc:Entry pkg:PInfo)
		     false
		  elseif
		     {List.some Entry.filelist
		      fun{$ F}
			 {List.some PInfo.filelist fun{$ E}
						     if E==F then Name=E true else false end
						  end}
		      end} then
		     Result=nameclash(name:Name
				      loc:Entry
				      pkg:PInfo)
		     false
		  else
		     true
		  end
	       end}
	 end
	 if {IsFree Result} then
	    %%
	    %% getting this far means the package can be installed
	    %%
	    {ForAll PInfo.filelist
	     proc{$ File}
		{A extract(File OZPMPKG#File)}
	     end}
	    %% update ozpminfo
	    {Pickle.save {Record.adjoinAt PInfo lsla PLS}|
	     {List.filter OzpmInfo
	      fun{$ Entry}
		 Entry.name\=PInfo.name %% forcing an install keeps only one version
	      end}
	     OZPMINFO}
	    Result=success(pkg:PInfo)
	    {A close}
	 end
      end
      
      meth info(Name Info)
	 L={List.dropWhile OzpmInfo
	    fun{$ Entry}
	       Entry.name\=Name
	    end}
      in
	 Info=if L==nil then notFound else L.1 end
      end

      meth infoN(Name Info)
	 L={List.dropWhile OzpmInfo
	    fun{$ Entry}
	       Entry.pkg\=Name
	    end}
      in
	 Info=if L==nil then notFound else L.1 end
      end

      meth create(In Out Inf)
	 TXT={Slurp Inf}
	 {Show TXT}
	 O={Parse TXT}
	 _={O get(id $)} %% id is mandatory
	 fun{Loop D}
	    {Show dir#D}
	    {List.map {List.filter
		       {OS.getDir {Expand D}}
		       fun{$ F} F\="." andthen F\=".." end}
	     fun{$ F}
		if {OS.stat {WithSlash D}#F}.type==dir then
		   {List.map
		    {Loop {WithSlash D}#F}
		    fun{$ G}
		       {VirtualString.toAtom {WithSlash F}#G}
		    end}
		else
		   {VirtualString.toAtom F}
		end
	     end}
	 end
	 Files={List.flatten {Loop In}}
	 Info={Record.adjoinAt
	       {Record.map
		{List.toRecord package {O entries($)}}
		List.last}
	       filelist Files}
	 F
	 MFTPKL={Expand {FullName OZPMMANIFEST In}}
	 MFTTXT={Expand {FullName OZPMMANIFESTTEXT In}}
      in
	 {Pickle.save Info MFTPKL}
	 F={New Open.file init(name:MFTTXT
			       flags:[write create])}
	 {F write(vs:TXT)}
	 {F close}
	 {Archive.makeFrom Out OZPMMANIFEST|OZPMMANIFESTTEXT|Files In}
	 {OS.unlink MFTPKL}
	 {OS.unlink MFTTXT}
      end
      
%      meth create(Package)
%	 F={ParseFile Package}
%	 fun{Filter What}
%	    {List.map 
%	     {List.filter F fun{$ I} I.1==What end}
%	     fun{$ I}
%		I.2
%	     end}
%	 end
%	 fun{Get What}
%	    Fi={Filter What}
%	 in
%	    if {Length Fi}\=1 then
%	       raise error(parameterIsNotOccuringOnce What) end
%	       nil
%	    else
%	       Fi.1
%	    end
%	 end
%	 Name={Get name}
%	 Version={Get version}
%	 Files={Filter file}
%	 Desc={Get description}
%	 Pkg={Get 'pkg-name'}
%	 Info=package(name:Name
%		      version:Version
%		      filelist:Files
%		      description:Desc
%		      pkg:Pkg
%		      text:Package)
%      in
%	 {Pickle.save Info OZPMMANIFEST}
%	 {Archive.make Pkg OZPMMANIFEST|Package|Files}
%	 {OS.unlink OZPMMANIFEST}
%      end

      meth view(Package Info LS)
	 A = {New Archive.'class' init(Package)}
      in
	 LS={A lsla($)}
	 local
	    Tmp={OS.tmpnam}
	 in
	    {A extract(OZPMMANIFEST Tmp)}
	    {Pickle.load Tmp Info}
	    {OS.unlink Tmp}
	 end
	 {A close}
      end	 

      meth check()
	 skip
      end
      
   end
   
   class InteractiveManager

      meth init()
	 {Wait OzpmInfo}
	 Look={QTk.newLook}
	 
	 MenuDesc=lr(glue:nwe
		     menubutton(text:"File" glue:w
				menu:menu(
					command(text:"Install package...")
					command(text:"Remove package...")
					separator
					command(text:"Exit"
						action:toplevel#close)))
		     menubutton(text:"Help" glue:e
				menu:menu(command(text:"Help...")
					  separator
					  command(text:"About..."
						  action:proc{$}
							    {{QTk.build td(title:"About this application..."
									   label(text:"Mozart Package Installer\nBy Denys Duchier and Donatien Grolaux\n(c) 2000\n" glue:nw)
									   button(text:"Close" glue:s action:toplevel#close))} show(modal:true wait:true)}
							 end))))
	 
	 ToolbarDesc=lr(glue:nwe relief:sunken borderwidth:1
			tbbutton(text:"Install" glue:w)
			tbbutton(text:"Remove" glue:w)
			tdline(glue:nsw)
			tbbutton(text:"Help" glue:w)
			tbbutton(text:"Quit" glue:w))
	 
	 MainWindowDesc=lrrubberframe(glue:nswe
				      td(label(text:"Installed package" glue:nw)
					 listbox(glue:nswe tdscrollbar:true lrscrollbar:true))
				      td(label(text:"Remaining packages" glue:nw)
					 listbox(glue:nswe tdscrollbar:true lrscrollbar:true)))
	 
	 StatusBar
	 
	 StatusBarDesc=placeholder(glue:swe relief:sunken borderwidth:1
				   handle:StatusBar
				   label(glue:nswe text:"Mozart Package installer"))
	 
	 Desc=td(look:Look
		 title:"Mozart Package Installer"
		 action:toplevel#close
		 MenuDesc
		 ToolbarDesc
		 MainWindowDesc
		 StatusBarDesc)

      in

	 {{QTk.build Desc} show(wait:true)}
	 
	 
	 {Application.exit 0}
      end

   end
   
   Args={Application.getArgs
	 record('install'(single type:string char:&i)
		'create'(single char:&c)
		'in'(single type:string)
		'out'(single type:string)
		'view'(single type:string char:&v)
		'list'(single char:&l)
		'info'(single type:string)
		'check'(single type:string)
		'interactive'(single)
		'package'(single type:string)
		'force'(single)
		'remove'(single type:string)
		'help'(single char:&h)
	       )}

   Action
   
   local
      Actions=[list create install check interactive view remove help]
      fun{HasFeats L}
	 {List.some L fun{$ F} {HasFeature Args F} end}
      end
   in
      if {HasFeats Actions} then % at leat one action
	 Action={List.nth
		 {List.dropWhile Actions fun{$ F} {HasFeature Args F}==false end}
		 1}
	 if {HasFeats {List.subtract Actions Action}} then % more than one action
	    {ShowInfo "Error : more than one action specified"}
	    {Application.exit 1}
	 end
      else
	 Action=interactive
      end
   end
   
   InteractiveMode=Action==interactive
   
   OzpmInfo={ByNeed fun{$}
		       Ret
		    in
		       try Ret={Pickle.load OZPMINFO}
		       catch error(url(load OZPMINFO) ...) then
			  if InteractiveMode then
			     %% application not properly installed, try to recover
			     %% interactively
			     R
			     {{QTk.build td(title:"Oz Package Manager"
					    action:toplevel#close
					    label(text:"First time installation (or recovery)\n\nContinue installation ?"
						  padx:10 pady:10)
					    lr(glue:swe
					       button(text:"Ok"
						      action:toplevel#close
						      return:R)
					       button(text:"Cancel"
						      action:toplevel#close)))} show(wait:true)}
			  in
			     if R then
				{CreatePath {Dirname OZPMINFO}}
				{Pickle.save nil OZPMINFO}
				Ret=nil
			     else {Application.exit 1} end
			  else
			     {ShowInfo "ozpm not installed properly, please start it in interactive mode or reinstall"}
			     {Application.exit 1}
			  end
		       end
		       Ret
		    end}

   ArchiveManager={New ArchiveManagerClass init}

   fun{CondFeat R F D}
      if {HasFeature R F} then R.F else D end
   end

   proc{Print VS}
      {ShowInfo if VS==nil then "" else {VirtualString.toString VS} end}
   end
  
   fun{RAlign VS I}
      {List.map
       {List.make {Max I-{Length {VirtualString.toString VS}} 0}}
       fun{$ C} C=32 end}#VS
   end
 
   fun{LAlign VS I}
      VS#{List.map
	  {List.make {Max I-{Length {VirtualString.toString VS}} 0}}
	  fun{$ C} C=32 end}
   end

   proc{PrintInfo I L}
      {Print "Package id   : "#I.id}
      {Record.forAllInd I
       proc{$ I V}
	  if I\=filelist andthen I\=id then
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
    
   case Action
   of list then % list all installed packages
      {ForAll {ArchiveManager list($)} Print}
      {Application.exit 0}
   [] view then % view the contents of a package
      I L
   in
      {ArchiveManager view(Args.'view' I L)}
      {PrintInfo I L}
      {Application.exit 0}
   [] create then % create a new package
      {Show Args.'info'}
      {Show {CondFeat Args 'info' {WithSlash {CondFeat Args 'in' "."}}#"OZPMINFO"}}
      {ArchiveManager create({CondFeat Args 'in' ""}
			     Args.'out'
			     {CondFeat Args 'info' {WithSlash {CondFeat Args 'in' "."}}#"OZPMINFO"}
			    )}
      {Application.exit 0}
   [] install then % install/update a specified package
      R
   in
      {ArchiveManager install(Args.'install' switch:if {HasFeature Args 'force'} then
						       switch(force:unit)
						    else
						       switch()
						    end result:R)}
      case {Label R}
      of success then
	 {Print "Package "#R.pkg.name#" was successfully installed"}
	 {Application.exit 0}
      [] nameclash then
	 {Print "Unable to install package '"#R.pkg.name#"'"}
	 {Print "The file '"#R.name#"' is conflicting with the installed package '"#R.loc.name#"'"}
	 {Application.exit 1}
      [] alreadyinstalled then
	 {Print "Unable to install package '"#R.pkg.name#"', version "#R.pkg.version}
	 {Print "This package is already installed in version "#R.loc.version}
	 {Application.exit 1}
      end
   [] indfo then % displays information about an installed package
      Info
   in
      {ArchiveManager info(Args.'info' Info)}
      if Info==notFound then
	 Info
      in
	 {ArchiveManager infoN(Args.'info' Info)}
	 if Info==notFound then
	    {Print "Package '"#Args.'info'#"' is not installed."}
	    {Application.exit 1}
	 else
	    {PrintInfo Info Info.lsla}
	    {Application.exit 0}
	 end	    
      else
	 {PrintInfo Info Info.lsla}
	 {Application.exit 0}
      end
   [] check then % check installed packages integrity and rebuilds if necessary
      skip
   [] remove then % removes a package
      skip
   [] help then % display some help
      Help=[""
	    "ozpm is the Oz package manager"
	    ""
	    "ozpm                   : starts ozpm in interactive mode"
	    "ozpm --help|-h|-?      : displays this help"
	    "ozpm --install|-i file : installs the package File. Install can be forced by --force"
	    "ozpm --remove pkg      : removes package whose name or package file is pkg"
	    "ozpm --list            : list all installed packages"
	    "ozpm --info pkg        : displays informations for package whose name or package file is pkg"
	    "ozpm --check           : check package system's integrity"
	    ""
	    "ozpm --create pkgdesc  : creates the package file described in the text file pkgdesc"
	    "ozpm --view pkgfile    : displays informations about the package file pkgfile"
	    ""
	   ]
   in
      {ForAll Help Print}
      {Application.exit 0}
   [] interactive then % start the application in interactive mode
      _={New InteractiveManager init}
   end

end
