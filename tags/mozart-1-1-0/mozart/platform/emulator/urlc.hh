/*
 *  Authors:
 *    Flaviu Turean (tf@info.ucl.ac.be)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */
/* urlc.h
   declarations for the URL client 
 */

#ifndef URL_CLIENT
#define URL_CLIENT


/* gets in a local file the content of the URL passed as argument.
   returns (int) file descriptor of the file or error.
   the file dissapears when close(2) the fd.
   */
int openUrl(const char* Url, char *tmpfile);


/* gets in a local file the content of the URL passed as first argument.
   the second argument keeps the filename with the URL's contents.
   don't forget to free(3) *fnp!
   return 0 (URLC_OK) on success, reason on error.
   */
int localizeUrl(const char* Url, char* fnp); 


/* return codes */

#define     URLC_OK 0

#define     URLC_EALLOC        -1 
#define     URLC_EPARSE        -2 
#define     URLC_EINVAL        -3 
#define     URLC_ESOCK         -4 
#define     URLC_ERESP         -5 
#define     URLC_EFILE         -6 
#define     URLC_EUNKNOWN      -7 
#define     URLC_EEMPTY        -8 
#define     URLC_EAUTH         -9 
#define     URLC_AGAIN        -10 
#define     URLC_INTERM       -11 
#define     URLC_LATER        -12 
#define     URLC_REDIRECT     -13 
#define     URLC_UNLINK       -14 
#define     URLC_ETOOLONG     -15 


inline
char *urlcStrerror(int err)
{
  switch (err) {
  case URLC_EALLOC:   return "allocation error";
  case URLC_EPARSE:   return "parse error";
  case URLC_EINVAL:   return "invalid parameter/reply";
  case URLC_ESOCK:    return "socket manipulation error";
  case URLC_ERESP:    return "error in response";
  case URLC_EFILE:    return "file manipulation error";
  case URLC_EEMPTY:   return "empty resource";
  case URLC_EAUTH:    return "authentication failure";
  case URLC_AGAIN:    return "try again";
  case URLC_INTERM:   return "intermediate state";
  case URLC_LATER:    return "try later";
  case URLC_REDIRECT: return "resource redirect";
  case URLC_UNLINK:   return "unlink hint";
  case URLC_ETOOLONG: return "string too long";
  case URLC_EUNKNOWN:
  default:
    return "unknown urlc error";
  }
}

#endif
