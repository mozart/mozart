functor

import
   Global(background : Background
	  getParent  : GetParent
	  getLabel   : GetLabel
	  getImage   : GetImage)
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
   Tree(treeNode:TreeNode)
   

export
   dataView:CatDataView

define
   

   PackageIcon={GetImage 'package_small'}
   InstalledPackageIcon={GetImage 'installed_package_small'}
   InstallablePackageIcon={GetImage 'installable_package_small'}
   FolderIcon={GetImage 'folder_small'}
   FolderOpenIcon={GetImage 'folder_open_small'}


   class CatDataView
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
	 else skip end
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
					 label:"category")}
	    dictNode<-{NewDictionary}
	    {Dictionary.put @dictNode unit @rootNode}
	    local
	       proc{CreateNodes X Icon}
		  C={CondSelect X categories [undefined]}
	       in
		  {ForAll C
		   proc{$ Cat}
		      ParentNode={Dictionary.condGet @dictNode Cat
				  {ByNeed fun{$}
					     Node={New TreeNode
						   init(parent:@rootNode
							icon:FolderIcon
							eicon:FolderOpenIcon
							label:{VirtualString.toString Cat})}
					  in
					     {Dictionary.put @dictNode Cat Node}
					     {@rootNode addLeaf(node:Node)}
					     {Node expand}
					     Node
					  end}}
		      {Wait ParentNode}
		      Node={New TreeNode init(parent:ParentNode
					      icon:Icon
					      eicon:nil
					      label:{VirtualString.toString X.id})}
		   in
		      {ParentNode addLeaf(node:Node)}
		      {Node expand}
		      {Node bind(event:"<1>"
				 action:{self.parent.toplevel newAction(self#select(X Node) $)})}
		   end}
	       end
	    in
	       for
		  X in @info.info
		  I in 1 ; I+1
	       do
		  {CreateNodes X
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
	 C=CatDataView
      end
      meth select(Entry Node)
	 if @sel\=nil then {@sel select(state:false)} end
	 sel<-Node
	 {Node select(state:true)}
	 {self.parent displayInfo(r(info:Entry))}
      end
   end

end
