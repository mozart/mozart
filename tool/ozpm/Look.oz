functor
import
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
   Global(background : Background)
export
   Main Title Toolbar ActionButton Key Value
   NiceTitle NiceAuthor NiceDescription NiceWhite
   Mysterious Button
define
   Main = {QTk.newLook}
   Title = {QTk.newLook}
   {Title.set label(text	: ""
		    glue	: nwes
		    %bg		: lightslateblue
		    %fg		: white
		    %relief	: sunken
		    relief      : groove
		    borderwidth	: 2
		    justify	: left
		    anchor	: w
		    ipadx	: 10)}
   Toolbar = {QTk.newLook}
   {Toolbar.set tbradiobutton(glue:sw pady:2 padx:2
			      highlightborderwidth:1
			      selectedborderwidth:1
			      borderwidth:1
			      disabledborderwidth:1
			     )}
   {Toolbar.set tdline(glue:nsw)}
   {Toolbar.set tdspace(glue:nsw)}
   ActionButton={QTk.newLook}
   {ActionButton.set button(glue:w
			    padx:5
			    state:disabled)}
   Key={QTk.newLook}
   {Key.set label(ipadx:2 ipady:2
		  glue:news
		  anchor:ne
		  bg:lightblue
		  borderwidth:1
		  relief:raised
		 )}
   Font={QTk.newFont font(family:"Helvetica")}
   Value={QTk.newLook}
   {Value.set label(borderwidth:1
		    font:Font
		    ipadx:2
		    ipady:2
		    relief:raised
		    glue:nswe
		    anchor:nw
		    background:Background
		    justify:left)}
   {Value.set html(glue:news
		   bg:Background
		   borderwidth:1
		   relief:raised
		  )}
   NiceTitle={QTk.newLook}
   {NiceTitle.set label(bg:darkorange
			relief:raised
			glue:we
			fg:white
			font:{QTk.newFont font(family:'Helvetica'
					       weight:bold
					       size:16)})}
   NiceAuthor={QTk.newLook}
   {NiceAuthor.set label(bg:Background
			 font:{QTk.newFont font(family:'Helvetica'
						slant:italic
						size:12)})}
   NiceDescription={QTk.newLook}
   {NiceDescription.set text(bg:Background
			     font:{QTk.newFont font(family:'Helvetica'
						    size:12)})}
   NiceWhite={QTk.newLook}
   {NiceWhite.set label(bg:Background)}
   {NiceWhite.set td(bg:Background)}
   {NiceWhite.set lr(bg:Background)}
   Mysterious={QTk.newLook}
   {Mysterious.set label(font:Font background:Background glue:nwe ipadx:2)}
   Button={QTk.newLook}
   BoldFont={QTk.newFont font(family:"Helvetica" weight:bold)}
   {Button.set tdbutton(font:BoldFont)}
end
