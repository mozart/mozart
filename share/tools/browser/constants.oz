%% Programming Systems Lab, University of Saarland,
%% Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5337
%% Author: Konstantin Popov & Co.
%%  (i.e. all people who make proposals, advices and other rats at all:))
%% Last modified: $Date$ by $Author$
%% Version: $Revision$

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%
%%%
%%%  Local constants;
%%%  This file should be included in a declaration part
%%% (i.e. local ... \insert 'constants.oz' ... in ... end)
%%%
%%%

%%%
%%%
%%%  *Real* constants
%%% (i.e. don't touch them at all - they must be different names);
%%%

%%
%% INIT value;
%%
InitValue       = {NewName}

%%
%% Types of Terms -
%% should be parametric, because Oz has the strong trend to be modified :))
%% You have to modify 'terms.oz', 'textWidget.oz' and 'TermsStore.oz' if
%% you change types;
%%

%%
%% group #0: root term (top-level term);
T_RootTerm      = {NewName}

%%
%% group #1: primitive values;
T_Atom          = {NewName}
T_Int           = {NewName}
T_Float         = {NewName}
%%
T_Name          = {NewName}
%%
T_Procedure     = {NewName}
T_Cell          = {NewName}
%%
T_PrimChunk     = {NewName}
T_PrimObject    = {NewName}
T_PrimClass     = {NewName}

%%
%% group #2: compound values;
T_List          = {NewName}
T_FCons         = {NewName}
%%
T_Tuple         = {NewName}
%%
T_HashTuple     = {NewName}
%%
T_Record        = {NewName}
%% these are derived from records;
T_CompChunk     = {NewName}
T_CompObject    = {NewName}
T_CompClass     = {NewName}
%% other chunks:
T_Dictionary    = {NewName}
T_Array         = {NewName}
%% first-class threads and spaces;
T_Thread        = {NewName}
T_Space         = {NewName}

%%
%% group #3: variables (but not OFSs);
%% special: if a variable is not yet constrained somehow,
%% we say that this is *something* of the type T_Variable;
%% Of course, such operation is not monotonic, but we need it in browser.
T_Variable      = {NewName}
%%  ... and FD variable;
T_FDVariable    = {NewName}
%%  ... and Meta variable;
T_MetaVariable  = {NewName}

%%
%% group #4: What for a bullsh$t ??!
T_Unknown       = {NewName}

%%
%% group #5: specials (not constraint-system dependent);
%% Reference;
T_Reference     = {NewName}
%% Unshown (sub)term (leaf);
T_Shrunken      = {NewName}

%%
%% There are actually the following parameters in browser's store
%% (these are the names of corresponding features in store)":
%%
StoreTWWidth         = {NewName}
StoreXSize           = {NewName}
StoreXMinSize        = {NewName}
StoreYSize           = {NewName}
StoreYMinSize        = {NewName}
StoreDepth           = {NewName}
StoreWidth           = {NewName}
StoreFillStyle       = {NewName}
StoreArityType       = {NewName}
StoreWidthInc        = {NewName}
StoreDepthInc        = {NewName}
StoreAreSeparators   = {NewName}
StoreRepMode         = {NewName}
StoreSmallNames      = {NewName}
StoreAreStrings      = {NewName}
StoreAreVSs          = {NewName}
StoreTWFont          = {NewName}
StoreBufferSize      = {NewName}
StoreWithMenus       = {NewName}
%%
StoreIsWindow        = {NewName}
StoreAreMenus        = {NewName}
%%
StoreBrowserObj      = {NewName}
StoreStreamObj       = {NewName}
StoreOrigWindow      = {NewName}
StoreScreen          = {NewName}
StoreProcessAction   = {NewName}
%%
StoreBreak           = {NewName}
%%
StoreSeqNum          = {NewName}

%%
%% Types of representation (tree, graph, minimal graph);
TreeRep              = {NewName}
GraphRep             = {NewName}
MinGraphRep          = {NewName}
%% Types of (record) filling:
Expanded             = {NewName}
Filled               = {NewName}
%% Types of arity listing:
AtomicArity          = {NewName}
TrueArity            = {NewName}

%%%
%%%
%%%  (configurable) parameters (i.e. touch them at your own risk :-));
%%%

%% big enough:))
DInfinite       = 1000000

