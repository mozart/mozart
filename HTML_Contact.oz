functor
import
   HTML_Entry('class':HE)
   Text(htmlQuote:HtmlQuote)
export
   'class' : HTML_Contact
define
   class HTML_Contact from HE
      meth headTitle($) {HtmlQuote @name} end
      meth bodyTitle($) tt({HtmlQuote @name}) end
      meth formatHeaders($)
	 'div'(
	       'class' : 'formatheaders'
	    table(
	       'class' : 'formatheaders'
	       'width' : '100%'
	       'cellspacing' : '0'
	       'border': '0'
	       {self formatHeader('type' "contact" $)}
	       {self formatHeader('id' tt({HtmlQuote @id}) $)}
	       {self formatHeader('name' {HtmlQuote @name} $)}
	       if @email==unit then "" else
		  {self formatHeader(
			   'email'
			   a(href : "mailto:"#@email tt({HtmlQuote @email}))
			   $)}
	       end
	       if @www==unit then "" else
		  {self formatHeader(
			   'www'
			   a(href : @www tt({HtmlQuote @www}))
			   $)}
	       end
	       %% !!! this should list also the packages for which
	       %% this is a contact
	       )
	    )
      end
   end
end
