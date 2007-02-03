%%%
%%% Author:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Text
%%

TitleName              = 'Oz Debugger'
IconName               = 'Ozcar'

Platform               = {Property.get 'platform.os'}
WindowsPlatform        = 'win32'

NameOfBenni            = 'Benjamin Lorenz'
EmailOfBenni           = 'Please send bug reports to bugs@mozart-oz.org'

TreeTitle              = 'Thread Forest'
StackTitle             = 'Stack'
AltStackTitle          = 'Stack of Thread '
LocalEnvTitle          = 'Local Variables'
GlobalEnvTitle         = 'Global Variables'

StepInto               = 'step into'
StepOver               = 'step over'

IgnoreText             = 'Ignore'
AttachText             = 'Attach'
Unleash0Text           = 'Unleash 0'
Unleash1Text           = 'Unleash 1'

EmacsThreadsText       = 'Queries:'
EmacsThreadsList       = [IgnoreText # UnleashButtonColor
			  AttachText # StopButtonColor]
SubThreadsText         = 'SubThreads:'
SubThreadsList         = [IgnoreText   # UnleashButtonColor
			  AttachText   # StopButtonColor
			  Unleash0Text # NextButtonColor
			  Unleash1Text # NextButtonColor]


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% error, warning & debug messages
%%

OzcarMessagePrefix     = fun {$}
			    ThisThread = {Thread.this}
			 in
			    'Ozcar[' #
			    {Debug.getId       ThisThread} # '/' #
			    {Debug.getParentId ThisThread} # ']: '
			 end
OzcarErrorPrefix       = 'Ozcar ERROR: '
NoThreads              = 'There is no thread attached'


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Types, Names and Atoms
%%

ArrayType              = '<array>'
ThreadType             = '<thread>'
ClassType              = '<class>'
DictionaryType         = '<dict>'
BigFloatType           = '<bigfloat>'
BigIntType             = '<bigint>'
ListType               = '<list>'
UnitType               = 'unit'
NameType               = '<name>'
LockType               = '<lock>'
ObjectType             = '<object>'
PortType               = '<port>'
ProcedureType          = '<proc>'
TupleType              = '<tuple>'
RecordType             = '<record>'
KindedRecordType       = '<recordc>'
ChunkType              = '<chunk>'
SpaceType              = '<space>'
FSVarType              = '<fs var>'
FDVarType              = '<fd var>'
FutureType             = '<future>'
FailedType             = '<failed>'
UnboundType            = '_'
UnknownType            = '<???>'


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Geometry
%%

ToplevelGeometry       = '510x360'
ToplevelMinSize        = 459 # 324   %%  10% less than the default

ThreadTreeWidth        = 120
ThreadTreeStretchX     = 11
ThreadTreeStretchY     = 14
ThreadTreeOffset       = 4

StackTextWidth         = 0
EnvTextWidth           = 24
EnvVarWidth            = fun {$}
			    if {Cget envPrintTypes} then
			       if Platform == WindowsPlatform
			       then 12 else 14 end
			    else 6 end
			 end

ScrollbarWidth         = 10


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Fonts
%%

DefaultFont # BoldFont =
if Platform == WindowsPlatform then
   {New Tk.font tkInit(family:courier size:10)} #
   {New Tk.font tkInit(family:courier weight:bold size:10)}
else
   '7x13' # '7x13bold'
end

RunningWithXF86Display =
local
   Server = {Tk.return winfo(server '.')}
in
   {Member "XFree86" {String.tokens Server & }}
end

% Tk.font seems to behave differently on XFree86 displays... shit happens
ButtonFont # TitleFont # HelpTitleFont # HelpFont =
if RunningWithXF86Display then
   {New Tk.font tkInit(family:helvetica size:10)} #
   {New Tk.font tkInit(family:helvetica size:11 weight:bold)} #
   {New Tk.font tkInit(family:helvetica size:18 weight:bold)} #
   {New Tk.font tkInit(family:helvetica size:12)}
else
   {New Tk.font tkInit(family:helvetica size:8)} #
   {New Tk.font tkInit(family:helvetica size:8 weight:bold)} #
   {New Tk.font tkInit(family:helvetica size:14 weight:bold)} #
   {New Tk.font tkInit(family:helvetica size:10)}
end

ThreadTreeFont     = DefaultFont
ThreadTreeBoldFont = BoldFont
StatusFont         = TitleFont

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Files
%%

OzcarBitmap = fun {$ Bitmap}
		 '@' # {Tk.localize BitmapUrl # Bitmap # '.xbm'}
	      end

StepButtonBitmap       = step
NextButtonBitmap       = next
UnleashButtonBitmap    = unleash
StopButtonBitmap       = stop
DetachButtonBitmap     = detach
TermButtonBitmap       = term

AddQueriesBitmap       = {VirtualString.toAtom queries  # '.xbm'}
AddSubThreadsBitmap    = {VirtualString.toAtom children # '.xbm'}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Miscellaneous
%%

TextCursor             = left_ptr
HelpEvent              = '<3>'

TimeoutToUpdate        = 15         %% the TkSmoother will use this value

BigFloat               = {Int.toFloat 10 * 1000}
BigInt                 = 1000 * 1000 * 1000

DetachAllAction        = {NewName}
DetachAllButCurAction  = {NewName}
DetachAllDeadAction    = {NewName}

TermAllAction          = {NewName}
TermAllButCurAction    = {NewName}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Colors and colormodel related stuff
%%

DefaultBackground
DefaultForeground
SelectedBackground
SelectedForeground

ButtonForeground
CheckButtonSelectColor

StepButtonColor
NextButtonColor
UnleashButtonColor
StopButtonColor
DetachButtonColor
TermButtonColor

RunnableThreadColor
BlockedThreadColor
ExcThreadColor
DeadThreadColor
TrunkColor

ProcColor
BuiltinColor
DirtyColor

UseColors = Tk.isColor

if UseColors then
   %% main window
   DefaultBackground       = '#f0f0f0'
   DefaultForeground       = black
   SelectedBackground      = '#7070c0'
   SelectedForeground      = white

   ButtonForeground        = grey40
   CheckButtonSelectColor  = grey70

   StepButtonColor         = SelectedBackground
   NextButtonColor         = SelectedBackground
   UnleashButtonColor      = RunnableThreadColor
   StopButtonColor         = ExcThreadColor
   DetachButtonColor       = DefaultForeground
   TermButtonColor         = DefaultForeground

   %% thread forest window
   RunnableThreadColor     = '#00a500'
   BlockedThreadColor      = '#f0c000'
   ExcThreadColor          = '#e07070'
   DeadThreadColor         = '#b0b0b0'
   TrunkColor              = grey70

   %% application trace window
   ProcColor               = '#0000c0'
   BuiltinColor            = '#c00000'
   DirtyColor              = grey59
else
   %% main window
   DefaultBackground       = white
   DefaultForeground       = black
   SelectedBackground      = black
   SelectedForeground      = white

   ButtonForeground        = black
   CheckButtonSelectColor  = black

   StepButtonColor         = black
   NextButtonColor         = black
   UnleashButtonColor      = black
   StopButtonColor         = black
   DetachButtonColor       = black
   TermButtonColor         = black

   %% thread forest window
   RunnableThreadColor     = black
   BlockedThreadColor      = black
   ExcThreadColor          = black
   DeadThreadColor         = black
   TrunkColor              = black

   %% application trace window
   ProcColor               = black
   BuiltinColor            = black
   DirtyColor              = black
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% the config object to read/write changeable options
%% first, some initial values... (read from a config file someday?)

ConfigVerbose              = false  %% debug messages in Emulator buffer?
ConfigStepDotBuiltin       = false  %% step on builtin '.'?
ConfigStepNewNameBuiltin   = false  %% step on builtin 'NewName'?
ConfigEnvSystemVariables   = false  %% show system variables in Env Windows?
ConfigEnvPrintTypes        = true   %% use Ozcar's own type printer?
ConfigUpdateEnv            = true   %% update env windows after each step?

EmacsInterface             = {Emacs.getOPI}
ConfigUseEmacsBar          = true   %% use Emacs?

PrintWidth
PrintDepth
local
   P = {Property.get print}
in
   PrintWidth = P.width
   PrintDepth = P.depth
end

TimeoutToSwitch            = 100
TimeoutToUpdateEnv         = 200

Config =
{New
 class

    feat
       ConfAllowed: confAllowed(timeoutToSwitch:    true
				timeoutToUpdateEnv: true
				emacsInterface:     true
				closeAction:        true)

    attr
       verbose :               ConfigVerbose
       stepDotBuiltin :        ConfigStepDotBuiltin
       stepNewNameBuiltin :    ConfigStepNewNameBuiltin
       envSystemVariables :    ConfigEnvSystemVariables
       envPrintTypes :         ConfigEnvPrintTypes
       updateEnv :             ConfigUpdateEnv
       useEmacsBar :           ConfigUseEmacsBar
       printWidth:             PrintWidth
       printDepth:             PrintDepth
       timeoutToSwitch:        TimeoutToSwitch
       timeoutToUpdateEnv:     TimeoutToUpdateEnv
       emacsInterface:         EmacsInterface
       closeAction:            unit

    meth init
       skip
    end

    meth confAllowed(F $)
       {CondSelect self.ConfAllowed F false}
    end

    meth toggle(What)
       What <- {Not @What}
       if What == updateEnv andthen @What == true then
	  {Ozcar PrivateSend(What)}
       end
    end

    meth set(What Value)
       What <- Value
       if What == envPrintTypes then
	  {Ozcar PrivateSend(rebuildCurrentStack)}
       end
    end

    meth get(What $)
       @What
    end

 end init}

proc {Ctoggle What}
   {Config toggle(What)}
end

fun {Cget What}
   {Config get(What $)}
end

%%
%%%%%%%%%%%%%%%%%%%%%%%% End of config.oz %%%%%%%%%%%%%%%%%%%%%%
