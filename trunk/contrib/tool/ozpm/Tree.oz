functor
   
import Tk

export TreeNode

define

   NoArgs={NewName}
   
   class TreeNode
      
      feat canvas font height vtag htag btag itag ltag tag width root parent
	 Lock:{NewLock} del bg

      attr leaves box label icon eicon expanded shown fill

      meth padX(V)
%	 V=self.height+(self.height div 2)
%	 V=self.height div 2
	 V=self.height-2
      end

      meth init(...)=M
	 lock self.Lock then
	    {self {Record.adjoin M unlockedInit}}
	 end
      end

      meth unlockedInit(canvas:Canvas<=NoArgs
			font:Font<=NoArgs
			height:Height<=NoArgs
			parent:Parent<=NoArgs
			label:Label tag:Tag<=NoArgs
			icon:Icon<=nil
			eicon:EIcon<=nil)
	 if Parent\=NoArgs then
	    self.canvas=Parent.canvas
	    self.bg=Parent.bg
	    self.font=Parent.font
	    self.height=Parent.height
	    self.root=false
	    self.parent=Parent
	 else
	    self.canvas=Canvas
	    self.bg={Canvas tkReturn(cget(bg:unit) $)}
	    self.font=Font
	    self.height=Height
	    self.root=true
	    self.parent=self
	 end
	 fill<-black
	 label<-Label
	 icon<-Icon
	 eicon<-EIcon
	 if Icon==nil then self.width=0 else
	    self.width={Tk.returnInt image(width Icon)}
	 end
	 leaves<-nil
	 expanded<-false
	 {ForAll [vtag htag btag itag ltag]
	  proc{$ X}
	     self.X={New Tk.canvasTag tkInit(parent:self.canvas)}
	  end}
	 box<-plus
	 if Tag==NoArgs then self.tag={New Tk.canvasTag tkInit(parent:self.canvas)}
	 else self.tag=Tag end
	 shown<-false
	 {self.btag tkBind(event:"<1>"
			   action:self#switch)}
      end

      meth bind(event:E args:A<=nil action:Act)
%	 lock
	    {self.tag tkBind(event:E args:A action:Act)}
%	 end
      end
      
      meth draw(x:X y:Y height:H)
	 lock self.Lock then
	    if @shown==false then
	       H1=self.height
	       H2=H1 div 2
	       PadX=TreeNode,padX($)
	    in
	       {self.canvas tk(crea line
			       if self.root then X+H2 else X end    Y+H2
			       X+H1 Y+H2
			       stipple:gray50
			       tags:self.htag)}
	       TreeNode,drawBox
	       {self.canvas tk(crea image
			       X+H1 Y+H2
			       anchor:w
			       tags:self.itag
			       image:if @expanded andthen @eicon\=nil then
					@eicon
				     else
					@icon
				     end
			      )}
	       {self.canvas tk(crea text
			       X+H1+self.width+4 Y+H2
			       anchor:w
			       justify:left
			       font:self.font
			       fill:@fill
			       tags:self.ltag
			       text:@label)}
	       {self.canvas tk(addtag self.tag withtag self.itag)}
	       {self.canvas tk(addtag self.tag withtag self.ltag)}
	       if @expanded==true then
		  PY={NewCell self.height}
		  LE={Length @leaves}
	       in
		  {List.forAllInd @leaves
		   proc{$ I L}
		      H in
		      {L draw(x:X+PadX y:Y+{Access PY} height:H)}
		      if I==LE then
			 {self.canvas tk(crea line
					 X+PadX Y+H2
					 X+PadX Y+{Access PY}+H2
					 stipple:gray50
					 tags:self.vtag)}
		      else skip end
		      {Assign PY {Access PY}+H}
		   end}
		  H={Access PY}
	       else
		  H=self.height
	       end
	       if @expanded==false orelse @leaves==nil then
		  {self.canvas tk(crea line
				  X+PadX Y+H2
				  X+PadX Y+H2
				  stipple:gray50
				  tags:self.vtag)}
	       else skip end
	       {self.canvas tk(lower self.vtag)}
	       shown<-true
	    else
	       H=0
	    end
	 end
      end

      meth drawIcon
	 if @shown then
	    {self.canvas tk(itemconfigure self.itag
			    image:if @expanded andthen @eicon\=nil then
				     @eicon
				  else
				     @icon
				  end)}
	 end
      end
      
      meth drawBox
	 lock self.Lock then
	    H1=self.height
	    H2=H1 div 2
	    H4=H2 div 2
	    H6=H4+H2
	    X Y
	 in
	    {self.canvas tk(delete self.btag)}
	    TreeNode,get(x:X y:Y)
	    if @box==line then skip else
	       {self.canvas tk(crea rect
			       X+H4 Y+H4
			       X+H6 Y+H6
			       outline:black
			       fill:self.bg
			       tags:self.btag)}
	       if @box==point then
		  {self.canvas tk(crea rect
				  X+H2-1 Y+H2-1
				  X+H2+1 Y+H2+1
				  tags:self.btag)}
	       else
		  {self.canvas tk(crea line
				  X+H4+2 Y+H2
				  X+H6-2 Y+H2
				  tags:self.btag)}
		  if @box==plus then
		     {self.canvas tk(crea line
				     X+H2 Y+H4+2
				     X+H2 Y+H6-2
				     tags:self.btag)}
		  else skip end
	       end
	    end
	 end
      end
   
      meth hide(height:H del:Del<=false)
	 lock self.Lock then
	    if @shown==true then
	       {ForAll [vtag htag btag itag ltag]
		proc{$ Tag}
		   {self.canvas tk(delete self.Tag)}
		end}
	       if @expanded then
		  PH={NewCell self.height}
	       in
		  {List.forAllInd @leaves
		   proc{$ I L}
		      H in
		      {L hide(height:H del:Del)}
		      {Assign PH {Access PH}+H}
		   end}
		  H={Access PH}
	       else
		  H=self.height
	       end
	       shown<-false
	    else skip end
	    if Del then self.del=unit else skip end
	 end
      end

      meth delete(height:H)
	 TreeNode,hide(height:H del:true)
      end

      meth configure(box:Box<=NoArgs
		     icon:Icon<=NoArgs
		     eicon:EIcon<=NoArgs
		     label:Label<=NoArgs
		     fill:Fill<=NoArgs)
	 lock self.Lock then
	    if Box==NoArgs then skip else
	       if @box\=Box then
		  box<-Box
		  if @shown then TreeNode,drawBox else skip end
	       else skip end
	    end
	    if Icon==NoArgs then skip else
	       icon<-Icon
	       {self drawIcon}
	    end
	    if EIcon==NoArgs then skip else
	       eicon<-EIcon
	       {self drawIcon}
	    end
	    if Label==NoArgs then skip else
	       label<-Label
	       if @shown then
		  {self.canvas tk(itemconfigure self.ltag text:Label)}
	       else skip end
	    end
	    if Fill==NoArgs then skip else
	       fill<-Fill
	       if @shown then
		  {self.canvas tk(itemconfigure self.ltag fill:Fill)}
	       else skip end
	    end
	 end
      end
   
      meth expand(drawCmd:DrawCmd<=draw)
	 lock self.Lock then
	    if @expanded then skip else
	       if @shown then
		  X Y Tag Pad
		  H2=self.height div 2
		  PadX=TreeNode,padX($)
		  Bef
	       in
		  TreeNode,get(x:X y:Y)
		  Tag={New Tk.canvasTag tkInit(parent:self.canvas)}
		  {self.canvas tk(addtag Tag enclosed b([~1 Y+H2+2 1000000 1000000]))}
		  Bef={Length @leaves}*self.height
		  {self.canvas tk(move Tag 0 Bef)}
		  Pad={List.foldL @leaves
		       fun{$ V L}
			  Pad
			  {L {Record.adjoin draw(x:X+PadX y:Y+V+self.height height:Pad) DrawCmd} }
		       in
			  V+Pad
		       end
		       0}
		  {self.canvas tk(move Tag 0 Pad-Bef)}
		  {self.canvas tk(dtag Tag)}
		  if @leaves==nil then
		     TreeNode,configure(box:line)
		  else
		     {self setVert(leaf:self padN:Pad pad:Pad-{{List.last @leaves} get(height:$)}+self.height)}
		     TreeNode,configure(box:minus)	       
		  end
	       else
		  if @leaves==nil then
		     TreeNode,configure(box:line)
		  else
		     TreeNode,configure(box:minus)
		  end
	       end
	       expanded<-true
	       {self drawIcon}
	    end
	 end
      end

      meth collapse
	 lock self.Lock then
	    if @expanded==false then skip else
	       if @shown then
		  Y Tag Pad Y1 Y2
		  H2=self.height div 2
	       in
		  TreeNode,get(y:Y)
		  Tag={New Tk.canvasTag tkInit(parent:self.canvas)}
		  {self.canvas tk(addtag Tag enclosed b([~1 Y+H2+2 1000000 1000000]))}
		  Pad={List.foldL @leaves
		       fun{$ V L}
			  V+{L hide(height:$)}
		       end
		       0}
		  {self.canvas tk(move Tag 0 ~Pad)}
		  {self.canvas tk(dtag Tag)}
		  [_ Y1 _ Y2]={self.canvas tkReturnListInt(coords(self.vtag) $)}
		  {self setVert(leaf:self padN:~Pad pad:Y1-Y2)}
		  TreeNode,configure(box:if @leaves==nil then line else plus end)
	       else skip end
	       expanded<-false
	       {self drawIcon}
	    end
	 end
      end

      meth setVert(pad:Pad padN:PadN leaf:Leaf)
	 lock self.Lock then
	    X1 Y1 X2 Y2
	 in
	    if @leaves==nil orelse {List.last @leaves}==Leaf then
	       skip
	    else
	       [X1 Y1 X2 Y2]={self.canvas tkReturnListInt(coords(self.vtag) $)}
	       {self.canvas tk(coords self.vtag b([X1 Y1 X2 Y2+Pad]))}
	    end
	    if self.root then
	       X2 Y2
	    in
	       [_ _ X2 Y2]={self.canvas tkReturnListInt(bbox(all) $)}
	       {self.canvas tk(configure scrollregion:"0 0 "#X2#" "#Y2)}
	    else
	       {self.parent setVert(leaf:self pad:PadN padN:PadN)}
	    end
	 end
      end
	 
      meth switch
	 lock self.Lock then
	    if @expanded then {self collapse} else {self expand} end
	 end
      end

      meth get(label:Label<=NoArgs y:Y<=NoArgs x:X<=NoArgs
	       height:Height<=NoArgs shown:Shown<=NoArgs
	       parent:Parent<=NoArgs
	       box:Box<=NoArgs)
	 lock self.Lock then
	    if {IsFree Label} then Label=@label else skip end
	    if {IsFree Y} orelse {IsFree X} then
	       X1 Y1 in
	       [_ Y1 X1 _]={self.canvas tkReturnListInt(coords(self.htag) $)}
	       if {IsFree Y} then
		  Y=Y1-(self.height div 2)
	       else skip end
	       if {IsFree X} then
		  X=X1-self.height
	       end
	    else skip end
	    if {IsFree Height} then
	       if @shown then
		  if @expanded then
		     Height={List.foldL @leaves
			     fun{$ V L}
				V+{L get(height:$)}
			     end
			     self.height}
		  else
		     Height=self.height
		  end   
	       else
		  Height=0
	       end
	    else skip end
	    if {IsFree Shown} then
	       Shown=@shown
	    else skip end
	    if {IsFree Box} then
	       Box=@box
	    else skip end
	    if {IsFree Parent} then
	       Parent=self.parent
	    else skip end
	 end
      end
   
      meth addLeaf(node:Node<=NoArgs nodes:Nodes<=_ drawCmd:DrawCmd<=draw)
	 lock self.Lock then
	    NL=if Node==NoArgs then {List.append Nodes @leaves} else
		  Nodes=[Node]
		  Node|@leaves
	       end
	 in
	    leaves<-{List.sort NL fun{$ A B} {StringToAtom {A get(label:$)}}=<{StringToAtom {B get(label:$)}} end}
	    if @shown andthen @expanded then
	       Cur={NewCell self}
	       X=TreeNode,get(x:$)
	       Last={List.length @leaves}
	    in
	       {List.forAllInd @leaves
		proc{$ I L}
		   if {List.member L Nodes} then
		      Y H Pad
		      Tag={New Tk.canvasTag tkInit(parent:self.canvas)}
		      PadX=TreeNode,padX($)
		   in
		      if {Access Cur}==self then
			 {self get(y:Y)}
			 H=self.height
		      else
			 {{Access Cur} get(y:Y height:H)}
		      end
		      {self.canvas tk(addtag Tag enclosed b([~1 Y+H-(self.height div 2)+2 1000000 1000000]))}
		      {L {Record.adjoin draw(x:X+PadX y:Y+H height:Pad) DrawCmd}}
		      {self.canvas tk(move Tag 0 Pad)}
		      {self.canvas tk(dtag Tag)}
		      if I==Last then
			 if {Length @leaves}==1 then
			    {self setVert(leaf:self pad:self.height padN:Pad)}
			 else
			    {self setVert(leaf:self pad:{{List.nth @leaves {List.length @leaves}-1} get(height:$)}
					  padN:Pad)}
			 end
		      else
			 {self setVert(leaf:self pad:Pad padN:Pad)}
		      end   
		   else skip end
		   {Assign Cur L}
		end}
	       TreeNode,configure(box:minus)
	    elseif @expanded then
	       box<-minus
	    else skip end
	 end
      end

      meth deleteLeaf(node:Node)
	 lock self.Lock then
	    leaves<-{List.filter @leaves
		     fun{$ This}
			if This==Node then
			   if @shown andthen @expanded then
			      H
			      Tag={New Tk.canvasTag tkInit(parent:self.canvas)}
			      Y={This get(y:$)}
			   in
			      {self.canvas tk(addtag Tag enclosed b([~1 Y-(self.height div 2)+2 1000000 1000000]))}
%			      {This hide(height:H)}
			      {This delete(height:H)}
			      {self.canvas tk(move Tag 0 ~H)}
			      if This=={List.last @leaves} then
				 Pad
			      in
				 if {Length @leaves}>1 then
				    Pad={{List.nth @leaves {Length @leaves}-1} get(height:$)}
				 else
				    Pad=self.height
				 end
				 {self setVert(leaf:self pad:~Pad padN:~H)}
			      else
				 {self setVert(leaf:self pad:~H padN:~H)}
			      end
			   else skip end
			   false
			else
			   true
			end
		     end}
	    if @leaves==nil then {self configure(box:line)} else skip end
	 end
      end

      meth isDeleted(B)
	 B={IsDet self.del}
      end
   
   end
end
