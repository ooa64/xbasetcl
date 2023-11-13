#include <assert.h>

#include "tcldbf.hpp"
#include "tclfilter.hpp"
#include "tclndx.hpp"
#include "tclmdx.hpp"

#define F_SETLK 6
#define F_SETLKW 7

#if defined(DEBUG) && !defined(TCLDBF_DEBUG)
#   undef DEBUGLOG
#   define DEBUGLOG(_x_)
#endif

#define interp tclInterp

void TclDbf::Cleanup () {
  TclCmd * o;
  while ((o = pChildren)) {
    DEBUGLOG("TclDbf::Cleanup  *" << this << " for " << o);
    delete o;
  }
};

int TclDbf::Command (int objc, struct Tcl_Obj * const objv[])
{
  static const char *commands[] = {
    "create", "drop",   "open",    "close",  "pack",     "zap",      
    "name",   "alias",  "version", "status", "schema",   "autocommit", "encoding",
    "blank",  "fields", "record",  "append", "update",   "deleted",    "commit",   "abort",
    "first",  "last",   "prev",    "next",   "position", "size",
    "index",  "filter", "expr",
    0L
  };
  enum commands {
    cmCreate, cmDrop,   cmOpen,    cmClose,  cmPack,     cmZap,
    cmName,   cmAlias,  cmVersion, cmStatus, cmSchema,   cmAutocommit, cmEncoding,
    cmBlank,  cmFields, cmRecord,  cmAppend, cmUpdate,   cmDeleted,    cmCommit,   cmAbort,
    cmFirst,  cmLast,   cmPrev,    cmNext,   cmPosition, cmSize,
    cmIndex,  cmFilter, cmExpr
  };
  int index, rc;

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "command");
    return TCL_ERROR;
  }

  if (Tcl_GetIndexFromObj(interp, objv[1],
                          (const char **)commands, "command", 0, &index) != TCL_OK) {
    return TCL_ERROR;
  }

  assert(dbf);

  switch ((enum commands)(index)) {

  case cmCreate:

    {
      Tcl_Obj * schema = NULL;
      Tcl_Obj * filename = NULL;
      Tcl_Obj * alias = NULL;
      int overlay = XB_DONTOVERLAY;
      int share = XB_SINGLE_USER;
      int i = 2;

      if (i < objc && strcmp(Tcl_GetString(objv[i]), "-overlay") == 0) {
        overlay = XB_OVERLAY;
        i++;
      }
      if (i < objc && strcmp(Tcl_GetString(objv[i]), "-share") == 0) {
        share = XB_MULTI_USER;
        i++;
      }
      if (i < objc) {
        schema = objv[i];
        i++;
      }
      if (i < objc) {
        filename = objv[i];
        i++;
      }
      if (i < objc) {
        alias = objv[i];
        i++;
      }
      if (schema == NULL || filename == NULL || i != objc) {
        Tcl_WrongNumArgs(interp, 2, objv, "?-overlay? ?-share? schema filename ?alias?");
        return TCL_ERROR;
      }
      if (Create(filename, alias, schema, overlay, share) != TCL_OK) {
        return TCL_ERROR;
      }
      if (CheckRC(dbf->xbFflush()) != TCL_OK) {
        return TCL_ERROR; 
      }
      Tcl_AppendResult(interp, EncodeTclString(dbf->GetFqFileName().Str()), NULL);
    }

    break;

  case cmDrop:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(dbf->DeleteTable()) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmOpen:

    if (objc < 3 || objc > 4) {
      Tcl_WrongNumArgs(interp, 2, objv, "filename ?alias?");
      return TCL_ERROR;
    } else {
      int rc = dbf->Close();
      if (rc == XB_NO_ERROR) {
        rc = dbf->Open(DecodeTclString(Tcl_GetString(objv[2])),
          (objc == 4) ? Tcl_GetString(objv[3]) : Tcl_GetString(objv[2]));
      }
      if (CheckRC(rc) != TCL_OK) {
        return TCL_ERROR;
      }
      Tcl_AppendResult(interp, EncodeTclString(dbf->GetFqFileName().Str()), NULL);
    }

    break;

  case cmClose:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(dbf->Close()) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmPack:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(dbf->Pack()) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmZap:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(dbf->Zap()) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmName:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      Tcl_AppendResult(interp, (const char *)dbf->GetFqFileName(), NULL);
    }

    break;

  case cmAlias:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      Tcl_AppendResult(interp, (const char *)dbf->GetTblAlias(), NULL);
    }

    break;

  case cmVersion:
    
    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      Tcl_SetObjResult(interp, Tcl_NewIntObj(dbf->GetVersion()));
    } 
    
    break;

  case cmStatus:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      int status = dbf->GetDbfStatus();
      switch (status) {
        case XB_CLOSED:  Tcl_AppendResult(interp, "closed", NULL); break;
        case XB_OPEN:    Tcl_AppendResult(interp, "open", NULL); break;
        case XB_UPDATED: Tcl_AppendResult(interp, "updated", NULL); break;
        default:         Tcl_SetObjResult(interp, Tcl_NewIntObj(status)); break;
      }
    }
    
    break;
    
  case cmSchema:

    if (objc == 2) {
      if (Schema(Tcl_GetObjResult(interp), SchemaIncludeAll) != TCL_OK) {
        return TCL_ERROR;
      }
    } else if (objc == 4) {
      if (strcmp(Tcl_GetString(objv[2]), "-fieldno") == 0) {
        int fno = dbf->GetFieldNo(DecodeTclString(Tcl_GetString(objv[3])));
        if (fno >= 0) {
          Tcl_SetObjResult(interp, Tcl_NewIntObj(fno));
        } else {
          Tcl_AppendResult(interp, "field does not exist", NULL);
          return TCL_ERROR;
        }
      } else {
        Tcl_AppendResult(interp, "invalid option ",
                         Tcl_GetString(objv[2]), NULL);
        return TCL_ERROR;
      }
    } else {
      Tcl_WrongNumArgs(interp, 2, objv, "?-fieldno fieldname?");
      return TCL_ERROR;
    }

    break;

  case cmAutocommit:
    
    if (objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?enabled?");
      return TCL_ERROR;
    } else {
      if (objc == 3) {
        int autocommit;
        if (Tcl_GetBooleanFromObj(interp, objv[2], &autocommit)) {
          return TCL_ERROR;
        }
        if (CheckRC(dbf->SetAutoCommit(autocommit)) != TCL_OK) {
          return TCL_ERROR;
        }
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(dbf->GetAutoCommit()));
    }
    
    break;

  case cmEncoding:

    if (objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?encoding?");
      return TCL_ERROR;
    }
    else if (objc == 3) {
      Tcl_Encoding e = Tcl_GetEncoding(interp, Tcl_GetString(objv[2]));
      if (e) {
        Tcl_FreeEncoding(encoding);
        encoding = e;
      } else {
        return TCL_ERROR;
      }
    }
    Tcl_AppendResult(interp, Tcl_GetEncodingName(Encoding()), NULL);
    
    break;

  case cmBlank:
    
    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(dbf->BlankRecord()) != TCL_OK) {
      return TCL_ERROR;
    }
    
    break;

  case cmFields:

    if (objc > 4) {
      Tcl_WrongNumArgs(interp, 2, objv, "?names? ?values?");
      return TCL_ERROR;
    } else if (objc == 2) {
      Tcl_Obj * result = Tcl_GetObjResult(interp);
      xbString name;
      for (int i = 0; i < dbf->GetFieldCnt(); ++i) {
        if (CheckRC(dbf->GetFieldName(i,name)) != TCL_OK) {
          return TCL_ERROR;
        } else {
          Tcl_ListObjAppendElement(NULL, result, Tcl_NewStringObj(EncodeTclString(name.Str()), -1));
        }
      }
    } else if (Fields(Tcl_GetObjResult(interp), objv[2], (objc == 4) ? objv[3] : NULL) != TCL_OK) {
      return TCL_ERROR;
    }

    break;
    
  case cmRecord:
  case cmAppend:
  case cmUpdate:
    
    if (objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?values?");
      return TCL_ERROR;
    } else if (Fields(Tcl_GetObjResult(interp), NULL, (objc == 3) ? objv[2] : NULL) != TCL_OK) {
      return TCL_ERROR;
    }
    // NOTE: xbase64-4.1.4 crashes updating unopened dbf
    if (dbf->GetDbfStatus() == XB_CLOSED && index == cmUpdate) {
      Tcl_AppendResult(interp, "Database not open (crash)", NULL);
      return TCL_ERROR;
    }
    switch ((enum commands)index) {
      case cmAppend: rc = dbf->AppendRecord(); break;
      case cmUpdate: rc = dbf->PutRecord(); break;
      default: rc = XB_NO_ERROR;
    }
    if (CheckRC(rc) != TCL_OK) {
      return TCL_ERROR;
    }
    
    break;
    
  case cmCommit:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      dbf->Commit();
    }

    break;

  case cmAbort:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      dbf->Abort();
    }

    break;

  case cmDeleted:
    
    if (objc < 2 || objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?deleted?");
      return TCL_ERROR;
    } else {
      if (objc == 3) {
        int deleted;
        if (Tcl_GetBooleanFromObj(interp, objv[2], &deleted)) {
          return TCL_ERROR;
        }
        if (deleted) {
          if (CheckRC(dbf->DeleteRecord()) != TCL_OK) {
            return TCL_ERROR;
          }
        } else {
          if (CheckRC(dbf->UndeleteRecord()) != TCL_OK) {
            return TCL_ERROR;
          }
        }
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(dbf->RecordDeleted()));
    }
    
    break;

  case cmFirst:
  case cmLast:
  case cmPrev:
  case cmNext:

    if (objc < 2 || objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?-all|-deleted?");
      return TCL_ERROR;
    } else {
      int option = XB_ACTIVE_RECS;
      if (objc == 3) {
        if (strcmp(Tcl_GetString(objv[2]), "-all") == 0)
          option = XB_ALL_RECS;
        else if (strcmp(Tcl_GetString(objv[2]), "-deleted") == 0)
          option = XB_DELETED_RECS;
      }
      switch ((enum commands)index) {
        case cmFirst: rc = dbf->GetFirstRecord(option); break;
        case cmLast: rc = dbf->GetLastRecord(option); break;
        case cmPrev: rc = dbf->GetPrevRecord(option); break;
        case cmNext: rc = dbf->GetNextRecord(option); break;
        default: rc = XB_NO_ERROR;
      }
      if (rc == XB_EOF || rc == XB_BOF || rc == XB_EMPTY) {
        Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
      } else if (CheckRC(rc) != TCL_OK) {
        return TCL_ERROR;
      } else {
        Tcl_SetObjResult(interp, Tcl_NewLongObj(dbf->GetCurRecNo()));
      }
    }

    break;

    case cmPosition:

    if (objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?position?");
      return TCL_ERROR;
    } else {
      if (objc == 3) {
        long position;
        if (Tcl_GetLongFromObj(interp, objv[2], &position)) {
          return TCL_ERROR;
        }
        if (CheckRC(dbf->GetRecord(position)) != TCL_OK) {
          return TCL_ERROR;
        }
      }
      Tcl_SetObjResult(interp, Tcl_NewLongObj(dbf->GetCurRecNo()));
    }

    break;

  case cmSize:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      unsigned int count;
      if (CheckRC(dbf->GetRecordCnt(count)) != TCL_OK) {
        return TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewLongObj(count));
    }

    break;

  case cmExpr:

    if (objc != 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "expression");
      return TCL_ERROR;
    } else {
      // TODO: use expressions cache
      xbExp exp(XBase());
      if (CheckRC(exp.ParseExpression(dbf, Tcl_GetString(objv[2]))) != TCL_OK) {
        return TCL_ERROR;
      } else if (CheckRC(exp.ProcessExpression()) != TCL_OK) {
        return TCL_ERROR;
      } else {
        switch (exp.GetReturnType()) {
          case XB_EXP_LOGICAL: {
            xbBool value;
            exp.GetBoolResult(value);
            Tcl_SetObjResult(interp, Tcl_NewIntObj(value));
            break;
          }
          case XB_EXP_NUMERIC: {
            xbDouble value;
            exp.GetNumericResult(value);
            Tcl_SetObjResult(interp, Tcl_NewDoubleObj(value));
            break;
          }
          case XB_EXP_DATE: {
            xbDate value;
            exp.GetDateResult(value);
            Tcl_AppendResult(interp, value.Str(), NULL);
            break;
          }
          default: { // XB_EXP_CHAR
            xbString value;
            exp.GetStringResult(value);
            Tcl_AppendResult(interp, value.Str(), NULL);
          }
        }
      }
    }

    break;    

  case cmFilter:
    
    if (objc != 4) {
      Tcl_WrongNumArgs(interp, 2, objv, "name expression");
      return TCL_ERROR;
    }
    (void) new TclFilter(interp, Tcl_GetString(objv[2]), Tcl_GetString(objv[3]), this);
    Tcl_SetObjResult(interp, objv[2]);

    break;

  case cmIndex:

    return Index(objc, objv);
    
  default:

    return TCL_ERROR;

  } // switch index
  
  return TCL_OK;
}

