makefile(
   uri     : 'x-oz://system'
   lib     : ['DistributionPanel.ozf' 'DistributionPanelSrc.ozf']
   depends :
      o(
	 'DistributionPanelSrc.ozf'
	 : ['GUI.ozf' 'Main.ozf' 'MessageInfo.ozf' 'SiteInfo.ozf'
	    'Widgets.ozf' 'Colour.ozf' 'FieldDisplay.ozf' 'Graph.ozf'
	    'NetInfo.ozf' 'TableInfo.ozf' 'AdvancedListBox.ozf'
	    'TitleGraph.ozf' 'DistributionPanel.ozf']
	 )
   rules   :
      o( 'DistributionPanelSrc.ozf' : ozl('Main.ozf') )
   )