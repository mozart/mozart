functor
import
   Admin(manager:Manager)
   Text(
      htmlQuote:HtmlQuote
      makeBS
      )
   Open(file html)
export
   'class' : HTML_Entry
define
   class HTML_File from Open.file Open.html end
   class HTML_Entry
      meth headTitle($) {HtmlQuote @id} end
      meth bodyTitle($) tt({HtmlQuote @id}) end
      %%
      meth toPage($)
	 CSS           = {Manager get_css($)}
	 HeadTitle     = {self headTitle($)}
	 BodyTitle     = {self bodyTitle($)}
	 FormatHeaders = {self formatHeaders($)}
	 FormatBody    = {self formatBody($)}
      in
	 html(
	    head(
	       title(HeadTitle)
	       link(
		  rel : 'stylesheet'
		  type: 'text/css'
		  href: CSS))
	    body(
	       h1('class':'title' BodyTitle)
	       'div'(
		  'class':'entryinfo'
		  FormatHeaders
		  FormatBody)))
      end
      %%
      meth formatHeader(H V $ 'class':C<='header')
	 tr('class':C
	    'valign':'top'
	    td('class':C {HtmlQuote H})
	    td('class':C ":")
	    td('class':'value'   V ))
      end
      meth formatHeaderEnum(H L $ 'class':C<='header')
	 tr('class':C
	    'valign':'top'
	    td('class':C {HtmlQuote H})
	    td('class':C ":")
	    {AdjoinAt
	     {List.toTuple td
	      {FoldR L fun {$ V Accu}
			  if Accu==nil then [V] else
			     V|br|Accu
			  end
		       end nil}}
	     'class' 'value'})
      end
      %%
      meth formatBody($)
	 if @body==unit then ''
	 elseif @content_type=='text/html' then
	    'div'(
	       'class' : 'formatbody'
	       {Text.makeBS @body})
	 else
	    pre(
	       'class' : 'formatbody'
	       {Text.htmlQuote @body})
	 end
      end
      %%
      meth updateHtml(DB)
	 {Manager incTrace('--> updateHtml '#@id)}
	 try
	    Page = {self toPage($)}
	    Out  = {New HTML_File
		    init(name:{Manager id_to_html_filename(@id $)}
			 flags:[write truncate create])}
	 in
	    {Out tag(Page)}
	    {Out close}
	    {self updateSubHtml(DB)}
	 finally
	    {Manager decTrace('<-- updateHtml '#@id)}
	 end
      end
      meth updateSubHtml(_) skip end
   end
end
