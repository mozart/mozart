%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% some helpers
S2A = String.toAtom  %% string to atom
fun {VS2A X}         %% virtual string to atom
   {S2A {VirtualString.toString X}}
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Text
%%
Version                = 'Jun 22 1997'
TitleName              = 'Oz Debugger Interface'
IconName               = 'Ozcar'

NameOfBenni            = 'Benjamin Lorenz'
EmailOfBenni           = 'lorenz@ps.uni-sb.de'

InitStatus             = TitleName # ' initialized'

IgnoreFeeds            = 'Ignore Queries'
IgnoreThreads          = 'Ignore Subthreads'

TreeTitle              = 'Thread Tree'
StackTitle             = 'Stack'
AltStackTitle          = 'Stack of Thread  #'

LocalEnvTitle          = 'Local Variables'
GlobalEnvTitle         = 'Global Variables'

ApplPrefixText         = 'Application:'
ApplFilePrefixText     = 'File:'

InvalidThreadID        = 'Invalid Thread ID in step message'
NoFileInfo             = 'step message without line number information, ' #
                         'continuing thread #'
NoFileBlockInfo        = ' blocks without line number information'
EarlyThreadDeath       = 'won\'t add thread #'
EarlyTermThread        = 'Early terminating thread'
KnownThread            = 'Got known thread'
NewThread              = 'Got new thread'
NextOnLeave            = '\'next\' while leaving procedure - ' #
                         'substituting by \'step\''

UnknownSuspThread      = 'Unknown suspending thread'
UnknownWokenThread     = 'Unknown woken thread'
UnknownMessage         = 'Unknown message on stream'

WaitForThread          = 'waiting for thread to be added ' #
                         'to dictionary of debugged threads...'

ErrorExcText           = 'Exception: '
UserExcText            = 'Exception: '
NoStackText            = ' / no stack available'

FirstSelectThread      = 'You must select a thread first!'
IgnoreNoFileStep       = 'Ignoring new thread as there\'s' #
                         ' no line information available. ' #
                         ' Hint: save your Emacs buffer!'

DoneMessage            = ' done'

SwitchMessage          = 'You have selected thread #'
RebuildMessage         = 'Re-calculating stack of thread #'
ForgetMessage          = 'Thread #'
ForgetMessage2         = ' is not traced anymore'
TerminateMessage       = 'Thread #'
TerminateMessage2      = ' has been terminated'

ID                     = fun {$ I} ' (id ' # I # ')' end
OzcarMessagePrefix     = fun {$}
			    'Ozcar[' # {Thread.id {Thread.this}} # ']: '
			 end
OzcarErrorPrefix       = 'Ozcar ERROR: '

BraceLeft              = '{'
BraceRight             = '}'

BracketLeft            = '['
BracketRight           = ']'

DotEnd                 = '.end'

StepButtonText         = ' step'
NextButtonText         = ' next'
ContButtonText         = ' cont'
ForgetButtonText       = ' forget'
TermButtonText         = ' term'

StackAction            = {NewName}
ResetAction            = {NewName}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Types, Names and Atoms
%%
ArrayType              = '<array>'
ThreadType             = '<thread>'
CellType               = '<cell>'
ClassType              = '<class>'
DictionaryType         = '<dict>'
FloatType              = '<float>'
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
UnAllocatedType        = 'unalloc'

NilAtom                = "'nil'"
ConsAtom               = "'|'"
HashAtom               = "'#'"

TrueName               = 'true'
FalseName              = 'false'

NoAction               = {NewName}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Geometry
%%
%ToplevelGeometry       = '510x360+46+3'
ToplevelGeometry       = '510x360'

ThreadTreeWidth        = 120
ThreadTreeStretchX     = 11
ThreadTreeStretchY     = 14
ThreadTreeOffset       = 4

StackTextWidth         = 0
EnvTextWidth           = 24
EnvVarWidth            = 14

NoBorderSize           = 0
SmallBorderSize        = 1
BorderSize             = 2

