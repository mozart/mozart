functor

import
   Global(background : Background
	  getParent  : GetParent
	  getLabel   : GetLabel)
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
   Tree(treeNode:TreeNode)
   

export
   dataView:TreeDataView

define
   

   PackageIcon={QTk.newImage photo(file:"package_small.gif")}
   InstalledPackageIcon={QTk.newImage photo(file:"installed_package_small.gif")}
   InstallablePackageIcon={QTk.newImage photo(file:"installable_package_small.gif")}
   FolderIcon={QTk.newImage photo(file:"folder_small.gif")}
   FolderOpenIcon={QTk.newImage photo(file:"folder_open_small.gif")}


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
					 font:{QTk.newFont font(family:'Helvetica')}
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

end
