
declare [Path]={Module.link ['x-oz://system/os/Path.ozf']}

%%
%% This test file calls most Path methods on all common Path
%% cases. These tests are intended to run interactively for verifying
%% correct results by reading the results in the browser. 
%%
%% Path test cases are collected in a record with descriptive
%% features (like absDir for absolute directory). Each method iterates
%% over these test cases and all results are browsed. Additionally,
%% the unaltered path of the imput path is shown.
%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% Testing filenames in general
%%

declare
%% Creates a number of test filenames. Note that these files do not
%% necessarily exist on your system.
%% 
%% Each testcase consists of a pair of a path object, and the given
%% path as an atom (to simplify debugging).
TestCases1 = unit(
		%% Forced UNIX / windows pathes (value of optional arg
		%% 'windows' depends on your system if omitted)
		absFile1:{New Path.'class' init('/home/test/tmp.txt' windows:false)}#'/home/test/tmp.txt'
		absDir1:{New Path.'class' init('/home/test/' windows:false)}#'/home/test/'
		relFile1:{New Path.'class' init('home/test/tmp.txt' windows:false)}#'home/test/tmp.txt'
		relDir1:{New Path.'class' init('home/test/' windows:false)}#'home/test/'
		rootDir1:{New Path.'class' init('/' windows:false)}#'/'
		emptyPath1:{New Path.'class' init(nil windows:false)}#nil
		plainFile1:{New Path.'class' init('text.txt' windows:false)}#'text.txt'
		absFileWin1:{New Path.'class' init('D:\\home\\test\\tmp.txt' windows:true)}#'D:\\home\\test\\tmp.txt'
		relDirWin1:{New Path.'class' init('home\\test\\' windows:true)}#'home\\test\\'
		)
%% TestCases2 are the same as TestCases1, but the init argument
%% 'exact' always gets the value true
TestCases2 = unit(
		absFile2:{New Path.'class' init('/home/test/tmp.txt' windows:false exact:true)}#'/home/test/tmp.txt'
		absDir2:{New Path.'class' init('/home/test/' windows:false exact:true)}#'/home/test/'
		relFile2:{New Path.'class' init('home/test/tmp.txt' windows:false exact:true)}#'home/test/tmp.txt'
		relDir2:{New Path.'class' init('home/test/' windows:false exact:true)}#'home/test/'
		rootDir2:{New Path.'class' init('/' windows:false exact:true)}#'/'
		emptyPath2:{New Path.'class' init(nil windows:false exact:true)}#nil
		plainFile2:{New Path.'class' init('text.txt' windows:false exact:true)}#'text.txt'
		absFileWin2:{New Path.'class' init('D:\\home\\test\\tmp.txt' exact:true windows:true)}#'D:\\home\\test\\tmp.txt'
		relDirWin2:{New Path.'class' init('home\\test\\' exact:true windows:true)}#'home\\test\\'
		)
TestCases = {Adjoin TestCases1 TestCases2}

/*
%% getInfo was only tmp for debugging 
{Record.map TestCases
 fun {$ P#X} {P getInfo($)}#X end}
*/

{Browse toAtom#
 {Record.map TestCases
  fun {$ P#X} {P toAtom($)}#X end}}

{Browse toString#
 {Record.map TestCases
  fun {$ P#X} {P toString($)}#X end}}

{Browse isAbsolute#
 {Record.map TestCases
  fun {$ P#X} {P isAbsolute($)}#X end}}

{Browse isDir2#
 {Record.map TestCases
  fun {$ P#X} {P isDir2($)}#X end}}

{Browse basenameString#
 {Record.map TestCases
  fun {$ P#X} {P basenameString($)}#X end}}

{Browse basename_toString#
 {Record.map TestCases
  fun {$ P#X} {{P basename($)} toString($)}#X end}}

{Browse dirnameString#
 {Record.map TestCases
  fun {$ P#X} {P dirnameString($)}#X end}}

{Browse dirname_isDir2#
 {Record.map TestCases
  fun {$ P#X} {{P dirname($)} isDir2($)}#X end}}

{Browse basename_isDir2#
 {Record.map TestCases
  fun {$ P#X} {{P basename($)} isDir2($)}#X end}}

{Browse resolve('testDir/myFile.txt')#toString#
 {Record.map TestCases
  fun {$ P#X} {{P resolve('testDir/myFile.txt' $)} toString($)}#X end}}

{Browse resolve('/testDir/myFile.txt')#toString#
 {Record.map TestCases
  fun {$ P#X} {{P resolve('/testDir/myFile.txt' $)} toString($)}#X end}}

{Browse isRoot#
 {Record.map TestCases
  fun {$ P#X} {P isRoot($)}#X end}}

{Browse extension#
 {Record.map TestCases
  fun {$ P#X} {P extension($)}#X end}}

{Browse dropExtension_toString#
 {Record.map TestCases
  fun {$ P#X} {{P dropExtension($)} toString($)}#X end}}

{Browse addExtension('test')#toString#
 {Record.map TestCases
  fun {$ P#X} {{P addExtension('test' $)} toString($)}#X end}}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% Testing existing filenames 
%%

declare
TestCases3 = unit(
		cwd:{Path.getcwd}
		%% expects that OPI is started in this file
		thisFile:{Path.make {{Path.getcwd} toString($)}#'/Path-test.oz'}
		)

{Browse exists#
 {Record.map TestCases3
  fun {$ P} {P exists($)} end}}

{Browse isDir2#
 {Record.map TestCases3
  fun {$ P} {P isDir2($)} end}}

{Browse isDir#
 {Record.map TestCases3
  fun {$ P} {P isDir($)} end}}

{Browse stat#
 {Record.map TestCases3
  fun {$ P} {P stat($)} end}}

{Browse cwd_readdir_toString#
 {Map {{Path.getcwd} readdir($)}
  fun {$ P} {P toString($)} end}}

{Browse cwd_dirname_readdir_toString#
 {Map {{{Path.getcwd} dirname($)} readdir($)}
  fun {$ P} {P toString($)} end}}

{Browse maybeAddPlatform_toString#
 {Record.map TestCases3
  fun {$ P} {{P maybeAddPlatform($)} toString($)} end}}



