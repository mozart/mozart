functor
import
   Admin(manager:Manager)
   Text(
      htmlQuote:HtmlQuote
      makeBS
      capitalize:Capitalize
      join:Join
      )
   Open(file html)
   Directory(mkDirForFile)
   HTML_Navigation(getNavigationBar)
   Regex(compile search group) at 'x-oz://contrib/regex'
export
   'class' : HTML_Entry
define
   class HTML_File from Open.file Open.html end
   class HTML_Entry
      attr NavBar:_
      meth headTitle($) {HtmlQuote @id} end
      meth bodyTitle($) {HtmlQuote {self getPackageTitle($)}} end
      %%
      meth getPackageTitle($)
	 {self getPackageName($)}
      end
      %%
      meth getPackageName($)
	 {VirtualString.toString
	  {Join
	   {Map {String.tokens
		 {Reverse
		  {List.takeWhile
		   {Reverse {VirtualString.toString @id}}
		   fun {$ C} C\=&/ end}}
		 &-}
	    Capitalize}
	   " "}}
      end
      %%
      meth getNavigationBar($)
	 {HTML_Navigation.getNavigationBar}
      end
      %%
      meth toPage($)
	 CSS           = {Manager getCssLink($)}
	 HeadTitle     = {self headTitle($)}
	 BodyTitle     = {self bodyTitle($)}
	 FormatHeaders = {self formatHeaders($)}
	 FormatBody    = {self formatBody($)}
	 NavigationBar = {self getNavigationBar($)}
      in
	 html(head(
		 title(HeadTitle)
		 CSS)
	      body(
		 NavigationBar
		 h1('class':'title' BodyTitle)
		 'div'(
		    'class':'entryinfo'
		    FormatHeaders
		    FormatBody
		    )))
      end
      %%
      meth formatHeader(H V $ 'class':C<='header')
	 '#'(tr('class':C
		'valign':'top'
		td('class':C {HtmlQuote H})
		td('class':C ":")
		td('class':'value' V))
	    )
      end
      meth formatHeaderEnum(H L $ 'class':C<='header')
	 '#'(tr('class':C
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
	    )
      end
      %%
      meth formatBody($)
	 if @body==unit then ''
	 elseif @content_type=='text/html' then
	    'div'(
	       'class' : 'formatbody'
	       {Text.makeBS {JustGetBody @body}})
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
	    FileName = {Manager id_to_html_filename(@id $)}
	    {Directory.mkDirForFile FileName}
	    Out  = {New HTML_File
		    init(name:FileName flags:[write truncate create])}
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
   %%
   %% some bozos actually include a full html document in the body
   %%
   RE_BODY = {Regex.compile '<body[^>]*>(.*)</body[[:space:]]*>' [extended icase]}
   fun {JustGetBody S}
      case {Regex.search RE_BODY S} of false then S
      [] M then {Regex.group 1 M S} end
   end
end
