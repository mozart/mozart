%%%
%%% Author:
%%%   Christian Schulte <schulte@dfki.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor

require
   URL(make:    UrlMake
       resolve: UrlResolve)
   
prepare

   PickleExt   = '.ozp'
   FunctorExt  = '.ozf'
   OzScheme    = 'x-oz'
   
   HomeUrl    = {UrlMake 'http://mozart.ps.uni-sb.de/home/share/'}
   ContribUrl = {UrlMake 'http://mozart.ps.uni-sb.de/home/contrib/'}
   SystemUrl  = {UrlMake OzScheme#'://system/'}
   BootUrl    = {UrlMake OzScheme#'://system/'}

   proc {MakeUrlTable As ?R}
      R={MakeRecord table As}
      {ForAll As proc {$ A}
		    R.A={UrlResolve SystemUrl {UrlMake A#FunctorExt}}
		 end}
   end

   local
      Libs      = ['Application'
		   'Search' 'FD' 'Schedule' 'FS'
		   'Error' 'ErrorRegistry' 'Finalize'
		   'Fault' 'Connection' 'Remote' 'VirtualSite' 'URL'
		   'Open'
		   'Tk' 'TkTools'
		   'Compiler'
		   'Type' 'Narrator' 'Listener' 'ErrorListener'
		   'Misc'
		   'DefaultURL']
      Volatiles = ['Module'
		   'Resolve' 'OS' 'Property' 'Pickle' 'System'] 
      Tools     = ['OPI' 'Panel' 'Browser' 'Explorer' 'CompilerPanel'
		   'Emacs' 'Ozcar' 'Profiler' 'Gump' 'GumpScanner'
		   'GumpParser' 'ProductionTemplates' 'Inspector']
   in
      LibFuncs    = {MakeUrlTable Libs}
      VolFuncs    = {MakeUrlTable Volatiles}
      ToolFuncs   = {MakeUrlTable Tools}
      SystemFuncs = {Adjoin LibFuncs {Adjoin VolFuncs ToolFuncs}}
      
      FunctorNames = {Append Volatiles {Append Tools Libs}}
   end
		 



   local
      VS2A = VirtualString.toAtom

      fun {NewNameTest Table}
	 fun {$ Name}
	    {HasFeature Table {VS2A Name}}
	 end
      end
   in
      fun {NameToUrl Name}
	 NameA = {VS2A Name}
      in
	 if {HasFeature SystemFuncs NameA} then
	    SystemFuncs.NameA
	 else
	    {UrlMake Name#FunctorExt}
	 end
      end

      IsLibName      = {NewNameTest LibFuncs}
      IsVolatileName = {NewNameTest VolFuncs}
      IsToolsName    = {NewNameTest ToolFuncs}
      IsSystemName   = {NewNameTest SystemFuncs}
   end


export
   NameToUrl

   IsSystemName
   IsLibName
   IsVolatileName
   IsToolsName

   HomeUrl
   ContribUrl
   SystemUrl
   BootUrl

   OzScheme
   
   PickleExt
   FunctorExt

   FunctorNames
end
