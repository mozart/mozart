# -*-perl-*-
# Copyright © by Denys Duchier, Dec 1997, Universität des Saarlandes
#

package main;

sub make_meta {
    my $txt = shift;
    "<SPAN CLASS=META>$txt</SPAN>";
}

sub do_cmd_MV {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    &make_meta($next) . $_;
}

sub do_cmd_IN  {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    &make_meta("+$next") . $_;
}

sub do_cmd_OUT {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    &make_meta("?$next") . $_;
}

sub do_cmd_CIN  {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    &make_meta("*$next") . $_;
}

sub do_cmd_CNIN  {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    &make_meta("\$$next") . $_;
}

sub do_cmd_OzInline {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    "<CODE>$next</CODE>" . $_;
}

sub do_cmd_OzFwd {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    "&acute;$next&acute;" . $_;
}

sub do_cmd_OzBwd {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    "&grave;$next&grave;" . $_;
}

sub do_cmd_OzEolComment {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    "%$next<BR>$_";
}

sub do_env_oz2texdisplay {
    local ($_) = @_;
    s/\n/\\IGNOREEOL\n/g;
    '<PRE CLASS=CODE>' . &translate_environments($_) . '</PRE>';
}

sub do_cmd_IGNOREEOL {
    local ($_) = @_;
    s/^\n//;
    $_;
}

sub do_cmd_OzChar {
    local ($_) = @_;
    local ($next);
    $next = $& if s/$next_token_rx//o;
    if ($next =~ /^\\(.)$/) {
	'&#' . ord($1) . ';' . $_;
    } else {
	$next . $_;
    }
}

sub do_cmd_OzSpace {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    ('&nbsp;' x int($next)) . $_;
}

sub do_cmd_OzBsl {
    local ($_) = @_;
    '&#' . ord('\\') . ';' . $_;
}

sub do_cmd_OzBox {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    "<CODE>$next</CODE>$_";
}

sub do_cmd_OzEol {
    local ($_) = @_;
    "<BR>$_";
}

sub do_cmd_OzKeyword {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    "<B>$next</B>$_";
}

sub do_cmd_OzString {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    "\"$next\"$_";
}

1;;
