%%% -*-oz-*-
%%% Authors:
%%%   Denys Duchier (duchier@ps.uni-sb.de)
%%%
%%% Copyright:
%%%   Denys Duchier, 1998
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
%%%

%%% Here we initialize all system properties
%%%
%%% oz.home		OZHOME  OZ_HOME
%%% oz.search.path	OZPATH  OZ_PATH  OZ_SEARCH_PATH
%%% oz.search.load	OZLOAD  OZ_LOAD  OZ_SEARCH_LOAD
%%% os.name
%%% os.cpu (os.arch)
%%% path.separator	OZ_PATH_SEPARATOR
%%% path.escape		OZ_PATH_ESCAPE (to escape a path separator)
%%% user.home		HOME

PLATFORM_OS     = {GET 'platform.os'}

PATH_SEPARATOR	= case {Getenv 'OZ_PATH_SEPARATOR'} of [C] then C
		  elsecase PLATFORM_OS of win32 then &; else &: end

PATH_ESCAPE	= case {Getenv 'OZ_PATH_ESCAPE'} of [C] then C
		  elsecase PLATFORM_OS of win32 then unit else &\\ end

OZ_HOME_	= case {Getenv 'OZ_HOME'} of false then
		     case {Getenv 'OZHOME'} of false then
			{GET 'oz.configure.home'}
		     elseof V then V end
		  elseof V then V end
OZ_HOME		= case {Reverse {VirtualString.toString OZ_HOME_}}
		  of H|T then if H==&/ orelse H==&\\ then
				 {Reverse T}
			      else OZ_HOME_ end
		  else OZ_HOME_ end

OZ_SEARCH_PATH	= case {Getenv 'OZ_SEARCH_PATH'} of false then
		     case {Getenv 'OZ_PATH'} of false then
			case {Getenv 'OZPATH'} of false then
			   '.'#[PATH_SEPARATOR]#OZ_HOME#'/share'
			elseof V then V end
		     elseof V then V end
		  elseof V then V end

OZ_SEARCH_LOAD	= case {Getenv 'OZ_SEARCH_LOAD'} of false then
		     case {Getenv 'OZ_LOAD'} of false then
			case {Getenv 'OZLOAD'} of false then
			   'cache=~/.oz/cache'#[PATH_SEPARATOR]#
			   'cache='#OZ_HOME#'/cache'
			elseof V then V end
		     elseof V then V end
		  elseof V then V end

USER_HOME	= case {Getenv 'HOME'} of false then {OS.getCWD}
		  elseof V then V end

OZ_TRACE_LOAD	= case {Getenv 'OZ_TRACE_LOAD'} of false then false
		  else true end

{SET 'path.separator'	PATH_SEPARATOR	}
{SET 'path.escape'	PATH_ESCAPE	}
{SET 'oz.home'		OZ_HOME		}
{SET 'oz.search.path'	OZ_SEARCH_PATH	}
{SET 'oz.search.load'	OZ_SEARCH_LOAD	}
{SET 'user.home'	USER_HOME	}
{SET 'oz.trace.load'	OZ_TRACE_LOAD	}

