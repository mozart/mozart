#! /usr/local/bin/perl
###
### Here we declare all builtins in a general format which can be used
### to generate both the table of builtins used by the emulator and the
### information used by the Oz compiler.
###
###	bidecl.perl -ctable
###		generates the table of builtins for the emulator
###	bidecl.perl -cdecl
###		generates the extern declarations for the above table
###	bidecl.perl -oztable
###		generates the table of builtins for the Oz compiler
###
### ADDITIONAL OPTIONS
###
###	-include M1,M2,...,Mn
###	-exclude M1,M2,...,Mn
###
###		include (resp. exclude) only these modules. by default
###	all modules are included.  Only one of these options may be
###	present (actually there can be several of them as long as they
###	are all -include or all -exclude.  Their arguments are unioned).
###
### Each entry has the form: 'NAME' => { ... }, where 'NAME' is the
### string by which the builtin is known to the emulator.  The associative
### array describing the builtin contains at least:
###	in  => [...],
###	out => [...],
### describing respectively the input and output arguments.  Each argument
### is described by a type, possibly annotated by a prefix indicating
### the determinacy condition.  Thus an integer argument might be specified
### in one of these ways:
###	 'int'		possibly non-determined
###	'+int'		determined
###	'*int'		kinded (e.g. an FD variable)
### '+int' (resp. '*int') indicates that the builtin will suspend until
### this argument is determined (resp. kinded).
###
### Furthermore, there are builtins that overwrite their input arguments.
### This should be indicated by the prefix `!'. Thus '!+value' indicates
### an argument for which the builtin will suspend until it is determined
### and which may be overwriten by its execution.
###
### For most builtins it is OK if the output registers are not all distinct
### from the input registers: the compiler takes advantage of the possibility
### to eliminate certain moves.  However there are builtins that will not
### function properly unless certain output registers are guaranteed to
### be distinct from the input registers.  An output argument may be
### annotated with ^ to indicate that it needs its own register.
###
### The annotations +,*,! and ^ may be given in arbitrary order.
###
### A type may be simple or complex:
###
### SIMPLE    ::= unit
###		| atom
###		| nilAtom
###		| array
###		| bitArray
###		| bool
###		| cell
###		| char
###		| chunk
###		| comparable
###		| class
###		| dictionary
###		| feature
###		| float
###		| foreignPointer
###		| fset
###		| int
###		| fdint
###		| intC
###		| literal
###		| lock
###		| name
###		| number
###		| object
###		| port
###		| procedure
###		| procedure/0
###		| procedure/1
###		| procedure/2
###		| procedure/3
###		| procedure/4
###		| procedure/5
###		| procedure/6
###		| procedure/>6
###		| procedureOrObject
###		| unaryProcOrObject
###		| record
###		| recordOrChunk
###		| recordC
###		| recordCOrChunk
###		| space
###		| thread
###		| tuple
###		| pair
###		| cons
###		| list
###		| string
###		| virtualString
###		| value
###
### COMPLEX   ::= [SIMPLE]		(list of SIMPLE)
###		| [SIMPLE#SIMPLE]	(list of pairs of SIMPLE)
###		|  SIMPLE#SIMPLE	(pair or SIMPLE)
###
### determinacy annotations for subtypes of complex types are not yet
### supported.
###
### Old style builtins have: bi => OLDBI, where OLDBI is the name
### of the C procedure that implements it (normally defined using
### OZ_C_proc_begin(OLDBI,...)).
###
### New style builtins have: BI => NEWBI, where NEWBI is the name
### of the C procedure that implements it (defined using
### OZ_BI_define(NEWBI,...,...)).
###
### ifdef => MACRO, indicates that the entry for this builtin in the
### emulator's table should be bracketed by #ifdef MACRO ... #endif.
### Actually MACRO can be of the form M1,M2,...,Mn in which case there
### will be n bracketing, one for each macro M1 to Mn.
###
### ifndef => M1,...,Mn is similar for #ifndef ... #endif.  both ifdef
### and ifndef may be present.
###
### doesNotReturn => 1, indicates that the builtin does not return and
### therefore that the code following it will never be executed.  For
### example 'raise'.
###
### negated => BI, indicates that the builtin BI returns the negated
### result of the builtin.  For example `<' is the negated version of `>='.
### This only makes sense for builtins with a single output argument which
### is of type bool.
###
### module => M, indicates that the builtin belongs to module M.  This
### permits selective inclusion or exclusion through command line options
### -include or -exclude.
###
### native => true|false specifies whether the builtin
### is non-exportable.


