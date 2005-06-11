# -*-perl-*-
# Copyright © by Denys Duchier, Dec 1997, Universität des Saarlandes
#

package main;

&do_require_package('html');
&do_require_package('makeidx');
&do_require_package('ozbook-fix');

sub do_ozbook_nocover{}
sub do_ozbook_pdf{}

sub do_cmd_thesection {
    local($_) = @_;
    join('',&translate_commands("\\thechapter")
	,"." , &do_cmd_arabic('<<0>>section<<0>>'), $_);
}
sub do_cmd_thesubsection {
    local($_) = @_;
    join('',&translate_commands("\\thesection")
	,"." , &do_cmd_arabic('<<0>>subsection<<0>>'), $_);
}
sub do_cmd_thesubsubsection {
    local($_) = @_;
    join('',&translate_commands("\\thesubsection")
	,"." , &do_cmd_arabic('<<0>>subsubsection<<0>>'), $_);
}
sub do_cmd_theparagraph {
    local($_) = @_;
    join('',&translate_commands("\\thesubsubsection")
	,"." , &do_cmd_arabic('<<0>>paragraph<<0>>'), $_);
}


&addto_dependents('chapter','equation');
&addto_dependents('chapter','footnote');

sub do_cmd_theequation {
    local($_) = @_;
    join('',&translate_commands("\\thechapter")
        ,"." , &do_cmd_arabic('<<0>>equation<<0>>'), $_);
}

sub do_cmd_thefootnote {
    local($_) = @_;
    join('',&translate_commands("\\thechapter")
        ,"." , &do_cmd_arabic('<<0>>footnote<<0>>'), $_);
}

sub do_cmd_burstauthor {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    $_;
}

sub do_cmd_bursttitle {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    $_;
}

sub do_cmd_reportnumber {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    $_;
}

sub do_cmd_comics {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    $_;
}

$t_abstract = '';

sub do_cmd_abstract {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    ($t_abstract) = &simplify(&translate_commands($next));
    $_;
}

sub do_cmd_email {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    $_;
}

sub do_cmd_maketitle {
    local($_) = @_;
    local($the_title) = '';
    if ($t_title) {
	$the_title .= "<H1 ALIGN=\"CENTER\">$t_title</H1>";
    } else { &write_warnings("\nThis document has no title."); }
    if ($t_author) {
	$the_title .= "\n<P ALIGN=\"CENTER\"><STRONG>$t_author</STRONG></P>";
    } else { &write_warnings("\nThere is no author for this document."); }
    if ($t_institute) {
        $the_title .= "\n<P ALIGN=\"CENTER\"><SMALL>$t_institute</SMALL></P>";}
    if ($t_affil) {
	$the_title .= "\n<P ALIGN=\"CENTER\"><I>$t_affil</I></P>";}
    if ($t_date) {
	$the_title .= "\n<P ALIGN=\"CENTER\"><STRONG>$t_date</STRONG></P>";}
    if ($t_address) {
	$the_title .= "<BR>\n<P ALIGN=\"LEFT\"><SMALL>$t_address</SMALL></P>";
    } else { $the_title .= "\n<P ALIGN=\"LEFT\">"}
    if ($t_email) {
	$the_title .= "\n<P ALIGN=\"LEFT\"><SMALL>$t_email</SMALL></P>";
    } else { $the_title .= "</P>" }
    if ($t_abstract) {
	$the_title .= "\n<BLOCKQUOTE>$t_abstract</BLOCKQUOTE>";
    }
    $the_title . $_ ;
}

