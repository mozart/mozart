functor

import
   QHTMLDevel
   ConnectionServer
   QHTMLWebServer
   QUI
   QHTMLToplevel
   QHTMLTdLr
   QHTMLButton
   QHTMLText
   QHTMLCheckbox
   QHTMLFile
   QHTMLPassword
   QHTMLRadio
   QHTMLLabel
   QHTMLHtml
   QHTMLHr
   QHTMLListbox
   QHTMLPlaceholder
   QHTMLTextarea
   QHTMLImg
   QHTMLA
   QHTMLFrame
   QHTMLMenu
   HTMLmarshaller

export
   Build
   NewLook
   Server
   StartServer
   StartServerOnPort
   StopServer
   StartWebServer
   StartWebServerOnPort
   StopWebServer
   StartAll
   StartAllHTMLEntryFilename
   SetAlias
   UnSetAlias
   Vs2Html
   Html2Vs
   Undefined
      
define
   Undefined=QHTMLDevel.undefined
   {Wait QHTMLToplevel} % complete
   {Wait QHTMLTdLr}     % td&lr complete, pure table support missing thead tfoot & tbody
   {Wait QHTMLButton}   % complete
   {Wait QHTMLText}     % missing text range support
   {Wait QHTMLCheckbox} % missing a label support
   {Wait QHTMLFile}     % missing upload functionality
   {Wait QHTMLPassword} % missing text range support
   {Wait QHTMLRadio}    % missing a label support
   {Wait QHTMLLabel}    % complete
   {Wait QHTMLHtml}     % complete
   {Wait QHTMLHr}       % complete
   {Wait QHTMLListbox}  % complete
   {Wait QHTMLPlaceholder} % complete
   {Wait QHTMLTextarea} % missing text range support
   {Wait QHTMLImg}      % may find a better way to add map support
   {Wait QHTMLA}        % complete with nice anchor support
   {Wait QHTMLFrame}    % tdframe & lrframe complete. frameset needs support for adding/removing frames dynamically. frame needs support for dynamic changes (ref problem in html ???) also support for iframe missing
   {Wait QHTMLMenu}     % complete with support for colors (only at toplevel declaration), item menus, radio menus, check menus and submenus. An optimization will be to not redraw all menus from the root. Missing popupmenu support.
   % vml
   % ? javaapplet & activex

   Vs2Html=HTMLmarshaller.vS2HTML
   Html2Vs=HTMLmarshaller.hTML2VS
   
   SetAlias=QUI.setAlias
   UnSetAlias=QUI.unSetAlias

   fun{Build Desc}
      {QUI.build Desc unit}
   end
   
   NewLook=QUI.newLook
   
   Server=ConnectionServer.server
   
   StartServer=ConnectionServer.startServer
   StartServerOnPort=ConnectionServer.startServerOnPort
   StopServer=ConnectionServer.stopServer
   
   StartWebServer=QHTMLWebServer.startServer
   StartWebServerOnPort=QHTMLWebServer.startServerOnPort
   StopWebServer=QHTMLWebServer.stopServer

   proc{StartAll ConnectProc SPort WPort}
      SPort={StartServer ConnectProc}
      WPort={StartWebServer SPort}
   end

   proc{StartAllHTMLEntryFilename ConnectProc FileName}
      WPort
   in
      {StartAll ConnectProc _ WPort}
      {QHTMLWebServer.writeRedirector WPort FileName}
   end
   
end
