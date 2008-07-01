functor
import
   Application(postProcess exit)
   CB(checkBox:CheckBox) at 'x-oz://contrib/tk/CheckBox.ozf'
   SF(scrollFrame:ScrollFrame) at 'x-oz://contrib/tk/ScrollFrame.ozf'
   Tk TkTools
export
   GetGuiCmdArgs
   AddHandler
   OptionSheet
define
NONE = {NewName}

%% -------------------------------------------------------------------
%% {ProcessOptionSpec +SPEC ?NEWSPEC}
%%	given an option SPEC as expected by the usual option parser,
%% build a record of all information necessary for an OptionManager.
%% -------------------------------------------------------------------

fun {ProcessOptionSpec Spec}
   if {IsAtom {CondSelect Spec alias 0}} then unit
   else
      Option     = {Label Spec}
      Text       = if {HasFeature Spec text} then Spec.text
		   else '--'#Option end
      Type       = if {HasFeature Spec alias} then alias
		   else {CondSelect Spec type bool} end
      Default    = {CondSelect Spec default NONE}
      Optional   = {CondSelect Spec optional true}
      Aliases    = case {CondSelect Spec alias NONE}
		   of !NONE then nil
		   [] O#V   then [O#V]
		   [] L     then L end
      Clonable   = ({CondSelect Spec 1 multiple}==multiple orelse
		    {Label Type}==list)
      Collecting = ({CondSelect Spec 1 multiple}\=multiple andthen
		    {Label Type}==list)
      EditorClass= {GetEditor Type}
      Help       = {CondSelect Spec help unit}
   in
      option(option      : Option
	     text        : Text
	     type        : Type
	     default     : Default
	     optional    : Optional
	     aliases     : Aliases
	     clonable    : Clonable
	     collecting  : Collecting
	     editorClass : EditorClass
	     help        : Help)
   end
end

ArgumentSpec =
argument(option		 : NONE
	 text		 : 'Argument'
	 type		 : string
	 default	 : nil
	 optional	 : true
	 aliases	 : nil
	 clonable	 : true
	 collecting	 : false
	 editorClass	 : StringOptionEditor
	 help		 : unit
	)

%% -------------------------------------------------------------------
%% There is one OptionManager for each option understood by the
%% application.  It manages all editable occurrences of this option,
%% which are represented by individual OptionRows.
%% -------------------------------------------------------------------

class OptionManager
   feat frame spec sheet
   attr rows:nil
   meth init(frame:Frame spec:Spec sheet:Sheet)
      self.frame = Frame
      self.spec  = Spec
      self.sheet = Sheet
   end
   %% deactivateAlias is called when (1) this option is an alias,
   %% (2) it is currently aliasing some other options, (3) one of
   %% these options is being again selected by the user or aliased
   %% by an other option.
   meth deactivateAlias
      {ForAll @rows proc {$ R} {R exclude} end}
   end
   meth createRow(R)
      {New OptionRow init(manager:self) R}
      rows <- R|@rows
   end
   meth alias(O) {ForAll @rows proc {$ R} {R alias(O)} end} end
   meth unalias {ForAll @rows proc {$ R} {R unalias} end} end
   meth userSelectNotify(R) {self.sheet userSelectNotify(R)} end
   meth userDeSelectNotify(R) {self.sheet userDeSelectNotify(R)} end
   meth atLeastTwoIncluded($)
      {Length {Filter @rows fun {$ R} {R isIncluded($)} end}}
      > 1
   end
   meth collect(I $)
      %% check that I is the 1st included row
      %% collect is only called if I is included
      if try
	    {ForAll @rows
	     proc {$ R}
		if {R isIncluded($)} andthen
		   {R getRowIndex($)}<I
		then raise no end end
	     end}
	    true
	 catch no then false end
      then
	 rows <- {Sort @rows
		  fun {$ R1 R2}
		     {R1 getRowIndex($)} < {R2 getRowIndex($)}
		  end}
	 self.spec.option#
	 {Filter {Map @rows fun {$ R} {R collectValue($)} end}
	  fun {$ V} V\=NONE end}
      else omit end
   end
   meth atLeastTwo($) {Length @rows}>1 end
   meth userDeleteNotify(R)
      rows <- {Filter @rows fun {$ R2} R\=R2 end}
   end
