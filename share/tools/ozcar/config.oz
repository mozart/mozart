%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

%% Until the emacs interface has been fixed, you'll find
%% lines with "funny" comments like here:
%% StatusInitText = 'No current thread' /* end */


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% some helpers
S2A = String.toAtom  %% string to atom
fun {VS2A X}         %% virtual string to atom
   {S2A {VirtualString.toString X}}
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Text
%%
Version                = 'May 10 1997'
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

InvalidThreadID        = 'Invalid Thread ID in step message' /* end */
NoFileInfo             = 'step message without line number information, ' #
                         'continuing thread #' /* end */
NoFileBlockInfo        = ' blocks without line number information'
EarlyThreadDeath       = 'won\'t add thread #' /* end */
EarlyTermThread        = 'Early terminating thread' /* end */
KnownThread            = 'Got known thread' /* end */
NewThread              = 'Got new thread' /* end */
NextOnLeave            = '\'next\' while leaving procedure - ' #
                         'substituting by \'step\''

UnknownSuspThread      = 'Unknown suspending thread' /* end */
UnknownWokenThread     = 'Unknown woken thread' /* end */
UnknownMessage         = 'Unknown message on stream'

WaitForThread          = 'waiting for thread to be added ' # /* end */
                         'to dictionary of debugged threads...' /* end */

ErrorExcText           = 'Exception: '
UserExcText            = 'Exception: '
NoStackText            = ' / no stack available'

FirstSelectThread      = 'You must select a thread first!' /* end */
IgnoreNoFileStep       = 'Ignoring new thread as there\'s' # /* end */
                         ' no line information available. ' #
                         ' Hint: save your Emacs buffer!'

DoneMessage            = ' done'

SwitchMessage          = 'You have selected thread #' /* end */
RebuildMessage         = 'Re-calculating stack of thread #' /* end */
ForgetMessage          = 'Thread #'
ForgetMessage2         = ' is not traced anymore'
TerminateMessage       = 'Thread #'
TerminateMessage2      = ' has been terminated'

ID                     = fun {$ I} ' (id ' # I # ')' end
OzcarMessagePrefix     = 'Ozcar: '
OzcarErrorPrefix       = 'Ozcar ERROR: '

BraceLeft              = '{'  /* } */
BraceRight             = '}'

BracketLeft            = '['  /* ] */
BracketRight           = ']'

DotEnd                 = '.end'

StepButtonText         = ' step'
NextButtonText         = ' next'
ContButtonText         = ' cont'
ForgetButtonText       = ' forget'
TermButtonText         = ' term'

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Types, Names and Atoms
%%
ArrayType              = '<array>'
ThreadType             = '<thread>' /* end */
CellType               = '<cell>'
ClassType              = '<class>' /* end */
DictionaryType         = '<dict>'
FloatType              = '<float>'
ListType               = '<list>'
UnitType               = 'unit'
NameType               = '<name>'
LockType               = '<lock>' /* end */
ObjectType             = '<object>'
PortType               = '<port>'
ProcedureType          = '<proc>' /* end */
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
ToplevelGeometry       = '510x360+46+3'

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
TimeoutToBlock         = 720
TimeoutToUpdateEnv     = 430
TimeoutToUpdateScroll  = TimeoutToUpdateEnv
TimeoutToSwitch        = 260
TimeoutToStatus        = 240

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
ScrollbarApplColor
ScrollbarBlockedColor
ScrollbarStackColor
RunningThreadColor
BlockedThreadColor
DeadThreadColor
ZombieThreadColor
TrunkColor
RunningThreadText
BlockedThreadText
DeadThreadText
ProcColor
BuiltinColor

case Tk.isColor then
   %% main window
   DefaultBackground       = '#f0f0f0'
   DefaultForeground       = black
   SelectedBackground      = ScrollbarStackColor
   SelectedForeground      = white
   
   %% source window
   ScrollbarApplColor      = '#00a000'
   ScrollbarBlockedColor   = BlockedThreadColor
   ScrollbarStackColor     = '#7070c0'

   %% thread forest window
   RunningThreadColor      = ScrollbarApplColor
   BlockedThreadColor      = '#e07070'
   DeadThreadColor         = '#505050'
   ZombieThreadColor       = '#f000f0'
   TrunkColor              = black % '#a00000'

   RunningThreadText       = nil
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

   %% source window
   ScrollbarApplColor      = '#010000'
   ScrollbarBlockedColor   = '#000100'
   ScrollbarStackColor     = '#000001'

   %% thread forest window
   RunningThreadColor      = black
   BlockedThreadColor      = black
   DeadThreadColor         = black
   ZombieThreadColor       = black
   TrunkColor              = black

   RunningThreadText       = nil
   BlockedThreadText       = '(b)'
   DeadThreadText          = '(t)'
   
   %% application trace window
   ProcColor               = black
   BuiltinColor            = black
end

%% this is for the Emacs scrollbar function
ColorMeaning            = cm(ScrollbarApplColor    : 'runnable'
			     ScrollbarBlockedColor : 'blocked'
			     ScrollbarStackColor   : 'stack')
			     
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% the config object to read/write changeable options
%% first, some initial values... (read from a config file someday?)

ConfigVerbose              = false  %% debug messages in Emulator buffer?

ConfigStepSystemProcedures = false  %% step on all system procedures (`...`)?
ConfigStepRecordBuiltin    = false  %% step on builtin 'record'  ?
ConfigStepDotBuiltin       = false  %% step on builtin '.'       ?
ConfigStepWidthBuiltin     = false  %% step on builtin 'width'   ?
ConfigStepNewNameBuiltin   = false  %% step on builtin 'NewName' ?
ConfigStepSetSelfBuiltin   = false  %% step on builtin 'setSelf' ?

ConfigEnvSystemVariables   = true   %% filter system variables in Env Window?
ConfigEnvProcedures        = false  %% filter procedures in Env Window?

Config =
{New
 class
    
    attr
       verbose :               ConfigVerbose
    
       stepSystemProcedures :  ConfigStepSystemProcedures
       stepRecordBuiltin :     ConfigStepRecordBuiltin
       stepDotBuiltin :        ConfigStepDotBuiltin
       stepWidthBuiltin :      ConfigStepWidthBuiltin
       stepNewNameBuiltin :    ConfigStepNewNameBuiltin
       stepSetSelfBuiltin :    ConfigStepSetSelfBuiltin
    
       envSystemVariables :    ConfigEnvSystemVariables
       envProcedures :         ConfigEnvProcedures


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