int TclDbf::Index (int objc, struct Tcl_Obj * const objv[]) {
  static const char *const commands[] = {
    "create", "drop", "open", "close", "check", "reindex",
    "files",  "tags", "file", "tag",   "info",
    "first",  "last", "prev", "next",  "find",  0L
  };
  enum commands {
    cmCreate, cmDrop, cmOpen, cmClose, cmCheck, cmReindex,
    cmFiles,  cmTags, cmFile, cmTag,   cmInfo,
    cmFirst,  cmLast, cmPrev, cmNext,  cmFind
  };
  int index;

  if (objc < 3) {
    Tcl_WrongNumArgs(interp, 2, objv, "command");
    return TCL_ERROR;
  }
  
  if (Tcl_GetIndexFromObj(interp, objv[2], 
      (const char **)commands, "command", 0, &index) != TCL_OK) {
    return TCL_ERROR;
  }

  switch ((enum commands)(index)) {

    case cmCreate:

      // obj index create ?-ndx|-mdx? ?-filter filter? ?-unique? ?-overlay? ?-descending? indexname expression
      if (objc < 5) {
        Tcl_WrongNumArgs(interp, 3, objv, "?-ndx|-mdx? ?-filter filter? ?-unique? ?-overlay? ?-descending? expression indexname");
        return TCL_ERROR;
      } else {
        static const char *const options[] = {
          "-ndx", "-mdx", "-filter", "-unique", "-overlay", "-descending", NULL
        };
        enum options {
          optNdx, optMdx, optFilter, optUnique, optOverlay, optDescending
        };
        int descending = 0, unique = 0, overlay = 0;
        const char * ixtype = "MDX";
        Tcl_DString ixname;
        Tcl_DString filter;
        Tcl_DString expr;

        Tcl_DStringInit(&ixname);
        Tcl_DStringInit(&filter);
        Tcl_DStringInit(&expr);

        for (int i = 3; i < objc-2; i++) {
          int option;
          if (Tcl_GetIndexFromObj(interp, objv[i], options, "option", 0, &option) != TCL_OK) {
              return TCL_ERROR;
          }
          switch ((enum options) option) {
            case optNdx:
              ixtype = "NDX";
              break;
            case optMdx:
              ixtype = "MDX";
              break;
            case optFilter:
              if (++i < objc-2) {
                Tcl_UtfToExternalDString(Encoding(), Tcl_GetString(objv[i]), -1, &filter);
              } else {
                Tcl_AppendResult(interp, "missing filter expression", NULL);
                return TCL_ERROR;
              }
              break;
            case optUnique:
              unique = 1;
              break;
            case optOverlay:
              overlay = 1;
              break;
            case optDescending:
              descending = 1;
              break;
            default:
              return TCL_ERROR;
          }
        }

        Tcl_UtfToExternalDString(Encoding(), Tcl_GetString(objv[objc - 1]), -1, &ixname);
        Tcl_UtfToExternalDString(Encoding(), Tcl_GetString(objv[objc - 2]), -1, &expr);

        xbIx *ix = NULL;
        void *tag = NULL;
        int rc = dbf->CreateTag(ixtype, Tcl_DStringValue(&ixname), Tcl_DStringValue(&expr), Tcl_DStringValue(&filter),
          descending, unique, overlay, &ix, &tag);
        
        Tcl_DStringInit(&expr);
        Tcl_DStringInit(&filter);
        Tcl_DStringFree(&ixname);
        
        if (CheckRC(rc) != TCL_OK) {
          return TCL_ERROR;
        }

        Tcl_AppendResult(interp, (const char *)dbf->GetCurIx()->GetFqFileName().Str(), NULL);
      }
      break;

    case cmDrop:

      // obj index drop ?-ndx|-mdx? ixname
      if (objc < 4 || objc > 5) {
        Tcl_WrongNumArgs(interp, 3, objv, "?-ndx|-mdx? indexname");
        return TCL_ERROR;
      } else {
        const char * ixtype = "MDX";
        if (objc == 5) {
          if (strcmp(Tcl_GetString(objv[3]), "-ndx") == 0) {
            ixtype = "NDX";
          } else if (strcmp(Tcl_GetString(objv[3]), "-mdx") != 0) {
            Tcl_AppendResult(interp, "invalid index type", NULL);
            return TCL_ERROR;
          }
        }
        if (CheckRC(dbf->DeleteTag(ixtype, DecodeTclString(Tcl_GetString(objv[3])))) != TCL_OK) {
          return TCL_ERROR;
        }
      }
      break;

    case cmOpen:

      // obj index open ?-ndx|-mdx? ixname
      if (objc < 4 || objc > 5) {
        Tcl_WrongNumArgs(interp, 3, objv, "?-ndx|-mdx? indexname");
        return TCL_ERROR;
      } else {
        const char * ixtype = "MDX";
        if (objc == 5) {
          if (strcmp(Tcl_GetString(objv[3]), "-ndx") == 0) {
            ixtype = "NDX";
          } else if (strcmp(Tcl_GetString(objv[3]), "-mdx") != 0) {
            Tcl_AppendResult(interp, "invalid index type", NULL);
            return TCL_ERROR;
          }
        }
        if (CheckRC(dbf->OpenIndex(ixtype, DecodeTclString(Tcl_GetString(objv[objc - 1])))) != TCL_OK) {
          return TCL_ERROR;
        }
        Tcl_AppendResult(interp, (const char *)dbf->GetCurIx()->GetFqFileName().Str(), NULL);
      }
      break;

    case cmClose:

      // obj index close ixname
      if (objc != 4) {
        Tcl_WrongNumArgs(interp, 3, objv, "indexname");
        return TCL_ERROR;
      } else {
        const char * ixname = Tcl_GetString(objv[3]);
        xbIxList * node = dbf->GetIxList();
        while (node) {
          xbIx *ix = node->ix;
          if (strcmp(ixname, EncodeTclString(ix->GetFqFileName().Str())) == 0) {
            return CheckRC(dbf->CloseIndexFile(ix));
          }
          node = node->next;
        }
        Tcl_AppendResult(interp, "unknown index name", NULL);
        return TCL_ERROR;
      }
      break;

    case cmCheck:

      // obj index check ?-all?
      if (objc > 4) {
        Tcl_WrongNumArgs(interp, 3, objv, "?-all?");
        return TCL_ERROR;
      } else {
        int opt = 0;
        if (objc == 4) {
          opt = 1;
          if (strcmp(Tcl_GetString(objv[3]), "-all") != 0) {
            Tcl_AppendResult(interp, "invalid option", NULL);
            return TCL_ERROR;
          }
        }
        if (CheckRC(dbf->CheckTagIntegrity(opt, 1)) != TCL_OK) {
          return TCL_ERROR;
        }
      }
      break;

    case cmReindex:

      // obj index reindex ?-all?
      if (objc > 4) {
        Tcl_WrongNumArgs(interp, 3, objv, "?-all?");
        return TCL_ERROR;
      } else {
        int opt = 0;
        if (objc == 4) {
          opt = 1;
          if (strcmp(Tcl_GetString(objv[3]), "-all") != 0) {
            Tcl_AppendResult(interp, "invalid option", NULL);
            return TCL_ERROR;
          }
        }
        xbIx * ix = NULL;
        void * tag = NULL;
        if (CheckRC(dbf->Reindex(opt, &ix, &tag)) != TCL_OK) {
          return TCL_ERROR;
        }
      }
      break;

    case cmFiles:

      if (objc != 3) {
        Tcl_WrongNumArgs(interp, 3, objv, NULL);
        return TCL_ERROR;
      } else {
        Tcl_Obj * list = Tcl_NewObj();
        xbIxList *node = dbf->GetIxList();
        while (node) {
          xbIx * ix = node->ix;
          Tcl_ListObjAppendElement(NULL, list,
            Tcl_NewStringObj(EncodeTclString(ix->GetFqFileName().Str()), -1));
          node = node->next;
        }
        Tcl_SetObjResult(interp, list);
      }
      break;

    case cmTags:

      if (objc != 3) {
        Tcl_WrongNumArgs(interp, 3, objv, NULL);
        return TCL_ERROR;
      } else {
        Tcl_Obj * list = Tcl_NewObj();
        xbLinkListNode<xbTag *> * node = dbf->GetTagList();
        while (node) {
          xbTag * tag = node->GetKey();
          Tcl_ListObjAppendElement(NULL, list,
              Tcl_NewStringObj(EncodeTclString(tag->GetTagName().Str()), -1));
          node = node->GetNextNode();
        }
        Tcl_SetObjResult(interp, list);
      }
      break;

    case cmInfo:

      if (objc != 3) {
        Tcl_WrongNumArgs(interp, 3, objv, NULL);
        return TCL_ERROR;
      } else {
        xbIx * ix = dbf->GetCurIx();
        void * tag = dbf->GetCurTag();
        if (ix != NULL && tag != NULL) {
          Tcl_Obj * list = Tcl_NewObj();
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewStringObj("type", -1));
          // Tcl_ListObjAppendElement(NULL, list, Tcl_NewStringObj(dynamic_cast<xbIxNdx *>(ix) ? "NDX" : "MDX", -1));
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewStringObj(dbf->GetCurIxType().Str(), -1));
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewStringObj("file", -1));
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewStringObj(EncodeTclString(ix->GetFqFileName().Str()), -1));
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewStringObj("tag", -1));
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewStringObj(EncodeTclString(ix->GetTagName(tag).Str()), -1));
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewStringObj("expression", -1));
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewStringObj(EncodeTclString(ix->GetKeyExpression(tag).Str()), -1));
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewStringObj("filter", -1));
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewStringObj(EncodeTclString(ix->GetKeyFilter(tag).Str()), -1));
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewStringObj("unique", -1));
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewIntObj(ix->GetUnique(tag)));
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewStringObj("descending", -1));
          Tcl_ListObjAppendElement(NULL, list, Tcl_NewIntObj(ix->GetSortOrder(tag)));
          Tcl_SetObjResult(interp, list);
        } else {
          Tcl_AppendResult(interp, "undefined index or tag", NULL);
          return TCL_ERROR;
        }
      }
      break;

    case cmTag:

      if (objc > 4) {
        Tcl_WrongNumArgs(interp, 3, objv, "?tag?");
        return TCL_ERROR;
      } else {
        if (objc == 4) {
          if (CheckRC(dbf->SetCurTag(Tcl_GetString(objv[3]))) != TCL_OK) {
            return TCL_ERROR;
          }
        }
        Tcl_AppendResult(interp, EncodeTclString(dbf->GetCurTagName().Str()), NULL);
      }
      break;

    case cmFirst:
    case cmLast:
    case cmPrev:
    case cmNext:

      if (objc != 3) {
        Tcl_WrongNumArgs(interp, 3, objv, NULL);
        return TCL_ERROR;
      } else {
        int rc = XB_INVALID_OPTION;
        switch ((enum commands)(index)) {
          case cmFirst: rc = dbf->GetFirstKey(); break;
          case cmLast: rc = dbf->GetLastKey(); break;
          case cmPrev: rc = dbf->GetPrevKey(); break;
          case cmNext: rc = dbf->GetNextKey(); break;
        }
        if (rc == XB_BOF || rc == XB_EOF) {
          Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
        } else if (CheckRC(rc) != TCL_OK) {
          return TCL_ERROR;
        } else if (CheckRC(dbf->GetRecord(dbf->GetCurRecNo())) != TCL_OK) {
          return TCL_ERROR;
        } else {
          Tcl_SetObjResult(interp, Tcl_NewIntObj(dbf->GetCurRecNo()));
        }
      }
      break;

    case cmFind:

     if (objc != 4) {
        Tcl_WrongNumArgs(interp, 4, objv, "value");
        return TCL_ERROR;
      } else {
        xbIx * ix = dbf->GetCurIx();
        void * tag = dbf->GetCurTag();
        if (ix != NULL && tag != NULL) {
          int rc = XB_INVALID_OPTION;
          switch (ix->GetKeyType(tag)) {
            case 'F':
            case 'N':
              {
                double value;
                if (Tcl_GetDouble(interp, Tcl_GetString(objv[3]), &value) != TCL_OK) {
                  return TCL_ERROR;
                }
                rc = dbf->Find(value);
              }
              break;
            case 'D':
              {
                xbDate value = Tcl_GetString(objv[3]);
                if (value.Str() == "") {
                  Tcl_AppendResult(interp, "expected date as YYYYMMDD but got \"", Tcl_GetString(objv[3]), "\"", NULL);
                  return TCL_ERROR;
                }
                rc = dbf->Find(value);
              }
              break;
            default:
              {
                xbString value = DecodeTclString(Tcl_GetString(objv[3]));
                rc = dbf->Find(value);
              }
          }
          if (rc == XB_NOT_FOUND) {
            Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
          } else if (CheckRC(rc) != TCL_OK) {
            return TCL_ERROR;
          } else {
            Tcl_SetObjResult(interp, Tcl_NewIntObj(dbf->GetCurRecNo()));
          }
        }
      }
      break;

    default:

      return TCL_ERROR;

  } // switch index

  return TCL_OK;
}

