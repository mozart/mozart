functor
import
   HTML_Entry('class':HE)
   Text(htmlQuote:HtmlQuote)
   Admin(manager:Manager)
export
   'class' : HTML_Section
define
   class HTML_Section from HE
      meth formatHeaders($)
	 'div'(
	    'class' : 'formatheaders'
	    table(
	       'class' : 'formatheaders'
	       'width' : '100%'
	       'cellspacing' : '0'
	       'border': '0'
	       {self formatHeader('type' "section" $)}
	       {self formatHeader('id' tt({HtmlQuote @id}) $)}

% 	       if @email==unit then '' else
% 		  {self formatHeader(
% 			   'email'
% 			   a(href:"mailto:"#@email tt({HtmlQuote @email}))
% 			   $)}
% 	       end

	       {self formatHeaderEnum(
			'entries'
			{Map {Arity @toc}
			 fun {$ ID} L = {Manager id_to_href(ID $)} in
			    {Manager trace('creating link to '#ID#' '#L)}
			  a(href:L
			    tt({HtmlQuote ID}))
			 end}
			$)}
	       )
	    )
      end
      %%
      meth updateSubHtml(DB)
	 for K in {Arity @toc} do
	    {Manager trace('Processing entry '#K)}
	    {DB updateHtml(K)}
	 end
      end
   end
end
