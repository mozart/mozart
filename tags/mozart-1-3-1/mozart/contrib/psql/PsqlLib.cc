/*
%%%
%%% Authors:
%%%   Lars Rasmusson (Lars.Rasmusson@sics.se)
%%%
%%% Copyright:
%%%   Lars Rasmusson, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

// include stuff for PostgreSQL
#include "libpq-fe.h"
#ifdef Assert
#undef Assert
#endif

#include "PsqlLib.hh"

static struct {
  OZ_Term bid;
  OZ_Term price;
  OZ_Term until;

  OZ_Term EMPTY_QUERY;
  OZ_Term COMMAND_OK;
  OZ_Term TUPLES_OK;
  OZ_Term COPY_OUT;
  OZ_Term COPY_IN;
  OZ_Term BAD_RESPONSE;
  OZ_Term NONFATAL_ERROR;
  OZ_Term FATAL_ERROR;
} PSQL_tags;

static OZ_Term PSQL_arity;

void init_atoms() {
  PSQL_tags.bid   = OZ_atom("bid");    OZ_protect(&(PSQL_tags.bid));
  PSQL_tags.price = OZ_atom("price");  OZ_protect(&(PSQL_tags.price));
  PSQL_tags.until = OZ_atom("until");  OZ_protect(&(PSQL_tags.until));

  PSQL_tags.EMPTY_QUERY = OZ_atom("PGRES_EMPTY_QUERY");          OZ_protect(&(PSQL_tags.EMPTY_QUERY));
  PSQL_tags.COMMAND_OK = OZ_atom("PGRES_COMMAND_OK");            OZ_protect(&(PSQL_tags.COMMAND_OK));
  PSQL_tags.TUPLES_OK = OZ_atom("PGRES_TUPLES_OK");              OZ_protect(&(PSQL_tags.TUPLES_OK));
  PSQL_tags.COPY_OUT = OZ_atom("PGRES_COPY_OUT");                OZ_protect(&(PSQL_tags.COPY_OUT));
  PSQL_tags.COPY_IN = OZ_atom("PGRES_COPY_IN");                  OZ_protect(&(PSQL_tags.COPY_IN));
  PSQL_tags.BAD_RESPONSE = OZ_atom("PGRES_BAD_RESPONSE");        OZ_protect(&(PSQL_tags.BAD_RESPONSE));
  PSQL_tags.NONFATAL_ERROR = OZ_atom("PGRES_NONFATAL_ERROR");    OZ_protect(&(PSQL_tags.NONFATAL_ERROR));
  PSQL_tags.FATAL_ERROR = OZ_atom("PGRES_FATAL_ERROR");          OZ_protect(&(PSQL_tags.FATAL_ERROR));
}


PGconn *OZ_toPGconn(OZ_Term C) 
{
  return (PGconn*) OZ_getForeignPointer(C);
}

PGresult *OZ_toPGresult(OZ_Term C) 
{
  return (PGresult*) OZ_getForeignPointer(C);
}

OZ_Term PGconn_toTerm(PGconn *conn) 
{
  return OZ_makeForeignPointer(conn);
}

OZ_Term PGresult_toTerm(PGresult *res) 
{
  return OZ_makeForeignPointer(res);
}

//
// {PSQL.setdb +Host +Port +Options +TTY +DBName -Conn}
// opens a connection to postgress
// Takes two arguments, returns nothing.
//

OZ_BI_define(PSQL_setdb,5,1)
{
  int var;

  printf("in PSQL_setdb\n");

  // make the parameter to C strings
  // if the strings are "" then they should be NULL

  char *s;
  int l;

  // we must copy the arguments to the stack so
  // they are not destroyed

  s = OZ_virtualStringToC( OZ_in(0) , &var);
  l = strlen(s);
  char sHost[l+1];     strcpy( sHost, s );
  char *host = ( l != 0 ) ? sHost : (char*)NULL;

  s = OZ_virtualStringToC( OZ_in(1) , &var);
  l = strlen(s);
  char sPort[l+1];     strcpy( sPort, s );
  char *port = ( l != 0 ) ? sPort : (char*)NULL;

  s = OZ_virtualStringToC( OZ_in(2) , &var);
  l = strlen(s);
  char sOptions[l+1];  strcpy( sOptions, s );
  char *options = ( l != 0 ) ? sOptions : (char*)NULL;

  s = OZ_virtualStringToC( OZ_in(3) , &var);
  l = strlen(s);
  char sTty[l+1];      strcpy( sTty, s );
  char *tty = ( l != 0 ) ? sTty : (char*)NULL;

  /*
  printf("host    = \"%s\"\n", host==0?"NULL":host);
  printf("port    = \"%s\"\n", port==0?"NULL":port);
  printf("options = \"%s\"\n", options==0?"NULL":options);
  printf("tty     = \"%s\"\n", tty==0?"NULL":tty);
  */

   // name of database to connect to
   char *dbname  = OZ_virtualStringToC( OZ_in(4) , &var);

   // turn off interrupts
   int t = osGetAlarmTimerInterval();
   osSetAlarmTimer(0);

   // connect
   PGconn *conn  = PQsetdb(host, port, options, tty, dbname);

   // turn on interrupts
   osSetAlarmTimer(t);

   // check that the connection was ok, if not, return

   if (PQstatus(conn) == CONNECTION_BAD) {
     char *errorMsg = PQerrorMessage(conn);

     PQfinish(conn);

     return OZ_raiseErrorC("PostgreSQL", 2,
			   OZ_atom("CONNECTION_BAD"),
			   OZ_atom( errorMsg ));

   } else {

     OZ_RETURN( PGconn_toTerm(conn) );

   }
}
OZ_BI_end

