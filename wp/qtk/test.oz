declare
{Show 'here'}
[QTk]={Module.link ["QTk.ozf"]}
{Wait QTk}
{Show 'starting'}
Win={QTk.build td(%
		  lr(glue:we
		     feature:top
		     numberentry(min:10 max:100 init:20 glue:we)
		     dropdownlistbox(init:[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19]
				     feature:dropdown
				     glue:ns
				     action:proc{$} {Show clicked} end
				     tdscrollbar:true))
		  panel(feature:panel
			td(title:"Panel1"
			   feature:panel1
			   label(text:"Panel1" feature:label))
			td(title:"Panel2"
			   feature:panel2
			   label(text:"Panel2" feature:label)))
		  lrscrollbar(glue:we)
		  label(text:"Hello world")
		  placeholder(label(text:"blabla"))
		  listbox(init:[1 2 3] tdscrollbar:true feature:listbox)
		  lrscrollbar(glue:we)
		  text(glue:nswe tdscrollbar:true lrscrollbar:true)
		  menubutton(text:"test"
			     menu:menu(command(text:"Item")))
		  button(text:"Button")
		 )}
{Show Win}
{Win show}
{Show 'end'}
{Show Win.listbox}
{Win.listbox set(bg:blue)}
{Win.listbox.tdscrollbar set(cursor:right_ptr)}
{Win.listbox set(cursor:arrow)}
{Show 'there'}
{Show Win.top}
{Show Win.top.dropdown}
{Show Win.top.dropdown.tdscrollbar}
{Win.top.dropdown.tdscrollbar set(cursor:right_ptr)}
{Win.panel.panel2.label set(text:"BOUH")}