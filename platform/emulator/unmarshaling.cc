/*
 *  Authors:
 *    Andreas Sundstrom (andreas@sics.se)
 *    Kostja Popov (kost@sics.se)
 *
 *  Contributors:
 *
 *  Copyright:
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

//---------------------------------------------------------------------
// General unmarshaling procedures included in newmarshaler.cc
//---------------------------------------------------------------------

#ifdef ROBUST_UNMARSHALER
jmp_buf unmarshal_error_jmp;
#define RAISE_UNMARSHAL_ERROR longjmp(unmarshal_error_jmp,1)
// #define RAISE_UNMARSHAL_ERROR OZ_error("here")
#define TRY_UNMARSHAL_ERROR if(setjmp(unmarshal_error_jmp)==0)
#define CATCH_UNMARSHAL_ERROR else
#endif

//#include "base.hh"
//#include "gentraverser.hh"

//
// kost@ : there is no 'fsetcore.hh';
extern void makeFSetValue(OZ_Term,OZ_Term*);

//#include "pickle.hh"

//
//
#ifdef ROBUST_UNMARSHALER
OZ_Term newUnmarshalTermRobustInternal(MsgBuffer *bs)
#else
OZ_Term newUnmarshalTermInternal(MsgBuffer *bs)
#endif
{
  Assert(isInitialized);
  Assert(oz_onToplevel());
  builder.build();
  Builder *b;

#ifdef ROBUST_UNMARSHALER
  TRY_UNMARSHAL_ERROR {
#endif
    while(1) {
      b = &builder;
      MarshalTag tag = (MarshalTag) bs->get();
      dif_counter[tag].recv();  // kost@ : TODO: needed?
      //      printf("tag: %d\n", tag);

      switch (tag) {

      case DIF_SMALLINT:
        {
#ifdef ROBUST_UNMARSHALER
          int e;
          OZ_Term ozInt = OZ_int(unmarshalNumberRobust(bs, &e));
          if(e || !OZ_isSmallInt(ozInt)) {
            (void) b->finish();
            return 0;
          }
#else
          OZ_Term ozInt = OZ_int(unmarshalNumber(bs));
#endif
          b->buildValue_ROBUST(ozInt);
          break;
        }

      case DIF_FLOAT:
        {
#ifdef ROBUST_UNMARSHALER
          int e;
          double f = unmarshalFloatRobust(bs, &e);
          if(e) {
            (void) b->finish();
            return 0;
          }
#else
          double f = unmarshalFloat(bs);
#endif
          b->buildValue_ROBUST(OZ_float(f));
          break;
        }

      case DIF_NAME:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2,e3;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          char *printname = unmarshalStringRobust(bs, &e2);
          OZ_Term value;
          GName *gname    = unmarshalGNameRobust(&value, bs, &e3);
          if(e1 || e2 || e3) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
          char *printname = unmarshalString(bs);
          OZ_Term value;
          GName *gname    = unmarshalGName(&value, bs);
#endif

          if (gname) {
            Name *aux;
            if (strcmp("", printname) == 0) {
              aux = Name::newName(am.currentBoard());
            } else {
              aux = NamedName::newNamedName(strdup(printname));
            }
            aux->import(gname);
            value = makeTaggedLiteral(aux);
            b->buildValue_ROBUST(value);
            addGName(gname, value);
          } else {
            b->buildValue_ROBUST(value);
          }

          //
          b->set(value, refTag);
          delete printname;
          break;
        }

      case DIF_COPYABLENAME:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2;
          int refTag      = unmarshalRefTagRobust(bs, b, &e1);
          char *printname = unmarshalStringRobust(bs, &e2);
          if(e1 || e2 || (printname == NULL)) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag      = unmarshalRefTag(bs);
          char *printname = unmarshalString(bs);
#endif
          OZ_Term value;

          NamedName *aux = NamedName::newCopyableName(strdup(printname));
          value = makeTaggedLiteral(aux);
          b->buildValue_ROBUST(value);
          b->set(value, refTag);
          delete printname;
          break;
        }

      case DIF_UNIQUENAME:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2;
          int refTag      = unmarshalRefTagRobust(bs, b, &e1);
          char *printname = unmarshalStringRobust(bs, &e2);
          if(e1 || e2 || (printname == NULL)) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag      = unmarshalRefTag(bs);
          char *printname = unmarshalString(bs);
#endif
          OZ_Term value;

          value = oz_uniqueName(printname);
          b->buildValue_ROBUST(value);
          b->set(value, refTag);
          delete printname;
          break;
        }

      case DIF_ATOM:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          char *aux  = unmarshalStringRobust(bs, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
          char *aux  = unmarshalString(bs);
#endif
          OZ_Term value = OZ_atom(aux);
          b->buildValue_ROBUST(value);
          b->set(value, refTag);
          delete aux;
          break;
        }

      case DIF_BIGINT:
        {
#ifdef ROBUST_UNMARSHALER
          int e;
          char *aux  = unmarshalStringRobust(bs, &e);
          if(e || (aux == NULL)) {
            (void) b->finish();
            return 0;
          }
#else
          char *aux  = unmarshalString(bs);
#endif
          b->buildValue_ROBUST(OZ_CStringToNumber(aux));
          delete aux;
          break;
        }

      case DIF_LIST:
        {
#ifdef ROBUST_UNMARSHALER
          int e;
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          if(e) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildListRemember_ROBUST(refTag);
          break;
        }

      case DIF_TUPLE:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          int argno  = unmarshalNumberRobust(bs, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
          int argno  = unmarshalNumber(bs);
#endif
          b->buildTupleRemember(argno, refTag);
          break;
        }

      case DIF_RECORD:
        {
#ifdef ROBUST_UNMARSHALER
          int e;
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          if(e) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildRecordRemember(refTag);
          break;
        }

      case DIF_REF:
        {
#ifdef ROBUST_UNMARSHALER
          int e;
          int i = unmarshalNumberRobust(bs, &e);
          if(e || !b->checkIndexFound(i)) {
            (void) b->finish();
            return 0;
          }
#else
          int i = unmarshalNumber(bs);
#endif
          b->buildValue_ROBUST(b->get(i));
          break;
        }

        //
        // kost@ : remember that either all DIF_OBJECT, DIF_VAR_OBJECT and
        // DIF_OWNER are remembered, or none of them is remembered. That's
        // because both 'marshalVariable' and 'marshalObject' could yield
        // 'DIF_OWNER' (see also dpInterface.hh);
      case DIF_OWNER:
      case DIF_OWNER_SEC:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2;
          OZ_Term tert = (*unmarshalOwnerRobust)(bs, tag, &e1);
          int refTag = unmarshalRefTagRobust(bs, b, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
#else
          OZ_Term tert = (*unmarshalOwner)(bs, tag);
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildValueRemember_ROBUST(tert, refTag);
          break;
        }

      case DIF_RESOURCE_T:
      case DIF_PORT:
      case DIF_THREAD_UNUSED:
      case DIF_SPACE:
      case DIF_CELL:
      case DIF_LOCK:
      case DIF_OBJECT:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2;
          OZ_Term tert = (*unmarshalTertiaryRobust)(bs, tag, &e1);
          int refTag = unmarshalRefTagRobust(bs, b, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
#else
          OZ_Term tert = (*unmarshalTertiary)(bs, tag);
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildValueRemember_ROBUST(tert, refTag);
          break;
        }

      case DIF_RESOURCE_N:
        {
#ifdef ROBUST_UNMARSHALER
          int e;
          OZ_Term tert = (*unmarshalTertiaryRobust)(bs, tag, &e);
          if(e) {
            (void) b->finish();
            return 0;
          }
#else
          OZ_Term tert = (*unmarshalTertiary)(bs, tag);
#endif
          b->buildValue_ROBUST(tert);
          break;
        }

      case DIF_CHUNK:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          OZ_Term value;
          GName *gname = unmarshalGNameRobust(&value, bs, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
          OZ_Term value;
          GName *gname = unmarshalGName(&value, bs);
#endif

          if (gname) {
            b->buildChunkRemember(gname,refTag);
          } else {
            b->knownChunk_ROBUST(value);
            b->set(value,refTag);
          }
          break;
        }

      case DIF_CLASS:
        {
          OZ_Term value;
#ifdef ROBUST_UNMARSHALER
          int e1,e2,e3;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          GName *gname = unmarshalGNameRobust(&value, bs, &e2);
          int flags = unmarshalNumberRobust(bs, &e3);
          if(e1 || e2 || e3 || (flags > CLASS_FLAGS_MAX)) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
          GName *gname = unmarshalGName(&value, bs);
          int flags = unmarshalNumber(bs);
#endif

          if (gname) {
            b->buildClassRemember(gname,flags,refTag);
          } else {
            b->knownClass_ROBUST(value);
            b->set(value,refTag);
          }
          break;
        }

      case DIF_VAR:
        {
#ifdef ROBUST_UNMARSHALER
          int e;
          OZ_Term v = (*unmarshalVarRobust)(bs, FALSE, FALSE, &e);
          if(e) {
            (void) b->finish();
            return 0;
          }
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          if(e) {
            (void) b->finish();
            return 0;
          }
#else
          OZ_Term v = (*unmarshalVar)(bs, FALSE, FALSE);
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildValueRemember_ROBUST(v, refTag);
          break;
        }

      case DIF_FUTURE:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2;
          OZ_Term f = (*unmarshalVarRobust)(bs, TRUE, FALSE, &e1);
          int refTag = unmarshalRefTagRobust(bs, b, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
#else
          OZ_Term f = (*unmarshalVar)(bs, TRUE, FALSE);
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildValueRemember_ROBUST(f, refTag);
          break;
        }

      case DIF_VAR_AUTO:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2;
          OZ_Term va = (*unmarshalVarRobust)(bs, FALSE, TRUE, &e1);
          int refTag = unmarshalRefTagRobust(bs, b, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
#else
          OZ_Term va = (*unmarshalVar)(bs, FALSE, TRUE);
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildValueRemember_ROBUST(va, refTag);
          break;
        }

      case DIF_FUTURE_AUTO:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2;
          OZ_Term fa = (*unmarshalVarRobust)(bs, TRUE, TRUE, &e1);
          int refTag = unmarshalRefTagRobust(bs, b, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
#else
          OZ_Term fa = (*unmarshalVar)(bs, TRUE, TRUE);
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildValueRemember_ROBUST(fa, refTag);
          break;
        }

      case DIF_VAR_OBJECT:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2;
          OZ_Term obj = (*unmarshalTertiaryRobust)(bs, tag, &e1);
          int refTag = unmarshalRefTagRobust(bs, b, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
#else
          OZ_Term obj = (*unmarshalTertiary)(bs, tag);
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildValueRemember_ROBUST(obj, refTag);
          break;
        }

      case DIF_PROC:
        {
          OZ_Term value;
#ifdef ROBUST_UNMARSHALER
          int e1,e2,e3,e4,e5,e6,e7,e8;
          int refTag    = unmarshalRefTagRobust(bs, b, &e1);
          GName *gname  = unmarshalGNameRobust(&value, bs, &e2);
          int arity     = unmarshalNumberRobust(bs, &e3);
          int gsize     = unmarshalNumberRobust(bs, &e4);
          int maxX      = unmarshalNumberRobust(bs, &e5);
          int line      = unmarshalNumberRobust(bs, &e6);
          int column    = unmarshalNumberRobust(bs, &e7);
          int codesize  = unmarshalNumberRobust(bs, &e8); // in ByteCode"s;
          if(e1 || e2 || e3 || e4 || e5 || e6 || e7 || e8) {
            (void) b->finish();
            return 0;
          }
          if (maxX < 0 || maxX >= NumberOfXRegisters) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag    = unmarshalRefTag(bs);
          GName *gname  = unmarshalGName(&value, bs);
          int arity     = unmarshalNumber(bs);
          int gsize     = unmarshalNumber(bs);
          int maxX      = unmarshalNumber(bs);
          int line      = unmarshalNumber(bs);
          int column    = unmarshalNumber(bs);
          int codesize  = unmarshalNumber(bs); // in ByteCode"s;
#endif

          //
          if (gname) {
            //
            CodeArea *code = new CodeArea(codesize);
            ProgramCounter start = code->getStart();
            ProgramCounter pc = start + sizeOf(DEFINITION);
            //
            BuilderCodeAreaDescriptor *desc =
              new BuilderCodeAreaDescriptor(start, start+codesize, code);
            b->buildBinary(desc);

            //
            b->buildProcRemember(gname, arity, gsize, maxX, line, column,
                                 pc, refTag);
          } else {
            Assert(oz_isAbstraction(oz_deref(value)));
            // ('zero' descriptions are not allowed;)
            BuilderCodeAreaDescriptor *desc =
              new BuilderCodeAreaDescriptor(0, 0, 0);
            b->buildBinary(desc);

            //
            b->knownProcRemember_ROBUST(value, refTag);
          }
          break;
        }

        //
        // 'DIF_CODEAREA' is an artifact due to the non-recursive
        // unmarshaling of code areas: in order to unmarshal an Oz term
        // that occurs in an instruction, unmarshaling of instructions
        // must be interrupted and later resumed; 'DIF_CODEAREA' tells the
        // unmarshaler that a new code area chunk begins;
      case DIF_CODEAREA:
        {
          BuilderOpaqueBA opaque;
          BuilderCodeAreaDescriptor *desc =
            (BuilderCodeAreaDescriptor *) b->fillBinary(opaque);
          //
#ifdef ROBUST_UNMARSHALER
          switch (unmarshalCodeRobust(bs, b, desc)) {
          case ERR:
            (void) b->finish();
            return 0;
          case OK:
            b->finishFillBinary(opaque);
            break;
          case NO:
            b->suspendFillBinary(opaque);
            break;
          }
#else
          if (unmarshalCode(bs, b, desc))
            b->finishFillBinary(opaque);
          else
            b->suspendFillBinary(opaque);
#endif
          break;
        }

      case DIF_DICT:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          int size   = unmarshalNumberRobust(bs, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
          int size   = unmarshalNumber(bs);
#endif
          Assert(oz_onToplevel());
          b->buildDictionaryRemember_ROBUST(size,refTag);
          break;
        }

      case DIF_BUILTIN:
        {
#ifdef ROBUST_UNMARSHALER
          int e1,e2;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          char *name = unmarshalStringRobust(bs, &e2);
          if(e1 || e2 || (name == NULL)) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
          char *name = unmarshalString(bs);
#endif
          Builtin * found = string2CBuiltin(name);

          OZ_Term value;
          if (!found) {
            OZ_warning("Builtin '%s' not in table.", name);
            value = oz_nil();
            delete name;
          } else {
            if (found->isSited()) {
              OZ_warning("Unpickling sited builtin: '%s'", name);
            }

            delete name;
            value = makeTaggedConst(found);
          }
          b->buildValue_ROBUST(value);
          b->set(value, refTag);
          break;
        }

      case DIF_EXTENSION:
        {
#ifdef ROBUST_UNMARSHALER
          int e;
          int type = unmarshalNumberRobust(bs, &e);
          if(e) {
            (void) b->finish();
            return 0;
          }
#else
          int type = unmarshalNumber(bs);
#endif
          OZ_Term value = oz_extension_unmarshal(type,bs);
          if(value == 0) {
            break;  // next value is nogood
          }
          b->buildValue_ROBUST(value);
          break;
        }

      case DIF_FSETVALUE:
        b->buildFSETValue();
        break;

      case DIF_REF_DEBUG:
#ifdef ROBUST_UNMARSHALER
        (void) b->finish();
        return 0;
#else
        OZD_error("not implemented!");
#endif

      case DIF_ARRAY:
#ifdef ROBUST_UNMARSHALER
        (void) b->finish();
        return 0;
#else
        OZD_error("not implemented!");
        break;
#endif

        //
        // 'DIF_SYNC' and its handling is a part of the interfaca
        // between the builder object and the unmarshaler itself:
      case DIF_SYNC:
        b->processSync();
        break;

      case DIF_EOF:
        return (b->finish());

      default:
#ifdef ROBUST_UNMARSHALER
        (void) b->finish();
        return 0;
#else
        DebugCode(OZ_error("unmarshal: unexpected tag: %d\n",tag);)
        Assert(0);
        b->buildValue_ROBUST(oz_nil());
#endif
      }
    }
#ifdef ROBUST_UNMARSHALER
  }
  CATCH_UNMARSHAL_ERROR {
    (void) b->finish();
    return 0;
  }
#endif
}

//---------------------------------------------------------------------

#ifdef ROBUST_UNMARSHALER
Bool isArityList(OZ_Term l)
{
  OZ_Term old = l;
  while (oz_isCons(l)) {
    OZ_Term h = oz_head(l);
    if(!oz_isFeature(h)) return NO;
    l = oz_tail(l);
    if (l==old) return NO; // cyclic
  }
  if (oz_isNil(l)) {
    return OK;
  }
else {
    return NO;
  }
}
#endif

//
void
#ifdef ROBUST_UNMARSHALER
Builder::buildValueOutlineRobust(OZ_Term value, BTFrame *frame,
                                 BuilderTaskType type)
#else
Builder::buildValueOutline(OZ_Term value, BTFrame *frame,
                           BuilderTaskType type)
#endif
{
  Assert(type != BT_spointer);
  Bool doMemo = NO;

  //
  // Iteration invariant: there are correct 'frame' and 'type', but
  // no argument.
  // Procedure invariant: it gets frame but must get rid of it;
repeat:
  //
  switch (type) {

    //
    // Though it's handled inline, we can get here iteratively:
  case BT_spointer:
    {
      GetBTTaskPtr1(frame, OZ_Term*, spointer);
      DiscardBTFrame(frame);
      *spointer = value;
      break;
    }

  case BT_spointer_iterate:
    {
      GetBTTaskPtr1(frame, OZ_Term*, spointer);
      *spointer = value;
      CrazyDebug(incDebugNODES(););
      DiscardBTFrame(frame);
      DebugCode(value = (OZ_Term) -1;); // 'value' is expired;
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_buildValue:
    {
      GetBTTaskArg1NoDecl(frame, OZ_Term, value);
      DiscardBTFrame(frame);
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_makeTupleMemo:
    doMemo = OK;
    // fall through;
  case BT_makeTuple:
    {
      GetBTTaskArg1(frame, int, arity);
      SRecord *rec = SRecord::newSRecord(value, arity);
      OZ_Term recTerm = makeTaggedSRecord(rec);
      if (doMemo) {
        GetBTTaskArg2(frame, int, memoIndex);
        set(recTerm, memoIndex);
        doMemo = NO;
      }

      //
      GetBTNextTaskType(frame, nt);
      if (nt == BT_spointer) {
        CrazyDebug(incDebugNODES(););
        GetBTNextTaskPtr1(frame, OZ_Term*, spointer);
        *spointer = recTerm;
        DiscardBT2Frames(frame);

        //
        OZ_Term *args = rec->getRef();
        // put tasks in reverse order (since subtrees will appear in
        // the normal order):
        EnsureBTSpace(frame, arity);
        while(arity-- > 0) {
          PutBTTaskPtr(frame, BT_spointer, args++);
        }
      } else {
        //
        ReplaceBTTask1stArg(frame, BT_buildValue, recTerm);

        //
        OZ_Term *args = rec->getRef();
        EnsureBTSpace(frame, arity);
        arity--;
        PutBTTaskPtr(frame, BT_spointer_iterate, args++);
        while (arity-- > 0) {
          PutBTTaskPtr(frame, BT_spointer, args++);
        }
      }
      break;
    }

  case BT_takeRecordLabel:
    ReplaceBTTask1stArg(frame, BT_takeRecordArity, value);
    break;

  case BT_takeRecordLabelMemo:
    ReplaceBTTask1stArg(frame, BT_takeRecordArityMemo, value);
    break;

  case BT_takeRecordArity:
    ReplaceBTTask2ndArg(frame, BT_makeRecordSync, value);
    break;

  case BT_takeRecordArityMemo:
    ReplaceBTTask2ndArg(frame, BT_makeRecordMemoSync, value);
    break;

  case BT_makeRecordMemoSync:
    doMemo = OK;
    // fall through;

  case BT_makeRecordSync:
    {
      Assert(value == (OZ_Term) 0);
      GetBTTaskArg1(frame, OZ_Term, label);
      GetBTTaskArg2(frame, OZ_Term, arity);
#ifdef ROBUST_UNMARSHALER
      if(!OZ_isLiteral(label) || !isArityList(arity))
        RAISE_UNMARSHAL_ERROR;
#endif
      //
      OZ_Term sortedArity = arity;
      if (!isSorted(arity)) {
        int arityLen;
        TaggedRef aux = duplist(arity, arityLen);
        sortedArity = sortlist(aux, arityLen);
      }
      //
      SRecord *rec =
        SRecord::newSRecord(label, aritytable.find(sortedArity));
      OZ_Term recTerm = makeTaggedSRecord(rec);
      if (doMemo) {
        GetNextBTFrameArg1(frame, int, memoIndex);
        set(recTerm, memoIndex);
        doMemo = NO;
      }
      DiscardBT2Frames(frame);

      //
      GetBTTaskType(frame, nt);
      // An optimization (for the most frequent case?):
      // if the record is just to be stored somewhere ('spointer'
      // task), then let's do it now:
      if (nt == BT_spointer) {
        CrazyDebug(incDebugNODES(););
        GetBTTaskPtr1(frame, OZ_Term*, spointer);
        *spointer = recTerm;
        DiscardBTFrame(frame);

        //
        while (oz_isCons(arity)) {
          EnsureBTSpace1Frame(frame);
          PutBTTaskPtrArg(frame, BT_recordArg, rec, oz_head(arity));
          arity = oz_tail(arity);
        }
      } else {
        //
        // The last 'iterate' task will restore 'value' to the record
        // and iterate to the (non-spointer) task that handles it:
        EnsureBTSpace1Frame(frame);
        PutBTTaskPtrArg(frame, BT_recordArg_iterate, rec, oz_head(arity));
        arity = oz_tail(arity);
        while (oz_isCons(arity)) {
          EnsureBTSpace1Frame(frame);
          PutBTTaskPtrArg(frame, BT_recordArg, rec, oz_head(arity));
          arity = oz_tail(arity);
        }
      }

      //
      break;
    }

  case BT_recordArg:
    {
      GetBTTaskPtr1(frame, SRecord*, rec);
      GetBTTaskArg2(frame, OZ_Term, fea);
      DiscardBTFrame(frame);
      rec->setFeature(fea, value);
      break;
    }

  case BT_recordArg_iterate:
    {
      GetBTTaskPtr1(frame, SRecord*, rec);
      GetBTTaskArg2(frame, OZ_Term, fea);
      DiscardBTFrame(frame);
      rec->setFeature(fea, value);
      //
      CrazyDebug(incDebugNODES(););
      value = makeTaggedSRecord(rec); // new value;
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_dictKey:
    {
#ifdef ROBUST_UNMARSHALER
      if(!oz_isFeature(value))
        RAISE_UNMARSHAL_ERROR;
#endif
      // 'dict' remains in place:
      ReplaceBTTask2ndArg(frame, BT_dictVal, value);
      break;
    }

  case BT_dictVal:
    {
      GetBTTaskPtr1(frame, OzDictionary*, dict);
      GetBTTaskArg2(frame, OZ_Term, key);
      DiscardBTFrame(frame);
      dict->setArg(key, value);
      break;
    }

  case BT_fsetvalue:
    ReplaceBTTask2ndArg(frame, BT_fsetvalueSync, value);
    break;

  case BT_fsetvalueMemo:
    ReplaceBTTask2ndArg(frame, BT_fsetvalueMemoSync, value);
    break;

  case BT_fsetvalueMemoSync:
    doMemo = OK;

  case BT_fsetvalueSync:
    {
      Assert(value == (OZ_Term) 0);
      //
      GetBTTaskArg2(frame, OZ_Term, listRep);
      // will iterate to the task that handles value:
      makeFSetValue(listRep, &value);
      if (doMemo) {
        GetBTTaskArg1(frame, int, memoIndex);
        set(value, memoIndex);
        doMemo = NO;
      }
      DiscardBTFrame(frame);

      //
      // 'value' now is the 'fsetvalue' we've just built. Let's just
      // go and do whatever is supposed to happen with it:
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_chunkMemo:
    doMemo = OK;
    // fall through;
  case BT_chunk:
    {
      Assert(oz_onToplevel());
#ifdef ROBUST_UNMARSHALER
      if(!oz_onToplevel() || !OZ_isRecord(value))
        RAISE_UNMARSHAL_ERROR;
#endif
      GetBTTaskPtr1(frame, GName*, gname);

      //
      OZ_Term chunkTerm;
      SChunk *sc = new SChunk(am.currentBoard(), 0);
      sc->setGName(gname);
      chunkTerm = makeTaggedConst(sc);
      addGName(gname, chunkTerm);
      sc->import(value);

      //
      value = chunkTerm;
      if (doMemo) {
        GetBTTaskArg2(frame, int, memoIndex);
        set(value, memoIndex);
        doMemo = NO;
      }
      DiscardBTFrame(frame);
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_classFeatures:
    {
      Assert(oz_isSRecord(value));
#ifdef ROBUST_UNMARSHALER
      if(!oz_isSRecord(value))
        RAISE_UNMARSHAL_ERROR;
#endif
      GetBTTaskPtr1(frame, ObjectClass*, cl);
      GetBTTaskArg2(frame, int, flags);
      DiscardBTFrame(frame);

      //
      SRecord *feat = tagged2SRecord(value);
      TaggedRef ff = feat->getFeature(NameOoFeat);
      //
      cl->import(value,
                 feat->getFeature(NameOoFastMeth),
                 oz_isSRecord(ff) ? ff : makeTaggedNULL(),
                 feat->getFeature(NameOoDefaults),
                 flags);

      //
      value = makeTaggedConst(cl);
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_procFile:
    ReplaceBTTask1stArg(frame, BT_proc, value);
    break;

  case BT_procFileMemo:
    ReplaceBTTask1stArg(frame, BT_procMemo, value);
    break;

  case BT_procMemo:
    doMemo = OK;
    // fall through;
  case BT_proc:
    {
      OZ_Term name = value;
      GetBTTaskArg1(frame, OZ_Term, file);
      DiscardBTFrame(frame);
      GetBTFramePtr1(frame, GName*, gname);
      GetBTFramePtr2(frame, ProgramCounter, pc);
      GetBTFrameArg3(frame, int, maybeMemoIndex);
      DiscardBTFrame(frame);
      GetBTFrameArg1(frame, int, maxX);
      GetBTFrameArg2(frame, int, line);
      GetBTFrameArg3(frame, int, column);
      DiscardBTFrame(frame);
      GetBTFrameArg1(frame, int, arity);
      GetBTFrameArg2(frame, int, gsize);
      DiscardBTFrame(frame);

      //
      Assert(gname);            // must be an unknown procedure here;
      OZ_Term procTerm;
      // kost@ : 'flags' are obviously not used (otherwise something
      // would not work: flags are not passed as e.g. 'file' is);
      PrTabEntry *pr = new PrTabEntry(name, mkTupleWidth(arity),
                                      file, line, column,
                                      oz_nil(), maxX);
      pr->PC = pc;
      pr->setGSize(gsize);
      Abstraction *pp = Abstraction::newAbstraction(pr, am.currentBoard());
      procTerm = makeTaggedConst(pp);
      pp->setGName(gname);
      addGName(gname, procTerm);

      //
      if (doMemo) {
        set(procTerm, maybeMemoIndex);
        doMemo = NO;
      }

      //
      if (gsize > 0) {
        // reverse order... and don't bother with 'spointer' tasks:
        // just issue an '_iterate' task;
        EnsureBTSpace(frame, gsize);
        PutBTTaskPtrArg(frame, BT_closureElem_iterate, pp, 0);
        for (int i = 1; i < gsize; i++) {
          PutBTTaskPtrArg(frame, BT_closureElem, pp, i);
        }
        break;                  // BT_proc:
      } else {
        value = makeTaggedConst(pp);
        GetBTTaskTypeNoDecl(frame, type);
        goto repeat;
      }

      //
      // (code area is done by the user himself;)
      Assert(0);
    }

  case BT_closureElem:
    {
      GetBTTaskPtr1(frame, Abstraction*, pp);
      GetBTTaskArg2(frame, int, ind);
      DiscardBTFrame(frame);
      pp->initG(ind, value);
      break;
    }

  case BT_closureElem_iterate:
    {
      GetBTTaskPtr1(frame, Abstraction*, pp);
      GetBTTaskArg2(frame, int, ind);
      DiscardBTFrame(frame);
      pp->initG(ind, value);
      //
      CrazyDebug(incDebugNODES(););
      value = makeTaggedConst(pp);
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  //
  // 'BT_binary' is transient here: it must be either saved or
  // discarded if it's already done;
  case BT_binary:
    {
      GetBTTaskPtr1(frame, void*, arg);
      Assert(arg == 0);
      DiscardBTFrame(frame);
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_binary_getValue:
    ReplaceBTTask1stArg(frame, BT_binary_getValueSync, value);
    break;

  case BT_binary_getValueSync:
    {
      Assert(value == (OZ_Term) 0);
      GetBTTaskArg1(frame, OZ_Term, ozValue);
      DiscardBTFrame(frame);
      GetBTFramePtr1(frame, OzValueProcessor, proc);
      GetBTFramePtr2(frame, void*, arg);
      DiscardBTFrame(frame);
      //
      (*proc)(arg, ozValue);

      //
      break;
    }

  case BT_binary_doGenAction_intermediate:
    {
      GetBTTaskPtr1(frame, BuilderGenAction, proc);
      GetBTTaskPtr2(frame, void*, arg);
      DiscardBTFrame(frame);
      //
      (*proc)(arg);

      //
      // Note that 'value' is preserved;
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_nop:
    Assert(value == (OZ_Term) 0);
    DiscardBTFrame(frame);
    break;

  default:
#ifdef ROBUST_UNMARSHALER
    RAISE_UNMARSHAL_ERROR;
#else
    OZD_error("Builder: unknown task!");
#endif
  }

  //
  SetBTFrame(frame);
}

//
BTFrame* Builder::liftTask(int sz)
{
  GetBTFrame(frame);
  BTFrame *newTop = frame + bsFrameSize*sz;
  SetBTFrame(newTop);

  //
  // Iterate until a non-'goto iterate' task is found:
 repeat:
  GetBTTaskType(frame, type);
  switch (type) {

    // single frame:
  case BT_spointer:
  case BT_makeTupleMemo:
  case BT_makeTuple:
  case BT_recordArg:
  case BT_dictKey:
  case BT_dictVal:
  case BT_fsetvalue:
  case BT_fsetvalueMemo:
  case BT_closureElem:
  case BT_nop:
    CopyBTFrame(frame ,newTop);
    break;

    // single frame iterate:
  case BT_binary:
    {
      DebugCode(GetBTTaskPtr1(frame, void*, bp););
      Assert(bp == 0);
    }
  case BT_spointer_iterate:
  case BT_buildValue:
  case BT_recordArg_iterate:
  case BT_fsetvalueMemoSync:
  case BT_fsetvalueSync:
  case BT_chunkMemo:
  case BT_chunk:
  case BT_classFeatures:
  case BT_closureElem_iterate:
  case BT_binary_doGenAction_intermediate:
    CopyBTFrame(frame, newTop);
    goto repeat;

    // two frames:
  case BT_takeRecordLabel:
  case BT_takeRecordLabelMemo:
  case BT_takeRecordArity:
  case BT_takeRecordArityMemo:
  case BT_makeRecordMemoSync:
  case BT_makeRecordSync:
  case BT_binary_getValue:
  case BT_binary_getValueSync:
    CopyBTFrame(frame, newTop);
    CopyBTFrame(frame, newTop);
    break;

    // four frames:
  case BT_procFile:
  case BT_procFileMemo:
    CopyBTFrame(frame, newTop);
    CopyBTFrame(frame, newTop);
    CopyBTFrame(frame, newTop);
    CopyBTFrame(frame, newTop);
    break;

    // four frames a'la procMemo:
  case BT_procMemo:
  case BT_proc:
    {
      CopyBTFrame(frame, newTop);
      CopyBTFrame(frame, newTop);
      CopyBTFrame(frame, newTop);
      GetBTFrameArg2(frame, int, gsize);
      CopyBTFrame(frame, newTop);
      if (gsize > 0)
        break;
      else
        goto repeat;
    }

  default:
#ifdef ROBUST_UNMARSHALER
    RAISE_UNMARSHAL_ERROR;
#else
    OZD_error("Builder: unknown task!");
#endif
  }

  //
  return (frame);
}

//
BTFrame* Builder::findBinary(BTFrame *frame)
{
  void *bp;
  //
  // Iterate until a non-'goto iterate' task is found:
 repeat:
  GetBTTaskType(frame, type);
  switch (type) {

    // found:
  case BT_binary:
    GetBTTaskPtr1NoDecl(frame, void*, bp);
    if (bp)
      break;
    // else fall through;

    // single frame:
  case BT_spointer:
  case BT_makeTupleMemo:
  case BT_makeTuple:
  case BT_recordArg:
  case BT_dictKey:
  case BT_dictVal:
  case BT_fsetvalue:
  case BT_fsetvalueMemo:
  case BT_closureElem:
  case BT_nop:
    // single frame iterate:
  case BT_spointer_iterate:
  case BT_buildValue:
  case BT_recordArg_iterate:
  case BT_fsetvalueMemoSync:
  case BT_fsetvalueSync:
  case BT_chunkMemo:
  case BT_chunk:
  case BT_classFeatures:
  case BT_closureElem_iterate:
  case BT_binary_doGenAction_intermediate:
    NextBTFrame(frame);
    goto repeat;

    // two frames:
  case BT_takeRecordLabel:
  case BT_takeRecordLabelMemo:
  case BT_takeRecordArity:
  case BT_takeRecordArityMemo:
  case BT_makeRecordMemoSync:
  case BT_makeRecordSync:
  case BT_binary_getValue:
  case BT_binary_getValueSync:
    NextBTFrame(frame);
    NextBTFrame(frame);
    goto repeat;

    // four frames:
  case BT_procFile:
  case BT_procFileMemo:
    // four frames a'la procMemo:
  case BT_procMemo:
  case BT_proc:
    NextBTFrame(frame);
    NextBTFrame(frame);
    NextBTFrame(frame);
    NextBTFrame(frame);
    goto repeat;

  default:
#ifdef ROBUST_UNMARSHALER
    RAISE_UNMARSHAL_ERROR;
#else
    OZD_error("Builder: unknown task!");
#endif
  }

  //
  return (frame);
}
