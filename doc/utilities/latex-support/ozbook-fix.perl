# -*-perl-*-
# Copyright © by Denys Duchier, Dec 1997, Universität des Saarlandes
#

package main;

sub do_cmd_index {
    local($_) = @_;
    s/$next_pair_pr_rx//o;
    local ($one,$two) = ($1,$2);
    $two =~ s/(\|\(?)id/$1/;
    join('',&named_index_entry($one, $two),$_);
}

sub add_idx {
    print "\nDoing the index ...";
    local($key, @keys, $next, $index);
# RRM, 15.6.96:  index constructed from %printable_key, not %index
    @keys = keys %printable_key;
    @keys = sort makeidx_keysort @keys;
    @keys = grep(!/\001/, @keys);
    local (%fastkey) = ();
    local $fastkey = 0;
    local $fastindex = '';
    local $zkey;
    foreach $key (@keys) {
	if ($key =~ /^[a-zA-Z]/) {
	    $zkey = uc($&);
	    if (! exists $fastkey{$zkey}) {
		$fastkey{$zkey} = 1;
		$fastindex .= " | " if $fastkey;
		$fastkey++;
		$fastindex .=
		    "<A href='#KEY_$zkey'>$zkey</A>";
	    }
	}
    }
    $fastindex = "<P ALIGN=CENTER>$fastindex</P>" if $fastindex;
    %fastkey = ();
    foreach $key (@keys) {
	if ($key =~ /^[a-zA-Z]/) {
	    $zkey = uc($&);
	    if (!exists $fastkey{$zkey}) {
		$fastkey = "KEY_$zkey";
		$fastkey{$zkey} = 1;
	    } else { $fastkey=''; }
	} else { $fastkey = ''; }
	$index .= &add_idx_key($key);
    }
    $index = '<DD>'.$index unless ($index =~ /^\s*<D(D|T)>/);
    if ($SHORT_INDEX) { 
	print "(compact version with Legend)";
	local($num) = ( $index =~ s/\<D/<D/g ); 
	if ($num > 50 ) {
	    s/$idx_mark/$preindex$fastindex<HR><DL>\n$index\n<\/DL>$preindex/o;
	} else { 
	    s/$idx_mark/$preindex$fastindex<HR><DL>\n$index\n<\/DL>/o;
	}
    } else { s/$idx_mark/$fastindex<DL COMPACT>\n$index\n<\/DL>/o; }
}

sub print_key {
    local ($before,$after)=();
    if ($fastkey) {
	$before = "<A NAME='$fastkey'>";
	$after  = "</A>";
    }
    $fastkey = '';
   "$before<STRONG>$printable_key{$key}</STRONG>$after"
}

sub style_sheet {
    local($env,$id,$style);
    do{
	open(STYLESHEET, ">$FILE.css");
        if ( -f $EXTERNAL_STYLESHEET ) {
	    open(EXT_STYLES, "<$EXTERNAL_STYLESHEET");
	    while (<EXT_STYLES>) { print STYLESHEET $_; }
	    close(EXT_STYLES);
	} else {
	print STYLESHEET "
SMALL.XTINY		{ font-size : xx-small }
SMALL.TINY		{ font-size : x-small  }
SMALL.SCRIPTSIZE	{ font-size : smaller  }
SMALL.FOOTNOTESIZE	{ font-size : small    }
SMALL.SMALL		{  }
BIG.LARGE		{  }
BIG.XLARGE		{ font-size : large    }
BIG.XXLARGE		{ font-size : x-large  }
BIG.HUGE		{ font-size : larger   }
BIG.XHUGE		{ font-size : xx-large }
";
        }
	while (($env,$style) = each %env_style) {
	    if ($env =~ /inline/) {
		print STYLESHEET "SPAN.$env\t\t{ $style }\n";
	    } elsif ($env =~ /^(preform|\w*[Vv]erbatim(star)?)$/) {
		print STYLESHEET "PRE.$env\t\t{ $style }\n";
	    } else {
		print STYLESHEET "DIV.$env\t\t{ $style }\n";
	    }
	}
	
        while (($env,$style) = each %txt_style) {
            print STYLESHEET "SPAN.$env\t\t{ $style }\n";
        }


	foreach $id (sort(keys  %styleID)) {
	    print STYLESHEET "\#$id\t\t{ $styleID{$id} }\n" if ($styleID{$id});
	}
	close(STYLESHEET);
    #AXR:  don't overwrite existing .css
    }; #  unless -f $EXTERNAL_STYLESHEET; # "$FILE.css";
}

1;;
