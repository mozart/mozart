%
% Authors:
%   Donatien Grolaux (2000)
%
% Copyright:
%   (c) 2000 Université catholique de Louvain
%
% Last change:
%   $Date$ by $Author$
%   $Revision$
%
% This file is part of Mozart, an implementation
% of Oz 3:
%   http://www.mozart-oz.org
%
% See the file "LICENSE" or
%   http://www.mozart-oz.org/LICENSE.html
% for information on usage and redistribution
% of this file, and for a DISCLAIMER OF ALL
% WARRANTIES.
%
%  The development of QTk is supported by the PIRATES project at
%  the Université catholique de Louvain.

fun{NewFont D}
   {New QTkFont {Record.adjoin D Init}}
end

class QTkFont

   from Tk.font SetGet

   feat
      widgetType:font
      typeInfo:r(all:r(family:vs
		       size:int
		       weight:[normal bold]
		       slant:[roman italic]
		       underline:boolean
		       overstrike:boolean)
		 uninit:r
		 unset:r
		 unget:r
		)
      
      
   meth !Init(...)=M
      lock
	 {Assert self.widgetType self.typeInfo M}
	 Tk.font,{Record.adjoin M tkInit}
      end
   end
   meth set(...)=M
      lock
	 {Assert self.widgetType self.typeInfo M}
	 {ExecTk font configure(self d(M))}
      end
   end
   meth get(...)=M
      lock
	 Actual={CondSelect M actual false}
	 DisplayOf={HasFeature M displayof}
	 N={Subtracts M [actual displayof]}
      in
	 {Assert self.widgetType self.typeInfo N}
	 {Record.forAllInd N
	  proc{$ I R}
	     {ReturnTk font
	      if Actual then
		 if DisplayOf then
		    actual(self "-displayof" DisplayOf "-"#I R)
		 else
		    actual(self "-"#I R)
		 end
	      else
		 configure(self "-"#I R)
	      end
	      self.typeInfo.all.I}
	  end}
      end
   end
   meth delete
      lock
	 {ExecTk font delete(self)}
      end
   end
   meth families(F)
      lock
	 fun{Loop Str}
	    case Str
	    of &{|_ then
	       L R
	    in
	       {List.takeDropWhile Str fun{$ C} C\=&} end L R}
	       if R\=nil then
		  {List.drop L 1}|{Loop {List.drop R 2}}
	       else
		  {List.drop L 1}|nil
	       end
	    [] nil then nil
	    else
	       L R
	    in
	       {List.takeDropWhile Str fun{$ C} C\=&  end L R}
	       if R\=nil then
		  L|{Loop {List.drop R 1}}
	       else
		  L|nil
	       end
	    end
	 end
      in
	 F={Loop {ReturnTk font families($) no}}
      end
   end
   meth measure(Text Ret displayof:D<=NoArgs)
      lock
	 if D==NoArgs then
	    {ReturnTk font measure(self Text Ret)}
	 else
	    {ReturnTk font measure(self "-displayof" D Text Ret)}
	 end
      end
   end
   meth metrics(Opt Ret displayof:D<=NoArgs)
      lock
	 if D==NoArgs then
	    {ReturnTk font metrics(self "-"#Opt Ret)}
	 else
	    {ReturnTk font metrics(self "-displayof" D "-"#Opt Ret)}
	 end
      end
   end
end
