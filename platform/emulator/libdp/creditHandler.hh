#ifndef __CREDITHANDLER_HH
#define __CREDITHANDLER_HH

#include "dsite.hh"
#include "table.hh"
class DSite;


/*
  The distributed gc algorithm has been changed from 
  secondary weight to fractional weight. The old algorithm is 
  stored for future evaluation. 
  

 */


#ifndef SEC_CREDIT_HANDLER

typedef struct { 
  int            enumerator;
  int            denominator;
} Credit;

void marshalCredit(MarshalerBuffer *buf,Credit c);
// Since special difs are used to owner a special marshal exists.
// This also marshals the oti needed for DIF_OWNER and DIF_OWNER_SEC
void marshalCreditToOwner(MarshalerBuffer *buf,Credit c,int oti);
#ifndef USE_FAST_UNMARSHALER
Credit unmarshalCreditRobust(MarshalerBuffer *buf,int *error);
Credit unmarshalCreditToOwnerRobust(MarshalerBuffer *buf,
				    MarshalTag mt, int &oti,
				    int *error);
#else
Credit unmarshalCredit(MarshalerBuffer *buf);
Credit unmarshalCreditToOwner(MarshalerBuffer *buf,
			      MarshalTag mt, int &oti);
#endif

class EnumDenumPair;

#define MAXENUMERATOR INT_MAX >> 1 

/*
  The alpha parameter defines the spreading behavior. 
  A larger ALPHA results in handeling out small portions
  of weight, wheras a large ALPHA results in handeling out of
  large portions. 
  
  1 < ALPHA < INF

 */ 
#define ALPHA  10000      



class CreditHandler {
  friend class OB_Entry;
public:
  EnumDenumPair *frac;
  
  // To be used by the Owner/BorrowEntry
  Bool isPersistent();
  void makePersistent();
  Bool isExtended();

  EnumDenumPair *findPair(int k);
  Bool insertPair(int e, int k);
  EnumDenumPair *findLargest();
  
};


  
class OwnerCreditHandler : public CreditHandler {
  friend class OwnerEntry;

protected:


  // To be used by the OwnerEntry at gc.
  Bool hasFullCredit(){ return NO;}
  void setUp(int indx);

public:
  int oti;
  void addCredit(Credit c);
   

  void print();
  OZ_Term extract_info();
  
  Credit getCreditBig();
  Credit getCreditSmall();
  
};
  

class BorrowCreditHandler : public CreditHandler {
  friend class BorrowEntry;
  friend class BorrowTable;
protected:
  NetAddress netaddr; // Must somehow be able to identify whose credit it is.
                      // To save memory this was taken out of BorrowEntry.

  // To be used by the BorrowEntry
  void setUp(Credit c,DSite* s,int i);
  // Used by the GateMechanism. See perdio.cc
  void setUpPersistent(DSite* s,int i);  
  // When a reference is localy removed it must 
  // hand back its weight
  void giveBackAllCredit();
  // Garbage collection
  void copyHandler(BorrowCreditHandler *from);
  // The netaddress of the borrow is stored in the credithandler
  NetAddress* getNetAddress();

  //  The credithandler can have dependencies that makes it
  //  a temporary root for GC. These functins are used to see
  //  if the credit-handler is a root. 
  Bool maybeFreeCreditHandler();
  Bool canBeFreed();
public:
  void addCredit(Credit c);
  void print();
  OZ_Term extract_info();
  Credit getCreditBig();
  Credit getCreditSmall();
  
};

#else //SEC_CREDIT_HANDLER

class OwnerCreditExtension;
class BorrowCreditExtension;

typedef struct { 
  int            credit;
  DSite         *owner;
} Credit;

void marshalCredit(MarshalerBuffer *buf,Credit c);
// Since special difs are used to owner a special marshal exists.
// This also marshals the oti needed for DIF_OWNER and DIF_OWNER_SEC
void marshalCreditToOwner(MarshalerBuffer *buf,Credit c,int oti);
#ifndef USE_FAST_UNMARSHALER
Credit unmarshalCreditRobust(MarshalerBuffer *buf,int *error);
Credit unmarshalCreditToOwnerRobust(MarshalerBuffer *buf,
				    MarshalTag mt, int &oti,
				    int *error);
#else
Credit unmarshalCredit(MarshalerBuffer *buf);
Credit unmarshalCreditToOwner(MarshalerBuffer *buf,
			      MarshalTag mt, int &oti);