PadXButton             = 5
PadYButton             = 3

ScrollbarWidth         = 10

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Fonts
%%
SmallFont              = '6x13'
SmallBoldFont          = '6x13bold'
DefaultFont            = '7x13'
BoldFont               = '7x13bold'
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
HomeDir                = {VS2A {OS.getEnv 'HOME'} # '/'}

OzUnixPath             = {OS.getEnv 'OZPATH'}
OzPath
local
   fun {PathList UnixPath} % UnixPath must be of type string
      H T P in
      {List.takeDropWhile UnixPath fun {$ C} C \= &: end H T}
      P = {VS2A H#'/'}
      case T == nil then P|nil
      else P|{PathList T.2}
      end
   end
in
   OzPath = {PathList OzUnixPath}
end

BitMapDir              = {System.get home} # '/lib/bitmaps/'
BitMap                 = '@' # BitMapDir # 'debugger.xbm'

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Miscellaneous
%%
TextCursor             = left_ptr

MaxStackSize           = 40
MaxStackBrowseSize     = 15

TimeoutToCalcTree      = 380 % ms
TimeoutToBlock         = 620
TimeoutToUpdateEnv     = 430
TimeoutToUpdateBar     = TimeoutToUpdateEnv
TimeoutToConfigBar     = 70
TimeoutToSwitch        = 340
TimeoutToStatus        = 210

HelpEvent              = '<3>'

PrintDepth             = 2  % for System.valueToVirtualString
PrintWidth             = 3

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Colors and colormodel related stuff
%%

DefaultBackground
DefaultForeground
SelectedBackground
SelectedForeground
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

case Tk.isColor then
   %% main window
   DefaultBackground       = '#f0f0f0'
   DefaultForeground       = black
   SelectedBackground      = '#7070c0'
   SelectedForeground      = white
   
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
else
   %% main window
   DefaultBackground       = white
   DefaultForeground       = black
   SelectedBackground      = black
   SelectedForeground      = white

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
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% the config object to read/write changeable options
%% first, some initial values... (read from a config file someday?)

ConfigVerbose              = false  %% debug messages in Emulator buffer?

ConfigStepSystemProcedures = false  %% step on all system procedures (`...`)?

ConfigStepRecordBuiltin         = false  %% step on builtin 'record'  ?
ConfigStepDotBuiltin            = false  %% step on builtin '.'       ?
ConfigStepWidthBuiltin          = false  %% step on builtin 'width'   ?
ConfigStepNewNameBuiltin        = false  %% step on builtin 'NewName' ?
ConfigStepSetSelfBuiltin        = false  %% step on builtin 'setSelf' ?
ConfigStepWaitForArbiterBuiltin = false  %% step on builtin 'waitForArbiter'?

ConfigEnvSystemVariables   = true   %% filter system variables in Env Window?
ConfigEnvProcedures        = false  %% filter procedures in Env Window?

Config =
{New
 class
    
    attr
       verbose :                   ConfigVerbose
    
       stepSystemProcedures :      ConfigStepSystemProcedures
       stepRecordBuiltin :         ConfigStepRecordBuiltin
       stepDotBuiltin :            ConfigStepDotBuiltin
       stepWidthBuiltin :          ConfigStepWidthBuiltin
       stepNewNameBuiltin :        ConfigStepNewNameBuiltin
       stepSetSelfBuiltin :        ConfigStepSetSelfBuiltin
       stepWaitForArbiterBuiltin : ConfigStepWaitForArbiterBuiltin
    
       envSystemVariables :        ConfigEnvSystemVariables
       envProcedures :             ConfigEnvProcedures


    meth init
       skip
    end
    
    meth toggle(What)
       What <- {Not @What}
    end

    meth get(What $)
       @What
    end
    
 end init}

fun {Cget What}
   {Config get(What $)}
end

%%
%%%%%%%%%%%%%%%%%%%%%%%% End of config.oz %%%%%%%%%%%%%%%%%%%%%%
