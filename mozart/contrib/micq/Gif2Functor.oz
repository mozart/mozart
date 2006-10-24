%%%
%%% Author:
%%%   Nils Franzén (nilsf@sics.se)
%%%
%%% Copyright:
%%%   Nils Franzén, 1999
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

functor 
import
   Browser(browse)
   Application(getCmdArgs exit)
   GifToBase64(convert)
   System(showInfo printError)
   Pickle
   Property(get)
export
   ConvertToFile
define
   Spec=record('output'(single char:&o type:string default:nil)
	       'verbose'(single char:&v type:bool default:false))

   fun{Convert F}
      L={GifToBase64.convert F}
      BS={VirtualString.toByteString L}
   in
      functor
      import
	 Tk(image)
      export
	 Image
	 Data
      define
	 Data={ByteString.toString BS}
	 Image={New Tk.image tkInit(data:Data type:photo format:gif)}
      end
   end
   
   proc{ConvertToFile F O Verbose}
      F1=if O==nil then
	    {Map case F of A|As then
		    {VirtualString.toString {Char.toUpper A}|As}
		 else {VirtualString.toString F} end
	     fun{$ C}
		if {Char.isAlNum C} then C else &_ end
	     end}#".ozf"
	 else O end
      Funct={Convert F}
   in
      if Verbose then
	 {System.showInfo "Exporting image \""#F#"\" to functor \""#F1}
      end
      {Pickle.save Funct F1}
   end

   try
      Args={Application.getCmdArgs Spec}
   in
      case Args.1 of [F] then % Single file
	 try
	    {ConvertToFile F Args.output Args.verbose}
	    {Application.exit 0}
	 catch system(os(...) ...) then
	    {System.printError "Can't find input file: "#F#"\n"}
	    {Application.exit 2}
	 end
      elseof nil then % No input
	 {System.printError "No GIF file given\n"}
	 {Application.exit 2}
      else % Several files
	 {System.printError "Can't specify several input files!\n"}
	 {Application.exit 2}
      end
   catch X then
      case X of error(ap(usage M) ...) then
	 {System.printError
	  'Command line option error: '#M#'\n'#
	  'Usage: '#{Property.get 'application.url'}#' <GIF-FILE> [options]\n'#
	  '   --output=<File>    Alias: -o <File>\n'#
	  '   --verbose          Debug Info. Alias: -v <Url>\n'}
	 {Application.exit 2}
      elseof E then
	 {Browser.browse E}
%	 raise E end
      end
   end
end

/*

[A]={Module.apply [AA]}

{A.convertToFile "powered-by-oz-100.gif"}

*/
/*

declare
[F]={Module.link ['Powered_by_oz_100_gif.ozf']}
T={New Tk.toplevel tkInit()}
L={New Tk.label tkInit(parent:T image:F.image)}
{Tk.send pack(L)}

*/
