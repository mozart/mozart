%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 2001
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   Error(registerFormatter)
   GDK
   GTK
   GTKCANVAS
   GBuilderTypes(is fromOz toOz)
   GBuilderWidgets(widgets)
export
   Create
   KeyPress
define
   NONE = {NewName}

   WidgetSpecs = {NewDictionary}

   fun {GetAdds ClassName} MyAdds in
      MyAdds = {CondSelect WidgetSpecs.ClassName add add()}
      case {CondSelect WidgetSpecs.ClassName isa NONE} of !NONE then MyAdds
      elseof ParentClassName then {Adjoin {GetAdds ParentClassName} MyAdds}
      end
   end

   fun {GetFeature ClassName X}
      case {CondSelect WidgetSpecs.ClassName.features X NONE} of !NONE then
	 case {CondSelect WidgetSpecs.ClassName isa NONE} of !NONE then !NONE
	 elseof ParentClassName then {GetFeature ParentClassName X}
	 end
      elseof Spec then Spec
      end
   end

   fun {ConstructorHasArgs ClassName Names Args}
      case Names of Name|Rest then
	 ({Dictionary.member Args Name}
	  orelse {HasFeature {GetFeature ClassName Name} default})
	 andthen {ConstructorHasArgs ClassName Rest Args}
      [] nil then true
      end
   end

   fun {ConstructorGetArgs ClassName Names Args}
      {Map Names
       fun {$ Name} ArgSpec Value in
	  ArgSpec = {GetFeature ClassName Name}
	  if {Dictionary.member Args Name} then
	     Value = Args.Name
	     {Dictionary.remove Args Name}
	  else
	     Value = ArgSpec.default
	  end
	  {GBuilderTypes.fromOz ArgSpec.type Value}
       end}
   end

   fun {SetterHasArgs ClassName Names Args NonTrivial}
      case Names of Name|Rest then
	 if {Dictionary.member Args Name} then
	    {SetterHasArgs ClassName Rest Args true}
	 else
	    {HasFeature {GetFeature ClassName Name} get}
	    andthen {SetterHasArgs ClassName Rest Args NonTrivial}
	 end
      [] nil then NonTrivial
      end
   end

   fun {SetterGetArgs ClassName Names Args O}
      {Map Names
       fun {$ Name} ArgSpec in
	  ArgSpec = {GetFeature ClassName Name}
	  if {Dictionary.member Args Name} then Value in
	     Value = Args.Name
	     {Dictionary.remove Args Name}
	     {GBuilderTypes.fromOz ArgSpec.type Value}
	  elsecase ArgSpec.get of generic then
	     {O Get(ArgSpec.name $)}
	  elseof Getter then
	     if {IsAtom Getter} then {O Getter($)}
	     else {Getter O}
	     end
	  end
       end}
   end

   fun {SelectConstructor ClassName News Args}
      case News of (X=Inits#_)|Rest then
	 if {ConstructorHasArgs ClassName Inits Args} then X
	 else {SelectConstructor ClassName Rest Args}
	 end
      [] nil then !NONE
      end
   end

   proc {AnalyzeConfigurator Desc ClassName ?Handle ?Args ?Signals ?Children}
      ChildrenDict = {NewDictionary}
      Children0
   in
      Args = {NewDictionary}
      Signals = {NewDictionary}
      for F in {Arity Desc} do Value in
	 Value = Desc.F
	 case F of handle then
	    Handle = Value
	 [] children then
	    ChildrenDict.children := Value
	 elseif {IsAtom F} then
	    case {GetFeature ClassName F} of ArgSpec=arg(...) then
	       if {GBuilderTypes.is ArgSpec.type Value} then
		  Args.F := Value
	       else
		  {Exception.raiseError
		   gBuilder(type ClassName F ArgSpec.type Value)}
	       end
	    [] signal(...) then
	       Signals.F := Value
	    [] !NONE then
	       {Exception.raiseError
		gBuilder(unknownFeature ClassName F Value)}
	    end
	 elseif {IsInt F} then
	    ChildrenDict.F := Value
	 else
	    {Exception.raiseError gBuilder(unknownFeature ClassName F Value)}
	 end
      end
      Children0 = {Dictionary.toRecord children ChildrenDict}
      case Children0 of children(children: Cs) then
	 Children = {List.toTuple children Cs}
      elseif {IsTuple Children0} then
	 Children = Children0
      else
	 {Exception.raiseError
	  gBuilder(illegalChildren ClassName {Arity Children0})}
      end
   end

   ConfigureArgs = {NewName}
   ConfigureSignals = {NewName}
   ConfigureChildren = {NewName}
   Get = {NewName}

   fun {MakeCallback X}
      case X of Port#Message andthen {IsPort Port} then
	 proc {$ _} {Send Port Message} end
      [] Object#Message andthen {IsObject Object} then
	 proc {$ _} {Object Message} end
      elseof Procedure then Procedure
      end
   end

   proc {Create Desc Object}
      if {IsObject Desc} then Object = Desc
      else ClassName in
	 ClassName = {Label Desc}
	 case {Dictionary.condGet WidgetSpecs ClassName NONE} of !NONE then
	    {Exception.raiseError gBuilder(unknownClass ClassName)}
	 elseof ClassSpec then Args Signals Children in
	    {AnalyzeConfigurator Desc ClassName
	     ?Object ?Args ?Signals ?Children}
	    case {CondSelect ClassSpec new NONE} of !NONE then
	       {Exception.raiseError gBuilder(abstract ClassName)}
	    elseof News then
	       case {SelectConstructor ClassName News Args}
	       of Names#Constructor then InitArgs in
		  InitArgs = {ConstructorGetArgs ClassName Names Args}
		  %% Apply the constructor
		  if {IsAtom Constructor} then
		     Object = {New ClassSpec.'class'
			       {List.toTuple Constructor InitArgs}}
		  else
		     {Procedure.apply Constructor
		      ClassSpec.'class'|{Append InitArgs [Object]}}
		  end
	       [] !NONE then
		  {Exception.raiseError
		   gBuilder(constructor ClassName {Dictionary.keys Args})}
	       end
	    end
	    {Object ConfigureArgs(Args ClassName)}
	    {Object ConfigureSignals(Signals ClassName)}
	    {Object ConfigureChildren(Children ClassName)}
	 end
      end
   end

   class GBuilderMixin
      meth !ConfigureArgs(Args ClassName) Spec in
	 Spec = WidgetSpecs.ClassName
	 GBuilderMixin, TrySetters({CondSelect Spec set nil} Args ClassName)
	 for Name in {Dictionary.keys Args} do Value ArgSpec in
	    Value = Args.Name
	    ArgSpec = {GetFeature ClassName Name}
	    case {CondSelect ArgSpec set NONE} of !NONE then
	       {Exception.raiseError
		gBuilder(notSettable ClassName Name Value)}
	    elseof Setter then NativeValue in
	       NativeValue = {GBuilderTypes.fromOz ArgSpec.type Value}
	       case Setter of generic then
		  {self set(ArgSpec.name NativeValue)}
	       elseif {IsAtom Setter} then
		  {self Setter(NativeValue)}
	       else
		  {Setter self NativeValue}
	       end
	    end
	 end
      end
      meth TrySetters(Setters Args ClassName)
	 case Setters of Names#Setter|Rest then
	    if {SetterHasArgs ClassName Names Args false} then SetArgs in
	       SetArgs = {SetterGetArgs ClassName Names Args self}
	       %% apply the setter
	       if {IsAtom Setter} then
		  {self {List.toTuple Setter SetArgs}}
	       else
		  {Procedure.apply Setter self|SetArgs}
	       end
	    end
	    GBuilderMixin, TrySetters(Rest Args ClassName)
	 [] nil then
	    case {CondSelect WidgetSpecs.ClassName isa NONE} of !NONE then skip
	    elseof ParentClassName then Setters in
	       Setters = {CondSelect WidgetSpecs.ParentClassName set nil}
	       GBuilderMixin, TrySetters(Setters Args ParentClassName)
	    end
	 end
      end
      meth !ConfigureSignals(Signals ClassName)
	 for Signal in {Dictionary.keys Signals} do Callback in
	    Callback = {MakeCallback Signals.Signal}
	    case {GetFeature ClassName Signal} of signal(SignalName) then
	       {self signalConnect(SignalName Callback _)}
	    end
	 end
      end
      meth !ConfigureChildren(Children ClassName) Adds in
	 Adds = {GetAdds ClassName}
	 for I in 1..{Width Children} do Desc Args Child in
	    Desc = Children.I
	    Args = {NewDictionary}
	    for F in {Arity Desc} do
	       if F \= 1 then
		  Args.F := Desc.F
	       end
	    end
	    Child = case {CondSelect Desc 1 NONE} of !NONE then
		       {Exception.raiseError
			gBuilder(missingChild ClassName {Label Desc})} unit
		    elseof ChildDesc then {Create ChildDesc}
		    end
	    case {CondSelect Adds {Label Desc} NONE} of !NONE then
	       {Exception.raiseError
		gBuilder(unknownAddMethod ClassName {Label Desc})}
	    [] ArgsSpec#Adder then AddArgs in
	       AddArgs =
	       {Map ArgsSpec
		fun {$ ArgSpec} Name in
		   Name = {Label ArgSpec}
		   case {Dictionary.condGet Args Name NONE} of !NONE then
		      case {CondSelect ArgSpec default NONE} of !NONE then
			 {Exception.raiseError
			  gBuilder(missingAddOption
				   ClassName {Label Desc} Name)} unit
		      elseof X then {GBuilderTypes.fromOz ArgSpec.type X}
		      end
		   elseof X then
		      {Dictionary.remove Args Name}
		      if {Not {GBuilderTypes.is ArgSpec.type X}} then
			 {Exception.raiseError
			  gBuilder(type ClassName {Label Desc} Name
				   ArgSpec.type X)}
		      end
		      {GBuilderTypes.fromOz ArgSpec.type X}
		   end
		end}
	       if {Not {Dictionary.isEmpty Args}} then
		  {Exception.raiseError
		   gBuilder(unknownAddOptions ClassName {Label Desc}
			    {Dictionary.keys Args})}
	       end
	       %% Apply the adder
	       if {IsAtom Adder} then
		  {self {List.toTuple Adder Child|AddArgs}}
	       else
		  {Procedure.apply Adder self|Child|AddArgs}
	       end
	    end
	 end
      end
   end

   fun {AtomToGTKName Signal}
      {String.toAtom
       {FoldR {Atom.toString Signal}
	fun {$ C In}
	   if {Char.isUpper C} then &-|{Char.toLower C}|In
	   else C|In
	   end
	end nil}}
   end

   for ClassName in {Arity GBuilderWidgets.widgets} do
      Spec Features X0 X1 X2 X3 X4 C
   in
      Spec = GBuilderWidgets.widgets.ClassName
      Features = {FoldR {CondSelect Spec args nil}
		  fun {$ Arg In}
		     {Label Arg}#
		     {Adjoin Arg arg(name: {AtomToGTKName {Label Arg}})}|In
		  end
		  {Map {CondSelect Spec signals nil}
		   fun {$ Signal} Signal#signal({AtomToGTKName Signal}) end}}
      X0 = [features#{List.toRecord features Features}]
      X1 = case {CondSelect Spec isa NONE} of !NONE then X0
	   elseof ClassName then isa#ClassName|X0
	   end
      X2 = case {CondSelect Spec new NONE} of !NONE then X1
	   elseof News then new#News|X1
	   end
      X3 = case {CondSelect Spec set NONE} of !NONE then X2
	   elseof Sets then set#Sets|X2
	   end
      X4 = case {CondSelect Spec add NONE} of !NONE then X3
	   elseof Adds then
	      add#{List.toRecord add
		   {Map Adds
		    fun {$ Method#Args#Adder} Method#(Args#Adder) end}}|X3
	   end

      class C
	 from
	    case Spec.api of gdk then GDK
	    [] gtk then GTK
	    [] gtkcanvas then GTKCANVAS
	    end.ClassName
	    GBuilderMixin
	 meth conf(...)=M Args Signals Children in
	    {AnalyzeConfigurator M ClassName unit ?Args ?Signals ?Children}
	    GBuilderMixin, ConfigureArgs(Args ClassName)
	    GBuilderMixin, ConfigureSignals(Signals ClassName)
	    GBuilderMixin, ConfigureChildren(Children ClassName)
	 end
	 meth return(...)=M
	    for Name in {Arity M} do
	       case {GetFeature ClassName Name} of ArgSpec=arg(...) then
		  Value = case {CondSelect ArgSpec get NONE} of !NONE then
			     {Exception.raiseError
			      gBuilder(notGettable ClassName Name)} unit
			  [] generic then
			     C, Get(ArgSpec.name $)
			  elseof Getter then
			     if {IsAtom Getter} then {self Getter($)}
			     else {Getter self}
			     end
			  end
	       in
		  M.Name = {GBuilderTypes.toOz ArgSpec.type Value}
	       else
		  {Exception.raiseError
		   gBuilder(unknownArgument ClassName Name)}
	       end
	    end
	 end
	 meth !Get(X Y)
	    GTK.widget, get(X Y)
	 end
      end

      WidgetSpecs.ClassName := {List.toRecord ClassName 'class'#C|X4}
   end

   fun {KeyPress KeySpec}
      KeySpec2 = {Map KeySpec
		  fun {$ Mods#String#X}
		     Mods#{ByteString.make String}#{MakeCallback X}
		  end}
   in
      proc {$ Xs=['GDK_KEY_PRESS'(string: S state: X ...)]}
	 Mods = {GBuilderTypes.toOz modifierType X}
      in
	 {Some KeySpec2
	  fun {$ ExpectedMods#ExpectedString#Callback}
	     if S == ExpectedString andthen
		{All ExpectedMods fun {$ Mod} {Member Mod Mods} end}
	     then {Callback Xs} true else false
	     end
	  end _}
      end
   end

   {Error.registerFormatter gBuilder
    fun {$ E} T in
       T = 'GBuilder error'
       case E of gBuilder(unknownClass ClassName) then
	  error(kind: T
		msg: 'unknown widget class'
		items: [hint(l: 'Name' m: oz(ClassName))])
       [] gBuilder(unknownFeature ClassName Feature Value) then
	  error(kind: T
		msg: 'unknown feature'
		items: [hint(l: 'Widget class' m: ClassName)
			hint(l: 'Feature' m: oz(Feature))
			hint(l: 'Value' m: oz(Value))])
       [] gBuilder(unknownArgument ClassName Name) then
	  error(kind: T
		msg: 'unknown argument'
		items: [hint(l: 'Widget class' m: ClassName)
			hint(l: 'Name' m: oz(Name))])
       [] gBuilder(abstract ClassName) then
	  error(kind: T
		msg: 'widget class is abstract'
		items: [hint(l: 'Widget class' m: ClassName)])
       [] gBuilder(constructor ClassName Features) then
	  error(kind: T
		msg: 'no matching constructor'
		items: [hint(l: 'Widget class' m: ClassName)
			hint(l: 'Features' m: '{'#list(Features ', ')#'}')])
       [] gBuilder(notSettable ClassName ArgName Value) then
	  error(kind: T
		msg: 'trying to set read-only argument'
		items: [hint(l: 'Widget class' m: ClassName)
			hint(l: 'Argument name' m: ArgName)
			hint(l: 'Value' m: Value)])
       [] gBuilder(notGettable ClassName ArgName) then
	  error(kind: T
		msg: 'trying to get write-only argument'
		items: [hint(l: 'Widget class' m: ClassName)
			hint(l: 'Argument name' m: ArgName)])
       [] gBuilder(illegalChildren ClassName Features) then
	  error(kind: T
		msg: 'illegal features for specifying children'
		items: [hint(l: 'Widget class' m: ClassName)
			hint(l: 'Features' m: '{'#list(Features ', ')#'}')])
       [] gBuilder(missingChild ClassName Method) then
	  error(kind: T
		msg: 'missing child description'
		items: [hint(l: 'Widget class' m: ClassName)
			hint(l: 'Add method' m: oz(Method))])
       [] gBuilder(unknownAddMethod ClassName Method) then
	  error(kind: T
		msg: 'unknown add method'
		items: [hint(l: 'Widget class' m: ClassName)
			hint(l: 'Name' m: oz(Method))])
       [] gBuilder(unknownAddOptions ClassName Method Names) then
	  error(kind: T
		msg: 'unknown add option'
		items: (hint(l: 'Widget class' m: ClassName)|
			hint(l: 'Add method' m: Method)|
			{FoldR Names
			 fun {$ Name In}
			    hint(l: 'Unknown option' m: oz(Name))|In
			 end nil}))
       [] gBuilder(missingAddOption ClassName Method Name) then
	  error(kind: T
		msg: 'missing add option'
		items: [hint(l: 'Widget class' m: ClassName)
			hint(l: 'Add method' m: Method)
			hint(l: 'Missing option' m: Name)])
       [] gBuilder(type ClassName ArgName Type Value) then
	  error(kind: T
		msg: 'type error'
		items: [hint(l: 'Widget class' m: ClassName)
			hint(l: 'Argument name' m: ArgName)
			hint(l: 'Expected type' m: oz(Type))
			hint(l: 'Value' m: oz(Value))])
       [] gBuilder(type ClassName Method Name Type Value) then
	  error(kind: T
		msg: 'type error'
		items: [hint(l: 'Widget class' m: ClassName)
			hint(l: 'Add method' m: Method)
			hint(l: 'Option' m: Name)
			hint(l: 'Expected type' m: oz(Type))
			hint(l: 'Value' m: oz(Value))])
       end
    end}
end