//
// {PSQL.connectdb +ConnectStr -Conn}
// opens a connection to postgress
// Takes two arguments, returns nothing.
//

OZ_BI_define(PSQL_connectdb,1,1)
{
  int var;

  // turn off interrupts
  int t = osGetAlarmTimerInterval();
  osSetAlarmTimer(0);

  // connect
  PGconn *conn  = PQconnectdb(OZ_virtualStringToC( OZ_in(0) , &var));

  // turn on interrupts
  osSetAlarmTimer(t);

  // check that the connection was ok, if not, return

  if (PQstatus(conn) == CONNECTION_BAD) {
    char *errorMsg = PQerrorMessage(conn);
    
    PQfinish(conn);
    
    return OZ_raiseErrorC("PostgreSQL", 2,
			  OZ_atom("CONNECTION_BAD"),
			  OZ_atom( errorMsg ));

  } else {
     
    OZ_RETURN( PGconn_toTerm(conn) );
    
  }
}
OZ_BI_end


//
// {PSQL.finish +Conn}
// Closes the connection to postgress
//

OZ_BI_define(PSQL_finish,1,0)
{
  PGconn *conn = OZ_toPGconn( OZ_in(0) );

  PQfinish(conn);
  return PROCEED;
}
OZ_BI_end

//
// {PSQL.clear +Res}
// Releases a result struct
//

OZ_BI_define(PSQL_clear,1,0)
{
  PGresult *res  = OZ_toPGresult( OZ_in(0) );

  PQclear(res);
  return PROCEED;
}
OZ_BI_end

//
// {PSQL.exec +Conn +Str -Res}
// Executes the SQL string 
// Returns a result structure.
//

OZ_BI_define(PSQL_exec,2,1)
{
  int var;

  // make the parameter to C strings
  // if the strings are "" then they should be NULL
   
  PGconn *conn   = OZ_toPGconn( OZ_in(0) );
  char *str      = OZ_virtualStringToC( OZ_in(1) , &var);

  // turn off interrupts
  int t = osGetAlarmTimerInterval();
  osSetAlarmTimer(0);

  PGresult *res = PQexec(conn,str);

  // turn on interrupts
  osSetAlarmTimer(t);

  OZ_RETURN( PGresult_toTerm( res ) );
}
OZ_BI_end

//
// {PSQL.resultStatus +Res -Status}
// Takes a result structure.
// Returns the result (an integer) (should be an atom)
//

