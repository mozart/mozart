functor

export
   u2i:U2I
   i2u:I2U
   i2t:I2T
   
require Open Compiler
   
prepare

   Escape=27
   Length=l(226:3)
   
   class TextFile from Open.text Open.file end
   
   F={New TextFile init(name:"utf2iso.dat")}
   
   PatternDict={NewDictionary}
   InvDict={NewDictionary}
   proc{Put D L C}
      case L
      of X|nil then
	 {Dictionary.put D X C}
      [] X1|X2|Xs then
	 T={Dictionary.condGet D X1 unit}
      in
	 if T\=unit then
	    {Put T X2|Xs C}
	 else
	    T={NewDictionary}
	 in
	    {Dictionary.put D X1 T}
	    {Put T X2|Xs C}
	 end
      end
   end
   proc{Loop}
      S={F getS($)}
   in
      if S\=false then
	 L R {List.takeDropWhile S fun{$ C} C\=&: end L R}
	 C={Compiler.evalExpression L nil _}
	 V={Compiler.evalExpression R.2 nil _}
      in
	 {Put PatternDict V C}
	 {Dictionary.put InvDict C V}
	 {Loop}
      end
   end
   
   fun{ToRecord D}
      if {Dictionary.is D} then
	 {Record.map
	  {Dictionary.toRecord c D}
	  ToRecord}
      else
	 D
      end
   end
   {Loop}
   {F close}
   
   DU2I={ToRecord PatternDict}
   DI2U={Dictionary.toRecord c InvDict}

   {ForAll [&\\#"\\\\" &{#"\\{" &}#"\\}" &[#"\\[" &]#"\\]" &'#"\\'" &"#[&\\ &"]
	    & #"\\ "]
    proc{$ C#S}
       {Dictionary.put InvDict C S}
    end}
   
   DI2T={Dictionary.toRecord c InvDict}
   
   fun{Match D L S}
      if L==nil then false
      else
	 R={CondSelect D L.1 false}
      in
	 if R\=false then
	    if {Record.is R} then
	       {Match R L.2 S}
	    else
	       S=L.2
	       R
	    end
	 else R end
      end
   end

   fun{Insert Left Right}
      case Left of X|Xs then X|{Insert Xs Right}
      else {U2I Right}
      end
   end
   
   fun{U2I T}
      if T==nil then nil else
	 S
	 R={Match DU2I T S}
      in
	 if R==false then
	    if T.1<128 then
	       T.1|{U2I T.2}
	    else
	       L={CondSelect Length T.1 2}
	       A B
	       {List.takeDrop T L A B}
	    in
	       Escape|L|{Insert A B}
	    end
	 else
	    R|{U2I S}
	 end
      end
   end

   fun{Conv Data}
      
      fun{Insert Left Right}
	 case Left of X|Xs then X|{Insert Xs Right}
	 else {I2U Right}
	 end
      end
      fun{I2U T}
	 if T==nil then nil
	 elsecase T of 27|L|Xs then
	    A B
	    {List.takeDrop Xs L A B}
	 in
	    {Insert A B}
	 else
	    R={CondSelect Data T.1 false}
	 in
	    if R==false then
	       T.1|{I2U T.2}
	    else
	       {Insert R T.2}
	    end
	 end
      end
   in
      I2U
   end


   I2U={Conv DI2U}

   I2Ti={Conv DI2T}

   fun{I2T S}
      R={I2Ti S}
   in
      if R==nil then "" else v(R) end
   end
   
end

