functor
export 'class' : InteractiveManager
import
   Application
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
   Global(localDB packageMogulDB authorMogulDB)
   ActionInstall(install:Install)
   ActionRemove(remove:Remove)
   ActionInfo(view)
   System(show:Show)
%   Browser(browse:Browse)
   FileUtils(isExtension:IsExtension)
   Tree(treeNode:TreeNode)
   String(capitalize:Capitalize) at 'x-ozlib://duchier/lib/String.ozf'
define

   PackageIcon={QTk.newImage photo(file:"package_small.gif")}
   InstalledPackageIcon={QTk.newImage photo(file:"installed_package_small.gif")}
   InstallablePackageIcon={QTk.newImage photo(file:"installable_package_small.gif")}
   FolderIcon={QTk.newImage photo(file:"folder_small.gif")}
   FolderOpenIcon={QTk.newImage photo(file:"folder_open_small.gif")}
   Background=c(240 250 242)


   %% two functions on mogul names
   
   fun{GetParent X}
      %%
      %% returns the parent mogul name
      %% ignore last /, my return a name with an ending /
      %%
      {VirtualString.toAtom
       {Reverse
	{List.dropWhileInd {Reverse {VirtualString.toString X}}
	 fun{$ I C} C\=&/ orelse I==1 end}}}
   end
   fun{GetLabel X}
      %%
      %% returns the last basename of a mogul name
      %% ignore last last /
      %%
      VS={VirtualString.toString
	  {Reverse
	   {List.takeWhileInd {Reverse {VirtualString.toString X}}
	    fun{$ I C} C\=&/ orelse I==1 end}}}
   in
      if {List.last VS}\=&/ then
	 VS
      else
	 {List.take VS {Length VS}-1}
      end
   end	       

   class ListDataView
      feat
	 parent
	 setTitle
	 handle
      attr
	 info
	 title
      meth init(Parent ST Desc)
	 self.parent=Parent
	 self.setTitle=ST
	 Desc=listbox(glue:nswe
		      bg:Background
		      tdscrollbar:true
		      action:self#select
		      handle:self.handle)
      end
      meth display(Info)
	 info<-{Record.adjoinAt
		Info
		info
		{List.sort Info.info
		 fun{$ A B} {VirtualString.toAtom A.id}<{VirtualString.toAtom B.id} end}}
	 {self.setTitle Info.title}
	 {self.handle set({List.map @info.info fun{$ I} I.id end})}
     end
      meth get(Info)
	 Info=@info
	 skip
      end
      meth getClass(C)
	 C=ListDataView
      end
      meth select
	 N={self.handle get(firstselection:$)}
	 D
      in
	 try
	    Info={List.nth @info.info N}
	 in
	    D=r(info:Info)
	 catch _ then D=unit end
	 {self.parent displayInfo(D)}
      end
   end

   

   class TreeDataView
      feat
	 parent
	 setTitle
	 handle
	 allTag
	 selTag
      attr
	 info
	 title
	 rootNode
	 dictNode
	 sel:nil
      meth init(Parent ST Desc)
	 self.parent=Parent
	 self.setTitle=ST
	 rootNode<-nil
	 Desc=canvas(glue:nswe
		     bg:Background
		     tdscrollbar:true
		     lrscrollbar:true
		     handle:self.handle)
      end
