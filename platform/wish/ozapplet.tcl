# Plugin Remote Interpreter of Tcl/Tk commands

policy trusted

# Connect via pipe with Oz process OLS
if {[lsearch [array names env] OZHOME] < 0 } {
    set OZHOME /usr/local/oz
} else {
    set OZHOME $env(OZHOME)
}
set OLS [open |[concat $OZHOME/bin/ozapplet $embed_args(url)] w+]

# Configure device for proper newlines
fconfigure $OLS -translation {auto lf}

# Connect to socket
set HOST [gets $OLS]
set PORT [gets $OLS]
set OZSOCK [socket $HOST $PORT]

# Print out a magic string
puts $OLS "Oz aPpLeT: the MOZART release"

# Print the number of arguments that will be sent
puts $OLS [array size embed_args]

# Print all arg#value pairs
foreach arg [array names embed_args] {
    puts $OLS $arg
    puts $OLS $embed_args($arg)
}

flush $OLS

# Configure socket
fconfigure $OZSOCK -blocking 0
fconfigure $OZSOCK -translation {auto lf}

proc OzNbr {} {
    global OZSOCK

    set output ""
    set len    0

    while {($len >= 0) || ![info complete $output]} {
	set len [gets $OZSOCK line]
	append output "$line\n"
    }
     
    if ([eof $OZSOCK]) {
	error "Unexpected EOF on socket!"
    } else {
	return $output
    }
}

proc OzServe {} {    

    if { [catch {set cmds [OzNbr]} msg] } {
	error "Tcl Oz client: $msg"
    }

    if { [catch {eval $cmds} msg] } {
	error "Tcl Oz client: $msg"
    }
}


# Save old puts and flush
rename puts old_puts

rename flush old_flush

proc puts {args} {
    global OZSOCK
    
    set len [llength $args]

    if { $len == 1 } {
	old_puts $OZSOCK "[lindex $args 0]"
    } elseif { $len == 2 } {
	if { [lindex $args 0] == "-nonewline" } {
	    old_puts -nonewline $OZSOCK "[lindex $args 1]"
	} else {
	    old_puts $OZSOCK "[lindex $args 1]"
	}
    } else {
	old_puts -nonewline $OZSOCK "[lindex $args 2]"
    }
    
    old_flush $OZSOCK
}

proc flush {args} {
}

# Serve the socket
fileevent $OZSOCK readable "OzServe"