require "bidecl.all";

$builtins = { %builtins_all };


# this is the function that converts these descriptions to
# an array of declarations appropriate for the emulator

sub CTABLE {
    my ($key,$info);
    while (($key,$info) = each %$builtins) {
	next unless &included($info);
	my $inArity = @{$info->{in}};
	my $outArity = @{$info->{out}};
	my $BI = $info->{BI};
	my @ifdef  = split(/\,/,$info->{ifdef});
	my @ifndef = split(/\,/,$info->{ifndef});
	my $macro;
	foreach $macro (@ifdef)  { print "#ifdef $macro\n"; }
	foreach $macro (@ifddef) { print "#ifndef $macro\n"; }
	my $native = $info->{native};
	if ( $native eq "true") {
	    $native = "OK";
	} elsif ( $native eq "false") {
	    $native = "NO";
	} else {
	    die "*** native flag for $key must be 'true' or 'false'";
	}
	$BI = $info->{bi} unless $BI;
	print "{\"$key\",\t$inArity,\t$outArity,$BI,\t$native},\n";
	foreach $macro (@ifddef) { print "#endif\n"; }
	foreach $macro (@ifdef)  { print "#endif\n"; }
    }
}

# this is the function that converts these descriptions to
# an array of declarations appropriate for dynamic linking

sub CDYNTABLE {
    my ($key,$info);
    while (($key,$info) = each %$builtins) {
	next unless &included($info);
	my $inArity = @{$info->{in}};
	my $outArity = @{$info->{out}};
	my $BI = $info->{BI};
	my @ifdef  = split(/\,/,$info->{ifdef});
	my @ifndef = split(/\,/,$info->{ifndef});
	my $macro;
	foreach $macro (@ifdef)  { print "#ifdef $macro\n"; }
	foreach $macro (@ifddef) { print "#ifndef $macro\n"; }
	my $native = $info->{native};
	if ( !($native eq "true")) {
	    die "*** native flag for $key must be 'true'";
	}
	$BI = $info->{bi} unless $BI;
	print "{\"$key\",\t$inArity,\t$outArity,$BI},\n";
	foreach $macro (@ifddef) { print "#endif\n"; }
	foreach $macro (@ifdef)  { print "#endif\n"; }
    }
}

