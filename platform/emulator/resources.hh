/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#ifdef Assert
error include resources.hh before mozart.h
#endif

/*
 *
 * ### #     # ######   #####  ######  #######    #    #     # #######
 *  #  ##   ## #     # #     # #     #    #      # #   ##    #    #
 *  #  # # # # #     # #     # #     #    #     #   #  # #   #    #
 *  #  #  #  # ######  #     # ######     #    #     # #  #  #    #
 *  #  #     # #       #     # #   #      #    ####### #   # #    #
 *  #  #     # #       #     # #    #     #    #     # #    ##    #
 * ### #     # #        #####  #     #    #    #     # #     #    #
 *
 * Whenever you change this file, make sure that you also change the
 * appropriate handbook chapter: system/system.raw.
 */


#define PRINT_DEPTH             5
#define PRINT_WIDTH             20
#define ERROR_PRINT_DEPTH       PRINT_DEPTH
#define ERROR_PRINT_WIDTH       PRINT_WIDTH
#define ERROR_THREAD_DEPTH      20
#define ERROR_DEBUG             1

#define TIME_SLICE              50  /* ms */
#define CLOCK_TICK              TIME_SLICE*(1000/5)     /* usec */

/* task manager */
/* 4 tasks are needed by virtual sites, see virtual.cc */
/* 1 needed by ports se perdio.cc */
/* 1 for tcpCache, see network.cc */
#define MAXTASKS                6

#define NAMETRUE "true"
#define NAMEFALSE "false"
#define NAMEGROUPVOID "NameGroupVoid"
#define TRUEFALSEID 0


/* how to prefix errors/warnings, etc. for emacs */

#define MSG_ERROR 17
#define MSG_WARN 18
#define MSG_STATUS 19

/* how to prefix atoms/names */
#define ATOM_PREFIX 0
#define NAME_PREFIX 1
#define INTEGER_PREFIX 2

/* how to prefix arities for tuples and records  */
#define TUPLEWIDTH_PREFIX 0
#define RECORDARITY_PREFIX 1

/* how to mark EOF for scanner */
#define OZEOF 4

/* thread priorities */

#define HI_PRIORITY             2
#define MID_PRIORITY            1
#define LOW_PRIORITY            0

#define DEFAULT_PRIORITY        MID_PRIORITY

#define DEFAULT_HI_MID_RATIO    10
#define DEFAULT_MID_LOW_RATIO   10

/* dp tables default values */

#define DEFAULT_CLOSE_TIME      1000

#define DEFAULT_OWNER_TABLE_SIZE    100
#define DEFAULT_BORROW_TABLE_SIZE   100
#define DP_TABLE_LOW_LIMIT          20
#define DP_TABLE_EXPAND_FACTOR      200
#define DP_TABLE_BUFFER             50
#define DP_TABLE_WORTHWHILE_REALLOC 200


#define DEFAULT_TRAIL_SIZE      200

#define SHOW_IDLE_MESSAGE       0
#define SHOW_SUSPENSION         0
#define STOP_ON_TOPLEVEL_FAILURE        0

#define GC_FLAG         1
#define GC_VERBOSITY    0
#define CODE_GC_CYLES   5

#define RESIZESTACKMINSIZE 64 /* used for resizing a stack */

#define TASKFRAMESIZE   3
#define STACKMAXSIZE  8192 /* tasks */
#define STACKMINSIZE    4    /* tasks */

#define HEAPMAXSIZE        64 * 1024 /* kByte   */
#define HEAPMINSIZE             1024 /* kByte   */
#define HEAPFREE                  75 /* percent */
#define HEAPTOLERANCE             20 /* percent */
#define INITIALHEAPTHRESHOLD    2048 /* kByte   */
#ifdef CS_PROFILE
#define HEAPBLOCKSIZE        8*1048576 /* byte    */
#else
#ifdef WINDOWS
/* Under Windows (at leaster under NT 4.0) calling

     void *p1 = malloc(n);
     free(p1)
     void *p2 = malloc(n);

   does not mean that p1==p2: p2 usually gets a higher address. Thus
   after some time we run into an address space that has the top most
   bits set. The reason seems to be that if heap blocks are larger
   than the magic value gelow (approx 480kB) then windows calls
   VirtualMalloc
*/
#define HEAPBLOCKSIZE 0x75558
#else
#define HEAPBLOCKSIZE        512*1024  /* byte    */
#endif
#endif


#define QUEUEMINSIZE            32

#define TIMEDETAILED            0

#define NumberOfXRegisters      10000

/* threads */
#define THREAD_ID_SIZE          16
#define THREAD_ID_MAX           ((1 << THREAD_ID_SIZE) - 1)
#define THREAD_ID_MASK          THREAD_ID_MAX

//
// Tasks manager;
// Minimal frequency in ms.  A minimal frequency of '0' means there is
// no minimal frequency;
#define DEFAULT_MIN_INTERVAL   0

//
// Distributed Oz - general;
#define FLOW_BUFFER_SiZE    1000000
#define FLOW_BUFFER_TIME    1000
#define PERDIO_TIMEOUT      30000 /* ms */

