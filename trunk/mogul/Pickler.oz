%% Denys Duchier, Dec 2001
functor
export
   ToString FromString
   ToFile FromFile
import
   Open
define
   proc {IntToBytes I B1 B2 B3 B4}
      Q4 Q3
   in
      %% I is assumed to be non-negative
      B4 = I  mod 256
      Q4 = I  div 256
      B3 = Q4 mod 256
      Q3 = Q4 div 256
      B2 = Q3 mod 256
      B1 = Q3 div 256
   end
   proc {PickleToString V IN OUT}
      case V
      of unit  then IN=(&u|OUT)
      [] true  then IN=(&t|OUT)
      [] false then IN=(&f|OUT)
      [] X#Y   then MID in
	 {PickleToString Y IN MID}
	 {PickleToString X MID (&#|OUT)}
      elseif {IsInt V} then
	 if V < 0 then
	    {PickleToString ~V IN (&N|OUT)}
	 elsecase V %% optimize frequent special cases
	 of 0 then IN=(&0|OUT)
	 [] 1 then IN=(&1|OUT)
	 [] 2 then IN=(&2|OUT)
	 [] 3 then IN=(&3|OUT)
	 [] 4 then IN=(&4|OUT)
	 [] 5 then IN=(&5|OUT)
	 [] 6 then IN=(&6|OUT)
	 [] 7 then IN=(&7|OUT)
	 [] 8 then IN=(&8|OUT)
	 [] 9 then IN=(&9|OUT)
	 else B1 B2 B3 B4 in
	    %% use as few bytes as possible
	    {IntToBytes V B1 B2 B3 B4}
	    if B1==0 then
	       if B2==0 then
		  if B3==0 then
		     IN=(&F|B4|OUT)
		  else
		     IN=(&G|B3|B4|OUT)
		  end
	       else
		  IN=(&H|B2|B3|B4|OUT)
	       end
	    else
	       IN=(&I|B1|B2|B3|B4|OUT)
	    end
	 end
      elseif {IsAtom V} then
	 {PickleToString {AtomToString V} IN (&A|OUT)}
      elseif {IsString V} then N={Length V} MID in
	 {PickleToString N IN (&S|MID)}
	 {Append V OUT MID}
      elseif {IsList V} then N={Length V} MID in
	 {PickleListToString V IN MID}
	 {PickleToString N MID (&L|OUT)}
      elseif {IsTuple V} then MID in
	 {PickleToString {Record.toList V} IN MID}
	 {PickleToString {Label V} MID (&T|OUT)}
      elseif {IsRecord V} then MID in
	 {PickleToString {Record.toListInd V} IN MID}
	 {PickleToString {Label V} MID (&R|OUT)}
      elseif {IsDictionary V} then
	 {PickleToString {Dictionary.entries V} IN (&D|OUT)}
      elseif {IsByteString V} then N={ByteString.length V} MID in
	 {PickleToString N IN (&B|MID)}
	 {Append {ByteString.toString V} OUT MID}
      end
   end
   proc {PickleListToString L IN OUT}
      case L
      of nil then IN=OUT
      [] H|T then MID in
	 {PickleToString H IN MID}
	 {PickleListToString T MID OUT}
      end
   end

   proc {IntFromBytes I B1 B2 B3 B4}
      I=(((B1*256 + B2)*256 + B3)*256 + B4)
   end
   fun {PickleFromString L Stack}
      case L
      of nil then Stack
      [] &u|L then {PickleFromString L unit |Stack}
      [] &t|L then {PickleFromString L true |Stack}
      [] &f|L then {PickleFromString L false|Stack}
      [] &#|L then case Stack of X|Y|Stack then
		      {PickleFromString L (X#Y)|Stack}
		   end
      [] &F|B4|L then
	 {PickleFromString L {IntFromBytes $ 0 0 0 B4}|Stack}
      [] &G|B3|B4|L then
	 {PickleFromString L {IntFromBytes $ 0 0 B3 B4}|Stack}
      [] &H|B2|B3|B4|L then
	 {PickleFromString L {IntFromBytes $ 0 B2 B3 B4}|Stack}
      [] &I|B1|B2|B3|B4|L then
	 {PickleFromString L {IntFromBytes $ B1 B2 B3 B4}|Stack}
      [] &A|L then case Stack of S|Stack then
		      {PickleFromString L {StringToAtom S}|Stack}
		   end
      [] &S|L then case Stack of N|Stack then L1 L2 in
		      {List.takeDrop L N L1 L2}
		      {PickleFromString L2 L1|Stack}
		   end
      [] &B|L then case Stack of N|Stack then L1 L2 in
		      {List.takeDrop L N L1 L2}
		      {PickleFromString L2 {ByteString.make L1}|Stack}
		   end
      [] &L|L then case Stack of N|StackIn then StackOut Lst in
		      {Grab N StackIn StackOut nil Lst}
		      {PickleFromString L Lst|StackOut}
		   end
      [] &T|L then case Stack of Lab|Args|Stack then
		      {PickleFromString L {List.toTuple Lab Args}|Stack}
		   end
      [] &R|L then case Stack of Lab|Alist|Stack then
		      {PickleFromString L {List.toRecord Lab Alist}|Stack}
		   end
      [] &N|L then case Stack of N|Stack then
		      {PickleFromString L (~N)|Stack}
		   end
      [] &0|L then {PickleFromString L 0|Stack}
      [] &1|L then {PickleFromString L 1|Stack}
      [] &2|L then {PickleFromString L 2|Stack}
      [] &3|L then {PickleFromString L 3|Stack}
      [] &4|L then {PickleFromString L 4|Stack}
      [] &5|L then {PickleFromString L 5|Stack}
      [] &6|L then {PickleFromString L 6|Stack}
      [] &7|L then {PickleFromString L 7|Stack}
      [] &8|L then {PickleFromString L 8|Stack}
      [] &9|L then {PickleFromString L 9|Stack}
      [] &D|L then case Stack of Alist|Stack then
		      {PickleFromString L {AlistToDictionary Alist}|Stack}
		   end
      end
   end
   proc {Grab N StackIn StackOut Lin Lout}
      if N==0 then StackIn=StackOut Lin=Lout
      elsecase StackIn of X|StackIn then
	 {Grab N-1 StackIn StackOut X|Lin Lout}
      end
   end
   proc {AlistToDictionary Alist D}
      D={NewDictionary}
      {ForAll Alist
       proc {$ Key#Val} {Dictionary.put D Key Val} end}
   end

   fun {ToString V}
      {PickleToString V $ nil}
   end
   fun {FromString V}
      case {PickleFromString V nil} of [V] then V end
   end

   proc {ToFile V F}
      O={New Open.file init(name:F flags:[write create truncate])}
   in
      try
	 {O write(vs:{ToString V})}
      finally
	 try {O close} catch _ then skip end
      end
   end
   fun {FromFile F}
      O={New Open.file init(url:F)}
      S={O read(list:$ size:all)}
   in
      {O close}
      {FromString S}
   end
end