%      meth setScrollbar
%	 {self.handle set(scrollregion:{List.toRecord q
%					{List.mapInd
%					 {self.allTag bbox($)}
%					 fun{$ I V} I#V end}
%				       })}
%      end
      meth display(Info)
	 if {IsFree self.allTag} then
	    self.allTag={self.handle newTag($)}
	    {self.allTag addtag(withtag all)}
	    self.selTag={self.handle newTag($)}
	 else
	    {self.selTag delete}
	 end
	 info<-Info
	 {self.setTitle Info.title}
	 if @rootNode\=nil then
	    {@rootNode delete(height:_)}
	 end
	 if Info.info==nil then
	    rootNode<-nil
	 else
	    rootNode<-{New TreeNode init(canvas:self.handle
					 font:"Times 12"
					 height:18
					 icon:FolderIcon
					 eicon:FolderOpenIcon
					 label:"mogul")}
	    dictNode<-{NewDictionary}
	    {Dictionary.put @dictNode 'mogul:/' @rootNode}
	    local
	       fun{ToKey X}
		  {VirtualString.toAtom
		   if {List.last {VirtualString.toString X}}\=&/ then
		      X#"/" else X end}
	       end
	       proc{CreateNode I X Icon}
		  Parent={GetParent X}
		  ParentNode={Dictionary.condGet @dictNode Parent
			      {ByNeed fun{$}
					 {CreateNode 0 Parent FolderIcon}
					 {Dictionary.get @dictNode Parent}
				      end}
			     }
		  {Wait ParentNode}
		  Node={New TreeNode init(parent:ParentNode
					  icon:Icon
					  eicon:if I\=0 then
						   nil
						else
						   FolderOpenIcon
						end
					  label:{GetLabel X})}
	       in
		  {Dictionary.put @dictNode {ToKey X} Node}
		  {ParentNode addLeaf(node:Node)}
		  {Node expand}
		  if I\=0 then
		     {Node bind(event:"<1>"
				action:{self.parent.toplevel newAction(self#select(I Node) $)})}
		  end
	       end
	    in
	       for
		  X in @info.info
		  I in 1 ; I+1
	       do
		  {CreateNode I X.id
		   case {self.parent state(X $)}
		   of installed then InstalledPackageIcon
		   [] installedable then InstalledPackageIcon
		   [] installable then InstallablePackageIcon
		   else PackageIcon end}
	       end
	    end
	    {@rootNode draw(x:2 y:2 height:_)}
	    {@rootNode expand}
	 end
	 %%
	 %% 
	 %%
     end
      meth get(Info)
	 Info=@info
	 skip
      end
      meth getClass(C)
	 C=TreeDataView
      end
      meth select(I Node)
	 Info={List.nth @info.info I}
	 D=r(info:Info)
      in
	 if @sel\=nil then {@sel select(state:false)} end
	 sel<-Node
	 {Node select(state:true)}
	 {self.parent displayInfo(D)}
      end
   end
   %%
   %%
   %%
   class InfoView
      feat
	 setTitle
	 handle
	 parent
      attr
	 info:unit
      meth init(Parent ST Desc)
	 self.parent=Parent
	 self.setTitle=ST
	 Desc=listbox(glue:nswe
		      bg:Background
		      tdscrollbar:true
		      lrscrollbar:true
		      handle:self.handle)
      end
      meth display(Inf)
	 info<-Inf
	 {self.handle set(nil)}
	 if Inf==unit then
	    {self.setTitle ""}
	 else
	    Info=Inf.info
	 in
	    {self.setTitle {VirtualString.toString Info.id}}
	    local
	       L1={List.filter
		   {List.map
		    {Record.toListInd Info}
		    fun{$ En}
		       I#V=En
		    in
		       case I
		       of lsla then unit
		       [] filelist then unit
		       [] id then unit
		       else
			  if {List.is V} then
			     I#":"#{List.drop
				    {VirtualString.toString
				     {List.foldL V fun{$ S X} S#","#X end ""}}
				    1}
			  elseif {VirtualString.is V} then
			     I#":"#{VirtualString.toString V}
			  else
			     unit
			  end
		       end
		    end}
		   fun{$ L} L\=unit end}
	       L2=if {HasFeature Info filelist} then
		     ""|"Contains the following files :"|""|{List.map Info.filelist fun{$ R} R end}
		  else
		     nil
		  end
	       L={List.append L1 L2}
	    in
	       {self.handle set(L)}
	    end
	 end
      end
      meth get(Info)
	 Info=@info
	 skip
      end
      meth getClass(C)
	 C=InfoView
      end
   end
   
   fun{AuthorsToString L}
      fun{AuthorIDToName ID}
	 {Global.authorMogulDB condGet({VirtualString.toAtom ID} ID $)}.name
      end
   in
      if {List.is L} then
	 {List.drop
	  {VirtualString.toString
	   {List.foldL L fun{$ S X} S#"\n"#{AuthorIDToName X} end ""}
	  } 1}
      else
	 {VirtualString.toString {AuthorIDToName L}}
      end
   end
   %%
   %%
   %%
   TitleLook={QTk.newLook}
   {TitleLook.set label(bg:red
			glue:we
			fg:white
			font:{QTk.newFont font(family:'Times'
					       weight:bold
					       size:16)})}
   AuthorLook={QTk.newLook}
   {AuthorLook.set label(bg:Background
			 font:{QTk.newFont font(family:'Times'
						slant:italic
						size:12)})}

   DescLook={QTk.newLook}
   {DescLook.set text(bg:Background
		      font:{QTk.newFont font(family:'Times'
					     size:12)})}
					      
   
   WhiteLook={QTk.newLook}
   {WhiteLook.set label(bg:Background)}
   {WhiteLook.set td(bg:Background)}
   {WhiteLook.set lr(bg:Background)}
   
   class NiceInfoView
      feat
	 setTitle
	 handle
	 parent
      attr
	 info:unit
      meth init(Parent ST Desc)
	 self.parent=Parent
	 self.setTitle=ST
	 Desc=td(glue:nswe
		 bg:Background
		 look:WhiteLook
%		 tdscrollbar:true
		 td(glue:nswe
		    handle:self.handle
		    label(glue:nwe
			  look:TitleLook
			  feature:title)
		    lr(glue:nwe
		       feature:author
		       label(glue:w
			     feature:desc)
		       label(glue:nwe
			     anchor:nw
			     justify:left
			     look:AuthorLook
			     feature:data))
		    lr(glue:nswe
		       feature:blurb
		       label(glue:nw
			     feature:desc)
		       placeholder(feature:place
				   bg:Background
				   padx:5 pady:5
				   glue:nswe))
		))
      end
      meth display(Inf)
	 info<-Inf
	 {self.handle.title set("")}
	 {self.handle.author.desc set("")}
	 {self.handle.author.data set("")}
	 {self.handle.blurb.desc set("")}
	 {self.handle.blurb.place set(empty)}
	 if Inf==unit then
	    {self.setTitle ""}
	 else
	    Info=Inf.info
	    fun{ListToString L}
	       {List.map
		if {List.is L} then
		   {List.drop
		    {VirtualString.toString
		     {List.foldL L fun{$ S X} S#"\n"#X end ""}
		    } 1}
		else
		   {VirtualString.toString L}
		end
		fun{$ C} if C==&\n then & else C end end}
	    end
	 in
	    {self.setTitle {VirtualString.toString Info.id}}
	    {self.handle.title set({Capitalize {GetLabel Info.id}})}
	    if {CondSelect Info author unit}\=unit then 
	       {self.handle.author.desc set("Author : ")}
	       {self.handle.author.data set({AuthorsToString Info.author})}
%					   {ListToString Info.author})}
	    else
	       {self.handle.author.desc set("No author defined.")}
	    end
	    local
	       Desc=if {HasFeature Info body} andthen
		       Info.body\=unit andthen
		       {VirtualString.toString Info.body}\=nil then
		       Info.body
		    elseif {HasFeature Info blurb} andthen
		       Info.blurb\=unit andthen
		       {VirtualString.toString Info.blurb}\=nil then
		       Info.blurb
		    else
		       ""
		    end
	       Handle
	    in
	       if Desc\=nil then
		  {self.handle.blurb.desc set("Description : ")}
		  {self.handle.blurb.place
		   set(text(glue:nswe
			    look:DescLook
			    bg:Background
			    handle:Handle
			    wrap:word
			    tdscrollbar:true
			    init:{ListToString Desc}))}
		  {Handle set(state:disabled)}
	       else
		  {self.handle.blurb.desc set("No description available.")}
	       end
	    end
	 end
      end
      meth get(Info)
	 Info=@info
	 skip
      end
      meth getClass(C)
	 C=NiceInfoView
      end
   end
   
   class InteractiveManager

      feat
	 dataPlace
	 dataLabel
	 infoPlace
	 infoLabel
	 installButton
	 installTbButton
	 desinstallButton
	 downloadButton
	 toplevel

      attr
	 data
	 info
	 curpkg

      meth run
	 InteractiveManager,init
      end
	 
      meth init%(OI AM)
	 Look={QTk.newLook}
	 TitleLook={QTk.newLook}
	 InfoMain
	 DataMain
	 {TitleLook.set label(text:"" glue:nwes bg:darkblue fg:white relief:sunken borderwidth:2 justify:left anchor:w)}
	 %%
	 info<-{New NiceInfoView init(self
				      proc{$ Title} {self.infoLabel set(text:Title)} end
				      InfoMain)}
	 data<-{New TreeDataView init(self
				      proc{$ Title} {self.dataLabel set(text:Title)} end
				      DataMain)}
	 MenuDesc=
	 lr(glue:nwe
	    menubutton(
	       text:'File'
	       glue:w
	       menu:menu(
		       command(text:'Install package...')
		       command(text:'Remove package...')
		       separator
		       command(text:'Exit' action:toplevel#close)))
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
	 ToolbarLook={QTk.newLook}
	 {ToolbarLook.set tbradiobutton(glue:w pady:2)}
	 {ToolbarLook.set tdline(glue:nsw)}
	 ToolbarDesc=lr(glue:nwe relief:sunken borderwidth:1
			look:ToolbarLook
			tbradiobutton(text:'Installed'
				      init:true
				      action:self#displayInstalled
				      handle:self.installTbButton
				      group:viewmode)
			tbradiobutton(text:'Mogul'
				      action:self#displayMogul
				      group:viewmode)
			tbradiobutton(text:'File...'
				      action:self#displayFile
				      group:viewmode)
			tdline
			tbradiobutton(text:'Tree'
				      init:true
				      group:dataview
				      action:self#displayDataAs(TreeDataView))
			tbradiobutton(text:'List'
				      group:dataview
				      action:self#displayDataAs(ListDataView))
			tdline
			tbradiobutton(text:'Nice'
				      init:true
				      group:infoview
				      action:self#displayInfoAs(NiceInfoView))
			tbradiobutton(text:'All'
				      group:infoview
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
		       td(glue:nwe
			  lr(glue:we
			     label(look:TitleLook
				   handle:self.dataLabel)
			     tbbutton(text:"Detach"
				      action:self#detach(data)
				      glue:e))
			  placeholder(handle:self.dataPlace glue:nswe
				      DataMain
				     ))
		       td(glue:nwe
			  lr(glue:we
			     label(look:TitleLook
				   handle:self.infoLabel)
			     tbbutton(text:"Detach"
				      action:self#detach(info)
				      glue:e))
			  placeholder(handle:self.infoPlace glue:nswe
				      InfoMain
				     )
			 ))
	 %%
	 ActionButtonLook={QTk.newLook}
	 {ActionButtonLook.set button(glue:w
				      padx:5
				      state:disabled)}
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
	 Desc=td(look:Look
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
			  title:"Installed package"))}
      end

      meth displayMogul
	 {@data display(r(info:{Global.packageMogulDB items($)}
			  title:"Available packages from MOGUL"))}
      end

      meth displayFile
	 File={QTk.dialogbox load($
				  filetypes:q(q("Mozart Packages" "*.pkg"))
				  title:"Select a package")}
      in
	 if File==nil then skip else
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
	       {Show download#{VirtualString.toAtom ToInstall}}
	       {Show to#SaveFile}
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
      
   end
end