end

%% -------------------------------------------------------------------
%% an OptionRow represents an editable occurrence of an option.
%% It can be in one of the following states:
%%
%% [exclude]
%%	The row is in an inactive state; it does not contribute
%%	to the set of options.  The editor is not visible.
%% [include]
%%	The row is in an active state; it contributes to the set
%%	of options.  The editor is visible.
%% [alias(O)]
%%	The alias option O is currently selected which sets the
%%      present option to a certain value.  The row is in an inactive
%%	state.  The editor is not visible; instead a label is shown
%%	explaining that option O sets it to another value (also shown).
%%	-- it should just say: "overriden by alias --O"
%%
%% if it is discovered that an option row contains an illegal value
%% then the option name should be displayed e.g. in red; as soon as
%% it is edited in some fashion, the color of the option name (label)
%% should revert to the default.
%% -------------------------------------------------------------------

class OptionRow
   prop locking
   feat manager check label editor info
   attr index state:unit stateUndo:exclude selected:false
   meth init(manager:M)
      self.manager = M
      self.check   = {New OptionCheck        init(row:self)}
      self.label   = {New OptionLabel        init(row:self)}
      self.editor  = {New M.spec.editorClass init(row:self)}
      self.info    = {New OptionInfo         init(row:self)}
   end
   meth setRowIndex(I)
      lock
	 index <- I
	 {self.check  rowChangeNotify(I)}
	 {self.label  rowChangeNotify(I)}
	 {self.editor rowChangeNotify(I)}
	 {self.info   rowChangeNotify(I)}
      end
   end
   meth getRowIndex($) @index end
   meth stateChangeNotify(Old New)
      {self.check  stateChangeNotify(Old New)}
      {self.label  stateChangeNotify(Old New)}
      {self.editor stateChangeNotify(Old New)}
      {self.info   stateChangeNotify(Old New)}
   end
   meth userEditNotify(...)
      {self.check  userEditNotify}
      {self.label  userEditNotify}
      {self.editor userEditNotify}
      {self.info   userEditNotify}
   end
   meth valueErrorNotify
      {self.check  valueErrorNotify}
      {self.label  valueErrorNotify}
      {self.editor valueErrorNotify}
      {self.info   valueErrorNotify}
   end
   meth exclude
      if @state\=exclude then
	 {self stateChangeNotify(@state exclude)}
	 state <- exclude
      end
   end
   meth include
      if @state\=include then
	 case @state of alias(X) then {X deactivateAlias}
	 else skip end
	 {self stateChangeNotify(@state include)}
	 state <- include
      end
   end
   meth alias(O)
      if @state\=alias(O) then
	 case @state of alias(X) then {X deactivateAlias}
	 else skip end
	 stateUndo <- @state
	 {self stateChangeNotify(@state alias(O))}
	 state <- alias(O)
      end
   end
   meth unalias S=@state in
      case S of alias(_) then
	 {self stateChangeNotify(S @stateUndo)}
	 state <- @stateUndo
      else skip end
   end
   meth get($)
      if @state==include then
	 if self.manager.spec.collecting then
	    {self.manager collect(@index $)}
	 elseif self.manager.spec.type==alias then
	    alias(self.manager.spec.aliases)
	 else
	    try
	       V = {self.editor getValue($)}
	    in
	       self.manager.spec.option
	       #if {Label self.manager.spec.type}==list
		then [V] else V end
	    catch _ then
	       {self valueErrorNotify}
	       bad(self.manager.spec.option)
	    end
	 end
      else omit end
   end
   meth collectValue($)
      if @state==include then
	 try {self.editor getValue($)}
	 catch _ then
	    {self valueErrorNotify}
	    raise system(ap(illegalOptionValue
			    self.manager.spec.option)) end
	 end
      else NONE end
   end
   meth userSelectNotify
      selected <- true
      {self.manager userSelectNotify(self)}
      {self.check  userSelectNotify}
      {self.label  userSelectNotify}
      {self.editor userSelectNotify}
      {self.info   userSelectNotify}
   end
   meth userDeSelectNotify
      selected <- false
      {self.manager userDeSelectNotify(self)}
      {self.check  userDeSelectNotify}
      {self.label  userDeSelectNotify}
      {self.editor userDeSelectNotify}
      {self.info   userDeSelectNotify}
   end
   meth toggleSelect
      {self if @selected
	    then userDeSelectNotify
	    else userSelectNotify end}
   end
   meth userCheckNotify(B)
      if self.manager.spec.optional then
	 {self if B then include else exclude end}
      elseif B then {self include}
      elseif {self.manager atLeastTwoIncluded($)} then
	 {self exclude}
      end
   end
   meth isIncluded($) @state==include end
   meth userDeleteNotify
      {self.manager userDeleteNotify(self)}
      {self.check  userDeleteNotify}
      {self.label  userDeleteNotify}
      {self.editor userDeleteNotify}
      {self.info   userDeleteNotify}
   end
   meth setEditorFrom(R)
      {self.editor setEditorFrom(R.editor)}
   end
   meth userHelpNotify
      Help = self.manager.spec.help
      Clonable = self.manager.spec.clonable
   in
      {New OptionHelp
       if self.manager.spec.option==NONE then
	  init(
	     'This stands for an ordinary argument rather than an option\n'#
	     'type: '#{self.editor userHelpNotify($)}#
	     '\n\nMultiple occurrences are allowed\n'#
	     'select this row by clicking Mouse-1 on the Argument label\n'#
	     'then choose \`Clone\' from the menu or press Control-o')
       else
	  init('option: --'#self.manager.spec.option#'\n'#
	       'type  : '#{self.editor userHelpNotify($)}#
	       if Clonable then
		  '\n\nMultiple occurrences of this option are allowed\n'#
		  'select the option by clicking Mouse-1 on its name\n'#
		  'then choose \`Clone\' from the menu or press Control-o'
	       else nil end #
	       if Help==unit then nil else '\n\n'#Help end)
       end
       _}
   end
end

class OptionCheck from CheckBox
   feat row
   meth init(row:Row)
      self.row = Row
      CheckBox,init(parent:Row.manager.frame)
   end
   meth userEditNotify   skip end
   meth valueErrorNotify skip end
   meth stateChangeNotify(Old New)
      CheckBox,set(New==include)
   end
   meth rowChangeNotify(I)
      {Tk.send grid(self row:I column:0 sticky:nswe)}
   end
   meth action(B) {self.row userCheckNotify(B)} end
   meth userSelectNotify skip end
   meth userDeSelectNotify skip end
   meth userDeleteNotify CheckBox,tkClose end
end

class OptionLabel from Tk.label
   feat row
   attr ok:true
   meth init(row:Row)
      self.row = Row
      Tk.label,tkInit(parent : Row.manager.frame
		      text   : Row.manager.spec.text
		      anchor :nw
		      fg     :black
		      highlightcolor:red
		      highlightthickness:1)
      Tk.label,tkBind(event :'<1>'
		      action:Row#toggleSelect)
      Tk.label,tkBind(event :'<3>'
		      action:Row#userHelpNotify)
   end
   meth Normal
      if {Not @ok} then
	 ok <- true
	 Tk.label,tk(configure fg:black)
      end
   end
   meth Error
      if @ok then
	 ok <- false
	 Tk.label,tk(configure fg:red)
      end
   end
   meth userEditNotify         OptionLabel,Normal end
   meth valueErrorNotify       OptionLabel,Error  end
   meth stateChangeNotify(_ _) OptionLabel,Normal end
   meth rowChangeNotify(I)
      {Tk.send grid(self row:I column:1 sticky:nswe)}
   end
   meth userSelectNotify
      Tk.label,tk(configure relief:groove)
   end
   meth userDeSelectNotify
      Tk.label,tk(configure relief:flat)
   end
   meth userDeleteNotify Tk.label,tkClose end
end

InfoFont = {New Tk.font tkInit(family:helvetica size:10 weight:normal
			       slant:italic)}

class OptionInfo from Tk.label
   feat row
   attr clear:true hidden:true
   meth init(row:Row)
      self.row = Row
      Tk.label,tkInit(parent: Row.manager.frame font:InfoFont)
   end
   meth Clear
      if {Not @clear} then
	 clear <- true
	 Tk.label,tk(configure text:'')
      end
   end
   meth Hide
      if {Not @hidden} then
	 hidden <- true
	 {Tk.send grid(forget self)}
      end
   end
   meth Show
      if @hidden then
	 hidden <- false
	 {Tk.send grid(self
		       row:{self.row getRowIndex($)}
		       column:2
		       sticky:nswe)}
      end
   end
   meth Alias(O)
      clear <- false
      Tk.label,tk(configure
		  text:'overriden by '#O.spec.text)
   end
   meth userEditNotify OptionInfo,Clear end
   meth valueErrorNotify skip end
   meth stateChangeNotify(Old New)
      case New
      of include then
	 OptionInfo,Hide
      [] exclude then
	 OptionInfo,Clear
	 OptionInfo,Show
      [] alias(O) then
	 OptionInfo,Alias(O)
	 OptionInfo,Show
      end
   end
   meth rowChangeNotify(I)
      if {Not @hidden} then
	 {Tk.send grid(self row:I column:2 sticky:nswe)}
      end
   end
   meth userSelectNotify skip end
   meth userDeSelectNotify skip end
   meth userDeleteNotify Tk.label,tkClose end
end

class OptionEditor
   feat sticky:nw row
   attr Hidden:true
   meth init(row:Row) self.row=Row end
   meth userEditNotify skip end
   meth valueErrorNotify skip end
   meth stateChangeNotify(Old New)
      case New
      of include  then OptionEditor,Show
      [] exclude  then OptionEditor,Hide
      [] alias(_) then OptionEditor,Hide
      end
   end
   meth rowChangeNotify(I)
      if {Not @Hidden} then
	 {Tk.send grid(self row:I column:2 sticky:self.sticky)}
      end
   end
   meth Hide
      if {Not @Hidden} then
	 Hidden <- true
	 {Tk.send grid(forget self)}
      end
   end
   meth Show
      if @Hidden then
	 Hidden <- false
	 {Tk.send grid(self row:{self.row getRowIndex($)}
		       column:2 sticky:self.sticky)}
      end
   end
   meth userSelectNotify skip end
   meth userDeSelectNotify skip end
   meth userDeleteNotify {self tkClose} end
   meth setEditorFrom(E)
      try {self setValue({E getValue($)})}
      catch _ then skip end
   end
   meth userHelpNotify($)
      case @row.manager.spec.type
      of list(X) then X
      elseof  X  then {Label X} end
   end
end

class BoolOptionEditor from Tk.checkbutton OptionEditor
   feat Var
   meth init(row:Row)
      OptionEditor,init(row:Row)
      self.Var = {New Tk.variable tkInit}
      Tk.checkbutton,tkInit(parent   : Row.manager.frame
			    variable : self.Var)
      Tk.checkbutton,tkAction(action : Row#userEditNotify)
   end
   meth getValue($) {self.Var tkReturnInt($)}==1 end
   meth setValue(V) {self.Var tkSet(V)} end
   meth userHelpNotify($) 'boolean' end
end

class StringOptionEditor from Tk.entry OptionEditor
   feat sticky:nwe
   meth init(row:Row)
      OptionEditor,init(row:Row)
      Tk.entry,tkInit(parent : Row.manager.frame)
      Tk.entry,tkBind(event  : '<KeyPress>'
		      append : true
		      action : Row#userEditNotify)
   end
   meth getValue($) Tk.entry,tkReturn(get $) end
   meth setValue(V)
      Tk.entry,tk(delete 0 'end')
      Tk.entry,tk(insert 0 V)
   end
   meth userHelpNotify($) 'string' end
end

class AtomOptionEditor from StringOptionEditor
   meth getValue($)
      {String.toAtom StringOptionEditor,getValue($)}
   end
   meth userHelpNotify($) 'atom' end
end

class IntOptionEditor from TkTools.numberentry OptionEditor
   meth init(row:Row)
      OptionEditor,init(row:Row)
      TkTools.numberentry,tkInit(parent: Row.manager.frame
				 min   : {CondSelect
					  Row.manager.spec.type
					  min unit}
				 max   : {CondSelect
					  Row.manager.spec.type
					  max unit}
				 action: Row#userEditNotify)
   end
   meth getValue($) TkTools.numberentry,tkGet($) end
   meth setValue(V)
      {self.entry tk(delete 1 'end')}
      TkTools.numberentry,tkSet(V)
   end
   meth userHelpNotify($)
      'integer'
      # case {CondSelect self.row.manager.spec.type min unit}
	of unit then nil
	[] X then ' (>= '#X#')' end
      # case {CondSelect self.row.manager.spec.type max unit}
	of unit then nil
	[] X then ' (=< '#X#')' end
   end
end

class FloatOptionEditor from Tk.entry OptionEditor
   feat sticky:nwe
   meth init(row:Row)
      OptionEditor,init(row:Row)
      Tk.entry,tkInit(parent: Row.manager.frame)
      Tk.entry,tkBind(event : '<KeyPress>'
		      append: true
		      action: Row#userEditNotify)
   end
   meth getValue($)
      S = {self tkReturn(get $)}
   in
      try {String.toFloat S}
      catch _ then {Int.toFloat {String.toInt S}} end
   end
   meth setValue(V)
      Tk.entry,tk(delete 0 'end')
      Tk.entry,tk(insert 0 V)
   end
   meth userHelpNotify($)
      'float'
      # case {CondSelect self.row.manager.spec.type min unit}
	of unit then nil
	[] X then ' (>= '#X#')' end
      # case {CondSelect self.row.manager.spec.type max unit}
	of unit then nil
	[] X then ' (=< '#X#')' end
   end
end

class AtomChoiceOptionEditor from Tk.frame OptionEditor
   feat Choices Box
   meth init(row:Row)
      OptionEditor,init(row:Row)
      Tk.frame,tkInit(parent: Row.manager.frame)
      L = {Record.toList Row.manager.spec.type}
      Size   = {Length L}
      Max    = 5
      Height = {Min Max Size}
   in
      self.Box = {New Tk.listbox tkInit(parent:self height:Height)}
      self.Choices = L
      {ForAll L
       proc {$ A} {self.Box tk(insert 'end' A)} end}
      if Size>Max then
	 Bar = {New Tk.scrollbar tkInit(parent:self)}
      in
	 {Tk.addYScrollbar self.Box Bar}
	 {Tk.send pack(self.Box Bar fill:y side:left)}
      else
	 {Tk.send pack(self.Box fill:y side:left)}
      end
      {self.Box tkBind(event: '<1>' action: Row#userEditNotify)}
   end
   meth getValue($)
      {Nth self.Choices 1+{self.Box tkReturnInt(curselection $)}}
   end
   meth setValue(V)
      {self.Box tk(selection clear 0 'end')}
      try
	 {List.forAllInd self.Choices
	  proc {$ I A}
	     if V==A then raise ok(I) end end
	  end}
      catch ok(I) then
	 {self.Box tk(selection set I-1)}
      end
   end
   meth userHelpNotify($)
      'atom (one of:'#
      {FoldR {Record.toList self.row.manager.spec.type}
       fun {$ A L} ' '#A#L end ')'}
   end
end

class AliasOptionEditor from Tk.frame OptionEditor
   feat sticky:nwse
   attr On:false
   meth init(row:Row)
      OptionEditor,init(row:Row)
      Tk.frame,tkInit(parent: Row.manager.frame)
   end
   meth stateChangeNotify(Old New)
      case New
      of include then
	 On <- true
	 %% now, override aliases
	 {ForAll self.row.manager.spec.aliases
	  proc {$ A#_}
	     try {self.row.manager.sheet.map.A
		  alias(self.row.manager)}
	     catch _ then skip end
	  end}
      else AliasOptionEditor,Off
      end
      OptionEditor,stateChangeNotify(Old New)
   end
   meth getValue($) @On end
   meth Off
      On <- false
      {ForAll self.row.manager.spec.aliases
       proc {$ A#_}
	  try {self.row.manager.sheet.map.A unalias}
	  catch _ then skip end
       end}
   end
   meth userDeleteNotify
      if @On then AliasOptionEditor,Off end
      Tk.frame,tkClose
   end
   meth userHelpNotify($)
      'alias (overrides:'#
      {FoldR self.row.manager.spec.aliases
       fun {$ A#_ L} ' --'#A#L end ')'}
   end
end

proc {OptionEditorHandler Type}
   E =
   case Type
   of alias      then      AliasOptionEditor
   [] bool       then       BoolOptionEditor
   [] string     then     StringOptionEditor
   [] atom       then       AtomOptionEditor
   [] int(...)   then        IntOptionEditor
   [] float(...) then      FloatOptionEditor
   [] atom(...)  then AtomChoiceOptionEditor
   [] list(T)    then {GetEditor T}
   else unit end
in
   if E\=unit then raise ok(E) end end
end

OptionEditorHandlerAlist = {NewCell [OptionEditorHandler]}
fun {GetEditor Type}
   try {ForAll {Access OptionEditorHandlerAlist}
	proc {$ H} {H Type} end}
      {Exception.raiseError ap(noEditor Type)} unit
   catch ok(C) then C end
end
proc {AddHandler H}
   L in {Exchange OptionEditorHandlerAlist L H|L}
end

class OptionHelp from Tk.toplevel
   meth init(Text)
      Tk.toplevel,tkInit(withdraw:true title:'Option Help')
      B = {New Tk.button tkInit(parent:self text:'Close' relief:groove
				action:self#tkClose)}
      M = {New Tk.label
	   tkInit(parent:self wraplength:0 text:Text justify:left)}
   in
      {Tk.batch
       [ pack(M fill:both expand:true)
	 pack(B)
	 wm(deiconify self) ]}
   end
end

class OptionGlobalHelp from OptionHelp
   meth init
      HELP =
      'Every line represents an occurrence of an option, and\n'#
      'consists of 3 fields: (1) a check box, (2) the option\'s\n'#
      'name, and (3) an editor or info banner (usually empty).\n\n'#
      'The check box indicates whether the occurrence is active\n'#
      'or not (i.e. is it really included in the command line).\n'#
      'When the occurrence is active, an appropriate editor appears\n'#
      'in the 3rd field and allows you to edit the value of the\n'#
      'option.  Note that an active occurrence of a boolean option\n'#
      'may well have value `false\'.\n\n'#
      'Some options support multiple occurrences: to obtain an\n'#
      'additional occurrence of option --foo, click on option\n'#
      '--foo\'s name to select it (a box is drawn around it), then\n'#
      'choose `Clone\' from the menu or press Control-o.  You can\n'#
      'also delete an occurrence with Control-g or `Delete\' from\n'#
      'the menu.\n\n'#
      'An occurrence of an option can also be moved up and down.\n'#
      'First select it, then use the up and down arrow keys.\n\n'#
      'To obtain help information on an option: click Mouse-3 on\n'#
      'the option\'s name or select it and choose `Option Help\'\n'#
      'from the menu.'
   in
      OptionHelp,init(HELP)
   end
end

class OptionSheet from Tk.toplevel
   feat map frame list msg specs result sframe bar
   attr rows selected:unit clear:true
   meth init(Specs Result)
      self.specs  = Specs
      self.result = Result
      Tk.toplevel,tkInit
      Tk.toplevel,tkBind(event:'<Up>'   action:self#userUpNotify)
      Tk.toplevel,tkBind(event:'<Down>' action:self#userDownNotify)
      Tk.toplevel,tkBind(event:'<KeyPress>' append:true action:self#Clear)
      Tk.toplevel,tkBind(event:'<Button>'   append:true action:self#Clear)
      Tk.toplevel,tkBind(event:'<Control-o>' append:true
			 action:self#userCloneNotify)
      Tk.toplevel,tkBind(event:'<Control-g>' append:true
			 action:self#userDeleteNotify)
      self.sframe= {New ScrollFrame tkInit(parent:self)}%
      self.frame = self.sframe.frame
      self.msg   = {New Tk.label tkInit(parent:self relief:groove)}
      self.bar   = {TkTools.menubar self self
		    [menubutton(text:'Menu' feature:ed
				menu:[command(label:'Ok'
					      action:self#userAcceptNotify)
				      command(label:'Help'
					      action:self#userGlobalHelpNotify)
				      separator
				      command(label:'Clone Option'
					      action:self#userCloneNotify)
				      command(label:'Delete Option'
					      action:self#userDeleteNotify)
				      command(label:'Help on Option'
					      action:self#userHelpNotify)
				      separator
				      command(label:'Quit'
					      action:self#userAbortNotify)])]
		    nil}
      {self.bar.ed.menu tk(configure tearoff:false)}
      {Tk.send grid(columnconfigure self.frame 2 weight:1 minsize:150)}
      self.list =
      {Map
       ArgumentSpec|
       {Filter {Map {Filter {Record.toListInd Specs}
		     fun {$ I#_} {IsInt I} end}
		fun {$ _#Spec} {ProcessOptionSpec Spec} end}
	fun {$ S} S\=unit end}
       fun {$ Spec}
	  {New OptionManager init(frame:self.frame
				  sheet:self
				  spec :Spec)}
       end}
      self.map =
      {List.toRecord map {Map self.list fun {$ M} M.spec.option#M end}}
      rows <- {List.mapInd self.list
	       proc {$ I M R}
		  {M createRow(R)}
		  {R setRowIndex(I-1)}
		  {R if {Not M.spec.optional}
		     then include else exclude end}
	       end}
      {Tk.batch
       [ grid(rowconfigure self 2 weight:1)
	 grid(columnconfigure self 0 weight:1)
	 grid(self.bar row:0 column:0 sticky:nwe)
	 grid(self.msg row:1 column:0 sticky:nwe)
	 grid(self.sframe row:2 column:0 sticky:nswe)
       ]}
   end
   meth get($)
      optRec(Args) = {Application.postProcess
                      {FoldR {Map @rows fun {$ R} {R get($)} end}
                       fun {$ V L}
                          case V
                          of K#X       then if K==NONE then X else V end|L
                          [] alias(L2) then {Append L2 L}
                          [] omit      then L
                          [] bad(K)    then
                             raise system(ap(illegalOptionValue K)) end
                          end
                       end nil}
                      self.specs}
   in
      case {Label self.specs}
      of plain  then Args
      [] list   then Args
      [] record then optRec(Args)
      end
   end
   meth userSelectNotify(R)
      if @selected\=unit andthen @selected\=R then
	 {@selected userDeSelectNotify}
      end
      selected<-R
   end
   meth userDeSelectNotify(R)
      if R==@selected then selected<-unit end
   end
   meth NoSelected
      OptionSheet,message('No Option Selected')
   end
   meth userUpNotify
      OptionSheet,Clear
      if @selected==unit then
	 OptionSheet,NoSelected
      else I={@selected getRowIndex($)} in
	 if I>0 then Prefix Suffix R1 R2 in
	    {List.takeDrop @rows I-1 Prefix R1|R2|Suffix}
	    {R1 setRowIndex(I)}
	    {R2 setRowIndex(I-1)}
	    rows <- {Append Prefix R2|R1|Suffix}
	 end
      end
   end
   meth userDownNotify
      OptionSheet,Clear
      if @selected==unit then
	 OptionSheet,NoSelected
      else I={@selected getRowIndex($)} in
	 if I+1 < {Length @rows} then Prefix Suffix R1 R2 in
	    {List.takeDrop @rows I Prefix R1|R2|Suffix}
	    {R1 setRowIndex(I+1)}
	    {R2 setRowIndex(I)}
	    rows <- {Append Prefix R2|R1|Suffix}
	 end
      end
   end
   meth message(S)
      clear <- false
      {self.msg tk(configure text:S)}
   end
   meth Clear
      if {Not @clear} then
	 clear <- true
	 {self.msg tk(configure text:'')}
      end
   end
   meth userCloneNotify
      OptionSheet,Clear
      if @selected==unit then
	 OptionSheet,NoSelected
      elseif @selected.manager.spec.clonable then
	 R1 = {@selected.manager createRow($)}
	 Prefix Suffix R2 I={@selected getRowIndex($)}
      in
	 {List.takeDrop @rows I Prefix R2|Suffix}
	 {List.forAllInd R1|Suffix
	  proc {$ J R}
	     {R setRowIndex(I+J)}
	  end}
	 rows <- {Append Prefix R2|R1|Suffix}
	 {R1 if {@selected isIncluded($)}
	     then include else exclude end}
	 {R1 setEditorFrom(@selected)}
      else
	 OptionSheet,message('at most one occurrence allowed')
      end
   end
   meth userDeleteNotify
      OptionSheet,Clear
      if @selected==unit then
	 OptionSheet,NoSelected
      elseif
	 @selected.manager.spec.clonable andthen
	 {@selected.manager atLeastTwo($)}
      then
	 Prefix Suffix R I={@selected getRowIndex($)}
      in
	 selected <- unit
	 {List.takeDrop @rows I Prefix R|Suffix}
	 {R userDeleteNotify}
	 {List.forAllInd Suffix
	  proc {$ J R} {R setRowIndex(I+J-1)} end}
	 rows <- {Append Prefix Suffix}
      else
	 OptionSheet,message('Deletion Not Allowed')
      end
   end
   meth userAcceptNotify
      try R={self get($)} in
	 self.result=yes(R)
	 Tk.toplevel,tkClose
      catch system(ap(illegalOptionValue K)) then
	 {New TkTools.error
	  tkInit(master:self
		 text  :'Option --'#K#' does not have a legal value!')
	  _}
      end
   end
   meth userAbortNotify
      self.result=no
      Tk.toplevel,tkClose
   end
   meth userGlobalHelpNotify
      {New OptionGlobalHelp init _}
   end
   meth userHelpNotify
      OptionSheet,Clear
      if @selected==unit then
	 OptionSheet,NoSelected
      else
	 {@selected userHelpNotify}
      end
   end
end

fun {GetGuiCmdArgs Specs}
   Result
in
   {New OptionSheet init(Specs Result) _}
   case Result of yes(R) then R else
      {Application.exit 1}
      unit
   end
end
end
/*

{Show {GetGuiCmdArgs
       record(a(single type:bool optional:true)
	      b(single type:int)
	      c(single type:float optional:true)
	      d(single type:string optional:true)
	      e(single type:list(atom))
	      f(single type:atom(a b c d e fa) optional:true)
	      g(single alias:[b#3 e#qq a#true])
	     )}}

{Show {T get($)}}

{T tkClose}

*/
