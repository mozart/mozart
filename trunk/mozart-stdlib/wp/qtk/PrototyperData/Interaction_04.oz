local
   Left Right
   Desc=lr(label(bg:blue glue:nswe handle:Left)
	   label(bg:red  glue:nswe handle:Right))
in
   {{QTk.build Desc} show}
   {Left bind(event:"<Enter>" action:proc{$} {Show entering_left} end)}
   {Left bind(event:"<Leave>" action:proc{$} {Show leaving_left} end)}
   {Right bind(event:"<Enter>" action:proc{$} {Show entering_right} end)}
   {Right bind(event:"<Leave>" action:proc{$} {Show leaving_right} end)}
end