int TclDbf::Create (Tcl_Obj * filename, Tcl_Obj * alias, Tcl_Obj * schema, int overlay, int share)
{
  int fieldc;

  if (Tcl_ListObjLength(interp, schema, &fieldc) != TCL_OK) {
    return TCL_ERROR;
  } else if (fieldc <= 0) {
    Tcl_AppendResult(interp, "empty schema", NULL);
    return TCL_ERROR;
  } else {
    xbSchema * xschema = (xbSchema *)Tcl_Alloc(sizeof(xbSchema)*(fieldc+1));
    for (int i = 0; i < fieldc; ++i) {
      Tcl_Obj * field;
      Tcl_Obj * param;
      int paramc;
      if (Tcl_ListObjIndex(interp, schema, i, &field) != TCL_OK) {
        Tcl_Free((char *)xschema);
        return TCL_ERROR;
      }
      if (Tcl_ListObjLength(interp, field, &paramc) != TCL_OK) {
        Tcl_Free((char *)xschema);
        return TCL_ERROR;
      } else if (paramc != 4) {
        Tcl_Free((char *)xschema);
        Tcl_AppendResult(interp, "invalid schema, field #", NULL);
        Tcl_AppendObjToObj(Tcl_GetObjResult(interp), Tcl_NewIntObj(i));
        return TCL_ERROR;
      }
      char * fname;
      char * ftype;
      int flen;
      int fdec;
      if ((Tcl_ListObjIndex(NULL, field, 0, &param) != TCL_OK)
          || (param == NULL)
          || (strlen(fname = Tcl_GetString(param)) == 0)) {
        Tcl_Free((char *)xschema);
        Tcl_AppendResult(interp, "invalid fname, field #", NULL);
        Tcl_AppendObjToObj(Tcl_GetObjResult(interp), Tcl_NewIntObj(i));
        return TCL_ERROR;
      }
      if ((Tcl_ListObjIndex(NULL, field, 1, &param) != TCL_OK)
          || (param == NULL)
          || (strlen(ftype = Tcl_GetString(param)) != 1)) {
        Tcl_Free((char *)xschema);
        Tcl_AppendResult(interp, "invalid ftype, field #", NULL);
        Tcl_AppendObjToObj(Tcl_GetObjResult(interp), Tcl_NewIntObj(i));
        return TCL_ERROR;
      }
      if ((Tcl_ListObjIndex(NULL, field, 2, &param) != TCL_OK)
          || (param == NULL)
          || (Tcl_GetIntFromObj(NULL, param, &flen) != TCL_OK)
          || (flen < 0)
          || (flen > 255)) {
        Tcl_Free((char *)xschema);
        Tcl_AppendResult(interp, "invalid flen, field #", NULL);
        Tcl_AppendObjToObj(Tcl_GetObjResult(interp), Tcl_NewIntObj(i));
        return TCL_ERROR;
      }
      if ((Tcl_ListObjIndex(NULL, field, 3, &param) != TCL_OK)
          || (param == NULL)
          || (Tcl_GetIntFromObj(NULL, param, &fdec) != TCL_OK)
          || (fdec < 0)
          || (fdec > 255)) {
        Tcl_Free((char *)xschema);
        Tcl_AppendResult(interp, "invalid fdec, field #", NULL);
        Tcl_AppendObjToObj(Tcl_GetObjResult(interp), Tcl_NewIntObj(i));
        return TCL_ERROR;
      }
      strncpy(xschema[i].cFieldName, DecodeTclString(fname), MaxFieldNameLength);
      xschema[i].cType = ftype[0];
      xschema[i].iFieldLen = (unsigned char)flen;
      xschema[i].iNoOfDecs = (unsigned char)fdec;
      DEBUGLOG("CreateTable "     << \
               Tcl_GetString(filename) << " #" << i << " = " << \
               (xschema[i].cFieldName)        << ","  << \
               (xschema[i].cType)             << ","  << \
               (int)(xschema[i].iFieldLen)    << ","  << \
               (int)(xschema[i].iNoOfDecs));
    }
    memset(&(xschema[fieldc]), 0, sizeof(xbSchema));
    {
      int rc = dbf->Close();
      if (rc == XB_NO_ERROR) {
        rc = dbf->CreateTable(DecodeTclString(Tcl_GetString(filename)),
          alias ? Tcl_GetString(alias) : NULL, xschema, overlay, share);
      }
      if (CheckRC(rc) != TCL_OK) {
        Tcl_Free((char *)xschema);
        return TCL_ERROR;
      }
    }

    Tcl_Free((char *)xschema);
  }
  return TCL_OK;
}

