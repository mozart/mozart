functor
   
import
   QHTMLDevel(defineWidget:DefineWidget
	      baseClass:BaseClass
	      sync:Sync
	      onConnect:OnConnect
	      onDisconnect:OnDisconnect
	      qInit:QInit
	      buildHTML:BuildHTML
	      buildInnerHTML:BuildInnerHTML)
   QUI
   
define

   MBuildChild=QUI.mBuildChild
   MGetChildren=QUI.mGetChildren
   
   fun{Purge Str}
      case Str
      of &\n|Cs then &&|&#|&1|&3|&;|{Purge Cs}
      [] &\r|Cs then &&|&#|&1|&0|&;|{Purge Cs}
      [] C|Cs then C|{Purge Cs}
      else nil end
   end

   SimpleHtml={NewName}
   
   {DefineWidget
    r(desc:SimpleHtml
      tag:span
      params:[1#r(type:no
		  init:custom(true)
		  set:custom(false)
		  get:custom(false))]
      events:nil)
    class $ from BaseClass
       feat text
       meth !QInit(...)=M
	  self.text={Purge M.1}
       end
       meth !BuildInnerHTML($) self.text end
    end}
   
   {DefineWidget
    r(desc:html(container)
      tag:span
      params:nil
      events:nil)
    class $ from BaseClass
       meth !QInit(...)=M
	  {Record.forAll M
	   proc{$ V}
	      if {VirtualString.is V} then
		 {self MBuildChild(SimpleHtml(V) _)}
	      elseif {Record.is V} then
		 {self MBuildChild(V _)}
	      else
		 {QUI.raiseError self.widgetd "Invalid html declaration :"#V M}
	      end
	   end}
       end
       meth !BuildInnerHTML($)
	  {List.foldL {Reverse {self MGetChildren($)}}
	   fun{$ O N}
	      O#if {VirtualString.is N} then N else {N BuildHTML($)} end
	   end
	   ""}
       end
       meth Broadcast(M)
	  {ForAll {self MGetChildren($)}
	   proc{$ C} if {Object.is C} then {C M} end end}
       end
       meth !Sync
	  {self Broadcast(Sync)}
       end
       meth !OnConnect
	  {self Broadcast(OnConnect)}
       end
       meth !OnDisconnect
	  {self Broadcast(OnDisconnect)}
       end       
    end
   }   

end
   
      

   
