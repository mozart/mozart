%%%
%%% Authors:
%%%     Alberto Delgado <adelgado@cic.puj.edu.co>
%%%
%%% Copyright:
%%%     Alberto Delgado, 2007
%%%
%%% Last change:
%%%   $Date: 2006-10-19T01:44:35.108050Z $ by $Author: ggutierrez $
%%%   $Revision: 2 $
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

functor

require
   CpSupport(vectorToType:   VectorToType
	     vectorToList:   VectorToList
	     vectorToTuple:  VectorToTuple
	     %vectorMap:      VectorMap
	     %expand:         Expand
	     %formatOrigin:   FormatOrigin
	    )
   
import
   GFS at 'x-oz://boot/geoz-set'
   Space

export
   %% Telling domains
   var  : Var
   is   : IsVar
   sup  : Sup
   inf  : Inf
   compl : Compl
   complIn : ComplIn
   include : Include
   exclude : Exclude
   carVal  : CardVal
   carInt  : CardInt
   isIn    : IsInt

define
   %FsDecl = GFS.set
   IsVar = GFS.isVar
   Sup = {GFS.sup}
   Inf = {GFS.inf}
   Var
   Compl = GFS.comp
   ComplIn = GFS.complIn
   Include = GFS.incVal
   Exclude = GFS.excVal
   CardVal = GFS.cardVal
   CardInt = GFS.cardInt
   IsInt = GFS.isInt

in
   local
      fun {Decl}
	 {GFS.bounds nil Inf#Sup}
      end
   in      
      Var = var(decl : Decl
		bounds : GFS.bounds)
   end
   
end