%%
OzHome = {System.get home}

%%
%% window(graphic) parameters;
%%
%%

%% initial size of a 'browse' text widget (in cols/lines);
ITWWidth    = 60
ITWHeight   = 20

%% initial size of a messages' text widget;
IMWidth     = 45
IMHeight    = 5

%%
ITitle      = 'Oz Browser'
IMTitle     = ITitle#': Messages'
IATitle     = ITitle#': About'
IBOTitle    = ITitle#': Buffer Options'
IROTitle    = ITitle#': Representation Options'
IDOTitle    = ITitle#': Display Options'
ILOTitle    = ITitle#': Layout Options'

%%
IITitle     = ITitle
IMITitle    = 'Messages'

%%
IIBitmap    = !OzHome # '/lib/bitmaps/browserIcon.xbm'
IMIBitmap   = !OzHome # '/lib/bitmaps/browserMIcon.xbm'
%%
%IStopBitmap = '@' # !OzHome # '/lib/bitmaps/stop.xbm'
IEjectBitmap= '@' # !OzHome # '/lib/bitmaps/eject.xbm'
IPlayBitmap = '@' # !OzHome # '/lib/bitmaps/play.xbm'
IPauseBitmap= '@' # !OzHome # '/lib/bitmaps/pause.xbm'
%%
%IStopWidth  = 30
IEjectWidth  = 20
IEFrameWidth = 20
IPlayWidth   = 20
IPauseWidth  = 20
%%
%% offsets for a transient helper;
IXTransDist  = 10
IYTransDist  = 5

%% curosr name (see include file X11/cursorfont.h);
ICursorName    = 'left_ptr'
ICursorClock   = 'watch'

%%
IAboutColor = blue
IEntryColor  = wheat

%%
%% The following two are in pixels (no subwindows are gridded);
%% hese sizes are used if there is(are) no buttons and/or menus frame(s);
IXMinSize   = 200
IYMinSize   = 150

%%
IXSize      = 500
IYSize      = 350

%%  ... for messages' window (but it is gridded, so - in chars);
IMXMinSize  = 40
IMYMinSize  = 5

%% colours on text widget;
IBackGround   = 'white'
IForeGround   = 'black'

%% borders and scrollbars' sizes are in pixels;
IBigBorder    = 3
ISmallBorder  = 2

%% ext widgets are raised,
%% while menus, buttons and scrollbars frames are sunken;
ITextRelief   = raised
IFrameRelief  = sunken
IButtonRelief = raised

%% width (or height) of scrollbars (pixels);
ISWidth       = 13

%%
%% default font for text widgets, 'x' and 'y' resolutions for it;
ITWFontUnknown = font(size:14 wght:medium
                      font:'-*-*-*-*-*-*-*-*-*-*-*-*-*-1'
                      xRes:0 yRes:0)   % these are just some values!
%%
ITWFont1       = font(size:14 wght:medium
                      font:'-*-courier-medium-r-*-*-14-*-*-*-*-*-*-*'
                      xRes:0 yRes:0)
ITWFont2       = font(size:14 wght:medium
                      font:'-*-*-*-r-*-*-14-*-*-*-*-*-*-*'
                      xRes:0 yRes:0)
ITWFont3       = font(size:14 wght:medium
                      font:'-*-*-*-*-*-*-*-*-*-*-*-*-*-1'
                      xRes:0 yRes:0)   % these are just some values!