int TclDbf::Schema (Tcl_Obj * result, unsigned include)
{
  xbString fname;
  char ftypech;
  short flen;
  short fdec;
  for (int i = 0; i < dbf->GetFieldCnt(); ++i) {
    if (CheckRC(dbf->GetFieldName(i,fname)) != TCL_OK) {
      return TCL_ERROR;
    }
    if (CheckRC(dbf->GetFieldType(i,ftypech)) != TCL_OK) {
      return TCL_ERROR;
    }
    if (CheckRC(dbf->GetFieldLen(i,flen)) != TCL_OK) {
      return TCL_ERROR;
    }
    if (CheckRC(dbf->GetFieldDecimal(i,fdec)) != TCL_OK) {
      return TCL_ERROR;
    }
    
    Tcl_Obj * field = Tcl_NewObj();

    if (include & SchemaIncludeName)
      Tcl_ListObjAppendElement(NULL, field,
                               Tcl_NewStringObj(EncodeTclString(fname.Str()), -1));
    if (include & SchemaIncludeType) 
      Tcl_ListObjAppendElement(NULL, field, Tcl_NewStringObj(&ftypech, 1));
    if (include & SchemaIncludeLen) 
      Tcl_ListObjAppendElement(NULL, field, Tcl_NewIntObj(flen));
    if (include & SchemaIncludeDec) 
      Tcl_ListObjAppendElement(NULL, field, Tcl_NewIntObj(fdec));
    
    Tcl_ListObjAppendElement(NULL, result, field);
  }
  return TCL_OK;
}

