%%%
%%% Authors:
%%%   Martin Mueller (mmueller@ps.uni-sb.de)
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Martin Mueller, 1998
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

html(title:   'Lifts'
     name:    lift
     authors: [mmueller schulte]

     p('This demo performs a simple simulation of lifts. It can be operated '
       'both in an '
       a(href:'#interactive' interactive)
       ' mode or fully '
       a(href:'#automatic' automatically) '.')

     h2(a(name:'interactive' 'Interactive Mode'))

     dl(dt('Request')

	dd('Clicking one of the triangles on the left '
	   'issues a request for a lift to the respective floor '
	   'for service in the indicated direction. The lift that '
	   'can serve the request quickest (according to its current '
	   'schedule) '
	   'will be sent to the requesting floor.')
	
	dt('Send')
	
	dd('On arrival of a lift, a console will pop up requesting the user '
	   'to indicate the goal. '
	   'While the console is visible, the corresponding lift is not '
	   'available for other requests. '
	   'Once the user clicks one of the circles, '
	   'the lift will add the corresponding request into its schedule and '
	   'serve it.'))

     h2(a(name:'automatic' 'Automatic Mode'))
							   
     dl(dt('Start')
	
	dd('Clicking the button marked <i>start</i> generates '
	   'a random sequence of lift requests and serves them just as the '
	   'requests generated interactively.')
	
	dt('Stop')
	
	dd('Clicking the button again, now marked <i>stop</i>, '
	   'stops the automatic generation of lift requests.'))

     p('Interactive lift requests can be '
       'issued in addition.'))


