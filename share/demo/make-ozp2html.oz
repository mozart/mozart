%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local

   local
      Authors =
      authors(duchier:
		 a(href: 'http://www.ps.uni-sb.de/~duchier/'
		   'Denys Duchier')
	      mmueller:
		 a(href: 'http://www.ps.uni-sb.de/~mmueller/'
		   'Martin Müller')
	      schulte:
		 a(href: 'http://www.ps.uni-sb.de/~schulte/'
		   'Christian Schulte')
	      wuertz:
		 'Jörg Würtz'
	      smolka:
		 a(href: 'http://www.ps.uni-sb.de/~smolka/'
		   'Gert Smolka')
	      henz:
		 'Martin Henz'
	      mehl:
		 a(href: 'http://www.ps.uni-sb.de/~mehl/'
		   'Michael Mehl')
	     )
      
   in
      fun {FormatAuthors As}
	 TAs = {Map As fun {$ A} Authors.A end}
      in
	 case TAs of [TA] then TA
	 [] TA|TAr then
	    {FoldL TAr fun {$ T TA}
			  '#'(T ', ' TA)
		       end TA}
	 end
      end
   end


   functor MakeOzp2Html

   import
      Open
      Syslet.{args exit}
      Component
      
   body
      
      class HtmlOut
	 from Open.html Open.file
	    
	 meth init
	    Open.file, init(name:stdout)
	 end
      end

      F = {New HtmlOut init}
      
      Page = {Component.load Syslet.args.ozp}
      
      {F
       tag(html(head(title(Page.title)
		     link(rel:stylesheet type:'text/css' href:'demo.css'))
		'body'(h1(Page.title)
		       
		       'div'('class':ignore
			     'If you can read this, your browser provides '
			     'insufficient support for cascading style '
			     'sheets. The visual presentation of this page '
			     'will suffer.')
		       
		       menu('class':margin
			    li(p(a(href: Page.name#'.oza'
				   'Start Applet'))))
		       
		       hr
		       
		       p('class': smallmenu
			 a(href: Page.name#'.oza'
			   '[Start Applet]')
			 a(href:'../install.html'
			   '[How to enable Oz Applets]'))
		       
		       local
			  Body={Adjoin
				{Record.filterInd Page fun {$ F _}
							  {IsInt F}
						       end}
				'#'}
		       in
			  case Body=='#' then
			     h2('Not yet available')
			  else
			     Body
			  end
		       end
		       
		       menu('class':margin
			    li(p(a(href:'../install.html'
				   'How to enable Oz Applets'))))
		       
		       hr
		       
		       address({FormatAuthors Page.authors}))))}
      
      {F close}
      {Syslet.exit 0}
   end
      
      
in

   {Application.syslet

    'ozp2html'

    MakeOzp2Html

    single(ozp(type:string optional:false))

   }

end
