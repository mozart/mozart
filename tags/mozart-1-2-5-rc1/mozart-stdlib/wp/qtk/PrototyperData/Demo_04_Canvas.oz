local
   Canvas
   Desc=td(canvas(handle:Canvas
		  glue:nswe
		  bg:white))
   Window={QTk.build Desc}
   proc{CreateRectangle X Y}
      Tag={Canvas newTag($)}
      {Canvas create(rectangle X-50 Y-50 X+50 Y+50 fill:red tags:Tag)}
      PX={NewCell 0}
      PY={NewCell 0}
   in
      {Tag bind(event:"<1>"
		args:[int(x) int(y)]
		action:proc{$ X Y}
			  {Assign PX X}
			  {Assign PY Y}
		       end)}
      {Tag bind(event:"<B1-Motion>"
		args:[int(x) int(y)]
		action:proc{$ X Y}
			  {Tag move(X-{Access PX} Y-{Access PY})}
			  {Assign PX X}
			  {Assign PY Y}
		       end)}
      {Tag bind(event:"<3>"
		action:proc{$}
			  {Tag delete}
		       end)}
   end
   {Canvas bind(event:"<2>"
		args:[int(x) int(y)]
		action:CreateRectangle)}
in
   {Window show(wait:true)}
end