//
// Distributed Oz - virtual sites
#define PROBE_INTERVAL      1000 /* ms */
#define CHECKMAIL_INTERVAL  1000 /* ms */
#define SITEQUEUE_INTERVAL  10 /* ms */
//
#define PERDIO_ID       0xa3
// 128k mailbox (messages 8bytes; so 16k messages);
#define VS_MAILBOX_SIZE (128*1024)
// 128k segments - 256 chunks .5k each;
#define VS_CHUNK_SIZE   512
#define VS_CHUNKS_NUM   256
// 12,5% fill-up for 32 sites?
#define VS_REGISTER_HT_SIZE   256
#define VS_VSTABLE_SIZE       16
#define VS_KEYSREG_SIZE       16
//
// Wait time for "ping" probing. Note that this time should be
// considerably larger than 'PROBE_INTERVAL', since ping messages
// ('M_SITE_IS_ALIVE'/'M_SITE_ALIVE') are delivered with the same
// priority as all other messages. This time is effectively made
// at least as large as 'PROBE_INTERVAL';
#define PROBE_WAIT_TIME 30000 /* ms */
// GCing of chunk pool' segments - see vs_msgbuffer.*;
#define VS_MSGCHUNKS_USAGE         3
#define VS_SEGS_MAXIDLE_PHASES     5
// ... but during "burst scavenging" done by the resource manager:
// cleanup as much as possible...
#define VS_MSGCHUNKS_USAGE_GC      1
#define VS_SEGS_MAXIDLE_PHASES_GC  0
// A new allocation phase starts at most after 'VS_SEGS_MAXPHASE_MS'
// (or, in other words, GC takes places at most every XXX ms,
// regardless whether it's needed or not);
#define VS_SEGS_MAXPHASE_MS        60000
// Whenever we are trying to allocate and/or map a shared memory page
// and nothing that can be reclaimed is found, we try to wait for
// 'VS_RESOURCE_WAIT' secs 'VS_WAIT_ROUNDS' times:
#define VS_RESOURCE_WAIT           1
#define VS_WAIT_ROUNDS             60

/*
 * Switches
 */

/* if defined the builtin print long is available to print detailed
 * informations about values */
#ifdef MISC_BUILTINS
# define DEBUG_PRINT
#endif

/* enable disjunctive construction */
#define FDCD

/* all debug switches for the emulator */
#ifdef DEBUG_EMULATOR

#define DEBUG_CHECK // enable assertions

/* always define debug print for debugging */
#define DEBUG_PRINT

#define DEBUG_TRACE     // MM: enable low level debugging: step instructions

//#define SLOW_DEBUG_CHECK
//#define DEBUG_GC
//#define DEBUG_VERBOSE
#define RECINSTRFETCH 500

//#define DEBUG_FD_CONSTRREP
//#define DEBUG_FSET
//#define DEBUG_FSET_CONSTRREP

// #define NEW_NAMER_DEBUG

//#define DEBUG_FDCD

#define DEBUG_DET       // use counter instead of alarm timer for scheduling

//#define PROFILE_FD
#define DEBUG_MEM

#define OUTLINE         // do not inline functions

//#define DEBUG_META
//#define DEBUG_STACK
//#define DEBUG_STABLE
//#define DEBUG_THREADS
//#define DEBUG_NONMONOTONIC
//#define DEBUG_LTQ
//#define DEBUG_INDICES
//#define DEBUG_THREADCOUNT
//#define DEBUG_PROPAGATORS
//#define ALLDIFF

//#define MM_DEBUG

// don't create UVAR's
//#define DEBUG_NO_UVAR

// #define DEBUG_PERDIO

//#define DEBUG_PROP_STABILTY_TEST

// test if the liveness routine for X registers works
//#define DEBUG_LIVENESS

#endif

#ifdef PROFILE_EMULATOR

// #define DISABLE_INSTRPROFILE
#define DEBUG_DET
// #define HEAP_PROFILE
// #define PROFILE_INSTR
// #define PROFILE_BI
// #define CS_PROFILE
#define OUTLINE
#endif

#ifdef RS_PROFILE
#define HEAP_PROFILE
#define OUTLINE
#define PROFILE_INSTR
#endif


// avoid inlining some functions
#if defined(OUTLINE)
#define INLINE
#else
#define INLINE inline
#endif



/*
 *
 * ###   #     # ######  ####### ######  #######    #    #     # #######
 *  #    ##   ## #     # #     # #     #    #      # #   ##    #    #
 *  #    # # # # #     # #     # #     #    #     #   #  # #   #    #
 *  #    #  #  # ######  #     # ######     #    #     # #  #  #    #
 *  #    #     # #       #     # #   #      #    ####### #   # #    #
 *  #    #     # #       #     # #    #     #    #     # #    ##    #
 * ###   #     # #       ####### #     #    #    #     # #     #    #
 *
 * Whenever you change this file, make sure that you also change the
 * appropriate handbook chapter: system/system.raw!!!!!!!!
 */

// mm2: what is this
#define PICKLE2TEXTHACK 1

#endif