OZ_BI_define(PSQL_resultStatus,1,1)
{

  PGresult *res  = OZ_toPGresult( OZ_in(0) );

  int status = PQresultStatus(res);
  OZ_Term result ;

  switch ( status ) 
    {
      case PGRES_EMPTY_QUERY:     result = PSQL_tags.EMPTY_QUERY; break;
      case PGRES_COMMAND_OK:      result = PSQL_tags.COMMAND_OK; break;
      case PGRES_TUPLES_OK:       result = PSQL_tags.TUPLES_OK; break;
      case PGRES_COPY_OUT:        result = PSQL_tags.COPY_OUT; break;
      case PGRES_COPY_IN:         result = PSQL_tags.COPY_IN; break;
      case PGRES_BAD_RESPONSE:    result = PSQL_tags.BAD_RESPONSE; break;
      case PGRES_NONFATAL_ERROR:  result = PSQL_tags.NONFATAL_ERROR; break;
      case PGRES_FATAL_ERROR:     result = PSQL_tags.FATAL_ERROR; break;
    default: result = OZ_atom("Unknown error");
    }

  OZ_RETURN( result );
}
OZ_BI_end



//
// {PSQL.cmdStatus +Res -Status}
// Takes a PGresult structure.
// Returns the result (an integer) (should be an atom)
//

OZ_BI_define(PSQL_cmdStatus,1,1)
{
  PGresult *res  = OZ_toPGresult( OZ_in(0) );

  OZ_RETURN_STRING( PQcmdStatus( res ) );
}
OZ_BI_end



//
// {PSQL.ntuples +Res -N}
// Takes a result structure.
// Returns the number of tuples in the query result
//

OZ_BI_define(PSQL_ntuples,1,1)
{

  PGresult *res  = OZ_toPGresult( OZ_in(0) );

  if( PQresultStatus(res) == PGRES_TUPLES_OK)
    {
      OZ_RETURN_INT( PQntuples(res));
    }
  else
    {
      OZ_RETURN_INT( -1 );   // maybe it should raise an exception?
    }
}
OZ_BI_end


//
// {PSQL.nfields +Res -N}
// Takes a result structure.
// Returns the number of fields (attributes) in the result tuples
//

OZ_BI_define(PSQL_nfields,1,1)
{

  PGresult *res  = OZ_toPGresult( OZ_in(0) );

  if( PQresultStatus(res) == PGRES_TUPLES_OK)
    {
      OZ_RETURN_INT( PQnfields(res) );
    }
  else
    {
      OZ_RETURN_INT( -1 );   // maybe it should raise an exception?
    }
}
OZ_BI_end


//
// {PSQL.fname +Res +FieldNum -Name}
// Takes a result structure and one int
// Returns the name of the attribute.
// Numbering starts with 0
//

OZ_BI_define(PSQL_fname,2,1)
{

  PGresult *res  = OZ_toPGresult( OZ_in(0) );
  int field_index = OZ_intToC( OZ_in(1) );   // should perhaps check ranges

  if( PQresultStatus(res) == PGRES_TUPLES_OK)
    {
      char *name = PQfname( res, field_index );

      OZ_RETURN_ATOM( name );
    }
  else
    {
      OZ_RETURN_STRING( "" );   // maybe it should raise an exception?
    }
}
OZ_BI_end


//
// {PSQL.fsize +Res +FieldNum -N}
// Takes a result structure and one int
// Returns the defined size of the field
// Variable size fields return -1
// Numbering starts with 0
//

OZ_BI_define(PSQL_fsize,2,1)
{

  PGresult *res  = OZ_toPGresult( OZ_in(0) );
  int field_num  = OZ_intToC( OZ_in(1) );   // should perhaps check ranges

  if( PQresultStatus(res) == PGRES_TUPLES_OK)
    {
      OZ_RETURN_INT( PQfsize(res, field_num) );
    }
  else
    {
      OZ_RETURN_INT( -1 );   // maybe it should raise an exception?
    }
}
OZ_BI_end


//
// {PSQL.getlength +Res +TupleNum +FieldNum -N}
// Takes a result structure and two ints
// Returns the actual size of the field in the specified tuple
// Numbering starts with 0
//

