%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
reflected propagator:

prop := susp(
            type: prop
            params: <getParams>
            name: <getPropName>
            ref: <propToTerm>
            space: local | ask | wait | waittop
            )

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
reflected thread:

thread := susp(
              type: thread
              space: flat | ask | wait | waittop
              )

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
reflected suspension list

susp_list := [thread | prop}+ | nil

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
reflect_variable

refl_var := var(
               var: <var itself>
               name: <getVarName>
               type: any | fd | bool | fs | ct
               susplists: sl(@type)
               )

sl(any) := susplists(
                   any: <reflect_susplist(any)>
                   )

sl(fd)  := susplists(
                   any: <reflect_susplist(any)>
                   bounds: <reflect_susplist(bounds)>
                   val: <reflect_susplist(val)>
                   )

sl(bool):= susplists(
                   any: <reflect_susplist(any)>
                   )

sl(fs)  := susplists(
                   any: <reflect_susplist(any)>
                   glb: <reflect_susplist(any)>
                   lub: <reflect_susplist(any)>
                   val: <reflect_susplist(any)>
                   )

sl(ct)  := susplists(
                   any: <reflect_susplist(any)>
                   event_1: <reflect_susplist(event_1)>
                          ...
                   event_n: <reflect_susplist(event_n)>
                   )

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

var       := "var"  
any       := "any"  
type      := "type"  
fd        := "fd"  
fs        := "fs"  
bool      := "bool"  
bounds    := "bounds"  
val       := "val"  
glb       := "glb"  
lub       := "lub"  
flat      := "flat actor"  
local     := "home"  
ask       := "ask actor"  
wait      := "wait actor"  
waittop   := "waittop actor"  
oops      := "oops"  
prop      := "propagator"  
params    := "params"  
name      := "name"  
space     := "space"  
susp      := "suspension"  
thread    := "thread"  
ct        := "ct"    
susplists := "susplists"  
ref       := "reference"  
 
