
#ifdef DEBUG_PERDIO

DebugVector *DV = NULL;

char *debugTypeStr[LAST] = {
  "MSG_RECEIVED", //0
    "MSG_SENT",
    "MSG_PREP",
    "TABLE",
    "TABLE2",
    "GC",       //  5
    "CREDIT",
    "LOOKUP",
    "GLOBALIZING",
    "LOCALIZING",
    "PD_VAR",  // 10
    "CELL",
    "LOCK",
    "SITE_OP",
    "THREAD_D",

    "MARSHAL",  // 15
    "MARSHAL_CT",
    "UNMARSHAL",
    "UNMARSHAL_CT",
    "MARSHAL_BE",
    "REF_COUNTER", // 20

    "TCP",
    "WEIRD",
    "TCP_INTERFACE",
    "TCPCACHE",
    "TCPQUEUE",   // 25
    "SITE",
    "REMOTE",
    "MESSAGE",
    "OS",
    "BUFFER",    // 30
    "READ",
    "WRITE",
    "CONTENTS",
    "HASH",
    "HASH2",    // 35

  "USER",
  "SPECIAL",
  "ERROR_DET",    //38
  "WRITE_QUEUE",
  "ACK_QUEUE",    //40
  "CELL_MGR",      //41
  "PROBES"
};

#endif
