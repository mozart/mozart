// See /System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Headers/Script.h for language IDs.
data 'LPic' (5000) {
  // Default language ID, 0 = English
  $"0000"
  // Number of entries in list
  $"0001"

  // Entry 1
  // Language ID, 0 = English
  $"0000"
  // Resource ID, 0 = STR#/TEXT/styl 5000
  $"0000"
  // Multibyte language, 0 = no
  $"0000"
};

resource 'STR#' (5000, "English") {
  {
    // Language (unused?) = English
    "English",
    // Agree
    "Agree",
    // Disagree
    "Disagree",
    // Print, ellipsis is 0xC9
    "Print…",
    // Save As, ellipsis is 0xC9
    "Save As…",
    // Descriptive text, curly quotes are 0xD2 and 0xD3
    "If you agree to the terms of this license agreement, click “Agree” to access the software.  If you do not agree, press “Disagree.”"
  };
};

// Beware of 1024(?) byte (character?) line length limitation.  Split up long
// lines.
// If straight quotes are used ("), remember to escape them (\").
// Newline is \n, to leave a blank line, use two of them.
// 0xD2 and 0xD3 are curly double-quotes ("), 0xD4 and 0xD5 are curly
//   single quotes ('), 0xD5 is also the apostrophe.
data 'TEXT' (5000, "English") {
  "     License Agreement"
  "\n"
  "             for"
  "\n"
  "Mozart, an implementation of Oz 3 "
  "\n"
  "----------------------------------"
  "\n"
  "\n"
  "This software and its documentation are copyrighted"
  "\n"
  "by Saarland University, the Swedish Institute of"
  "\n"
  "Computer Science (SICS), the German Research Center"
  "\n"
  "for Artificial Intelligence (DFKI), and other"
  "\n"
  "parties.  The following terms apply to all files"
  "\n"
  "associated with the software unless explicitly"
  "\n"
  "disclaimed in individual files (see the file"
  "\n"
  "LICENSE-others for a list of these exceptions)."
  "\n"
  "\n"
  "The authors hereby grant permission to use, copy,"
  "\n"
  "modify, distribute, and license this software and"
  "\n"
  "its documentation for any purpose, provided that"
  "\n"
  "existing copyright notices are retained in all"
  "\n"
  "copies and that this notice is included verbatim"
  "\n"
  "in any distributions. No written agreement,"
  "\n"
  "license, or royalty fee is required for any of the"
  "\n"
  "authorized uses. Modifications to this software"
  "\n"
  "may be copyrighted by their authors and need not"
  "\n"
  "follow the licensing terms described here,"
  "\n"
  "provided that the new terms are clearly indicated"
  "\n"
  "on the first page of each file where they apply."
  "\n"
  "\n"
  "IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE"
  "\n"
  "LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL,"
  "\n"
  "INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT"
  "\n"
  "OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR"
  "\n"
  "ANY DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE"
  "\n"
  "BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
  "\n"
  "\n"
  "THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM"
  "\n"
  "ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE"
  "\n"
  "IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR"
  "\n"
  "A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS"
  "\n"
  "SOFTWARE AND ITS DOCUMENTATION ARE PROVIDED ON AN"
  "\n"
  "\"AS IS\" BASIS, AND THE AUTHORS AND DISTRIBUTORS"
  "\n"
  "HAVE NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,"
  "\n"
  "UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
  "\n"
};

data 'styl' (5000, "English") {
  // Number of styles following = 1
  $"0001"

  // Style 1.  This is used to display the first two lines in bold text.
  // Start character = 0
  $"0000 0000"
  // Height = 16
  $"0010"
  // Ascent = 12
  $"000C"
  // Font family = 1024 (Lucida Grande)
  $"0400"
  // Style bitfield, 0x1=bold 0x2=italic 0x4=underline 0x8=outline
  // 0x10=shadow 0x20=condensed 0x40=extended
  $"00"
  // Style, unused?
  $"02"
  // Size = 12 point
  $"000C"
  // Color, RGB
  $"0000 0000 0000"
};
