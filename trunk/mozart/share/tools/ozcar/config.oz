%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% some helpers to be used already in this file
%% (prelude.oz not yet inserted here)

fun {VS2A X}         %% virtual string to atom
   {String.toAtom {VirtualString.toString X}}
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Text
%%

Version                = \insert version.oz
TitleName              = 'Oz Debugger'
IconName               = 'Ozcar'

Platform               = local
			    X#Y = {System.get platform}
			 in
			    {VS2A X#'-'#Y}
			 end
WindowsPlatform        = 'win32-i486'

NameOfBenni            = 'Benjamin Lorenz'
EmailOfBenni           = 'lorenz@ps.uni-sb.de'

IgnoreQueries          = 'Ignore Queries'
IgnoreSubThreads       = 'Ignore Subthreads'

TreeTitle              = 'Thread Tree'
StackTitle             = 'Stack'
AltStackTitle          = 'Stack of Thread  #'

LocalEnvTitle          = 'Local Variables'
GlobalEnvTitle         = 'Global Variables'


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% error, warning & debug messages
%%

ID                     = fun {$ I} ' (id ' # I # ')' end
OzcarMessagePrefix     = fun {$}
			    'Ozcar[' # {Thread.id {Thread.this}} # ']: '
			 end
OzcarErrorPrefix       = 'Ozcar ERROR: '

InvalidThreadID        = 'Invalid Thread ID in step message'
NoFileInfo             = ('step message without line number information, ' #
			  'continuing thread #')
NoFileBlockInfo        = ' blocks without line number information'
EarlyThreadDeath       = 'won\'t add short living thread #'
EarlyTermThread        = 'Early terminating thread'
KnownThread            = 'Got known thread'
NewThread              = 'Got new thread'
WaitForThread          = ('waiting for thread to be added ' #
			  'to dictionary of debugged threads...')

FirstSelectThread      = 'You must select a thread first!'

StackAction            = {NewName}
RemoveAllAction        = {NewName}
RemoveAllDeadAction    = {NewName}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Types, Names and Atoms
%%

ArrayType              = '<array>'
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
ChunkType              = '<chunk>'
UnknownType            = '<???>'
UnboundType            = '_'


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
EnvVarWidth            = 14

PadXButton             = 5
PadYButton             = 3

ScrollbarWidth         = 10

CheckButtonWidth       = 60
CheckButtonHeight      = 18


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

GlobalBitMapDir        = '@' # {System.get home} # '/lib/bitmaps/'
LocalBitMapDir         = GlobalBitMapDir # 'ozcar/'
BitmapExtension        = '.xbm'
IconBitMap             = GlobalBitMapDir # debugger # BitmapExtension

StepButtonBitmap       = step
NextButtonBitmap       = next
UnleashButtonBitmap    = unleash
StopButtonBitmap       = stop
ForgetButtonBitmap     = forget
TermButtonBitmap       = term

IgnoreQueriesBitmap    = {VS2A queries  # BitmapExtension}
IgnoreSubThreadsBitmap = {VS2A children # BitmapExtension}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Miscellaneous
%%

TextCursor             = left_ptr

%% the timeout variables have critical values --
%% you should know what you are doing when changing them...
TimeoutToCalcTree      = 380 % ms
TimeoutToBlock         = 620
TimeoutToUpdateEnv     = 430
TimeoutToSwitch        = 340
TimeoutToStatus        = 210

HelpEvent              = '<3>'

BigFloat               = {Int.toFloat 10 * 1000}
BigInt                 = 1000 * 1000 * 1000


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
ForgetButtonColor
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
   ForgetButtonColor       = DefaultForeground
   TermButtonColor         = DefaultForeground

   %% thread forest window
   RunnableThreadColor     = '#00a500'
   RunningThreadColor      = '#f0c000'

   BlockedThreadColor      = '#e07070'
   DeadThreadColor         = '#b0b0b0'

   ZombieThreadColor       = '#f000f0'
   TrunkColor              = black % '#a00000'

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
   ForgetButtonColor       = black
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

ConfigEnvPrintTypes        = true   %% use builtin printer (instead of
				    %%   System.valueToVirtualString

ConfigEmacsThreads         = true   %% default value of Emulator
ConfigSubThreads           = true   %% dito

ConfigUpdateEnv            = true   %% update env windows after each step?
ConfigUseEmacsBar          = {Not {System.get standalone}} % use Emacs?

Config =
{New
 class

    attr
       verbose :               ConfigVerbose

       stepDotBuiltin :        ConfigStepDotBuiltin
       stepNewNameBuiltin :    ConfigStepNewNameBuiltin

       envSystemVariables :    ConfigEnvSystemVariables
       envPrintTypes :         ConfigEnvPrintTypes

       emacsThreads :          {New Tk.variable tkInit(ConfigEmacsThreads)}
       subThreads :            {New Tk.variable tkInit(ConfigSubThreads)}

       updateEnv :             ConfigUpdateEnv
       useEmacsBar :           ConfigUseEmacsBar

    meth init
       skip
    end

    meth toggle(What)
       What <- {Not @What}
       {OzcarMessage 'Config: setting `' # What #
	'\' to value `' # {V2VS @What} # '\''}
       case What == verbose then
	  {Emacs setVerbose(@What)}
       elsecase What == envSystemVariables then
	  {Ozcar PrivateSend(rebuildCurrentStack)}
       elsecase What == updateEnv andthen @What == true then
	  {Ozcar PrivateSend(What)}
       else skip end
    end

    meth set(What Value)
       {OzcarMessage 'Config: setting `' # What #
	'\' to value `' # {V2VS Value} # '\''}
       What <- Value
       case What == envPrintTypes then
	  {Ozcar PrivateSend(rebuildCurrentStack)}
       else skip end
    end

    meth get(What $)
       @What
    end

    meth getTk(What $)
       Value = {@What tkReturnInt($)}
    in
       case Value == 0 then false else true end
    end

 end init}

proc {Ctoggle What}
   {Config toggle(What)}
end

fun {Cget What}
   {Config get(What $)}
end

fun {CgetTk What}
   {Config getTk(What $)}
end

%%
%%%%%%%%%%%%%%%%%%%%%%%% End of config.oz %%%%%%%%%%%%%%%%%%%%%%
