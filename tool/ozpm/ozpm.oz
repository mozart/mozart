functor

import Archive QTk
   System(show:Show
	  showInfo:ShowInfo)
   Application
   Property
   OS
   Pickle
   Open
   Browser(browse:Browse)
   
define

   PlatformWindows={Property.get 'platform.os'}==win32
   
   OZPMINFO="~/.oz/ozpm/ozpm.info"
   OZPMMANIFEST='OZPM-MANIFEST'

   fun{ExtractPath P}
      {Reverse
       {List.dropWhile
	{Reverse P}
	fun{$ C}
	   (PlatformWindows andthen C=="\\") orelse
	   (PlatformWindows==false andthen C=="/")
	end}}
   end

   proc{CreatePath P}
      Stat
   in
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

   class TextFile from Open.text Open.file end
   
   fun{ParseFile F}
      File={New TextFile init(name:F)}
      fun{Loop}
	 Str={File getS($)}
      in
	 if Str==false then nil else
	    Left Right
	 in
	    {List.takeDropWhile Str fun{$ C} C\=&: end Left Right}
	    r({String.toAtom Left}
	      {List.drop Right 1})|{Loop}
	 end
      end
   in
      {Loop}
   end
   
   class ArchiveManagerClass

      meth init skip end

      meth list()
	 
	 skip
      end

      meth install(Package switch:S<=none)
	 A = {New Archive.'class' init(Package)}
	 TLS={A lsla($)}
	 Info
      in
	 local
	    Tmp={OS.tmpnam}
	 in
	    {A extract(OZPMMANIFEST Tmp)}
	    {Pickle.load Tmp Info}
	    {OS.unlink Tmp}
	 end
	 %% now we have the information and file names of the package


	 
	 {A close}
      end
      
      meth info()
	 skip
      end

      meth create(Package)
	 F={ParseFile Package}
	 fun{Filter What}
	    {List.map 
	     {List.filter F fun{$ I} I.1==What end}
	     fun{$ I}
		I.2
	     end}
	 end
	 fun{Get What}
	    Fi={Filter What}
	 in
	    if {Length Fi}\=1 then
	       raise error(parameterIsNotOccuringOnce What) end
	       nil
	    else
	       Fi.1
	    end
	 end
	 Name={Get name}
	 Version={Get version}
	 Files={Filter file}
	 Desc={Get description}
	 Pkg={Get 'pkg-name'}
	 Info=r(name:Name
		version:Version
		filelist:Files
		description:Desc
		pkg:Pkg
		text:Package)
      in
	 {Pickle.save Info OZPMMANIFEST}
	 {Archive.make Pkg OZPMMANIFEST|Package|Files}
	 {OS.unlink OZPMMANIFEST}
      end

      meth view(Package Info LS)
	 A = {New Archive.'class' init(Package)}
	 TLS={A lsla($)}
	 InfoA
      in
	 local
	    Tmp={OS.tmpnam}
	 in
	    {A extract(OZPMMANIFEST Tmp)}
	    {Pickle.load Tmp Info}
	    {OS.unlink Tmp}
	 end
	 InfoA={VirtualString.toAtom Info.text}
	 LS={List.filter TLS
	     fun{$ E}
		E.path\=OZPMMANIFEST andthen
		E.path\=InfoA
	     end}
	 {A close}
      end	 

      meth check()
	 skip
      end
      
   end
   
   class InteractiveManager

      meth init()
	 skip
      end

   end
   
   Args={Application.getArgs
	 record('install'(single type:string char:&i)
		'create'(single type:string char:&c)
		'view'(single type:string char:&v )
		'list'(char:&l alias:'action'#list)
		'info'(single type:string)
		'check'(single type:string)
		'interactive'(single)
		'package'(single type:string)
	       )}

   Action
   
   local
      Actions=[list create install info check interactive view]
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
				{CreatePath {ExtractPath OZPMINFO}}
				{Pickle.save nil OZPMINFO}
				Ret=nil
			     else {Application.exit 1} end
			  else
			     {ShowInfo "ozpm not installed properly, please start it in interactive mode or reinstall"}
			     {Application.exit 1}
			  end
		       end
		       Ret
		    end

   ArchiveManager={New ArchiveManagerClass init}

   fun{CondFeat R F D}
      if {HasFeature R F} then R.F else D end
   end

   proc{Print VS}
      {ShowInfo {VirtualString.toString VS}}
   end

   fun{LAlign VS I}
      VS#{List.map
	  {List.make {Max I-{Length {VirtualString.toString VS}} 0}}
	  fun{$ C} C=32 end}
   end
   
   case Action
   of list then % list all installed packages
      skip
   [] view then % view the contents of a package
      I L
   in
      {ArchiveManager view(Args.'view' I L)}
      {Print "Package name : "#I.name}
      {Print "version      : "#I.version}
      {Print "description  : "#I.description}
      {Print " "}
      {Print "Contains the following files :"}
      {ForAll L
       proc{$ R}
	  {Print {LAlign R.size 20}#" "#R.path}
       end}
      {Application.exit 0}
   [] create then % create a new package
      {ArchiveManager create(Args.'create')}
      {Application.exit 0}
   [] install then % install/update a specified package
      {ArchiveManager install(Args.'install')}
      {Application.exit 0}
   [] info then % displays information about an installed package
      skip
   [] check then % check installed packages integrity and rebuilds if necessary
      skip
   [] interactive then % start the application in interactive mode
      _={New InteractiveManager init}
   end

   {Browse Args}

end