#endif

class CreditHandler {
  friend class OB_Entry;
protected:
  union {
    int                    credit;
    OwnerCreditExtension  *oExt;
    BorrowCreditExtension *bExt;
  } cu;
  unsigned short flags;

  // To be used by the Owner/BorrowEntry
  Bool isPersistent();
  void makePersistent();
public:
  // To be used e.g. when a reference is passed to a remote site and
  // the credit should be shared with that site.
//    virtual Credit getCreditBig()=0;
  // To be used when an entity is simply refered to and some
  // credit is needed to keep consistency. In this primary/secondary credit
  // protocol, it will return one credit.
//    virtual Credit getCreditSmall()=0;
//    virtual void addCredit(Credit c)=0;

protected:
  // For internal usage only
  inline unsigned short getFlags()         {return flags;}
  inline void setFlags(unsigned short f)   {flags=f;}
  inline void removeFlags(unsigned short f) {flags = flags & (~f);}
  inline void addFlags(unsigned short f)    {flags = flags | f;}

  Bool isExtended();

  void initCreditOB();
  void addCreditOB(int c);
  void subCreditOB(int c);
  void setCreditOB(int c); 
  int getCreditOB();
};

class OwnerCreditHandler : public CreditHandler {
  friend class OwnerEntry;

protected:
  int oti;

  // To be used by the OwnerEntry at gc.
  Bool hasFullCredit();
  void setUp(int oti);

public:
  Credit getCreditBig();
  Credit getCreditSmall();
  void addCredit(Credit c);
  void print();
  OZ_Term extract_info();

private:
  // Methods copied from previous implementation in table.hh/cc
  // Now only used internally.
  OwnerCreditExtension* getOwnerCreditExtension();
  void setOwnerCreditExtension(OwnerCreditExtension* oce);
  void extend();
  void getCredit(int req);
  void addCreditExtended(int back);
  void removeExtension();
};

class BorrowCreditHandler : public CreditHandler {
  friend class BorrowEntry;
  friend class BorrowTable;
protected:
  NetAddress netaddr; // Must somehow be able to identify whose credit it is.
                      // To save memory this was taken out of BorrowEntry.

  // To be used by the BorrowEntry
  void setUp(Credit c,DSite* s,int i);
  void setUpPersistent(DSite* s,int i);
//    void getAllCredit();
//    Bool hasFullCredit();
  void giveBackAllCredit();
  void copyHandler(BorrowCreditHandler *from);
  NetAddress* getNetAddress();
  Bool maybeFreeCreditHandler();
  Bool canBeFreed();

public:
  Credit getCreditBig();
  Credit getCreditSmall();
  void addCredit(Credit c);
  void print();
  OZ_Term extract_info();

private:
  // Methods copied from previous implementation in table.hh/cc
  // Now only used internally.
  BorrowCreditExtension* getSlave();
  BorrowCreditExtension* getMaster();
  void setSlave(BorrowCreditExtension* bce);
  void setMaster(BorrowCreditExtension* bce);
  Bool getOnePrimaryCredit_E();
  int getSmallPrimaryCredit_E();
  void thresholdCheck(int c);
  void removeSoleExtension(int c);
  void createSecMaster();
  void removeSlave();
  void createSecSlave(int cred,DSite *s);
  int extendGetPrimCredit();
  void extendSetPrimCredit(int c);
  void generalTryToReduce();
  void giveBackSecCredit(DSite *s,int c);
  void removeBig(BorrowCreditExtension* master);
  void removeMaster_SM(BorrowCreditExtension* master);
  void removeMaster_M(BorrowCreditExtension* master);
  void addSecCredit_MasterBig(int c,BorrowCreditExtension *master);
  void addSecCredit_Master(int c,BorrowCreditExtension *master);
  Bool addSecCredit_Slave(int c,BorrowCreditExtension *slave);
  int getExtendFlags();
  void addPrimaryCreditExtended(int c);  
  void addSecondaryCredit(int c,DSite *s);
  void addPrimaryCredit(int c);
  Bool getOnePrimaryCredit();
  int getSmallPrimaryCredit();
  DSite *getOneSecondaryCredit() ;
  DSite *getSmallSecondaryCredit(int &cred);
  void giveBackCredit(int c);
  void moreCredit();
};
#endif //SEC_CREDIT_HANDLER

#endif