OZ_BI_define(PSQL_getlength,3,1)
{

  PGresult *res  = OZ_toPGresult( OZ_in(0) );
  int tuple_num  = OZ_intToC( OZ_in(1) );   // should perhaps check ranges
  int field_num  = OZ_intToC( OZ_in(2) );

  if( PQresultStatus(res) == PGRES_TUPLES_OK)
    {
      OZ_RETURN_INT( PQgetlength(res, tuple_num, field_num) );
    }
  else
    {
      OZ_RETURN_INT( -1 );   // maybe it should raise an exception?
    }
}
OZ_BI_end


//
// {PSQL.getvalue +Res +TupleNum +FieldNum -Str}
// Takes a result structure and two ints
// Returns the value of the tuple field as a string
// Numbering starts with 0
//

OZ_BI_define(PSQL_getvalue,3,1)
{

  PGresult *res  = OZ_toPGresult( OZ_in(0) );
  int tuple_num  = OZ_intToC( OZ_in(1) );   // should perhaps check ranges
  int field_num  = OZ_intToC( OZ_in(2) );

  if( PQresultStatus(res) == PGRES_TUPLES_OK)
    {
      char *value = PQgetvalue(res, tuple_num, field_num);
      OZ_RETURN_STRING( value );
    }
  else
    {
      OZ_RETURN_STRING( "" );   // maybe it should raise an exception?
    }
}
OZ_BI_end


// turn off timer and return the old value
int osGetAlarmTimerInterval()
{
#ifdef DEBUG_DET
    return;
#endif

#ifdef WINDOWS

    /* NOT IMPLEMENTED! SEE os.cc AND osSetAlarmTimer FOR SOME INFO */
    return 100;
#else
  struct itimerval oldT;

  if( getitimer(ITIMER_REAL,&oldT) < 0) {
    ozpwarning("getitimer");
  }

  int sec =  oldT.it_interval.tv_sec;
  int usec = oldT.it_interval.tv_usec;

  int t = sec*1000+usec;

  return t;
#endif
}

// Copied from os.cc
// 't' is in miliseconds;
void osSetAlarmTimer(int t)
{

#ifdef DEBUG_DET
    return;
#endif

#ifdef WINDOWS

  if (timerthread==NULL) {
    unsigned tid;
    timerthread = new TimerThread(t);
  }

  if (t==0) {
    SuspendThread(timerthread->thrd);
  } else {
    timerthread->wait = t;
    ResumeThread(timerthread->thrd);
  }
#else
  struct itimerval newT;

  int sec  = t/1000;
  int usec = (t*1000)%1000000;
  newT.it_interval.tv_sec  = sec;
  newT.it_interval.tv_usec = usec;
  newT.it_value.tv_sec     = sec;
  newT.it_value.tv_usec    = usec;

  if (setitimer(ITIMER_REAL,&newT,NULL) < 0) {
    ozpwarning("setitimer");
  }
#endif
}


extern "C" 
{
  OZ_C_proc_interface * oz_init_module(void) 
  {
    static OZ_C_proc_interface i_table[] = {
      {"setdb"          ,5,1,PSQL_setdb},
      {"connectdb"      ,1,1,PSQL_connectdb},
      {"finish"         ,1,0,PSQL_finish},
      {"clear"          ,1,0,PSQL_clear},
      {"exec"           ,2,1,PSQL_exec},
      {"resultStatus"   ,1,1,PSQL_resultStatus},
      {"cmdStatus"      ,1,1,PSQL_cmdStatus},
      {"ntuples"        ,1,1,PSQL_ntuples},
      {"nfields"        ,1,1,PSQL_nfields},
      {"fname"          ,2,1,PSQL_fname},
      {"fsize"          ,2,1,PSQL_fsize},
      {"getlength"      ,3,1,PSQL_getlength},
      {"getvalue"       ,3,1,PSQL_getvalue},


      {0,0,0,0}  /* must end with four zeros */
    };

    /* init stuff */
    init_atoms();
    PSQL_arity = OZ_cons(PSQL_tags.price, OZ_cons(PSQL_tags.until, OZ_nil()));
    OZ_protect(&PSQL_arity);

    return i_table;
  }
} /* extern "C" */





