functor

import
   Archive
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
   System(show:Show
	  showInfo:Print)
   Application
   Property
   OS
   Pickle
   Open
   Browser(browse:Browse)
   FileUtils(expand:Expand withSlash:WithSlash fullName:FullName
	     exists:Exists
	     mkdir:Mkdir
	     fileTree:FileTree
	     dirname:Dirname
	     addToPath:AddToPath)
   Resolve
   Message(parse:Parse slurp:Slurp)
   InteractiveManager
   
define

   PlatformWindows={Property.get 'platform.os'}==win32   
   OZPMINFO={Expand ".ozpm.info"}
   OZPMMANIFEST='OZPMMFT.PKL'
   OZPMMANIFESTTEXT='OZPMMFT.TXT'
   OZPMPKG={Expand "~/.oz/"}
   MOGUL = "http://www.mozart-oz.org/mogul/" %"./"
   INFO  = "ozpm.info"

   proc{CreatePath P}
      if {Exists P} then skip else
	 if {Exists {Dirname P}}==false then
	    {CreatePath {Dirname P}}
	 end
	 {Mkdir P}
      end
      if {OS.stat P}.type\=dir then
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

      meth list(L prefix:Prefix<=OZPMPKG)
	 L={List.map OzpmInfo fun{$ R} R.id end}
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
	 if {CondSelect Switch force false} then %% always install, ignore all tests
	    skip
	 else
	    _={List.takeWhile OzpmInfo
	       fun{$ Entry}
		  Name
	       in
		  if Entry.id==PInfo.id then %% Same package
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
	    {Print Prefix}
	    {ForAll PInfo.filelist
	     proc{$ File}
		{CreatePath {Dirname {AddToPath Prefix File}}}
		{A extract(File {AddToPath Prefix File})}
	     end}
	    %% update ozpminfo
	    {Pickle.save {Record.adjoinAt PInfo lsla PLS}|
	     {List.filter OzpmInfo
	      fun{$ Entry}
		 Entry.id\=PInfo.id %% forcing an install keeps only one version
	      end}
	     OzpmInfoFile
	    }
	    Result=success(pkg:PInfo)
	    {A close}
	 end
      end
      
      meth info(Id1 Info)
	 Id={ByteString.make Id1}
	 L={List.dropWhile OzpmInfo
	    fun{$ Entry}
	       Entry.id\=Id
	    end}
      in
	 Info=if L==nil then notFound else L.1 end
      end

      meth create(In Out Inf)
	 TXT={Slurp Inf}
	 O={Parse TXT}
	 _={O get(id $)} %% id is mandatory
	 FT={FileTree if In=="" then "." else In end}
	 Files
	 local
	    L={Length {VirtualString.toString FT.path}}+1
	    fun{Loop R}
	       if R.type==dir then
		  {List.map R.entries Loop}
	       else
		  {String.toAtom {List.drop {VirtualString.toString R.path} L}}
	       end
	    end
	 in
	    Files={List.subtract {List.subtract {List.flatten {Loop FT}} OZPMMANIFEST} OZPMMANIFESTTEXT}
	    {ForAll Files Print}
	 end
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
	 {Show 1}
	 {Archive.makeFrom Out OZPMMANIFEST|OZPMMANIFESTTEXT|Files In}
	 {Show 2}
	 {OS.unlink MFTPKL}
	 {OS.unlink MFTTXT}
      end
      
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

   
   Args={Application.getArgs
	 record('action'(single type:atom(install create info check interactive remove help) default:interactive)
		%%
		%% aliases for actions
		%%
		'install'(alias:['action'#install '<install>'#true])
		'create'( alias:['action'#create  '<create>' #true])
		'info'(   alias:['action'#info    '<info>'   #true])
		'list'(   alias:['action'#list    '<list>'   #true])
		'check'(  alias:['action'#check   '<check>'  #true])
		'interactive'(alias:['action'#interactive '<interactive>'#true])
		'remove'( alias:['action'#remove  '<remove>' #true])
		'help'(   alias:['action'#help    '<help>'   #true])
		%%
		%% corresponding flags
		%%
		'<install>'(single type:bool)
		'<create>'( single type:bool)
		'<info>'(   single type:bool)
		'<list>'(   single type:bool)
		'<check>'(  single type:bool)
		'<interactive>'(single type:bool)
		'<remove>'( single type:bool)
		'<help>'(   single type:bool)
		%%
		%% arguments
		%%
		'in'(    single type:string
			 validate:alt(when('<create>' optional)
				      when(disj('<install>' '<info>' '<remove>') true)
				      when(true false)))
		'prefix'(single type:string optional:true)
		'out'(   single type:string
			 validate:alt(when('<create>' true)
				      when(true false)))
		'force'( single type:bool default:false)
		'update'(single type:bool default:false)
	       )}

   
   Action = Args.action
   InteractiveMode=Action==interactive

   Prefix={CondSelect Args prefix OZPMPKG}
   
   OzpmInfoFile={AddToPath Prefix OZPMINFO}

   
   OzpmInfo={ByNeed fun{$}
		       Ret
		    in
		       try
			  Ret={Pickle.load OzpmInfoFile}
		       catch error(url(load ...) ...) then
			  {CreatePath {Dirname OzpmInfoFile}}
			  {Pickle.save nil OzpmInfoFile}
			  Ret=nil
		       end
		       Ret
		    end}
   
%   OzpmInfo={ByNeed fun{$}
%		       Ret
%		    in
%		       try Ret={Pickle.load OZPMINFO}
%		       catch error(url(load OZPMINFO) ...) then
%			  if InteractiveMode then
%			     %% application not properly installed, try to recover
%			     %% interactively
%			     R
%			     {{QTk.build td(title:"Oz Package Manager"
%					    action:toplevel#close
%					    label(text:"First time installation (or recovery)\n\nContinue installation ?"
%						  padx:10 pady:10)
%					    lr(glue:swe
%					       button(text:"Ok"
%						      action:toplevel#close
%						      return:R)
%					       button(text:"Cancel"
%						      action:toplevel#close)))} show(wait:true)}
%			  in
%			     if R then
%				{CreatePath {Dirname OZPMINFO}}
%				{Pickle.save nil OZPMINFO}
%				Ret=nil
%			     else {Application.exit 1} end
%			  else
%			     {ShowInfo "ozpm not installed properly, please start it in interactive mode or reinstall"}
%			     {Application.exit 1}
%			  end
%		       end
%		       Ret
%		    end}

   ArchiveManager={New ArchiveManagerClass init}

  
   fun{RAlign VS I}
      {List.map
       {List.make {Max I-{Length {VirtualString.toString VS}} 0}}
       fun{$ C} C=&  end}#VS
   end
 
   fun{LAlign VS I}
      VS#{List.map
	  {List.make {Max I-{Length {VirtualString.toString VS}} 0}}
	  fun{$ C} C=&  end}
   end

   
   proc{PrintInfo I L}
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

   
   case Action
   of list then % list all installed packages
      {ForAll {ArchiveManager list($)} Print}
      {Application.exit 0}
   [] info then % view the contents of a package
      %%
      %% determine it is a package file or an installed file
      %%
      I L
   in
      %% first : try to use it as a file
      try
	 {ArchiveManager view(Args.'in' I L)}
      catch _ then
	 %% second : consider it as an installed package
	 {ArchiveManager info(Args.'in' I)}
	 if I==notFound then
	    {Print "Package '"#Args.'in'#"' is not found and not installed."}
	    {Application.exit 1}
	 else
	    L=I.lsla
	 end
      end
      {PrintInfo I L}
      {Application.exit 0}
   [] create then % create a new package
      {ArchiveManager create({CondSelect Args 'prefix' ""}
			     Args.'out'
			     {CondSelect Args 'in' {AddToPath {CondSelect Args 'prefix' "."} OZPMINFO}})}
      {Application.exit 0}
   [] install then % install/update a specified package
      R
   in
      {ArchiveManager install(Args.'in' switch:switch(force:Args.'force') result:R)}
      case {Label R}
      of success then
	 {Print "Package "#R.pkg.id#" was successfully installed"}
	 {Application.exit 0}
      [] nameclash then
	 {Print "Unable to install package '"#R.pkg.id#"'"}
	 {Print "The file '"#R.id#"' is conflicting with the installed package '"#R.loc.id#"'"}
	 {Application.exit 1}
      [] alreadyinstalled then
	 if {HasFeature R.pkg version} then
	    {Print "Unable to install package '"#R.pkg.id#"', version "#R.pkg.version}
	 else
	    {Print "Unable to install package '"#R.pkg.id#"'"}
	 end
	 if {HasFeature R.loc version} then
	    {Print "This package is already installed in version "#R.loc.version}
	 else
	    {Print "A package of the same id is already installed"}
	 end
	 {Application.exit 1}
      end
%   [] indfo then % displays information about an installed package
%      Info
%   in
%      {ArchiveManager info(Args.'info' Info)}
%      if Info==notFound then
%	 Info
%      in
%	 {ArchiveManager infoN(Args.'info' Info)}
%	 if Info==notFound then
%	    {Print "Package '"#Args.'info'#"' is not installed."}
%	    {Application.exit 1}
%	 else
%	    {PrintInfo Info Info.lsla}
%	    {Application.exit 0}
%	 end	    
%      else
%	 {PrintInfo Info Info.lsla}
%	 {Application.exit 0}
%      end
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
      {New InteractiveManager.'class' init(OzpmInfo ArchiveManager) _}
   end

end
