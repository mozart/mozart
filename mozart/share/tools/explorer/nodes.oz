%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local

   \insert 'tk-nodes.oz'

   class FailedNode
      from CombineNodes.failed TkNodes.failed
   end

   class SuspendedNode
      from CombineNodes.suspended TkNodes.suspended
   end

   class EntailedNode
      from CombineNodes.succeeded TkNodes.entailed
   end

   class StuckNode
      from CombineNodes.succeeded TkNodes.stuck
   end
   
   class ChooseNode
      from CombineNodes.choose TkNodes.choose
   end

   class SentinelNode
      from CombineNodes.sentinel TkNodes.sentinel
   end

   fun {DerefSuspended S}
      case S of suspended(S) then {DerefSuspended S} else S end
   end
   
in

   fun {MakeRoot Manager Query Order AwaitStable}
      Sentinel={New SentinelNode dirtyUp}
      Features=f(classes:   Classes
		 canvas:    Manager.canvas
		 order:     Order
		 status:    Manager.status
		 manager:   Manager)
      Classes =c(failed:   {Class.new [FailedNode] a Features [final]}
		 suspended:{Class.new [SuspendedNode] a Features [final]}
		 entailed: {Class.new [EntailedNode] a Features [final]}
		 stuck:    {Class.new [StuckNode] a Features [final]}
		 choose:   {Class.new [ChooseNode] a Features [final]})
      S = {Space.new Query}
   in   
      case
	 if AwaitStable then {Space.ask S _} end
	 {Space.askVerbose S}
      of failed then
	 {New Classes.failed init(Sentinel 1)}
      [] succeeded(SA) then
	 {New Classes.SA init(Sentinel 1 S persistent)}
      [] alternatives(MaxAlt) then
	 {New Classes.choose  init(Sentinel 1 false persistent S MaxAlt)}
      [] suspended(Ctrl) then
	 {New Classes.suspended init(Sentinel 1 Ctrl)}
      end
   end
			
end
