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
   Application
   Module
   System
   Inspector
   Parser at 'parser.so{native}'
   Prepare at 'Prepare.ozf'
   Flatten at 'Flatten.ozf'
   Collect at 'Collect.ozf'
   ToolKit at 'ToolKit.ozf'
define
   TreeSource = "gtkraw.c"
   PrepTree   = "gtkheader.c"
   {Prepare.'prepare' TreeSource PrepTree}
   ParseTree = {Parser.parse PrepTree}
   case ParseTree
   of 'parse error'(...) then {System.show 'parse error'} {Inspector.inspect ParseTree}
   [] ParseTree then
      FlatTree  = {Record.toList {Flatten.flatten ParseTree}}
      [Wrapper] = {Module.link {Application.getArgs plain}}
   in
      try
	 {Wrapper.create {ToolKit.create {Collect.collect FlatTree}}}
	 {Application.exit 0}
      catch E then
	 {Inspector.configure widgetUseNodeSet 2}
	 {Inspector.configure widgetTreeFont font(family:'courier' size:10 weight:normal)}
	 {Inspector.inspect error(E)}
	 {Inspector.inspect ParseTree}
	 {Inspector.inspect FlatTree}
      end
   end
end
