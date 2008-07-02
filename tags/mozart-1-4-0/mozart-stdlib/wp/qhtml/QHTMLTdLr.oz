functor
   
import
   QUI
   QHTMLDevel(defineWidget:DefineWidget
	      onConnect:OnConnect
	      onDisconnect:OnDisconnect
	      baseClass:BaseClass
	      setTooltips:SetTooltips
	      sync:Sync
	      join:Join
	      buildHTML:BuildHTML
	      buildInnerHTML:BuildInnerHTML
	      qInit:QInit)
   
define

   MBuildChild=QUI.mBuildChild
   MGetChildren=QUI.mGetChildren

   %% td and lr widgets
   
   local

      fun{DefAlign G}
	 if G.exph then
	    ""
	 elseif G.w then
	    " align=left"
	 elseif G.e then
	    " align=right"
	 else
	    " align=middle"
	 end#
	 if G.expv then
	    ""
	 elseif G.n then
	    " vAlign=top"
	 elseif G.s then
	    " vAlign=bottom"
	 else
	    " vAlign=middle"
	 end
      end
      
      fun{Count T}
	 {List.foldL T
	  fun{$ O N}
	     O+if N then 1 else 0 end
	  end 0}
      end

      fun{Pct N}
	 (N div 100)#"."#(N mod 100)
      end

      fun{BuildLR Array TblInfo}
	 MH#MV={Count TblInfo.v}#{Count TblInfo.h}
	 fun{Loop Line Info Divider Divisor}
	    case Line
	    of X|Xs then
	       Y|Ys=Info
	       N=if Y then (Divider div Divisor) else 0 end
	       Width="width="#if Y then '"'#{Pct N}#'%"' else "0" end
	    in
	       case X
	       of empty(...) then "<TD "#Width#"></TD>\n"#{Loop Xs Ys Divider-N
							   Divisor-if N==0 then 0 else 1 end}
	       [] continue(...) then {Loop Xs Ys Divider-N
				      Divisor-if N==0 then 0 else 1 end}
	       else
		  "<TD"#{DefAlign X.glue}#" "#Width#" COLSPAN="#{QUI.continueLength Xs}+1#">\n"#{X.child BuildHTML($)}#"\n</TD>\n"#{Loop Xs Ys Divider-N Divisor-if N==0 then 0 else 1 end}
	       end
	    else nil end
	 end
	 fun{LoopLines Lines Info Divider Divisor}
	    case Lines
	    of X|Xs then
	       Y|Ys=Info
	       N=if Y then (Divider div Divisor) else 0 end
	       Height="height="#if Y then '"'#{Pct N}#'%"' else "0" end
	    in
	       "<TR "#Height#">"#
	       {Loop X TblInfo.h 10000 MV}#"</TR>\n"#{LoopLines Xs Ys Divider-N
						      Divisor-if N==0 then 0 else 1 end}
	    else nil end
	 end
      in
	 {LoopLines Array TblInfo.v 10000 MH}
      end

      fun{BuildTD Array TblInfo}
	 MH#MV={Count TblInfo.v}#{Count TblInfo.h}
	 fun{Loop Line Info Divider Divisor}
	    case Line
	    of X|Xs then
	       E|Es=X
	       Y|Ys=Info
	       N=if Y then (Divider div Divisor) else 0 end
	       Width="width="#if Y then '"'#{Pct N}#'%"' else "0" end
	    in
	       case E
	       of empty(...) then "<TD "#Width#"></TD>\n"#{Loop Xs Ys Divider-N
							   Divisor-if N==0 then 0 else 1 end}
	       [] continue(...) then {Loop Xs Ys Divider-N
				      Divisor-if N==0 then 0 else 1 end}
	       else
		  "<TD"#{DefAlign E.glue}#" "#Width#" ROWSPAN="#{QUI.continueLength Es}+1#">\n"#{E.child BuildHTML($)}#"\n</TD>\n"#{Loop Xs Ys Divider-N Divisor-if N==0 then 0 else 1 end}
	       end
	    else nil end
	 end
	 fun{LoopColumns Columns Info Divider Divisor}
	    if Columns.1==nil then nil
	    else
	       Xs={List.map Columns
		   fun{$ C} {List.drop C 1} end}
	       Y|Ys=Info
	       N=if Y then (Divider div Divisor) else 0 end
	       Height="height="#if Y then '"'#{Pct N}#'%"' else "0" end
	    in
	       "<TR "#Height#">"#
	       {Loop Columns TblInfo.h 10000 MV}#"</TR>\n"#{LoopColumns Xs Ys Divider-N
							    Divisor-if N==0 then 0 else 1 end}
	    end
	 end
      in
	 {LoopColumns Array TblInfo.v 10000 MH}
      end

      proc{Define What Build}
	 {DefineWidget
	  r(desc:{Record.adjoin
		  r(container
		    tooltips(proc{$ O V} {O SetTooltips(V)} end)) What}
	    tag:table
	    params:[background#r(type:html
				 init:outer(true)
				 set:outer(true)
				 get:internal(true))
% 		    bgColor#r(type:color
% 			      init:outer(true)
% 			      set:outer(true)
% 			      get:internal(true))
		    border#r(type:int
			     default:0
			     init:outer(false)
			     set:outer(false)
			     get:internal(false))
		    cellSpacing#r(type:int
				  default:0
				  init:outer(false)
				  set:outer(false)
				  get:internal(false))
		    cellPadding#r(type:int
				  default:0
				  init:outer(false)
				  set:outer(false)
				  get:internal(false))
		    dir#r(type:[ltr rtl]
			  init:outer(true)
			  set:outer(true)
			  get:internal(true)
			  unmarshall:String.toAtom)
		    summary#r(type:html
			      init:outer(true)
			      set:outer(true)
			      get:internal(true))
		   ]
	    events:nil)
	  class $ from BaseClass
	     feat
		Array
		TblInfo
	     meth !QInit(...)=M
		Tab={QUI.toArray M}
	     in
		self.Array={List.map Tab
			    fun{$ Line}
			       {List.map Line
				fun{$ Elem}
				   case {Label Elem}
				   of continue then continue
				   [] empty then empty
				   else
				      Child={self MBuildChild(Elem $)}
				   in
				      r(glue:Elem.glue
					child:Child)
				   end
				end}
			    end}
		self.TblInfo={QUI.calcLR Tab What==lr}
	     end
	     meth !BuildInnerHTML($)
		{Build self.Array self.TblInfo}
	     end
	     meth Broadcast(M)
		{ForAll self.Array
		 proc{$ Line}
		    {ForAll Line
		     proc{$ E}
			if {HasFeature E child} then {E.child M} end
		     end}
		 end}
	     end
	     meth !Sync=M
		{self Broadcast(M)}
	     end
	     meth !OnConnect=M
		{self Broadcast(M)}
	     end
	     meth !OnDisconnect=M
		{self Broadcast(M)}
	     end
	  end}
      end
   in
      {Define lr BuildLR}
      {Define td BuildTD}
   end

   {DefineWidget
    r(desc:table(container
		 tooltips(proc{$ O V} {O SetTooltips(V)} end))
      tag:table
      params:[background#r(type:html
			   init:outer(true)
			   set:outer(true)
			   get:internal(true))
	      bgColor#r(type:color
			init:outer(true)
			set:outer(true)
			get:internal(true))
	      border#r(type:int
		       init:outer(true)
		       set:outer(true)
		       get:internal(true))
	      borderColor#r(type:color
			    init:outer(true)
			    set:outer(true)
			    get:internal(true))
	      borderColorDark#r(type:color
				init:outer(true)
				set:outer(true)
				get:internal(true))
	      borderColorLight#r(type:color
				 init:outer(true)
				 set:outer(true)
				 get:internal(true))
	      cellPadding#r(type:int
			    init:outer(true)
			    set:outer(true)
			    get:internal(true))
	      cellSpacing#r(type:int
			    init:outer(true)
			    set:outer(true)
			    get:internal(true))
	      cols#r(type:int
		     init:outer(true)
		     set:outer(true)
		     get:internal(true))
	      dir#r(type:[ltr rtl]
		    init:outer(true)
		    set:outer(true)
		    get:outer(true)
		    unmarshall:String.toAtom)
	      frame#r(type:[above below border box hsides lhs rhs void vsides]
		      init:outer(true)
		      set:outer(true)
		      get:outer(true)
		      unmarshall:String.toAtom)
	      rules#r(type:[all cols groups none rows]
		      init:outer(true)
		      set:outer(true)
		      get:outer(true)
		      unmarshall:String.toAtom)
	      tabIndex#r(type:int
			 init:html(true)
			 set:html(true)
			 get:html(true))
	      summary#r(type:html
			init:outer(true)
			set:outer(true)
			get:outer(true))
	     ]
      events:nil)
    class $ from BaseClass
       meth !QInit(...)=M
	  {Record.forAll
	   {Record.filterInd M
	    fun{$ I _} {Int.is I} end}
	   proc{$ D}
	      if {Record.is D} andthen
		 ({Label D}==tr) then
		 {self MBuildChild(D _)}
	      else
		 {QUI.raiseError self.widgetId
		  "Only tr widgets can be placed inside table widgets" M}
	      end
	   end}
       end
       meth !BuildInnerHTML($)
	  {Join
	   {List.map {Reverse {self MGetChildren($)}} fun{$ C} {C BuildHTML($)} end}
	   ""}
       end
       meth Broadcast(M)
	  {ForAll {self MGetChildren($)}
	   proc{$ C}
	      {C M}
	   end}
       end
       meth !Sync=M
	  {self Broadcast(M)}
       end
       meth !OnConnect=M
	  {self Broadcast(M)}
       end
       meth !OnDisconnect=M
	  {self Broadcast(M)}
       end
    end}

   {DefineWidget
    r(desc:tr(container)
      tag:tr
      params:[bgColor#r(type:color
			init:outer(true)
			set:outer(true)
			get:internal(true))
	      borderColor#r(type:color
			    init:outer(true)
			    set:outer(true)
			    get:internal(true))
	      borderColorDark#r(type:color
				init:outer(true)
				set:outer(true)
				get:internal(true))
	      borderColorLight#r(type:color
				 init:outer(true)
				 set:outer(true)
				 get:internal(true))
	     ]
      events:nil)
    class $ from BaseClass
       meth !QInit(...)=M
	  {Record.forAll
	   {Record.filterInd M
	    fun{$ I _} {Int.is I} end}
	   proc{$ D}
	      if {Record.is D} andthen
		 ({Label D}==tc) then
		 {self MBuildChild(D _)}
	      else
		 {QUI.raiseError self.widgetId
		  "Only tc widgets can be placed inside tr widgets" M}
	      end
	   end}
       end
       meth !BuildInnerHTML($)
	  {Join
	   {List.map {Reverse {self MGetChildren($)}} fun{$ C} {C BuildHTML($)} end}
	   ""}
       end
       meth Broadcast(M)
	  {ForAll {self MGetChildren($)}
	   proc{$ C}
	      {C M}
	   end}
       end
       meth !Sync=M
	  {self Broadcast(M)}
       end
       meth !OnConnect=M
	  {self Broadcast(M)}
       end
       meth !OnDisconnect=M
	  {self Broadcast(M)}
       end
    end}

   {DefineWidget
    r(desc:tc(container)
      tag:td
      params:[bgColor#r(type:color
			init:outer(true)
			set:outer(true)
			get:internal(true))
	      borderColor#r(type:color
			    init:outer(true)
			    set:outer(true)
			    get:internal(true))
	      borderColorDark#r(type:color
				init:outer(true)
				set:outer(true)
				get:internal(true))
	      borderColorLight#r(type:color
				 init:outer(true)
				 set:outer(true)
				 get:internal(true))
	     ]
      events:nil)
    class $ from BaseClass
       meth !QInit(...)=M
	  if {HasFeature M 2} then
	     {QUI.raiseError self.widgetId
	      "Only one widget can be placed inside tc container" M}
	  elseif {HasFeature M 1} andthen {Record.is M.1} then
	     {self MBuildChild(M.1 _)}
	  elseif {HasFeature M 1} then
	     {QUI.raiseError self.widgetId
	      "Invalid widget definition :"#M.1 M}
	  end
       end
       meth !BuildInnerHTML($)
	  Child={self MGetChildren($)}
       in
	  if Child==nil then "" else
	     {{List.last Child} BuildHTML($)}
	  end
       end
       meth Broadcast(M)
	  {ForAll {self MGetChildren($)}
	   proc{$ C} {C M} end}
       end
       meth !Sync=M
	  {self Broadcast(M)}
       end
       meth !OnConnect=M
	  {self Broadcast(M)}
       end
       meth !OnDisconnect=M
	  {self Broadcast(M)}
       end
    end}
   
   
end
   
      

   
