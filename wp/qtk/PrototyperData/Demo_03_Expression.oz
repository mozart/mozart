local
   proc {E Q R}
      {Compiler.evalExpression Q env _ R} 
   end
   In Out
   D=td(tdrubberframe(glue:nswe
          lr(glue:nswe
             label(text:"Expression")
             text(handle:In glue:nswe))
          lr(glue:nswe
             label(text:"Result")
             text(handle:Out glue:nswe tdscrollbar:true)))
        lr(glue:we button(glue:we text:"Eval"
                  action: proc {$}
                             V={E {In get($)}}
                          in
                             {Out set(V)}
                          end)
           button(glue:we text:"Quit"
                  action:toplevel#close)))
   W={QTk.build D}
in
   {W show(wait:true)}
end

