makefile(
   uri     : 'x-oz://system/wp'
   depends : o(
		'HTMLmarshaller.ozf'
		: ['charref.html']
		'Params.ozf'
		: ['CSS2.html']
		'QHTML.ozf'
		: ['QHTMLnl.ozf' 'HTMLmarshaller.ozf' 'QHTMLWebServer.ozf'
		   'LiteWebServer.ozf' 'SiteData.ozf' 'QHTMLDevel.ozf'
		   'QHTMLType.ozf' 'ConnectionServer.ozf' 'QUI.ozf' 'QHTMLToplevel.ozf'
		   'QHTMLTdLr.ozf' 'QHTMLButton.ozf' 'QHTMLCheckbox.ozf' 'QHTMLFile.ozf'
		   'QHTMLPassword.ozf' 'QHTMLRadio.ozf' 'QHTMLText.ozf' 'QHTMLLabel.ozf'
		   'QHTMLHtml.ozf' 'QHTMLHr.ozf' 'QHTMLListbox.ozf' 'QHTMLPlaceholder.ozf'
		   'QHTMLTextarea.ozf' 'QHTMLImg.ozf' 'QHTMLA.ozf' 'QHTMLFrame.ozf'
		   'QHTMLMenu.ozf' 'Params.ozf']
		'SiteData.ozf'
		: ['html/default.htm' 'html/OzLink.class' 'html/GJoz.class'])
   lib     : ['QHTML.ozf']
   rules   : o(
		'QHTML.ozf'   : ozl('QHTMLnl.ozf')
		'QHTMLnl.ozf' : ozc('QHTML.oz'))
   subdirs : [manual]
   )
