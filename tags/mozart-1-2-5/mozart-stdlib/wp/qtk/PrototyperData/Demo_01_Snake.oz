local
   Canvas
   Desc=td(canvas(bg:green
		  width:200
		  height:200
		  handle:Canvas))
   Window={QTk.build Desc}
   Dir={NewCell r(~10 0)}
   Points={NewDictionary}
   {Window bind(event:"<Up>" action:proc{$} {Assign Dir r(~10 0)} end)}
   {Window bind(event:"<Left>" action:proc{$} {Assign Dir r(0 ~10)} end)}
   {Window bind(event:"<Down>" action:proc{$} {Assign Dir r(10 0)} end)}
   {Window bind(event:"<Right>" action:proc{$} {Assign Dir r(0 10)} end)}
   proc{Game X Y}
      D={Access Dir}
      LY=Y+{Access Dir}.1
      LX=X+{Access Dir}.2
      Key=LX*100+LY % Create a unique valid key for each X,Y position
   in
      {Canvas create(line X Y LX LY fill:red)}
      if LX>200 orelse LX<0 orelse LY>200 orelse LY<0 % Out of the window
	 orelse {Dictionary.member Points Key}        % Eat tail
      then % Lost
	 {QTk.bell}
	 {Window close}
      else
	 {Dictionary.put Points Key nil} % Remember the point
	 {Delay 250}
	 {Game LX LY}
      end
   end
in
   {Window show}
   {Delay 2000}
   {QTk.bell}
   {Game 100 100}
   {Show {Length {Dictionary.entries Points}}}
end
