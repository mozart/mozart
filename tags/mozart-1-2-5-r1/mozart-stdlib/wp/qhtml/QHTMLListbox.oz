functor
   
import
   QUI
   QHTMLDevel(defineWidget:DefineWidget
	      onConnect:OnConnect
	      set:Set
	      get:Get
	      send:Send
	      return:Return
	      undefined:Undefined
	      quoteDouble:QuoteDouble
	      baseClass:BaseClass
	      qInit:QInit
	      buildInnerHTML:BuildInnerHTML)
   HTMLmarshaller(vS2HTML:Vs2html)

define

   NoArgs={NewName}
   
%    fun{ToHTML V}
%       fun{Loop S}
% 	 case S
% 	 of &\\|&\\|Xs then &\\|&\\|&\\|&\\|{Loop Xs}
% 	 [] X|Xs then X|{Loop Xs}
% 	 else nil end
%       end
%    in
%       {Loop if {VirtualString.is V} then {Vs2html V} else "" end}
%    end
   
   {DefineWidget
    r(desc:listbox(action(proc{$ O A} {O bind(event:onclick action:A)} end))
      tag:select
      params:[accessKey#r(type:vs
			 init:outer(true)
			 set:outer(true)
			 get:outer(true))
	      align#r(type:[absbottom absmiddle baseline bottom left middle right texttop top]
		      unmarshall:String.toAtom
		      init:outer(true)
		      set:outer(true)
		      get:outer(true))
	      disabled#r(type:bool
			 default:false
			 init:html(true)
			 set:html(true)
			 get:internal(true))
	      multiple#r(type:bool
			 default:true
			 init:html(true)
			 set:html(true)
			 get:internal(true))
	      size#r(type:int
		     default:5
		     init:outer(true)
		     set:outer(false)
		     get:internal(true))
	      tabIndex#r(type:int
			 init:outer(true)
			 set:outer(true)
			 get:outer(true))
	      selection#r(type:lbool
			  init:custom(true)
			  set:custom(true)
			  get:custom(true))
	      list#r(type:lvs
		     default:nil
		     init:custom(true)
		     set:custom(true)
		     get:internal(true))
	      selectedIndex#r(type:int
			      init:custom(true)
			      set:custom(true)
			      get:custom(true))
	     ]
      events:nil
     )
    class $ from BaseClass
       attr list
       meth !QInit(...)=M
	  list<-nil
	  {self {Record.adjoin M set}}
       end
       meth set(...)=M
	  {Record.forAllInd M
	   proc{$ I V}
	      case I
	      of list then
		 if {self get(isdisplayed:$)} then
		    %% make a diff with @list and Add or Remove as necessary
		    proc{Loop1 I N O}
		       case N of NX|NXs then
			  fun{Search I L Ls}
			     case L
			     of X|Xs then
				if X==NX then Ls=Xs I
				else {Search I+1 Xs Ls}
				end
			     else
				false
			     end
			  end
			  Os
			  J={Search 0 O Os}
		       in
			  if J==false then
			     %% new option in the list : adds it
			     {self Add(NX I)}
			     {Loop1 I+1 NXs O}
			  else
			     %% delete option until found position
			     proc{Loop K}
				if K>0 then
				   {self Remove(I)}
				   {Loop K-1}
				end
			     end
			  in
			     {Loop J}
			     {Loop1 I+1 NXs Os}
			  end
		       else
			  % remove remaining options
			  {ForAll O proc{$ _} {self Remove(I)} end}
		       end
		    end
		 in
		    {Loop1 0 V @list}
		 end
		 list<-V
	      [] selection then
		 if {self get(isdisplayed:$)} then
		    {self Send('top.setSelection('#{self get(refId:$)}#',"'#
			       {List.map V fun{$ C} if C then &1 else &0 end end}
			       #'")')}
		 end
		 {self Set(selection V)}
		 local
		    First
		 in
		    {List.takeWhileInd V
		     fun{$ I B}
			if B then First=I false else true end
		     end _}
		    if {IsFree First} then First=0 end
		    {self Set(selectedIndex First)}
		 end
	      [] selectedIndex then
		 {self set(selection:{List.mapInd {List.make V} fun{$ I X} I==V end})}
	      end
	   end}
       end
       meth get(...)=M
	  {Record.forAllInd M
	   proc{$ I V}
	      case I
	      of selection then
		 if {self get(isdisplayed:$)} then
		    Ret={self Return('ozreturn=getSelection('#{self get(refId:$)}#')' $)}
		 in
		    V=if Ret==Undefined then Undefined else
			 {List.map Ret
			  fun{$ C} C==&1 end}
		      end
		 else
		    V={self Get(selection $)}
		 end
	      [] selectedIndex then
		 if {self get(isdisplayed:$)} then
		    Ret={self Return('ozreturn=getSelectedIndex('#{self get(refId:$)}#')' $)}
		 in
		    V=if Ret==Undefined then Undefined else
			 {String.toInt Ret}+1
		      end
		 else
		    V={self Get(selectedIndex $)}
		 end		 
	      end
	   end}
       end
       meth blur
	  {self Send({self get(refId:$)}#".blur()")}
       end
       meth focus
	  {self Send({self get(refId:$)}#".focus()")}
       end
       meth !BuildInnerHTML($)
	  {List.foldL {self Get(list $)}
	   fun{$ Old New}
	      Old#"<OPTION>"#{Vs2html New}
	   end
	   ""}
       end
       meth !OnConnect
	  OS={self Get(selection $)}
       in
	  if {List.is OS} then
	     {self set(selection:OS)}
	  end
       end
       meth add(Str Index<=NoArgs)
	  if Index==NoArgs then
	     {self Set(list {List.append {self Get(list $)} [Str]})}
	  else
	     L R
	  in
	     {List.takeDrop {self Get(list $)} Index-1 L R}
	     {self Set(list {List.append L Str|R})}
	  end
	  list<-{self Get(list $)}
	  {self Add(Str if Index==NoArgs then '"end"' else Index-1 end)}
       end
       meth Add(Str Index)
	  {self Send("top.addOption("#{self get(refId:$)}#',"'#{QuoteDouble Str}#'",'#
		     Index#")")}
       end
       meth remove(Index)
	  {self Set(list
		    {List.filterInd {self Get(list $)}
		     fun{$ I _} I\=Index end})}
	  list<-{self Get(list $)}
	  {self Remove(Index-1)}
       end
       meth Remove(Index)
	  {self Send("top.delOption("#{self get(refId:$)}#','#Index#')')}
       end
    end}

   {QUI.setAlias dropdownlistbox listbox(multiple:false size:1)}
   
end
   
      

   
