functor
export 'class' : InteractiveManager
import
   Application
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
   Look
   Global(localDB
	  packageMogulDB
	  authorMogulDB
	  background       : Background
	  getLabel         : GetLabel
	  getImage         : GetImage)
   ActionInstall(install:Install)
   ActionRemove(remove:Remove)
   ActionInfo(view)
   System(show:Show)
%   Browser(browse:Browse)
   FileUtils(isExtension:IsExtension)
   OS(stat unlink)
   Open
   Resolve(localize)
   InteractiveListDataView(dataView:ListDataView)
   InteractiveTreeDataView(dataView:TreeDataView)
   InteractiveCategoryDataView(dataView:CatDataView)
   InteractiveFullInfoView(infoView:InfoView
			   simpleInfoView:NiceInfoView)
   
define
   
   class InteractiveManager

      feat
	 dataPlace
	 dataLabel
	 infoPlace
	 infoLabel
	 installButton
	 desinstallButton
	 downloadButton
	 toplevel
	 installTbButton
	 mogulTbButton
	 fileTbButton
	 treeTbButton
	 listTbButton
	 catTbButton
	 niceTbButton
	 allTbButton
      attr
	 data
	 info
	 curpkg

      meth run
	 InteractiveManager,init
      end
	 
      meth init%(OI AM)
	 InfoMain
	 DataMain
	 %%
	 info<-{New NiceInfoView init(self
				      proc{$ Title} {self.infoLabel set(text:Title)} end
				      InfoMain)}
	 data<-{New TreeDataView init(self
				      proc{$ Title} {self.dataLabel set(text:Title)} end
				      DataMain)}
	 InstallM
	 DesinstallM
	 DownloadM
	 
	 MenuDesc=
	 lr(glue:nwe
	    menubutton(
	       action:proc{$}
			 {InstallM set(state:{self.installButton get(state:$)})}
			 {DesinstallM set(state:{self.desinstallButton get(state:$)})}
			 {DownloadM set(state:{self.downloadButton get(state:$)})}
		      end
	       text:'File'
	       glue:w
	       menu:menu(
		       command(text:'Install package...'
			       handle:InstallM
			       action:self#install)
		       command(text:'Desinstall package...'
			       handle:DesinstallM
			       action:self#desinstall)
		       command(text:'Download package...'
			       handle:DownloadM
			       action:self#download)
		       separator
		       command(text:'Check database integrity...'
			       action:self#check)
		       separator
		       command(text:'Exit'
			       accelerator:alt(x)
			       action:toplevel#close)))
	    menubutton(
	       text:'View'
	       glue:w
	       menu:menu(
		       command(text:'Installed packages'
			       action:self#displayInstalled)
		       command(text:'Mogul entries'
			       action:self#displayMogul)
		       command(text:'Package from file...'
			       action:self#displayFile)
		       separator
		       command(text:'Packages as tree'
			       action:self#displayDataAs(TreeDataView)
			      )
		       command(text:'Packages as list'
			       action:self#displayDataAs(ListDataView))
		       command(text:'Packages sorted by categories'
			       action:self#displayDataAs(CatDataView))
		       separator
		       command(text:'Formatted package information'
			       action:self#displayInfoAs(NiceInfoView))
		       command(text:'Full package information'
			       action:self#displayInfoAs(InfoView))
		       ))
	    menubutton(
	       text:'Help'
	       glue:e
	       menu:menu(command(text:'Help...')
			 separator
			 command(text:'About...'
				 action:proc{$}
					   {{QTk.build
					     td(title:'About this application...'
						label(text:'Mozart Package Installer\n'#
						      'By Denys Duchier and Donatien Grolaux\n'#
						      '(c) 2000\n'
						      glue:nw)
						button(text:'Close'
						       glue:s
						       action:toplevel#close))}
					    show(modal:true wait:true)}
					end))))
	 %%
	 ToolbarLook=Look.toolbar
	 ToolbarDesc=lr(glue:nwe relief:sunken borderwidth:1
			look:ToolbarLook
			tbradiobutton(%text:'Installed'
				      image:{GetImage 'installed_package_small'}
				      tooltips:'Display installed packages'
				      init:true
				      action:self#displayInstalled
				      handle:self.installTbButton
				      group:viewmode)
			tbradiobutton(%text:'Mogul'
				      image:{GetImage 'mogul_icon'}
				      tooltips:'Display mogul entries'
				      action:self#displayMogul
				      handle:self.mogulTbButton
				      group:viewmode)
			tbradiobutton(%text:'File...'
				      image:{GetImage 'open_icon'}
				      tooltips:'Display the contents of a file package'
				      action:self#displayFile
				      handle:self.fileTbButton
				      group:viewmode)
			tdspace
			tbradiobutton(%text:'Tree'
				      image:{GetImage 'tree_icon'}
				      tooltips:'Display packages as a tree'
				      init:true
				      group:dataview
				      handle:self.treeTbButton
				      action:self#displayDataAs(TreeDataView))
			tbradiobutton(%text:'List'
				      image:{GetImage 'list_icon'}
				      tooltips:'Display packages as a list'
				      group:dataview
				      handle:self.listTbButton
				      action:self#displayDataAs(ListDataView))
			tbradiobutton(text:'cat'
				      tooltips:'Display packages according to their categories'
				      group:dataview
				      handle:self.catTbButton
				      action:self#displayDataAs(CatDataView))
			tdspace
			tbradiobutton(%text:'Nice'
				      image:{GetImage 'formated_icon'}
				      tooltips:'Display package informations in a formatted way'
				      init:true
				      group:infoview
				      handle:self.niceTbButton
				      action:self#displayInfoAs(NiceInfoView))
			tbradiobutton(%text:'All'
				      image:{GetImage 'unformated_icon'}
				      tooltips:'Display all package informations as a list'
				      group:infoview
				      handle:self.allTbButton
				      action:self#displayInfoAs(InfoView))
		       )
				 
%			tbbutton(text:'Install' glue:w)
%			tbbutton(text:'Remove' glue:w)
%			tdline(glue:nsw)
%			tbbutton(text:'Help' glue:w)
%			tbbutton(text:'Quit' glue:w))
	 %%
	 MainWindowDesc=
	 tdrubberframe(glue:nswe
%		       continue:true
		       td(glue:nwe
			  lr(glue:we
			     label(look:Look.title
				   handle:self.dataLabel)
			     tbbutton(text:"Detach" look:Look.button
				      action:self#detach(data)
				      glue:e))
			  placeholder(handle:self.dataPlace glue:nswe
				      DataMain
				     ))
		       td(glue:nwe
			  lr(glue:we
			     label(look:Look.title
				   handle:self.infoLabel)
			     tbbutton(text:"Detach" look:Look.button
				      action:self#detach(info)
				      glue:e))
			  placeholder(handle:self.infoPlace glue:nswe
				      InfoMain
				     )
			 ))
	 %%
	 ActionButtonLook=Look.actionButton
	 ActionBarDesc=
	 lr(glue:swe
	    look:ActionButtonLook
	    button(text:"Install"
		   handle:self.installButton
		   action:self#install)
	    button(text:"Desinstall"
		   handle:self.desinstallButton
		   action:self#desinstall)
	    button(text:"Download"
		   handle:self.downloadButton
		   action:self#download)
	   )
	 %%
	 StatusBar
	 StatusBarDesc=
	 placeholder(glue:swe relief:sunken borderwidth:1
		     handle:StatusBar
		     label(glue:nswe text:'Mozart Package installer'))
	 %%
	 Desc=td(look:Look.main
		 title:'Mozart Package Installer'
		 action:toplevel#close
		 MenuDesc
		 ToolbarDesc
		 MainWindowDesc
		 ActionBarDesc
		 StatusBarDesc)
	 Window={QTk.build Desc}
      in
	 {Window show}
	 self.toplevel=Window
	 {Wait Global.localDB}
	 {Wait Global.packageMogulDB}
	 {self displayInstalled}
	 {Window wait}
	 {Application.exit 0}
      end

      meth detach(W)
	 What=if W==data then @data else @info end
	 Class={What getClass($)}
	 Window
	 Desc
	 N={New Class init(self
			   proc{$ Title} {Window set(title:Title)} end
			   Desc)}
	 Window={QTk.build td(Desc)}
	 {N display({What get($)})}
      in
	 {Window show}
      end

      meth displayDataAs(Class)
	 case Class
	 of !ListDataView then {self.listTbButton set(true)}
	 [] !TreeDataView then {self.treeTbButton set(true)}
	 [] !CatDataView then {self.catTbButton set(true)}
	 end
	 Desc
	 Info={@data get($)}
      in
	 data<-{New Class init(self
			       proc{$ Title} {self.dataLabel set(text:Title)} end
			       Desc)}
	 {self.dataPlace set(Desc)}
	 {@data display(Info)}
      end

      meth displayInfoAs(Class)
	 if Class==NiceInfoView then
	    {self.niceTbButton set(true)}
	 else
	    {self.allTbButton set(true)}
	 end
	 Desc
	 Info={@info get($)}
      in
	 info<-{New Class init(self
			       proc{$ Title} {self.infoLabel set(text:Title)} end
			       Desc)}
	 {self.infoPlace set(Desc)}
	 {@info display(Info)}
      end
      
      meth displayInstalled
	 {self.installTbButton set(true)}
	 {@data display(r(info:{Global.localDB items($)}
			  title:"Local Installation"))}
      end

      meth displayMogul
	 {self.mogulTbButton set(true)}
	 {@data display(r(info:{Global.packageMogulDB items($)}
			  title:"MOGUL Archive"))}
      end

      meth displayFile
	 {self.fileTbButton set(true)}
	 File={QTk.dialogbox load($
				  filetypes:q(q("Mozart Packages" "*.pkg"))
				  title:"Select a package")}
      in
	 if File==nil then
	    {@data display(r(info:nil title:nil))}
	 else
	    Package#_={ActionInfo.view File}
	    Info={Record.adjoinAt Package
		  url_pkg [File]}
	 in
	    {@data display(r(info:[Info]
			     title:File))}
	    {self displayInfo(r(info:Info))}
	 end
      end

      meth state(Pkg $)
	 case {Label Pkg}
	 of ipackage then installed
	 [] package then
	    if {Global.localDB member(Pkg.id $)} then installedable
	    elseif {HasFeature Pkg url_pkg} andthen
	       {List.some Pkg.url_pkg fun{$ URL}
					 {IsExtension "pkg" URL}
				      end}
	    then
	       installable
	    else
	       normal
	    end
	 end
      end
      
      meth displayInfo(Info)
	 {ForAll [installButton desinstallButton downloadButton]
	  proc{$ B} {self.B set(state:disabled)} end}
	 if Info==unit then
	    curpkg<-unit
	 else
	    curpkg<-Info.info
	    case {self state(@curpkg $)}
	    of installed then
	       {self.desinstallButton set(state:normal)}
	    [] installedable then
	       {self.desinstallButton set(state:normal)}
	       {self.installButton set(state:normal)}
	       {self.downloadButton set(state:normal)}
	    [] installable then
	       {self.installButton set(state:normal)}
	       {self.downloadButton set(state:normal)}
	    else
	       {self.downloadButton set(state:normal)}
	    end
	 end
	 {@info display({Record.adjoinAt Info info
			 {Record.adjoin
			  {Global.packageMogulDB condGet(Info.info.id r $)}
			  Info.info}}
		       )}
      end

      meth filterPkg(Pkg Cond $)
	 %% filter Pkg.url_pkg according to filter
	 %% add the equivalent mogul adress
	 fun{Mogul Name}
	    %%
	    %% Takes a uri, keeps the last part and adds the correct
	    %% mogul header
	    %%
	    {VirtualString.toString
	     "http://www.mozart-oz.org/mogul/pkg/"#
	     {List.drop {VirtualString.toString Pkg.id} 7}#"/"#
	     {Reverse
	      {List.takeWhile {Reverse {VirtualString.toString Name}}
	       fun{$ C} C\=&/ end}}}
	 end
	 fun{Loop L}
	    case L of X|Xs then
	       if {Cond X} then
		  if {Global.packageMogulDB member(Pkg.id $)} then
		     {VirtualString.toString X}|{Mogul X}|{Loop Xs}
		  else
		     {VirtualString.toString X}|{Loop Xs}
		  end
	       else
		  {Loop Xs}
	       end
	    else nil end
	 end
      in
	 {Loop Pkg.url_pkg}
      end
      
      meth install(pkg:Pkg<=@curpkg nu:Nu<=0 force:Force<=false leave:Leave<=false)
	 Packages={self filterPkg(Pkg fun{$ URL} {IsExtension "pkg" URL} end $)}
	 N
      in
	 if Nu==0 then
	    N={self selectPackages("Install a package" Packages $)}
	 else
	    N=Nu
	 end
	 if N>0 then
	    ToInstall={List.nth Packages N}
	 in
	    case {Install ToInstall Force Leave}
	    of success(pkg:P) then
	       {{QTk.build td(title:"Package installation"
			      label(padx:10 pady:10
				    text:P.id#" was successfully installed")
			      button(glue:s
				     text:"Close"
				     action:toplevel#close))} show(wait:true modal:true)}
	       {self displayInstalled}
	    []  nameclash(L) then
	       Return
	    in
	       {self conflict("Package installation"
			      "Files are conflicting with other installed packages"
			      "Overwrite these files"
			      "Don't overwrite these files"
			      L
			      Return)}
	       if Return\=cancel then
		  {self install(pkg:Pkg nu:N force:Return==choice1 leave:Return==choice2)}
	       end
	    [] alreadyinstalled(loc:L pkg:P) then
	       UnInstall Overwrite
	    in
	       {{QTk.build td(title:"Installation failed"
			      label(padx:10 pady:10
				    text:
				       if {HasFeature P version} then
					  "Unable to install package '"#P.id#"', version "#P.version
				       else
					  "Unable to install package '"#P.id#"'"
				       end#"\n"#
				    if {HasFeature L version} then
				       "This package is already installed in version "#L.version
				    else
				       "A package of the same id is already installed"
				    end)
			      lr(glue:swe
				 button(text:"Uninstall first"
					tooltips:"Properly uninstall the old package, then install this one"
					return:UnInstall
					action:toplevel#close)
				 button(text:"Overwrite installation"
					tooltips:"Install this package on top of the old one.\nUse with care..."
					return:Overwrite
					action:toplevel#close)
				 button(text:"Cancel"
					action:toplevel#close)))} show(wait:true modal:true)}
	       if UnInstall then
		  %% first uninstall the other package
		  {self desinstall(pkg:Pkg)}
		  {self install(pkg:Pkg nu:N)}
	       elseif Overwrite then
		  %% force installation of this package
		  {self install(pkg:Pkg nu:N force:true)}
	       end
	    end
	 end
      end

      meth desinstall(pkg:Pkg<=@curpkg)
	 Confirm
      in
	 {{QTk.build td(title:"Package removal"
			label(text:"Are you sure you want to completely remove package : "#Pkg.id#" ?"
			      padx:10
			      pady:10)
			lr(glue:swe
			   button(text:"Ok"
				  action:toplevel#close
				  return:Confirm)
			   button(text:"Cancel"
				  action:toplevel#close)))} show(wait:true modal:true)}
	 if Confirm then %% starts to remove the package
	    case {Remove Pkg.id false false}
	    of success(pkg:P) then
	       {{QTk.build td(title:"Package removal"
			      label(text:"Package "#P.id#" was successfully uninstalled")
			      button(text:"Close" glue:s action:toplevel#close))}
		show(wait:true modal:true)}
	       {self displayInstalled}
	    [] notFound then
	       {{QTk.build td(label(text:"No package "#Pkg.id#" currently installed")
			      button(text:"Close" glue:s action:toplevel#close))}
		show(wait:true modal:true)}
	    [] conflict(L) then
	       Return
	    in
	       {self conflict("Package removal"
			      "Files are used by other packages"
			      "Leave these files"
			      "Remove these files also"
			      L
			      Return)}
	       if Return\=cancel then
		  case {Remove Pkg.id Return==choice2 Return==choice1}
		  of success(pkg:Pkg) then
		     {{QTk.build td(title:"Package removal"
				    label(text:"Package "#Pkg.id#" was successfully uninstalled")
				    button(text:"Close" glue:s action:toplevel#close))}
		      show(wait:true modal:true)}
		     {self displayInstalled}
		  else
		     {{QTk.build td(title:"Package removal"
				    label(text:"Unable to remove package, unexpected error !")
				    button(text:"Close" glue:s action:toplevel#close))}
		      show(wait:true modal:true)}
		  end
	       end
	    end
	 else skip end
      end

      meth download(pkg:Pkg<=@curpkg nu:Nu<=0)
	 Packages={self filterPkg(Pkg fun{$ _} true end $)}
	 N
      in
	 if Nu==0 then
	    N={self selectPackages("Download a package" Packages $)}
	 else
	    N=Nu
	 end
	 if N>0 then
	    ToInstall={List.nth Packages N}
	    SaveFile={QTk.dialogbox save($
					 title:"Save as..."
					 initialfile:{Reverse
						      {List.takeWhile
						       {Reverse ToInstall}
						       fun{$ C}
							  C\=&/
						       end}})}
	 in
	    if SaveFile\=nil then
	       In
	       Out
	       PackageFile
	       proc{Copy}
		  Buffer
	       in
		  {In read(list:Buffer
			   size:8096)}
		  if Buffer==nil then
		     {In close}
		     {Out close}
		  else
		     {Out write(vs:Buffer)}
		     {Copy}
		  end
	       end
	    in
	       try
		  PackageFile = {Resolve.localize ToInstall}
		  In={New Open.file init(name:PackageFile.1)}
		  Out={New Open.file init(name:SaveFile
					  flags:[write create])}
		  {Copy}
		  try
		     if {OS.stat SaveFile}.type==reg then
			{{QTk.build td(title:"Download package"
				       label(text:"Downdload successfull")
				       button(text:"Close"
					      action:toplevel#close
					      glue:s))}
			 show(wait:true modal:true)}
		     else
			raise error end
		     end
		  catch _ then
		     {{QTk.build td(title:"Download Package"
				    label(text:"An unexpected error occured")
				    button(text:"Close"
					   action:toplevel#close
					   glue:s))}
		      show(wait:true modal:true)}
		  end
%		  {Show download#{VirtualString.toAtom ToInstall}}
%		  {Show to#SaveFile}
	       finally
		  if {IsDet In} then try {In close} catch _ then skip end end
		  if {IsDet Out} then try {Out close} catch _ then skip end end
		  case PackageFile of new(F) then
		     {OS.unlink F}
		  end
	       end
	    end
	 end
      end

      meth conflict(Title Label Choice1 Choice2 FileList Return)
	 Place Leave More Ok
      in
	 {{QTk.build td(title:Title
			label(text:Label)
			radiobutton(text:Choice1
				    group:leaveOrForce
				    glue:sw
				    return:Leave)
			radiobutton(text:Choice2
				    glue:w
				    group:leaveOrForce)
			placeholder(handle:Place glue:nswe)
			lr(glue:swe
			   button(text:"More..."
				  handle:More
				  action:proc{$}
					    if {More get(text:$)}=="More..." then
					       %% place file informations
					       fun{Loop L I}
						  case L
						  of X|Xs then
						     I#label(text:{VirtualString.toString X.loc.id}
							     glue:w)|I+1#label(text:{VirtualString.toString X.name} glue:w)|I+2#newline|{Loop Xs I+3}
						  else nil
						  end
					       end
					    in
					       {More set(text:"Less...")}
					       {Place set(
							 scrollframe(glue:nswe
								     tdscrollbar:true
								     lrscrollbar:true
								     {List.toRecord lr
								      glue#nswe|
								      1#label(text:"Package"  glue:we relief:raised borderwidth:1)|
								      2#label(text:"Filename" glue:we relief:raised borderwidth:1)|
								      3#newline|{Loop FileList 4}}
								    ))}
					    else
					       {More set(text:"More...")}
					       {Place set(empty)}
					    end
					 end)
			   button(text:"Ok"
				  action:toplevel#close
				  return:Ok)
			   button(text:"Cancel"
				  action:toplevel#close)))} show(wait:true modal:true)}
	 Return=if Ok then
		   if Leave then choice1
		   else choice2 end
		else
		   cancel
		end
      end

      meth selectPackages(Title Packages $)
	 LB
	 Ok
	 N={NewCell 1}
      in
	 {{QTk.build td(title:Title
			listbox(padx:10 pady:10
				glue:nswe
				tdscrollbar:true
				handle:LB
				action:proc{$}
					  {Assign N {LB get(firstselection:$)}}
				       end
				height:{Length Packages}
				width:{List.foldL Packages
				       fun{$ Z S}
					  if Z>{Length S} then Z
					  else {Length S} end
				       end 0}
				init:Packages)
			lr(glue:swe
			   button(text:"Ok"
				  action:toplevel#close
				  return:Ok)
			   button(text:"Cancel"
				  action:toplevel#close)))} show(modal:true)}
	 {LB set(selection:[true])} % select the first element
	 {Wait Ok} % wait for the window to close
	 if Ok==false then 0 else {Access N} end
      end

      meth check
	 skip
      end
      
   end
end