sub remove_document_env {
    if (/\\begin${match_br_rx}document$match_br_rx/) {
    s/\\begin$match_br_rx[d]ocument$match_br_rx/\\latextohtmlditchpreceding \\maketitle /;
}
if (/\\end${match_br_rx}document$match_br_rx/) { $_ = $` }
s/maxifigure/figure/g;
#s/(\\begin${match_br_rx})maxifigure($match_br_rx)/$1figure$2$1makeimage$2/g;
#s/(\\end${match_br_rx})maxifigure($match_br_rx)/$1makeimage$2$1figure$2/g;
}

sub do_env_explain {
    local ($_) = @_;
    local ($next) = &get_next_argument;
    local ($rest) = &translate_environments($_);
    $next = &translate_environments($next);
    "<DL COMPACT><DT><CODE>$next</CODE><DD>$rest</DL>";
}

sub do_cmd_section {
    local($after, *open_tags) = @_;
    &reset_dependents('section');
    &do_cmd_section_helper('H2','section');
}

sub do_cmd_subsection {
    local($after, *open_tags) = @_;
    &reset_dependents('subsection');
    &do_cmd_section_helper('H3','subsection');
}

sub do_cmd_mchapter {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_rx//o)&&(($next=$2),1));
    &do_cmd_chapter($_);
}

sub do_cmd_msection {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_rx//o)&&(($next=$2),1));
    &do_cmd_section($_);
}

%section_commands = ('mchapter',2,'msection',3,%section_commands);

sub do_env_multiexplain {
    local ($_) = @_;
    s/^\s*$comment_mark\s*//o;
    local ($next) = &missing_braces
	unless ((s/$next_pair_rx//o)&&(($next=$2),1));
    local ($rest) = &translate_environments($_);
    local ($one,$two);
    local ($accu);
    while ($next) {
	$_ = $next;
	$one=$two=undef;
	s/^\s*$comment_mark//o;
	if (s/$next_pair_rx//o) { $one = $2; }
	else { last; }
	s/^\s*$comment_mark//o;
	$next = $_;
	if (s/$next_pair_rx//o) { $two = $2; }
	else { last; }
	s/^\s*$comment_mark//o;
	$next = $_;
	$one = &translate_environments($one);
	$two = &translate_environments($two);
	$accu .= "<DT><CODE>$one</CODE></DT><DD>$two<DD>";
    }
    $next = &translate_environments($next) if $next;
    "<DL COMPACT>$accu<DD>$next<P>$rest</DD></DL>";
}

sub do_cmd_ozbar {
    local ($_) = @_;
    "<B>|</B>$_";
}

sub do_cmd_ozoneormore  {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    "$next<SUP>+</SUP>$_";
}
    
sub do_cmd_ozpar  {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    "<B>(</B>$next<B>)</B>$_";
}

sub do_cmd_langle {
    local ($_) = @_;
    "&lt;$_";
}

sub do_cmd_rangle {
    local ($_) = @_;
    "&gt;$_";
}

sub do_cmd_marginpar {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    "<SPAN CLASS=MARGINNOTE>$next</SPAN>$_";
}

sub do_cmd_Marginpar { &do_cmd_marginpar; }

sub do_cmd_inexact {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    $_;
}
    
sub do_cmd_exact {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    "$next$_";
}

sub do_cmd_HTMLNOLINEBREAK {
    local ($_) = @_;
    local ($next);
    $next = &missing_braces
	unless ((s/$next_pair_pr_rx//o)&&(($next=$2),1));
    "<SPAN CLASS=NOLINEBREAK>$next</SPAN>$_";
}

sub do_cmd_danger {
    local ($_) = @_;
    "<IMG SRC='/img/danger.gif' ALT='[DANGER]'>$_";
}

sub do_cmd_dbend { &do_cmd_danger; }

sub do_cmd_ddanger {
    local ($_) = @_;
    "<IMG SRC='/img/danger.gif' ALT='[DANGER]'>"
	. "<IMG SRC='/img/danger.gif' ALT='[DANGER]'>"
	    . $_;
}

sub do_cmd_doreferences {
    local ($_) = @_;
    $_;
}

sub lib_add_bbl_and_idx_dummy_commands {
    s/[\\]doreferences/\\bibliography /;
    s/[\\]doindex/\\textohtmlindex /;
}

&ignore_commands( <<EOF);
ozfont
EOF

sub do_env_denum { &do_env_enumerate; }

1;;
