/* urlc.h
   declarations for the URL client 
 */

#define URL_CLIENT


/* gets in a local file the content of the URL passed as argument.
   returns (int) file descriptor of the file or error.
   the file dissapears when close(2) the fd.
   */
int openUrl(const char* Url);


/* gets in a local file the content of the URL passed as first argument.
   the second argument keeps the filename with the URL's contents.
   don't forget to free(3) *fnp!
   return 0 (URLC_OK) on success, reason on error.
   */
int localizeUrl(const char* Url, char** fnp); 


/* return codes */

#define     URLC_OK 0

#define     URLC_EALLOC        -1 /* allocation error */
#define     URLC_EPARSE        -2 /* parse error */
#define     URLC_EINVAL        -3 /* invalid parameter/reply */
#define     URLC_ESOCK         -4 /* socket manipulation error */
#define     URLC_ERESP         -5 /* error in response */
#define     URLC_EFILE         -6 /* file manipulation error */
#define     URLC_EUNKNOWN      -7 /* unknown error, none of this list */
#define     URLC_EEMPTY        -8 /* empty resource */
#define     URLC_EAUTH         -9 /* authentication failure */
#define     URLC_AGAIN        -10 /* try again */
#define     URLC_INTERM       -11 /* intermediate state */
#define     URLC_LATER        -12 /* try later */
#define     URLC_REDIRECT     -13 /* resource redirect */
#define     URLC_UNLINK       -14 /* unlink hint */


