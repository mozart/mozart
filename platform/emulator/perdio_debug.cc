
#ifdef DEBUG_PERDIO

DebugVector *DV = new DebugVector();

char *debugTypeStr[LAST] = {
  "SEND_EMIT",  // 0
  "SEND_DONE",
  "DEBT_SEC",
  "DEBT_MAIN",
  "MSG_RECEIVED",
  "MSG_SENT",   // 5
  "DELAYED_MSG_SENT",
  "TABLE",
  "TABLE2",
  "GC",
  "CREDIT",     // 10
  "LOOKUP",
  "GLOBALIZING",
  "AUXILLARY",
  "DEBT",
  "MARSHALL",   // 15
  "MARSHALL_CT",
  "UNMARSHALL",
  "UNMARSHALL_CT",
  "MARSHALL_BE",
  "PENDLINK",     // 20
  "HASH",
  "HASH2",
  "USER",
  "TCP",
  "WEIRD",        // 25
  "TCP_INTERFACE",
  "TCPCACHE",
  "TCPQUEUE",
  "SITE",
  "REMOTE",       // 30
  "MESSAGE",
  "OS",
  "BUFFER",
  "READ",
  "WRITE",        // 35
  "CONTENTS",
  "SPECIAL",
  "PD_VAR",
  "CELL",
  "SITE_OP"
};

void dvset(int i){
  if(i==0){
    /*    DV->set(SEND_DONE);
    DV->set(SEND_EMIT);
    DV->set(DEBT_SEC);
    DV->set(DEBT_MAIN);
    DV->set(MSG_RECEIVED);
    DV->set(MSG_SENT);
    DV->set(DELAYED_MSG_SENT);
    DV->set(TABLE);
    DV->set(GC);
    DV->set(CREDIT);
    DV->set(LOOKUP);
    DV->set(GLOBALIZING);
    DV->set(AUXILLARY);
    DV->set(DEBT);
    DV->set(PENDLINK);
    DV->set(HASH);
    DV->set(TABLE2);
    DV->set(MARSHALL);
    DV->set(MARSHALL_CT);
    DV->set(UNMARSHALL);
    DV->set(UNMARSHALL_CT);
    DV->set(MARSHALL_BE);
    DV->set(USER);
    DV->set(TCP);
    DV->set(TCPCACHE);
    DV->set(TCP_INTERFACE);*/
    DV->set(TCPQUEUE);
    /*    DV->set(SITE);
    DV->set(REMOTE);
    DV->set(MESSAGE);
    DV->set(WEIRD);
    DV->set(OS);
    DV->set(BUFFER);*/
    DV->set(READ);
    /*    DV->set(WRITE);
    DV->set(CONTENTS);
    DV->set(SPECIAL);*/
    return;
    }
  if(i==1){
    DV->set(SEND_DONE);
    DV->set(SEND_EMIT);
    DV->set(DEBT_SEC);
    DV->set(DEBT_MAIN);
    DV->set(MSG_RECEIVED);
    DV->set(MSG_SENT);
    DV->set(DELAYED_MSG_SENT);
    DV->set(TABLE);
    DV->set(GC);
    DV->set(CREDIT);
    DV->set(LOOKUP);
    DV->set(GLOBALIZING);
    DV->set(AUXILLARY);
    DV->set(DEBT);
    DV->set(PENDLINK);
    DV->set(HASH);
    DV->set(TABLE2);
    /*    DV->set(MARSHALL);
    DV->set(MARSHALL_CT);
    DV->set(UNMARSHALL);
    DV->set(UNMARSHALL_CT);*/
    DV->set(MARSHALL_BE);
    DV->set(USER);
    DV->set(TCP);
    DV->set(TCPCACHE);
    DV->set(TCP_INTERFACE);
    DV->set(TCPQUEUE);
    DV->set(SITE);
    DV->set(REMOTE);
    DV->set(MESSAGE);
    DV->set(WEIRD);
    DV->set(OS);
    DV->set(BUFFER);
    DV->set(READ);
    DV->set(WRITE);
    DV->set(CONTENTS);
    DV->set(SPECIAL);
    return;}

  if(i==2){
    /*    DV->set(SEND_DONE);
    DV->set(SEND_EMIT);
    DV->set(DEBT_SEC);
    DV->set(DEBT_MAIN);
    DV->set(MSG_RECEIVED);
    DV->set(MSG_SENT);
    DV->set(DELAYED_MSG_SENT);
    DV->set(TABLE);
    DV->set(GC);
    DV->set(CREDIT);
    DV->set(LOOKUP);
    DV->set(GLOBALIZING);
    DV->set(AUXILLARY);
    DV->set(DEBT);
    DV->set(PENDLINK);
    DV->set(HASH);
    DV->set(TABLE2);
    DV->set(MARSHALL);
    DV->set(MARSHALL_CT);
    DV->set(UNMARSHALL);
    DV->set(UNMARSHALL_CT);*/
    DV->set(MARSHALL_BE);
    DV->set(USER);
    DV->set(TCP);
    DV->set(TCPCACHE);
    DV->set(TCP_INTERFACE);
    DV->set(TCPQUEUE);
    DV->set(SITE);
    DV->set(REMOTE);
    DV->set(MESSAGE);
    DV->set(WEIRD);
    DV->set(OS);
    DV->set(BUFFER);
    DV->set(READ);
    DV->set(WRITE);
    DV->set(CONTENTS);
    DV->set(SPECIAL);
    return;}
  if(i==3){
    return;}
  Assert(0);
}

#endif
