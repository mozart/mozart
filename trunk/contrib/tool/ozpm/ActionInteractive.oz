functor
export 'class' : InteractiveManager
import
   Application
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
   Global(localDB mogulDB readDB)
   ActionInstall(install:Install)
   ActionInfo(view)
   System(show:Show)
   Browser(browse:Browse)
   FileUtils(isExtension:IsExtension)
   Tree(treeNode:TreeNode)
define

   ArchiveManager
   OzpmInfo

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
		      bg:white
		      tdscrollbar:true
		      action:self#select
		      handle:self.handle)
      end
      meth display(Info)
	 info<-Info
	 {self.setTitle Info.title}
	 {self.handle set({List.map Info.info fun{$ I} I.id end})}
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
      attr
	 info
	 title
	 rootNode
	 dictNode
      meth init(Parent ST Desc)
	 self.parent=Parent
	 self.setTitle=ST
	 rootNode<-nil
	 Desc=canvas(glue:nswe
		     bg:white
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
					 height:16
					 label:"mogul")}
	    dictNode<-{NewDictionary}
	    {Dictionary.put @dictNode 'mogul:/' @rootNode}
	    local
	       fun{ToKey X}
		  {VirtualString.toAtom
		   if {List.last {VirtualString.toString X}}\=&/ then
		      X#"/" else X end}
	       end
	       proc{CreateNode I X}
		  Parent={GetParent X}
		  ParentNode={Dictionary.condGet @dictNode Parent
			      {ByNeed fun{$}
					 {CreateNode 0 Parent}
					 {Dictionary.get @dictNode Parent}
				      end}
			     }
		  {Wait ParentNode}
		  Node={New TreeNode init(parent:ParentNode
					  label:{GetLabel X})}
	       in
		  {Dictionary.put @dictNode {ToKey X} Node}
		  {ParentNode addLeaf(node:Node)}
		  {Node expand}
		  if I\=0 then
		     {Node bind(event:"<1>"
				action:{self.parent.toplevel newAction(self#select(I) $)})}
		  end
	       end
	    in
	       for
		  X in @info.info
		  I in 1 ; I+1
	       do
		  {CreateNode I X.id}
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
      meth select(I)
	 Info={List.nth @info.info I}
	 D=r(info:Info)
      in
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
	 info
      meth init(Parent ST Desc)
	 self.parent=Parent
	 self.setTitle=ST
	 Desc=listbox(glue:nswe
		      bg:white
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
   %%
   %%
   %%
   TitleLook={QTk.newLook}
   {TitleLook.set label(font:{QTk.newFont font(family:'Times' size:16)}
			
		       )}
   
   class NiceInfoView
      feat
	 setTitle
	 handle
	 parent
      attr
	 info
      meth init(Parent ST Desc)
	 self.parent=Parent
	 self.setTitle=ST
	 Desc=scrollframe(glue:nswe
			  bg:white
			  tdscrollbar:true
			  td(glue:nswe
			     handle:self.handle
			     label(glue:nwe
				   look:TitleLook
				   feature:title)
			     lr(glue:nwe
				feature:f1
				label(glue:w
				      text:"Author : ")
				label(glue:nw
				      look:AuthorLook
				      feature:author))
			     lr(glue:nwe
				feature:f2
				label(glue:nw
				      text:"Description : ")
				label(glue:nswe
				      anchor:nw
				      justify:left
				      feature:description))
			     
			     ))
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
	       {ForAll L proc{$ Line} {self.handle insert('end' Line#"\n")} end}
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
	 desinstallButton
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
	 info<-{New InfoView init(self
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
				      action:self#detach(@data)
				      glue:e))
			  placeholder(handle:self.dataPlace glue:nswe
				      DataMain
				     ))
		       td(glue:nwe
			  lr(glue:we
			     label(look:TitleLook
				   handle:self.infoLabel)
			     tbbutton(text:"Detach"
				      action:self#detach(@info)
				      glue:e))
			  placeholder(handle:self.infoPlace glue:nswe
				      InfoMain
				     )
			 ))
	 %%
	 ActionBarDesc=
	 lr(glue:swe
	    button(glue:w
		   text:"Install"
		   handle:self.installButton
		   action:self#install
		   state:disabled)
	    button(glue:w
		   text:"Desinstall"
		   handle:self.desinstallButton
		   action:self#desinstall
		   state:disabled))
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
	 {Wait Global.mogulDB}
	 {self displayInstalled}
	 {Window wait}
	 {Application.exit 0}
      end

      meth detach(What)
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
	 {@data display(r(info:Global.localDB title:"Installed package"))}
      end

      meth displayMogul
	 {@data display(r(info:Global.mogulDB.packages title:"Available packages from MOGUL"))}
      end

      meth displayFile
	 File={QTk.dialogbox load($
				  filetypes:q(q("Mozart Packages" "*.pkg"))
				  title:"Select a package")}
      in
	 if File==nil then skip else
	    Package#List={ActionInfo.view File}
	    Info={Record.adjoinAt Package
		  url_pkg [File]}
	 in
	    {@data display(r(info:[Info]
			     title:File))}
	    {self displayInfo(r(info:Info))}
	 end
      end

      meth displayInfo(Info)
	 {ForAll [installButton desinstallButton]
	  proc{$ B} {self.B set(state:disabled)} end}
	 if Info==unit then
	    curpkg<-unit
	 else
	    curpkg<-Info.info
	    case {Label @curpkg}
	    of ipackage then {self.desinstallButton set(state:normal)}
	    [] package then
	       if {HasFeature @curpkg url_pkg} andthen
		  {List.some @curpkg.url_pkg fun{$ URL} {IsExtension "pkg" URL} end}
	       then
		  {self.installButton set(state:normal)}
	       end
	    else skip end
	 end
	 {@info display(Info)}
      end

      meth install
	 Packages={List.filter @curpkg.url_pkg fun{$ URL} {IsExtension "pkg" URL} end}
	 LB
	 Ok
	 N={NewCell 1}
      in
	 {{QTk.build td(title:"Install a package"
			listbox(padx:10 pady:10
				glue:nswe
				tdscrollbar:true
				handle:LB
				action:proc{$}
					  {Assign N {LB get(firstselection:$)}}
				       end
				height:{Length Packages}
				init:Packages)
			lr(glue:swe
			   button(text:"Ok"
				  action:toplevel#close
				  return:Ok)
			   button(text:"Cancel"
				  action:toplevel#close)))} show(modal:true)}
	 {LB set(selection:[true])} % select the first element
	 {Wait Ok} % wait for the window to close
	 if Ok andthen {Access N}>0 then
	    ToInstall={List.nth Packages {Access N}}
	 in
	    case {Install ToInstall false}
	    of success(pkg:P) then
	       {{QTk.build td(title:"Installation succeeded"
			      label(padx:10 pady:10
				    text:P.id#" was successfully installed")
			      button(glue:s
				     text:"Close"
				     action:toplevel#close))} show(wait:true modal:true)}
	       {self displayInstalled}
	    []  nameclash(name:N loc:L pkg:P) then
	       {{QTk.build td(title:"Installation failed"
			      label(padx:10 pady:10
				    text:"Unable to install package '"#P.id#"'\n"#
				    "The file '"#N#"' is conflicting with the installed package '"#L.id#"'")
			      button(glue:s
				     text:"Close"
				     action:toplevel#close))} show(wait:true modal:true)}
	    [] alreadyinstalled(loc:L pkg:P) then
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
			      button(glue:s
				     text:"Close"
				     action:toplevel#close))} show(wait:true modal:true)}
	    end
	 end
      end

      meth desinstall
	 {{QTk.build td(label(text:"Not yet implemented")
			button(text:"Close" action:toplevel#close
			       glue:s))} show(wait:true modal:true)}
      end
      
   end
end
