functor
import
   HTML_Entry('class':HE)
   Text(htmlQuote:HtmlQuote)
   Admin(manager:Manager)
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
	       if @pid==unit then '' else
		  {self formatHeader(
			   'section'
			   a(href:{Manager id_to_href(@pid $)}
			     {HtmlQuote @pid})
			   $)}
	       end
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
	       local
		  TOC={self getContributions($)}
	       in
		  {self formatHeaderEnum(
			   'contributions'
			   {Map TOC
			    fun {$ ID}
			       L={Manager id_to_href(ID $)}
			       N={{Manager condGetId(ID ReturnNil $)} getPackageName($)}
			    in
			       {Manager trace('creating link to '#ID#' '#L)}
			       a(href:L
				 if N==nil then
				    tt({HtmlQuote ID})
				 else
				    N
				 end)
			    end}
			   $)}
	       end
	       ))
      end
   end
   fun{ReturnNil _} nil end
end
