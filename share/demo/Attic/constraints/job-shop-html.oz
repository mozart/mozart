%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
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

html(title:   'Job-Shop Scheduling'
     name:    'job-shop'
     authors: [schulte]

     menu('class':margin
          li(a(href:'#edit' 'Edit Jobs'))
          li(a(href:'#schedule' 'Schedule Jobs')))

     p('The scheduler allows to '
       a(href:'#edit' edit)
       ' job-shop scheduling problems and to compute '
       a(href:'#schedule' schedules)
       ' for the edited problem.')

     h2(a(name:'edit' 'Edit Jobs'))

     dl(dt('Resource')

        dd('Clicking a resource selects it and makes the '
           'resource tool active.  While the resource tool is '
           'active, the resource of a task can be changed by '
           'clicking the task.')

        dt('Duration')

        dd('Clicking a duration value selects this value '
           'and makes the duration tool active. While the '
           'duration tool is active, the duration of a task '
           'can be changed to the selected value by clicking '
           'the task.')

        dt('Create Task')

        dd('After clicking this tool, new tasks can be '
           'created. Newly created tasks are created as being '
           'the last task of a job.  The resource and duration '
           'of a newly created task is described by the '
           'setting of the Resource- and Duration-tool.')

        dt('Delete Task')

        dd('After clicking this tool, tasks can be '
           'deleted by clicking them.'))


     h2(a(name:'schedule' 'Schedule Jobs'))

     dl(dt('Reset')

        dd('Resets the scheduling engine.')

        dt('Next')

        dd('Searches for a next schedule and displays the '
           'schedule found and its maximal span.  Search can '
           'be interrupted by clicking '
           code('Stop') '.')

        dt('Best')

        dd('Searches for a best schedule and displays the '
           'schedule found and its maximal span. All schedules '
           'found in between are displayed together with their '
           'maximal span.  Search can be interrupted by '
           'clicking <code>Stop</code>.')

        dt('Stop')

        dd('Stops the search for a next or a best '
           'schedule.')

        dt('Search Tree')
        dd('Displays the search tree of the scheduling '
           'problem in the Oz Explorer.')))
