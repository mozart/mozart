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
   Open
   Util
export
   'create' : Create
define
   FilePrefix = ["#include <mozart.h>"
		 "#include <gtk/gtk.h>"
		 "#include <GOZData.h>"
		 "#include <stdio.h>\n"
		 "extern OZ_Term createGdkEvent(GdkEvent *event);\n"
		 "OZ_Term makeArgTerm(GtkArg *arg) {"
		 "  GtkType type = arg->type;\n"
		 "  if (type == GTK_TYPE_INT) {"
		 "    return GOZ_ARG_int(arg->d.int_data);"
		 "  } else if (type == GTK_TYPE_UINT) {"
		 "    return GOZ_ARG_int(arg->d.uint_data);"
		 "  } else if (type == GTK_TYPE_LONG) {"
		 "    return GOZ_ARG_int(arg->d.long_data);"
		 "  } else if (type == GTK_TYPE_ULONG) {"
		 "    return GOZ_ARG_int(arg->d.ulong_data);"
		 "  } else if (type == GTK_TYPE_CHAR) {"
		 "    return GOZ_ARG_int(arg->d.char_data);"
		 "  } else if (type == GTK_TYPE_UCHAR) {"
		 "    return GOZ_ARG_int(arg->d.uchar_data);"
		 "  } else if (type == GTK_TYPE_FLOAT) {"
		 "    return GOZ_ARG_double(arg->d.float_data);"
		 "  } else if (type == GTK_TYPE_DOUBLE) {"
		 "    return GOZ_ARG_double(arg->d.double_data);"
		 "  } else if (type == GTK_TYPE_BOOL) {"
		 "    return GOZ_ARG_bool(arg->d.bool_data);"
 		 "  } else if (type == GTK_TYPE_ENUM) {"
		 "    return GOZ_ARG_int(arg->d.int_data);"
 		 "  } else if (type == GTK_TYPE_FLAGS) {"
		 "    return GOZ_ARG_int(arg->d.int_data);"
		 "  } else if (type == GTK_TYPE_STRING) {"
		 "    return GOZ_ARG_string(arg->d.string_data);"
		 "  } else if (type == GTK_TYPE_POINTER) {"
		 "    return GOZ_ARG_pointer(arg->d.pointer_data);"
		 "  } else if (type == GTK_TYPE_OBJECT) {"
		 "    return GOZ_ARG_object(arg->d.object_data);"
		]

   
   class TextFile from Open.file Open.text end
   
   ToS = VirtualString.toString

   fun {IsLegal C}
      case C
      of 'GTK_TYPE_CANVAS_POINTS'   then false
      [] 'GTK_TYPE_GDK_IMLIB_IMAGE' then false
      else true
      end
   end
   local
      fun {DoLower S}
	 case S
	 of "CTREE" then "CTree"
	 [] "CLIST" then "CList"
	 [] _       then {Util.firstUpper {Map S Char.toLower}}
	 end
      end
   in
      fun {CreateKey Ks}
	 case Ks
	 of K|Kr  then {DoLower K}#{CreateKey Kr}
	 [] nil   then nil
	 end
      end
   end
   
   class GtkConstants from TextFile
      attr
	 types
      meth init(Types Cs)
	 @types = Types
	 TextFile, init(name: "GOZArguments.c"
			flags:[write create truncate])
	 {ForAll FilePrefix proc {$ L}
			       TextFile, putS(L)
			    end}
	 GtkConstants, writeConstants(Cs)
	 TextFile, putS("}")
      end
      meth writeConstants(Cs)
	 case Cs
	 of C|Cr then
	    if {IsLegal C}
	    then
	       Conv = if {self isEnum(C $)}
		      then "GOZ_ARG_int(arg->d.int_data);"
		      elsecase C
		      of 'GTK_TYPE_GDK_EVENT' then
			 "GOZ_ARG_event(createGdkEvent((GdkEvent *) (arg->d.object_data)));"
		      [] 'GTK_TYPE_GDK_COLOR' then
			 "GOZ_ARG_color(arg->d.object_data);"
		      [] 'GTK_TYPE_GDK_COLOR_CONTEXT' then
			 "GOZ_ARG_context(arg->d.object_data);"
		      [] 'GTK_TYPE_GDK_COLORMAP' then
			 "GOZ_ARG_map(arg->d.object_data);"
		      [] 'GTK_TYPE_GDK_DRAG_CONTEXT' then
			 "GOZ_ARG_drag(arg->d.object_data);"
		      [] 'GTK_TYPE_GDK_DRAWABLE' then
			 "GOZ_ARG_drawable(arg->d.object_data);"
		      [] 'GTK_TYPE_GDK_FONT' then
			 "GOZ_ARG_font(arg->d.object_data);"
		      [] 'GTK_TYPE_GDK_GC' then
			 "GOZ_ARG_gc(arg->d.object_data);"
		      [] 'GTK_TYPE_GDK_IMAGE' then
			 "GOZ_ARG_image(arg->d.object_data);"
		      [] 'GTK_TYPE_GDK_VISUAL' then
			 "GOZ_ARG_visual(arg->d.object_data);"
		      [] 'GTK_TYPE_GDK_WINDOW' then
			 "GOZ_ARG_window(arg->d.object_data);"
		      [] 'GTK_TYPE_ACCEL_GROUP' then
			 "GOZ_ARG_accel(arg->d.object_data);"
		      [] 'GTK_TYPE_STYLE' then
			 "GOZ_ARG_style(arg->d.object_data);"
		      else "GOZ_ARG_pointer(arg->d.object_data);"
		      end
	    in
	       TextFile, putS({ToS "  } else if (type == "#C#") {"})
	       TextFile, putS({ToS "    return "#Conv})
	    end
	    GtkConstants, writeConstants(Cr)
	 [] nil  then
	    TextFile, putS({ToS "  }"})
	    TextFile, putS({ToS "  fprintf(stderr, \"makeArgTerm: selecting default fallback on type '%d'\\n\", arg->type);"})
	    TextFile, putS("  return GOZ_ARG_object(arg->d.object_data);")
	 end
      end
      meth isEnum(C $)
	 Cs   = {ToS C}
	 KeyS = if {Util.checkPrefix "GTK_TYPE_CANVAS" Cs}
		then "GTK_CANVAS"#{Util.cutPrefix "GTK_TYPE_CANVAS" Cs}
		elseif {Util.checkPrefix "GTK_TYPE_GDK" Cs}
		then "GDK"#{Util.cutPrefix "GTK_TYPE_GDK" Cs}
		elseif {Util.checkPrefix "GTK_TYPE" Cs}
		then "GTK"#{Util.cutPrefix "GTK_TYPE" Cs}
		else Cs
		end
	 TokS = {Util.tokens {Util.toString KeyS} [&_]}
	 Key  = {Util.toAtom {CreateKey TokS}}
	 Res  = {Dictionary.condGet @types Key nil} 
      in
	 case Res
	 of enum(...) then true
	 [] _         then false
	 end
      end
   end

   proc {Create Types}
      Constants = {Dictionary.condGet Types gtk_const_kit nil}
      Emitter   = {New GtkConstants init(Types Constants)}
   in
      {Emitter close}
   end
end