sub argspec {
    my $spec = shift;
    my ($mod,$det,$typ,$own) = (0,'any','value',0);
    my $again = 1;

    # first we handle the various annotations

    while ($again) {
	# is the argument register side effected?
	if    ($spec =~ /^\!/) { $spec=$'; $mod=1; }
	# what is the determinacy condition on the argument?
	elsif ($spec =~ /^\+/) { $spec=$'; $det='det'; }
	elsif ($spec =~ /^\*/) { $spec=$'; $det='detOrKinded'; }
	# does it need its own register
	elsif ($spec =~ /^\^/) { $spec=$'; $own=1; }
	else { $again=0; }
    }

    # now parse the type of the argument

    if    ($spec =~ /^\[(.+)\#(.+)\]$/) { $typ="list(pair('$1' '$2'))"; }
    elsif ($spec =~ /^\[(.+)\]$/      ) { $typ="list('$1')"; }
    elsif ($spec =~ /^(.+)\#(.+)$/    ) { $typ="pair('$1' '$2')"; }
    else                                { $typ="'$spec'"; }

    return ($mod,$det,$typ,$own);
}

sub OZTABLE {
    my ($key,$info);
    while (($key,$info) = each %$builtins) {
	next unless &included($info);
	my (@imods,@idets,@ityps,$spec,$destroys,@oowns);
	foreach $spec (@{$info->{in}}) {
	    my ($mod,$det,$typ,$own) = &argspec($spec);
	    $destroys=1 if $mod;
	    push @imods,($mod?'true':'false');
	    push @idets,$det;
	    push @ityps,$typ;
	    die "found ^ annotation on input arg spec for builtin $key"
		if $own;
	}
	my (@odets,@otyps);
	foreach $spec (@{$info->{out}}) {
	    my ($mod,$det,$typ,$own) = &argspec($spec);
	    $det="any(det)" if $det eq 'det';
	    push @odets,$det;
	    push @otyps,$typ;
	    push @oowns,($own?'true':'false');
	}
	print "'$key':\n\tbuiltin(\n";
	if ((@ityps+@otyps)>0) {
	    print "\t\ttypes: [",join(' ',@ityps,@otyps),"]\n";
	    print "\t\tdet: [",join(' ',@idets,@odets),"]\n";
	} else {
	    print "\t\ttypes: nil\n";
	    print "\t\tdet: nil\n";
	}

	if (@imods) {
	    print "\t\timods: [",join(' ',@imods),"]\n";
	} else {
	    print "\t\timods: nil\n";
	}
	if (@oowns) {
	    print "\t\toowns: [",join(' ',@oowns),"]\n";
	} else {
	    print "\t\toowns: nil\n";
	}
	if ($#otyps == 0 && $otyps[0] eq '\'bool\''
	    && $odets[0] eq 'any(det)') {
	    print "\t\ttest: true\n";
	}

	print "\t\tdoesNotReturn: true\n" if $info->{doesNotReturn};
	my $negated = $info->{negated};
	print "\t\tnegated: '$negated'\n" if $negated;
	my $native = $info->{native};
	if ($native eq "true") {
	    print "\t\tsited: true\n";
	}
	print "\t)\n";
    }
}

sub CDECL {
    my ($key,$info,$bi);
    while (($key,$info) = each %$builtins) {
	next unless &included($info);
	$bi = $info->{bi} || $info->{BI};
	print "OZ_C_proc_proto($bi);\n";
    }
}


sub checkNative {
    my $value = shift;
    printf "Builtins with nativeness: '$value'\n";
    printf "**************************************\n";
    my ($key,$info);
    foreach $key (sort keys %$builtins) {
	my $info = $builtins->{$key};
	if ( $info->{native} eq $value) {
	    print "   $key\n";
	}
    }
    print "\n\n";
}

sub SORTNATIVENESS {
    &checkNative("true");
    &checkNative("false");
}

sub STRUCTURE {
    exec "grep '#\\*' $0 | sed -e 's/[ \t]*#/  /'g";
}

my %include = ();
my %exclude = ();
my $includedefault = 1;

sub included {
    my $info = shift;
    my $module = $info->{module} || 'oz';
    return 0 if $exclude{$module};
    return 1 if $include{$module};
    return $includedefault;
}

my ($option,$choice,@include,@exclude);

while (@ARGV) {
    $option = shift;
    if    ($option eq '-ctable' )    { $choice='ctable';  }
    elsif ($option eq '-cdecl'  )    { $choice='cdecl';   }
    elsif ($option eq '-cdyntable')  { $choice='cdyntable'; }
    elsif ($option eq '-oztable')    { $choice='oztable'; }
    elsif ($option eq '-sortnativeness') { $choice='sortnativeness'; }
    elsif ($option eq '-structure')   { $choice='structure'; }
    elsif ($option eq '-include')    { push @include,split(/\,/,shift); }
    elsif ($option eq '-exclude')    { push @exclude,split(/\,/,shift); }
    else { die "unrecognized option: $option"; }
}

if (@include!=0 && @exclude!=0) {
    die "cannot have both -include and -exclude";
}

foreach $option (@include) { $include{$option} = 1; }
foreach $option (@exclude) { $exclude{$option} = 1; }

$includedefault = 0 if @include!=0;

if    ($choice eq 'ctable' )    { &CTABLE;  }
elsif ($choice eq 'cdecl'  )    { &CDECL;   }
elsif ($choice eq 'cdyntable' ) { &CDYNTABLE;   }
elsif ($choice eq 'oztable')    { &OZTABLE; }
elsif ($choice eq 'sortnativeness') { &SORTNATIVENESS; }
elsif ($choice eq 'structure')   { &STRUCTURE; }
else { die "must specify one of: -ctable -cdecl -oztable -structure -sortnativeness"; }