%%
%% Note: actually, precise dimentions of the courier fonts are not
%% known (and even more, they depend on many other factors!). So, we
%% just left them unspecified (i.e. == 0).
%%
IKnownCourFonts = [font(size:24 wght:medium
                        font:'-*-courier-medium-r-*-*-24-*-*-*-*-*-*-*'
                        xRes:0 yRes:0)
                   font(size:24 wght:bold
                        font:'-*-courier-bold-r-*-*-24-*-*-*-*-*-*-*'
                        xRes:0 yRes:0)
                   font(size:18 wght:medium
                        font:'-*-courier-medium-r-*-*-18-*-*-*-*-*-*-*'
                        xRes:0 yRes:0)
                   font(size:18 wght:bold
                        font:'-*-courier-bold-r-*-*-18-*-*-*-*-*-*-*'
                        xRes:0 yRes:0)
                   font(size:14 wght:medium
                        font:'-*-courier-medium-r-*-*-14-*-*-*-*-*-*-*'
                        xRes:0 yRes:0)
                   font(size:14 wght:bold
                        font:'-*-courier-bold-r-*-*-14-*-*-*-*-*-*-*'
                        xRes:0 yRes:0)
                   font(size:12 wght:medium
                        font:'-*-courier-medium-r-*-*-12-*-*-*-*-*-*-*'
                        xRes:0 yRes:0)
                   font(size:12 wght:bold
                        font:'-*-courier-bold-r-*-*-12-*-*-*-*-*-*-*'
                        xRes:0 yRes:0)
                   font(size:10 wght:medium
                        font:'-*-courier-medium-r-*-*-10-*-*-*-*-*-*-*'
                        xRes:0 yRes:0)
                   font(size:10 wght:bold
                        font:'-*-courier-bold-r-*-*-10-*-*-*-*-*-*-*'
                        xRes:0 yRes:0)]

%%
%% (external) pads (pixels);
IPad          = 1
IButtonPad    = 1
ITWPad        = 3
IBigPad       = 4
ISEntryWidth  = 8

%% width of a button (in cm);
IButtonWidth  = 10

%%
IBFont1       = '-*-lucida-bold-r-*-sans-12-*-*-*-*-*-*-*'
IBFont2       = '-*-courier-bold-r-*-*-12-*-*-*-*-*-*-1'
IBFont3       = '-*-*-*-r-*-*-12-*-*-*-*-*-*-1'

%%
IAFont1       = '-*-times-bold-r-*-*-24-*-*-*-*-*-*-*'
IAFont2       = '-*-*-bold-r-*-*-24-*-*-*-*-*-*-1'
IAFont3       = '-*-*-*-r-*-*-24-*-*-*-*-*-*-1'

%%  "any" font: if the specified one is not found;
IReservedFont = '-*-*-*-*-*-*-*-*-*-*-*-*-*-1'

%%
%% offset (<x,y> from the upper left conner) for a message window;
IMWXOffset    = 50
IMWYOffset    = 50

%%%
%%%
%%%  defaults section (for "store") (i.e. it should be changeable);
%%%

%%
IDepth            = 15
IWidth            = 50
IFillStyle        = !Expanded
IArityType        = !AtomicArity
ISmallNames       = true
IAreStrings       = false
IAreVSs           = false
IDepthInc         = 1
IWidthInc         = 1
ISeparators       = true
IRepMode          = !TreeRep
%% ... only one of the previous two should be toggled one;
IBufferSize       = 15
%% would the menus frame come at start?
IWithMenus        = true

%%
%% template sizes;
IBSSmall          = 5
IBSMedium         = !IBufferSize
IBSLarge          = 100
IDSmall           = 5
IWSmall           = 15
IDMedium          = !IDepth
IWMedium            = !IWidth
IDLarge           = 50
IWLarge           = 1000
IDISmall          = !IDepthInc
IWISmall          = !IWidthInc
IDIMedium         = 3
IWIMedium         = 15
IDILarge          = 10
IWILarge          = 200

%%
%% options;
SpecialON         = 'special'
BrowserXSize      = 'xSize'
BrowserYSize      = 'ySize'
BrowserXMinSize   = 'xMinSize'
BrowserYMinSize   = 'yMinSize'
BufferON          = 'buffer'
BrowserBufferSize = 'size'
BrowserSeparators = 'separateBufferEntries'
RepresentationON  = 'representation'
BrowserRepMode    = 'mode'
BrowserChunkFields        = 'privateChunkFields'
BrowserNamesAndProcs      = 'detailedNamesAndProcedures'
BrowserVirtualStrings     = 'virtualStrings'
BrowserStrings    = 'strings'
DisplayON         = 'display'
BrowserDepth      = 'depth'
BrowserWidth      = 'width'
BrowserDepthInc   = 'depthInc'
BrowserWidthInc   = 'widthInc'
LayoutON          = 'layout'
BrowserFontSize   = 'size'
BrowserBold       = 'bold'
BrowserRecordFieldsAligned = 'alignRecordFields'

%%%
%%%
%%%  internal parameters
%%% (i.e. you probably have to know what they actually mean);
%%%

