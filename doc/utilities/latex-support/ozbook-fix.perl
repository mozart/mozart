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

1;;