int TclDbf::Fields (Tcl_Obj * result, Tcl_Obj * namev, Tcl_Obj * valuev)
{
  int namec;

  if (namev == NULL) {
    namec = dbf->GetFieldCnt();
  } else if (Tcl_ListObjLength(interp, namev, &namec) != TCL_OK) {
    return TCL_ERROR;
  };


  for (int i = 0; i < namec; i++) {
    Tcl_Obj * name;
    Tcl_Obj * value;
    int fno;

    if (namev == NULL) {
      fno = i;
    } else {
      if (Tcl_ListObjIndex(interp, namev, i, &name) != TCL_OK) {
        return TCL_ERROR;
      } 
      fno = dbf->GetFieldNo(DecodeTclString(Tcl_GetString(name)));
      if (fno < 0) {
        Tcl_ResetResult(interp);
        Tcl_AppendResult(interp, "invalid field name ", Tcl_GetString(name), NULL);
        return TCL_ERROR;
      }
    }

    if (valuev != NULL) {
      if (Tcl_ListObjIndex(interp, valuev, i, &value) != TCL_OK) {
        return TCL_ERROR;
      }
      if (value != NULL) {
        int rc;
        char ftypech;
        if (CheckRC(dbf->GetFieldType(fno,ftypech)) != TCL_OK) {
          return TCL_ERROR;
        }
        switch (ftypech) {
        case 'M':
          {
            const char * s = DecodeTclString(Tcl_GetString(value));
            if (strlen(s)) {
              DEBUGLOG("UpdateMemo " << dbf->GetFileName().Str() << " #"  << fno << " = " << \
                       strlen(s) << " chars: " << s);
              rc = dbf->UpdateMemoField(fno, s);
            } else {
              DEBUGLOG("DeleteMemo " << dbf->GetFileName().Str() << " #" << fno);
              rc = dbf->UpdateMemoField(fno, "");
            }
          }
          break;
        case 'L':
          {
            int b;
            if (Tcl_GetBooleanFromObj(NULL, value, &b) == TCL_OK) {
              rc = dbf->PutLogicalField(fno, b ? "T" : "F");
            } else {
              rc = dbf->PutLogicalField(fno, Tcl_GetString(value));
            }
          }
          break;
        case 'C':
          rc = dbf->PutField(fno, DecodeTclString(Tcl_GetString(value)));
          break;
        default:
          rc = dbf->PutField(fno, Tcl_GetString(value));
        }
        if (CheckRC(rc)) {
          Tcl_Obj * result = Tcl_GetObjResult(interp);
          Tcl_AppendObjToObj(result, Tcl_NewStringObj(" at value #", -1));
          Tcl_AppendObjToObj(result, Tcl_NewIntObj(i));
          return TCL_ERROR;
        }
      }
    }

    Tcl_Obj * obj;

    char ftypech;
    if (CheckRC(dbf->GetFieldType(fno, ftypech)) != TCL_OK) {
      return TCL_ERROR;
    }
    switch (ftypech) {
    case 'M':
      if (dbf->MemoFieldExists(fno)) {
        xbString buf;
        if (CheckRC(dbf->GetMemoField(fno, buf)) != TCL_OK) {
          return TCL_ERROR;
        }
        int flen = buf.GetSize() > 0 ? buf.GetSize() - 1 : 0;
        obj = Tcl_NewStringObj(EncodeTclString(buf.Str()), flen);
#if DEBUG        
        // buf[flen] = 0; ???
        DEBUGLOG("GetMemo " << dbf->GetFileName().Str() << " #" << fno << " = " << \
                 flen << " chars: " << buf.Str());
#endif
      } else {
        obj = Tcl_NewStringObj(NULL, 0);
      }
      break;
      //case 'L':
      //obj = Tcl_NewIntObj(dbf->GetLogicalField(fno));
      //obj = Tcl_NewStringObj(dbf->GetField(fno), -1);
      //break;
    case 'C':
      //{
      //    xbString s = dbf->GetStringField(fno); 
      //    while (s[0] && s[0] == ' ') s.remove(0, 1);
      //    s.trim();
      //    obj = Tcl_NewStringObj(EncodeTclString(s), -1);
      //}
      //obj = Tcl_NewStringObj(EncodeTclString(dbf->GetStringField(fno)), -1);
      //obj = Tcl_NewStringObj(EncodeTclString(dbf->GetField(fno)), -1);
        {
          xbString s;
          if (CheckRC(dbf->GetField(fno, s)) != TCL_OK) {
            return TCL_ERROR;
          }
          // if (s[0] == ' ' && s[1] == 0) {
          //   // workaround to the GetField() feature
          //   obj = Tcl_NewStringObj(NULL, 0);
          // } else {
          //   obj = Tcl_NewStringObj(EncodeTclString(s), -1);
          // }
          obj = Tcl_NewStringObj(EncodeTclString(s.Trim()), -1);
        }
        break;
      //case 'N':
      //case 'F':
      //if (dbf->GetFieldDecimal(fno) == 0 && dbf->GetFieldDecimal(fno) < 10) {
      //    obj = Tcl_NewLongObj(dbf->GetLongField(fno));
      //} else {
      //    obj = Tcl_NewDoubleObj(dbf->GetDoubleField(fno));
      //}
      //{
      //  const char * s = dbf->GetField(fno); 
      //  while (s && *s == ' ') s++; 
      //  obj = Tcl_NewStringObj(s, -1);
      // }
      // break;
    default:
      // obj = Tcl_NewStringObj(dbf->GetField(fno), -1);
      {
        xbString s;
        if (CheckRC(dbf->GetField(fno, s)) != TCL_OK) {
          return TCL_ERROR;
        }
        obj = Tcl_NewStringObj(s.Ltrim(), -1);
      }
    }
    if (Tcl_ListObjAppendElement(interp, result, obj) != TCL_OK) {
      Tcl_Obj * obj = Tcl_GetObjResult(interp);
      Tcl_AppendObjToObj(obj, Tcl_NewStringObj(" at value #", -1));
      Tcl_AppendObjToObj(obj, Tcl_NewIntObj(i));
      return TCL_ERROR;
    }
  }
  
  return TCL_OK;
}