%%
%% How much elements in a term store list should be per one failure
%% during searching;
TermsStoreGCRatio  = 5
%%  ... The base for comparision, i.e. it looks like
%% case Fails * TermsStoreGCRatio > Size + TermsStoreGCBase
%% hen {DO_GC}
%% else skip
%% end
TermsStoreGCBase    = 100
%% How much a real size of a 'ResizableArray' can be bigger
%% han 'visible';
RArrayRRatio       = 10

%%%
%%%
%%%  Various definitions for debugging
%%% (turning them on will end up with many debug Show"s);
%%%

/*
\define    DEBUG_BO
%\undef     DEBUG_BO
\define    DEBUG_WM
%\undef     DEBUG_WM
\define    DEBUG_MO
%\undef     DEBUG_MO
\define    DEBUG_CO
%\undef     DEBUG_CO
\define    DEBUG_RM
%\undef     DEBUG_RM
\define    DEBUG_TO
%\undef     DEBUG_TO
\define    DEBUG_TI
%\undef     DEBUG_TI
*/

% /*
%\define    DEBUG_BO
\undef     DEBUG_BO
%\define    DEBUG_WM
\undef     DEBUG_WM
%\define    DEBUG_MO
\undef     DEBUG_MO
%\define    DEBUG_CO
\undef     DEBUG_CO
%\define    DEBUG_RM
\undef     DEBUG_RM
%\define    DEBUG_TO
\undef     DEBUG_TO
%\define    DEBUG_TI
\undef     DEBUG_TI
% */

%%%
%%%
%%%  Work-Arounds;
%%%

%% none at the moment :-)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%
%%%
%%%      Don't change anything below this line!
%%%
%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%
%%%  ... it means that otherwise something will definitely go wrong !!!
%%%

%% spacing between atoms, etc. in output:
%% size of blank, hash, colon, braces and '=';
DSpace          = 1
DDSpace         = 2
DTSpace         = 3
DQSpace         = 4

%% spacing to subterms of a {record, tuple};
DOffset         = 3

%% max approx;
DReference      = 2
%% 'R?'

%%
%% Note that glues, etc. below *must be atoms*. Otherwise, the
%% CoreTerms.delimiterLEQ must be updated;
%%
%% glues; should be of the same length (DSpace);
DSpaceGlue      = ' '           % DSpace;
DHashGlue       = "#"           % note: it must be a string!
DVBarGlue       = '|'

%% symbols;
DLRBraceS       = '('
DRRBraceS       = ')'
DLSBraceS       = '['
DRSBraceS       = ']'
DLABraceS       = '<'
DRABraceS       = '>'
DEqualS         = '='
DColonS         = ':'
DLCBraceS       = '{'
DRCBraceS       = '}'

%%
DNameUnshown    = ',,,'         % DTSpace;
DOpenFS         = '...'         % DTSpace;
DDBar           = '||'          % DDSpace;
DUnderscore     = '_'           % DSpace;
DUnshownPFs     = '?'           % DSpace

%% char values;
CNameDelimiter  = &:
BQuote          = &`
CharDot         = &.
CharSpace       = &
/* ` oz emacs modus problems; */
%%

%%
DRootGroup      = 0#1

%%
%% In general, there are five block in each of contemporary compound
%% term's representation:
%%
%% The leading group number;
DLeadingBlock   = 0
DMainBlock      = 1
%% the following block can contain a ',,,' group;
DCommasBlock    = 2
%% ... and this one - some ellipses, currently, there are just
%% ellipses for records and a '|| _' sequence for lists;
DSpecialBlock   = 3
DTailBlock      = 4

%%
%% in a leading block, there can be the following groups:
DLabelGroup     = 1
DLRBraceGroup   = 2
%% or, for lists:
DLSBraceGroup   = 1

%% within a commas block, there can be an ellipses group:
DCommasGroup    = 1
%% within a special block, there can be following groups:
DEllipsesGroup  = 1
%% or,
DDBarGroup      = 1
DLTGroup        = 2

%%
%% tail block can either absent, of contain a brace (round or square):
DBraceGroup     = 1

class BatchObject from BaseObject
   meth '|'(M Mr)
      {self M} BatchObject,Mr
   end
   meth nil
      skip
   end
end
