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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Text
%%

Version                = \insert ozcar-version.oz
TitleName              = 'Oz Debugger'
IconName               = 'Ozcar'

Platform               = local
			    X#Y = {Property.get platform}
			 in
			    {VirtualString.toAtom X#'-'#Y}
			 end
WindowsPlatform        = 'win32-i486'

NameOfBenni            = 'Benjamin Lorenz'
EmailOfBenni           = 'lorenz@ps.uni-sb.de'

TreeTitle              = 'Thread Forest'
StackTitle             = 'Stack'
AltStackTitle          = 'Stack of Thread  #'
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
			    {Thread.id       ThisThread} # '/' #
			    {Thread.parentId ThisThread} # ']: '
			 end
OzcarErrorPrefix       = 'Ozcar ERROR: '
NoThreads              = 'There is no thread attached'


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Types, Names and Atoms
%%

ArrayType              = '<array>'
BitArrayType           = '<bitarray>'
ThreadType             = '<thread>'
CellType               = '<cell>'
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
FSValueType            = '<fs val>'
FSVarType              = '<fs var>'
FDVarType              = '<fd var>'
ForeignPointerType     = '<foreign>'
UnknownType            = '<???>'


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Geometry
%%

ToplevelGeometry       = '510x360'
ToplevelMinSize        = 459 # 324   %%  10% less than the default
ToplevelMaxSize        = 765 # 540   %%  50% more...

ThreadTreeWidth        = 120
ThreadTreeStretchX     = 11
ThreadTreeStretchY     = 14
ThreadTreeOffset       = 4

StackTextWidth         = 0
EnvTextWidth           = 24
EnvVarWidth            = fun {$}
			    case {Cget envPrintTypes} then 14 else 6 end
			 end

ScrollbarWidth         = 10


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Fonts
%%

SmallFont
SmallBoldFont
DefaultFont
BoldFont

case Platform == WindowsPlatform then
   SmallFont           = '-*-courier new-medium-r-*-*-12-*-*-*-*-*-*-*'
   SmallBoldFont       = '-*-courier new-bold-r-*-*-12-*-*-*-*-*-*-*'
   DefaultFont         = SmallFont
   BoldFont            = SmallBoldFont
else
   SmallFont           = '6x13'
   SmallBoldFont       = '6x13bold'
   DefaultFont         = '7x13'
   BoldFont            = '7x13bold'
end

ThreadTreeFont         = DefaultFont
ThreadTreeBoldFont     = BoldFont
ButtonFont             = '-adobe-helvetica-medium-r-normal-*-10-*-*-*-*-*-*-*'
TitleFont              = '-adobe-helvetica-bold-r-normal-*-10-*-*-*-*-*-*-*'
StatusFont             = TitleFont
HelpTitleFont          = '-adobe-helvetica-bold-r-*-*-18-*-*-*-*-*-*-*'
HelpFont               = '-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*'


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Files
%%

UrlDefaults = \insert '../../url-defaults.oz'
OzcarBitmap = fun {$ Bitmap}
		 '@' # {Tk.localize UrlDefaults.home #
			'images/ozcar/' # Bitmap # '.xbm'}
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
RunningThreadColor

BlockedThreadColor
DeadThreadColor

ZombieThreadColor
TrunkColor

RunnableThreadText
BlockedThreadText
DeadThreadText

ProcColor
BuiltinColor
DirtyColor

UseColors = {And Tk.isColor Platform \= WindowsPlatform}

case UseColors then
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
   StopButtonColor         = BlockedThreadColor
   DetachButtonColor       = DefaultForeground
   TermButtonColor         = DefaultForeground

   %% thread forest window
   RunnableThreadColor     = '#00a500'
   RunningThreadColor      = '#f0c000'

   BlockedThreadColor      = '#e07070'
   DeadThreadColor         = '#b0b0b0'

   ZombieThreadColor       = '#f000f0'
   TrunkColor              = grey70

   RunnableThreadText      = nil
   BlockedThreadText       = nil
   DeadThreadText          = nil

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
   RunningThreadColor      = black
   BlockedThreadColor      = black
   DeadThreadColor         = black
   ZombieThreadColor       = black
   TrunkColor              = black

   RunnableThreadText      = nil
   BlockedThreadText       = '(b)'
   DeadThreadText          = '(t)'

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

RunningWithOPI             = {Emacs.getOPI} \= false
ConfigUseEmacsBar          = RunningWithOPI  % use Emacs?

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
       ConfAllowed

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

    meth init
       D = {Dictionary.new}
    in
       {Dictionary.put D timeoutToSwitch unit}
       {Dictionary.put D timeoutToUpdateEnv unit}
       self.ConfAllowed = D
    end

    meth confAllowed(F $)
       {Dictionary.member self.ConfAllowed F}
    end

    meth toggle(What)
       What <- {Not @What}
       {OzcarMessage 'Config: setting `' # What #
	'\' to value `' # {V2VS @What} # '\''}
       case What == updateEnv andthen @What == true then
	  {Ozcar PrivateSend(What)}
       else skip end
    end

    meth set(What Value)
       What <- Value
       {OzcarMessage 'Config: setting `' # What #
	'\' to value `' # {V2VS Value} # '\''}
       case What == envPrintTypes then
	  {Ozcar PrivateSend(rebuildCurrentStack)}
       else skip end
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
