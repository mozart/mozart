%%%
%%% Written by Nils Franzen (nilsf@sics.se), 980120
%%% Creates a small pop-up window given a list describing the menu
%%%
%%% Create pop-up menu:
%%%  {Popup Description ParentWindow}
%%%
%%% Format of Description: [X1 X2 X3 ... Xn]
%%% where each entry is one of following:
%%% * separator
%%% * "Entry"#proc{$} ... end
%%% * "Subentry"#[Y1 ... Yn]
%%% * ignore
%%%
%%% ParentWindow speaks for itself
%declare

functor
import
   Tk
   Browser(browse)
export
   popup:Popup
define
   proc{Popup List T}
      proc{MakeMenu Parent List}
         case List of nil then
            skip
         elseof A#B | Xs then
            if {IsList B} then
               M = {New Tk.menu tkInit(parent:Parent tearoff:false)}
            in
              {New Tk.menuentry.cascade tkInit(parent:Parent label:A menu:M) _}
               {MakeMenu M B}
               {MakeMenu Parent Xs}
            else
               {New Tk.menuentry.command
                tkInit(parent:Parent label:A action:B) _}
               {MakeMenu Parent Xs}
            end
         elseof separator | Xs then
            {New Tk.menuentry.separator tkInit(parent:Parent) _}
            {MakeMenu Parent Xs}
         elseof ignore | Xs then
            {MakeMenu Parent Xs}
         else
            {Browser.browse popupError(List)}
         end
      end

      MouseX = {Tk.returnInt winfo(pointerx T)}
      MouseY = {Tk.returnInt winfo(pointery T)}
      MainMenu = {New Tk.menu tkInit(parent:T tearoff:false)}
   in
      {MakeMenu MainMenu {Append List [separator "Cancel"#proc{$} skip end]}}
      {MainMenu tk(post MouseX MouseY)}
   end
end

   /* Example

   {Popup ["Subentry"#["Hi"#proc{$} {Show hi} end]
           separator
           "Entry"#proc{$} skip end]
    {New Tk.toplevel tkInit(title:"popup menu")}}

   */
