%%%
%%% Author:
%%%   Donatien Grolaux (ned@info.ucl.ac.be)
%%%
%%% Copyright:
%%%   Donatien Grolaux, 1999
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
   Open OS
export
   Convert
   ConvertList
   ConvertSubDir
define
   CArray={NewArray 0 63 0}
   {List.forAllInd "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" proc{$ I C} {Array.put CArray I-1 C} end}
       
   fun{Convert File}
      Dump
      Handler={New Open.file init(name:File flags:[read])}
   in
      local T in
	 T={Handler read(list:$ size:all)}
	 case ({Length T} mod 3) of 0 then Dump=T
	 [] 1 then Dump={List.append T [255 255]}
	 [] 2 then Dump={List.append T [255]}
	 end
      end
      {Handler close}
      local
	 proc{ByteToBit B B0 B1 B2 B3 B4 B5 B6 B7}
	    fun {GetBit V B}
	       B=V mod 2
	       V div 2
	    end
	 in
	    _={GetBit {GetBit {GetBit {GetBit {GetBit {GetBit {GetBit {GetBit B B0} B1} B2} B3} B4} B5} B6} B7}
	 end
	 fun{TB A0 A1 A2 A3 A4 A5}
	    {Array.get CArray A5*32+A4*16+A3*8+A2*4+A1*2+A0}
	 end
	 fun{Loop X N}
	    case X of A|B|C|Xs then
	       local
		  A0 A1 A2 A3 A4 A5 A6 A7
		  B0 B1 B2 B3 B4 B5 B6 B7
		  C0 C1 C2 C3 C4 C5 C6 C7
	       in
		  {ByteToBit A A0 A1 A2 A3 A4 A5 A6 A7}
		  {ByteToBit B B0 B1 B2 B3 B4 B5 B6 B7}
		  {ByteToBit C C0 C1 C2 C3 C4 C5 C6 C7}
		  if N>=68 then
		     {TB A2 A3 A4 A5 A6 A7}|{TB B4 B5 B6 B7 A0 A1}|{TB C6 C7 B0 B1 B2 B3}|{TB C0 C1 C2 C3 C4 C5}|10|32|32|32|32|{Loop Xs 0}
		  else
		     {TB A2 A3 A4 A5 A6 A7}|{TB B4 B5 B6 B7 A0 A1}|{TB C6 C7 B0 B1 B2 B3}|{TB C0 C1 C2 C3 C4 C5}|{Loop Xs N+4}
		  end
	       end
	    else if N>0 then 10|nil else nil end
	    end
	 end
      in
	 32|32|32|32|{Loop Dump 0}
      end
   end

   fun{ConvertList L} D={NewDictionary} in
      {ForAll L proc{$ I} {Dictionary.put D I {Convert I}} end}
      {Dictionary.entries D}
   end

   fun{ConvertSubDir PathS}
      GifList
      Path
      VSPath={VirtualString.toString PathS}
      Index
   in
      if {Member {List.nth VSPath 1} "/~"} then Path=VSPath else Path={VirtualString.toString {OS.getCWD}#"/"#VSPath} end
      if {List.nth {Reverse VSPath} 1}==47 then Index=VSPath else Index=VSPath#"/" end
      GifList={List.filter {OS.getDir {VirtualString.toString Path}}
	       fun{$ E} {List.length E}>4 andthen {List.take {Reverse E} 4}=="fig." end}
      {ConvertList {List.map GifList fun{$ I} {StringToAtom {VirtualString.toString Index#I}} end}}
   end
end
