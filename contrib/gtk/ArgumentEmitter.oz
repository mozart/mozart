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
                 "#include <gtk/gtk.h>\n"
                 "extern OZ_Term createGdkEvent(GdkEvent *event);\n"
                 "OZ_Term makeArgTerm(GtkArg *arg) {"
                 "  GtkType type = arg->type;\n"
                 "  if (type == GTK_TYPE_INT) {"
                 "    return OZ_int(arg->d.int_data);"
                 "  } else if (type == GTK_TYPE_DOUBLE) {"
                 "    return OZ_float(arg->d.float_data);"
                 "  } else if (type == GTK_TYPE_BOOL) {"
                 "    return OZ_int(arg->d.bool_data);"
                 "  } else if (type == GTK_TYPE_STRING) {"
                 "    return OZ_string(arg->d.string_data);"
                 "  } else if (type == GTK_TYPE_POINTER) {"
                 "    return OZ_makeForeignPointer(arg->d.pointer_data);"
                 "  } else if (type == GTK_TYPE_OBJECT) {"
                 "    return OZ_makeForeignPointer(arg->d.object_data);"
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
                      then "OZ_int(arg->d.int_data);"
                      elseif C == 'GTK_TYPE_GDK_EVENT'
                      then "createGdkEvent((GdkEvent *) (arg->d.object_data));"
                      else "OZ_makeForeignPointer(arg->d.object_data);"
                      end
            in
               TextFile, putS({ToS "  } else if (type == "#C#") {"})
               TextFile, putS({ToS "    return "#Conv})
            end
            GtkConstants, writeConstants(Cr)
         [] nil  then
            TextFile, putS({ToS "  }"})
            TextFile, putS("  return OZ_atom(\"unknown type\");")
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
