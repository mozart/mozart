%%%
%%% Author:
%%%   Thorsten Brunklaus <bruni@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Thorsten Brunklaus, 2001
%%%
%%% Last Change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor $
import
   Pickle
   Open
   Util
   System(show)
   OS(getCWD)
export
   'funcEmitter'    : Emitter
   'fieldEmitter'   : FieldEmitter
   'coreResultType' : CoreTypeFun
define
   local
      %% Hack: Something goes wrong with these symbols:
      %% Omit them (Windows specific)
      ErrorSymbols = [%% Missing GDK Functions
		      "gdk_drawable_get_data" "gdk_imlib_inlined_png_to_image"
		      "gdk_gc_init" "gdk_keyval_convert_case"
		      "gdk_set_sm_client_id" "gdk_ic_attr_destroy"
		      "gdk_ic_attr_new" "gdk_add_client_message_filter"
		      %% Missing GTK Functions
		      "gtk_theme_engine_ref" "gtk_binding_parse_binding"
		      "gtk_binding_entry_add_signall"
		      "gtk_binding_entry_remove"
		      "gtk_container_dequeue_resize_handler"
		      "gtk_container_child_args_collect" "gtk_draw_cross"
		      "gtk_draw_handle" "gtk_draw_slider" "gtk_draw_focus"
		      "gtk_draw_extension" "gtk_draw_box_gap"
		      "gtk_draw_shadow_gap"
		      "gtk_draw_tab" "gtk_draw_ramp" "gtk_draw_option"
		      "gtk_draw_check"
		      "gtk_draw_flat_box" "gtk_theme_engine_unref"
		      "gtk_arg_to_valueloc" "gtk_arg_info_hash"
		      "gtk_arg_info_equal"
		      "gtk_arg_name_strip_type" "gtk_args_query"
		      "gtk_arg_type_new_static" "gtk_arg_get_info"
		      "gtk_args_collect_cleanup" "gtk_args_collect"
		      "gtk_arg_values_equal" "gtk_arg_reset" "gtk_arg_free"
		      "gtk_arg_copy" "gtk_arg_new"
		      "gtk_target_list_find" "gtk_target_list_remove"
		      "gtk_target_list_add_table" "gtk_target_list_add"
		      "gtk_target_list_ref" "gtk_theme_engine_get"
		      "gtk_identifier_get_type" "gtk_propagate_event"
		      "gtk_window_reposition"
		      %% Missing GTK Canvas Functions (Affine Transformations)
		      "gtk_canvas_item_scale"
		      "gtk_canvas_item_rotate"]
      
      class TextFile from Open.file Open.text end

      GtkCanvasFilePrepend = ["/*"
			      " * This file is generated. Please do not edit."
			      " */"
			      ""
			      "#include <mozart.h>"
			      "#include <gdk_imlib.h>"
			      "#include <gtk/gtk.h>"
			      "#include <gtk-canvas.h>"
			      "#include \"GOZData.h\""
			      "#include \"GOZCanvas.h\""]
      
      GtkFilePrepend = ["/*"
			" * This file is generated. Please do not edit."
			" */"
			""
			"#include <mozart.h>"
			"#include <gtk/gtk.h>"
			"#include \"GOZData.h\""]

      GdkFilePrepend = ["/*"
			" * This file is generated. Please do not edit."
			" */"
			""
			"#include <mozart.h>"
			"#include <gdk/gdk.h>"
			"#include <gdk_imlib.h>"
			"#include \"GOZData.h\""]

      GtkCanvasInitPrepend = ["/*"
			      " * This file is generated. Please do not edit."
			      " */"
			      ""
			      "#include <mozart.h>"
			      "#include <gdk_imlib.h>"
			      "#include <gtk/gtk.h>"
			      "#include <gtk-canvas.h>"
			      "#include \"GOZData.h\""]
      
      GtkCanvasInitStub = ["char oz_module_name[] = \"GtkCanvasNative\";"
			   "#if defined(__CYGWIN32__) || defined(__MINGW32__)"
			   "__declspec(dllexport) int _impure_ptr;"
			   "#endif"
			   "OZ_C_proc_interface *oz_init_module() {"
			   "\tgdk_init(0, 0);"
			   "\tgtk_init(0, 0);"
			   "\tgtk_canvas_init();"
			   "\tnative_system_type_init();"
			   "\treturn oz_interface;"
			   "}"]

      GtkInitStub = ["char oz_module_name[] = \"GtkNative\";"
		     "OZ_C_proc_interface *oz_init_module() {"
		     "\tgdk_init(0, 0);"
		     "\tgtk_init(0, 0);"
		     "\treturn oz_interface;"
		     "}"]

      GdkInitStub = ["char oz_module_name[] = \"GdkNative\";"
		     "OZ_C_proc_interface *oz_init_module() {"
		     "\tgdk_init(0, 0);"
		     "\treturn oz_interface;"
		     "}"]
      
      fun {Base M}
	 M#"("
      end

      fun {Enum M}
	 "GOZ_declareEnumType("#M#", "
      end
      
      fun {Generic M P}
	 "GOZ_declareForeignType("#M#case P of "" then ", " else " "#P#", " end
      end
      
      fun {IsEnumType T}
	 case T of 'enum'(...) then true else false end
      end

      fun {IsCoreAliasType T}
	 case T of type(_ "") then true else false end
      end

      fun {GetAlias T}
	 case T of type(Name _) then Name end
      end

      fun {IsValidAlias Alias}
	 if {IsAtom Alias}
	 then true
	 elsecase Alias
	 of 'struct_member(decl, expr)'(...) then true
	 else false
	 end
      end
      
      fun {DeclareCoreType Types Type}
	 case Type
	 of "gboolean" then {Base "GOZ_declareBool"}
	 [] "gchar"    then {Base "GOZ_declareChar"}
	 [] "guchar"   then {Base "GOZ_declareUnsignedChar"}
	 [] "gint"     then {Base "GOZ_declareInt"}
	 [] "int"      then {Base "GOZ_declareInt"}
	 [] "guint"    then {Base "GOZ_declareUnsignedInt"}
	 [] "unsigned int" then {Base "GOZ_declareUnsignedInt"}
	 [] "gshort"   then {Base "GOZ_declareShort"}
	 [] "gushort"  then {Base "GOZ_declareUnsignedShort"}
	 [] "glong"    then {Base "GOZ_declareLong"}
	 [] "gulong"   then {Base "GOZ_declareUnsignedLong"}
	 [] "gint8"    then {Base "GOZ_declareInt8"}
	 [] "guint8"   then {Base "GOZ_declareUnsignedInt8"}
	 [] "gint16"   then {Base "GOZ_declareInt16"}
	 [] "guint16"  then {Base "GOZ_declareUnsignedInt16"}
	 [] "gint32"   then {Base "GOZ_declareInt32"}
	 [] "guint32"  then {Base "GOZ_declareUnsignedInt32"}
	 [] "gint64"   then {Base "GOZ_declareInt64"}
	 [] "guint64"  then {Base "GOZ_declareUnsignedInt64"}
	 [] "gfloat"   then {Base "GOZ_declareFloat"}
	 [] "gdouble"  then {Base "GOZ_declareDouble"}
	 [] "double"   then {Base "OZ_declareFloat"}
	 [] "gsize"    then {Base "GOZ_declareSize"}
	 [] "gssize"   then {Base "GOZ_declareSignedSize"}
	 else
	    TA = {Dictionary.condGet Types {Util.toAtom Type} nil}
	 in
	    if {IsEnumType TA}
	    then {Enum Type}
	    elseif {IsCoreAliasType TA}
	    then {DeclareCoreType Types {GetAlias TA}}
	    else {Generic Type ""}
	    end
	 end
      end

      fun {CoreResultType Type Types}
	 case Type
	 of "gboolean" then "GOZ_bool"
	 [] "gchar"    then "OZ_int"
	 [] "guchar"   then "OZ_int"
	 [] "gint"     then "OZ_int"
	 [] "int"      then "OZ_int"
	 [] "guint"    then "OZ_long"
	 [] "unsigned int" then "OZ_long"
	 [] "gshort"   then "OZ_int"
	 [] "gushort"  then "OZ_int"
	 [] "glong"    then "OZ_long"
	 [] "gulong"   then "OZ_long"
	 [] "gint8"    then "OZ_int"
	 [] "guint8"   then "OZ_int"
	 [] "gint16"   then "OZ_int"
	 [] "guint16"  then "OZ_int"
	 [] "gint32"   then "OZ_long"
	 [] "guint32"  then "OZ_long"
	 [] "gint64"   then "OZ_long"
	 [] "guint64"  then "OZ_long"
	 [] "gfloat"   then "OZ_float"
	 [] "gdouble"  then "OZ_float"
	 [] "double"   then "OZ_float"
	 [] "gsize"    then "OZ_long"
	 [] "gssize"   then "OZ_int"
	 [] Type       then
	    TA = {Dictionary.condGet Types {Util.toAtom Type} nil}
	 in
	    if {IsEnumType TA}
	    then "GOZ_long"
	    elseif {IsCoreAliasType TA}
	    then {CoreResultType {GetAlias TA} Types}
	    else "OZ_makeForeignPointer"
	    end
	 end
      end

      fun {ArgChain I M}
	 "Arg"#I#if I < M then ", "#{ArgChain (I + 1) M} else "" end
      end
   in
      CoreTypeFun = CoreResultType
      
      class Emitter from TextFile
	 attr
	    types     %% Type Dictionary
	    interface %% Interface List
	 meth init(Types Name)
	    FilePrepend = case {Util.toString Name}
			  of "GtkCanvasNative.c" then GtkCanvasFilePrepend
			  [] "GtkNative.c"       then GtkFilePrepend
			  [] "GdkNative.c"       then GdkFilePrepend
			  end
	 in
	    @types     = Types
	    @interface = nil
	    TextFile, init(name: Name
			   flags: [write create truncate])
	    {ForAll FilePrepend proc {$ L} TextFile, putS(L) end}
	 end
	 meth emit(Es)
	    case Es
	    of nil      then skip
	    [] Entry|Er then
	       Emitter, emitEntry(Entry)
	       Emitter, emit(Er)
	    end
	 end
	 meth emitEntry(Entry)
	    case Entry
	    of _#function(Name RetType Args) then
	       if {Member {Util.toString Name} ErrorSymbols}
	       then skip
	       else Emitter, emitFunc(Name RetType Args)
	       end
	    [] _ then skip
	    end
	 end
	 meth emitFunc(Name RetType Args)
	    IsVoid = case Args of [arg(type("void" "") _)] then true
		     else false
		     end
	    IA     = if IsVoid then 0 else {Length Args} end
	    OA     = case RetType
		     of type("void" _) then 0
		     [] _              then 1
		     end
	    IsDot
	 in
	    Emitter, emitHeader(Name IA OA)
	    IsDot = if IsVoid
		    then false
		    else Emitter, emitArgs(0 Args false $)
		    end
	    Emitter, emitReturnType(RetType)
	    Emitter, emitCall(Name IA OA IsDot)
	    Emitter, emitResultConversion(RetType)
	    Emitter, emitFinish
	    interface <- item(Name IA OA)|@interface
	 end
	 meth emitHeader(Key IA OA)
	    TextFile, putS({Util.toString "\nOZ_BI_define (native_"#Key#", "#IA#", "#OA#") {"})
	 end
	 meth emitArgs(I Args IsDot $)
	    case Args
	    of nil    then IsDot
	    [] Arg|Ar then
	       CurDot = Emitter, emitArgument(I Arg $)
	    in
	       Emitter, emitArgs((I + 1) Ar (IsDot orelse CurDot) $)
	    end
	 end
	 meth emitArgument(I Arg $)
	    ArgDecl#Dot = case Arg
			  of arg(type("..." "") _) then
			     {Base "GOZ_declareTerm"}#t
			  [] arg(type(Type "") _) then
			     {DeclareCoreType @types Type}#f
			  [] arg(type("char" "*") _) then
			     {Base "GOZ_declareString"}#f
			  [] arg(type("gchar" "*") _) then
			     {Base "GOZ_declareString"}#f
			  [] arg(type("const char" "*") _) then
			     {Base "GOZ_declareString"}#f
			  [] arg(type("const gchar" "*") _) then
			     {Base "GOZ_declareString"}#f
			  [] arg(type("GList" "*") _) then
			     {Base "GOZ_declareGList"}#f
			  [] arg(type("gchar" "*[]") _) then
			     {Base "GOZ_declareStringList"}#f
			  [] arg(type(Type Ptrs) _) then
			     {Generic Type {Util.cleanPointers Ptrs}}#f
			  end
	 in
	    TextFile, putS({Util.toString "\t"#ArgDecl#I#", Arg"#I#");"})
	    case Dot
	    of f then false
	    [] t then true
	    end
	 end
	 meth emitReturnType(RetType)
	    case RetType
	    of type("void" _)  then skip
	    [] type(Type "")   then
	       TextFile, putS({Util.toString "\t"#Type#" ret;"})
	    [] type(Type Ptrs) then
	       TextFile, putS({Util.toString "\t"#Type#" "#Ptrs#"ret;"})
	    end
	 end
	 meth emitCall(Key IA OA IsDot)
	    ToS  = Util.toString
	    Ret  = if OA == 0 then "" else "ret = " end
	    Args = if IA == 0 then "" else {ArgChain 0 (IA - 1)} end
	    Last = if IsDot then ", NULL" else "" end
	 in
	    if IsDot
	    then
	       Args = if IA == 0 then "" else {ArgChain 0 (IA - 2)} end
	       DA = "Arg"#(IA - 1)
	    in
	       TextFile, putS({ToS "\tif (OZ_isInt("#DA#")) {"})
	       TextFile, putS({ToS "\t\t"#Ret#Key#"("#Args#
			       ", OZ_intToC("#DA#"), NULL);"})
	       TextFile, putS({ToS "\t} else if (OZ_isFloat("#DA#")) {"})
	       TextFile, putS({ToS "\t\t"#Ret#Key#"("#Args#
			       ", OZ_floatToC("#DA#"), NULL);"})
	       TextFile, putS({ToS "\t} else if (OZ_isVirtualString("#
			       DA#", NULL)) {"})
	       TextFile, putS({ToS "\t\t"#Ret#Key#"("#Args#
			       ", GOZ_stringToC("#DA#"), NULL);"})
	       TextFile, putS({ToS "\t} else if (OZ_isForeignPointer("#
			       DA#")) {"})
	       TextFile, putS({ToS "\t\t"#Ret#Key#"("#Args#
			       ", OZ_getForeignPointer("#DA#"), NULL);"})
	       TextFile, putS("\t} else {")
	       TextFile, putS({ToS "\t\t"#Ret#Key#"("#{ArgChain 0 (IA - 2)}#
			       ", NULL);"})
	       TextFile, putS("\t}")
	    else
	       Args = if IA == 0 then "" else {ArgChain 0 (IA - 1)} end
	    in
	       TextFile, putS({ToS "\t"#Ret#Key#"("#Args#");"})
	    end
	 end
	 meth emitResultConversion(RetType)
	    case RetType
	    of type("void" _) then skip
	    else
	       Val = case RetType
		     of type(Type "")     then {CoreResultType Type @types}
		     [] type("char" "*")  then "OZ_string"
		     [] type("gchar" "*") then "OZ_string"
		     [] type("GList" "*") then "GOZ_makeGList"
		     [] type(_ _)         then "OZ_makeForeignPointer"
		     end
	    in
	       TextFile, putS({Util.toString "\tOZ_out(0) = "#Val#"(ret);"})
	    end
	 end
	 meth emitFinish
	    TextFile, putS("\treturn OZ_ENTAILED;")
	    TextFile, putS("} OZ_BI_end")
	 end
	 meth emitInterfaceFunc(Name RetType Args)
	    IsVoid = case Args of [arg(type("void" "") _)]
		     then true
		     else false
		     end
	    IA     = if IsVoid then 0 else {Length Args} end
	    OA     = case RetType
		     of type("void" _) then 0
		     [] _              then 1
		     end
	    Export = {Util.translateName Name}
	 in
	    TextFile, putS({Util.toString "\t{\""#Export#"\", "#
			    IA#", "#OA#", native_"#Name#"},"})
	 end
	 meth emitInterfaceEntry(Entry)
	    case Entry
	    of _#function(Name RetType Args) then
	       if {Member {Util.toString Name} ErrorSymbols}
	       then skip
	       else Emitter, emitInterfaceFunc(Name RetType Args)
	       end
	    [] _ then skip
	    end
	 end
	 meth emitInterfacePrefix
	    TextFile, putS("\nstatic OZ_C_proc_interface oz_interface[] = {")
	 end
	 meth emitInterface(Es)
	    case Es
	    of nil      then
	       TextFile, putS("\t#include \"GOZCanvasInterface.h\"")
	       TextFile, putS("\t{0, 0, 0, 0}\n};\n")
	    [] Entry|Er then
	       Emitter, emitInterfaceEntry(Entry)
	       Emitter, emitInterface(Er)
	    end
	 end
	 meth emitInitStub(Name)
	    InitStub = case {Util.toString Name}
		       of "GtkCanvasNative.c" then GtkCanvasInitStub
		       [] "GtkNative.c"       then GtkInitStub
		       [] "GdkNative.c"       then GdkInitStub
		       end
	 in
	    {ForAll InitStub proc {$ L} TextFile, putS(L) end}
	 end
      end

      %%
      %% Field Emitter constants and Classes
      %%

      GtkCanvasFieldInitStub =
      ["char oz_module_name[] = \"GtkCanvasFieldNative\";"
       "#if defined(__CYGWIN32__) || defined(__MINGW32__)"
       "__declspec(dllexport) int _impure_ptr;"
       "#endif"
       "OZ_C_proc_interface *oz_init_module() {"
       "\treturn oz_interface;"
       "}"]

      GtkFieldInitStub = ["char oz_module_name[] = \"GtkFieldNative\";"
			  "OZ_C_proc_interface *oz_init_module() {"
			  "\treturn oz_interface;"
			  "}"]
      
      GdkFieldInitStub = ["char oz_module_name[] = \"GdkFieldNative\";"
			  "OZ_C_proc_interface *oz_init_module() {"
			  "\treturn oz_interface;"
			  "}"]
      
      class FieldEmitter from Emitter
	 attr
	    names %% Class Names
	    first %% First Field Indicator
	 meth init(Types Name)
	    FilePrepend =
	    case {Util.toString Name}
	    of "GtkCanvasFieldNative.c" then @first = true GtkCanvasInitPrepend
	    [] "GtkFieldNative.c"       then @first = true GtkFilePrepend
	    [] "GdkFieldNative.c"       then @first = false GdkFilePrepend
	    end
	 in
	    @types     = Types
	    @interface = nil
	    TextFile, init(name: Name
			   flags: [write create truncate])
	    {ForAll FilePrepend proc {$ L} TextFile, putS(L) end}
	 end
	 meth emit(Class)
	    @names = {Pickle.load {Util.toString {OS.getCWD}#"/"#Class}}
	    {ForAll @names proc {$ Name}
			      case FieldEmitter, resolve(Name $)
			      of struct(Items) then
				 FieldEmitter, allocStruct(Name)
				 FieldEmitter, emitAccessors(Name Items @first)
			      [] _             then skip
			      end
			   end}
	 end
	 meth resolve(S $)
	    case {Dictionary.condGet @types S nil}
	    of nil         then nil
	    [] alias(S _)  then FieldEmitter, resolve(S $)
	    [] Struct      then
	       case Struct
	       of struct(...) then Struct
	       else raise resolve(unknown_data Struct) end
	       end
	    end
	 end
	 %% Debug Stuff
	 meth emitAccessors2(Name Items)
	    case Items
	    of item(text(Field) Ptrs Alias)|Ir then
	       Val = item({Util.toAtom Field} {Util.toAtom Ptrs} Alias)
	    in
	       TextFile, putS({Util.toString
			       {Value.toVirtualString Val 10000 10000}})
	       FieldEmitter, emitAccessors(Name Ir)
	    [] _|Ir then
	       FieldEmitter, emitAccessors(Name Ir)
	    [] nil then skip
	    end
	 end
	 meth allocStruct(Name)
	    ToS     = Util.toString
	    AccName = {ToS {Util.downTranslate Name}#"native_alloc"}
	 in
	    TextFile, putS({ToS "\nOZ_BI_define("#AccName#", 0, 1) {"})
	    TextFile, putS({ToS {Util.indent 1}#
			    "OZ_out(0) = OZ_makeForeignPointer("
			    #"malloc(sizeof("#Name#")));"})
	    TextFile, putS({ToS {Util.indent 1}#"return OZ_ENTAILED;"})
	    TextFile, putS("} OZ_BI_end")
	 end
	 meth emitAccessors(Name Items First)
	    case Items
	    of item(text(Field) Ptrs Alias)|Ir then
	       if {IsValidAlias Alias}
	       then
		  ToS       = Util.toString
		  FieldName = case Alias
			      of 'struct_member(decl, expr)'(Name _) then Name
			      [] _ then Alias
			      end
		  AccName   = {ToS {Util.downTranslate Name}#
			       "get_field_"#FieldName}
		  FieldTypR = {ToS Field#Ptrs}
		  FieldType = if {Util.checkPrefix "const " FieldTypR}
			      then {Util.cutPrefix "const " FieldTypR}
			      else FieldTypR
			      end
		  Cast      = if {Util.checkPrefix "const " FieldTypR}
			      then "("#FieldType#")"
			      else ""
			      end
		  ResType   = if Ptrs == nil
			      then {CoreResultType FieldType @types}
			      elsecase FieldType
			      of "GList*" then "GOZ_makeGList"
			      [] "char*"  then "OZ_string"
			      [] "gchar*" then "OZ_string"
			      else "OZ_makeForeignPointer"
			      end
	       in
		  TextFile, putS({ToS "\nOZ_BI_define ("#AccName#", 1, 1) {"})
		  TextFile, putS({ToS {Util.indent 1}#
				  "GOZ_declareForeignType("#
				  Name#" *, 0, Arg0);"})
		  %%
		  %% First Arg equals anchestor struct ptr
		  %% This is due to GTK's object simulation
		  %%
		  if First andthen {Util.checkPrefix "Gtk" FieldType}
		  then
		     TextFile, putS({ToS {Util.indent 1}#"OZ_out(0) = "#
				     "OZ_makeForeignPointer(Arg0);"})
		  else
		     NeedPtr = (ResType == "OZ_makeForeignPointer"
				andthen Ptrs == nil
				andthen Field \= "gpointer")
		     Star    = if NeedPtr then " *" else " " end
		     MemOp   = if NeedPtr then "\&" else nil end
		  in
		     TextFile, putS({ToS {Util.indent 1}#FieldType#Star#
				     "ret;"})
		     TextFile, putS({ToS {Util.indent 1}#
				     "ret = "#Cast#
				     MemOp#"Arg0->"#FieldName#";"})
		     TextFile, putS({ToS {Util.indent 1}#"OZ_out(0) = "#
				     ResType#"(ret);"})
		  end
		  TextFile, putS({ToS {Util.indent 1}#"return OZ_ENTAILED;"})
		  TextFile, putS("} OZ_BI_end")
	       end
	       FieldEmitter, emitAccessors(Name Ir false)
	    [] item(name(_) _ _)|Ir then
	       FieldEmitter, emitAccessors(Name Ir false)
	    [] nil then skip
	    end
	 end
	 meth emitInterface(Class)
	    ToS = Util.toString
	 in
	    {ForAll @names
	     proc {$ Name}
		case FieldEmitter, resolve(Name $)
		of struct(Items) then
		   Alloc    = {ToS {Util.downTranslate Name}#"native_alloc"}
		   ExpAlloc = {Util.translateName Alloc}
		   AI       = {ToS "\t\""#ExpAlloc#"\", 0, 1, "#Alloc#", "} 
		in
		   TextFile, putS(AI)
		   FieldEmitter, emitAccInterfaces(Name Items)
		[] _             then skip
		end
	     end}
	    TextFile, putS("\t{0, 0, 0, 0}\n};\n")
	 end
	 meth emitAccInterfaces(Name Items)
	    case Items
	    of item(text(_) _ Alias)|Ir then
	       if {IsValidAlias Alias}
	       then
		  ToS       = Util.toString
		  FieldName = case Alias
			      of 'struct_member(decl, expr)'(Name _) then Name
			      [] _ then Alias
			      end
		  AccName   = {ToS {Util.downTranslate Name}#
			       "get_field_"#FieldName}
		  ExpAccName = {Util.translateName AccName}
	       in
		  TextFile, putS({ToS "\t\""#ExpAccName#"\", "#
				  "1, 1, "#AccName#", "})
	       end
	       FieldEmitter, emitAccInterfaces(Name Ir)
	    [] item(name(_) _ _)| Ir then
	       FieldEmitter, emitAccInterfaces(Name Ir)
	    [] nil then skip
	    end
	 end
	 meth emitInitStub(Name)
	    InitStub = case {Util.toString Name}
		       of "GtkCanvasFieldNative.c" then GtkCanvasFieldInitStub
		       [] "GtkFieldNative.c"       then GtkFieldInitStub
		       [] "GdkFieldNative.c"       then GdkFieldInitStub
		       end
	 in
	    {ForAll InitStub proc {$ L} TextFile, putS(L) end}
	 end
      end
   end
end
