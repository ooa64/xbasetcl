// $Id$

#include "tclxbase.hpp"
#include "tcldbf3.hpp"
#include "tcldbf4.hpp"

#if defined(DEBUG) && !defined(TCLXBASE_DEBUG)
#   undef DEBUGLOG
#   define DEBUGLOG(_x_)
#endif

#define interp tclInterp

void TclXbase::Cleanup () {
  TclCmd * o;
  while ((o = pChildren)) {
    DEBUGLOG("TclXbase::Cleanup  *" << this << " for " << o);
    delete o;
  }
};

int TclXbase::Command (int objc, struct Tcl_Obj * CONST objv[])
{
  static CONST char *commands[] = {
    "version", "dateformat", "encoding", "log", "list", "dbf", "open", 0L
  };
  enum commands {
    cmVersion, cmDateformat, cmEncoding, cmLog, cmList, cmDbf, cmOpen
  };
  int index;

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "command");
    return TCL_ERROR;
  }
  
  if (Tcl_GetIndexFromObj(interp, objv[1], 
    (CONST char **)commands, "command", 0, &index) != TCL_OK) {
    return TCL_ERROR;
  }

  switch ((enum commands)(index)) {

  case cmVersion:

    if (objc > 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    }
    Tcl_AppendResult(interp, XBASE_VERSION, NULL);

    break;

  case cmDateformat:

    if (objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?format?");
      return TCL_ERROR;
    } else if (objc == 3) {
      xbase->SetDefaultDateFormat(Tcl_GetString(objv[2]));
    }
    Tcl_AppendResult(interp, (const char *)xbase->GetDefaultDateFormat(), NULL);

    break;

  case cmEncoding:

    if (objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?encoding?");
      return TCL_ERROR;
    } else if (objc == 3) {
      Tcl_Encoding e = Tcl_GetEncoding(interp, Tcl_GetString(objv[2]));
      if (e) {
        Tcl_FreeEncoding(encoding);
        encoding = e;
      } else {
        return TCL_ERROR;
      }
    }
    Tcl_AppendResult(interp, Tcl_GetEncodingName(encoding), NULL);

    break;

  case cmLog:

    return Log(objc, objv);

    break;
    
  case cmList:

    if (objc > 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      Tcl_Obj * result = Tcl_GetObjResult(interp);
      for (int i = 1; i <= xbase->GetOpenTableCount(); ++i) {
        xbDbf * dbf = xbase->GetDbfPtr(i);
        if (dbf) {
          Tcl_ListObjAppendElement(NULL, result, 
              Tcl_NewStringObj((const char *)dbf->GetFqFileName(), -1));
        }
      }
    }

    break;

  case cmDbf:

    if (objc < 3 || objc > 4) {
      Tcl_WrongNumArgs(interp, 2, objv, "?-dbf3|-dbf4? name");
      return TCL_ERROR;
    } else if (objc > 3) {
      if (strcmp(Tcl_GetString(objv[2]), "-dbf3") == 0) {
        (void) new TclDbf3(interp, Tcl_GetString(objv[3]), this);
      } else if (strcmp(Tcl_GetString(objv[2]), "-dbf4") == 0) {
        (void) new TclDbf4(interp, Tcl_GetString(objv[3]), this);
      } else {
        Tcl_AppendResult(interp, "bad option ", Tcl_GetString(objv[2]), NULL);
        return TCL_ERROR;
      }
    } else {
      (void) new TclDbf4(interp, Tcl_GetString(objv[2]), this);
    }
    Tcl_SetObjResult(interp, objv[objc - 1]);

    break;

  case cmOpen:

    if (objc < 4 || objc > 5) {
      Tcl_WrongNumArgs(interp, 2, objv, "filename name ?alias?");
      return TCL_ERROR;
    } else {
      xbDbf * dbf;
      if (CheckRC(xbase->OpenHighestVersion(Tcl_GetString(objv[2]), 
          (objc == 5) ? Tcl_GetString(objv[4]) : Tcl_GetString(objv[2]), &dbf)) != TCL_OK) {
        return TCL_ERROR;
      }
      DEBUGLOG("TclXbase cmOpen " << Tcl_GetString(objv[2]) << " as " << dbf->GetVersion());
      if (dynamic_cast<xbDbf3*>(dbf)) {
        (void) new TclDbf3(interp, Tcl_GetString(objv[3]), this, dynamic_cast<xbDbf3*>(dbf));
      } else if (dynamic_cast<xbDbf4*>(dbf)) {
        (void) new TclDbf4(interp, Tcl_GetString(objv[3]), this, dynamic_cast<xbDbf4*>(dbf));
      } else {
        delete dbf;
        Tcl_AppendResult(interp, "invalid file version ", Tcl_NewIntObj(dbf->GetVersion()), NULL);
        return TCL_ERROR;
      }
      Tcl_SetObjResult(interp, objv[3]);
    }

    break;

  } // switch index

  return TCL_OK;
}

int TclXbase::Log (int objc, struct Tcl_Obj * CONST objv[])
{
  static CONST char *commands[] = {
    "directory", "name", "size", "enabled", "string", "bytes", "flush", 0L
  };
  enum commands {
    cmDirectory, cmName, cmSize, cmEnabled, cmString, cmBytes, cmFlush
  };
  int index;

  if (objc < 3) {
    Tcl_WrongNumArgs(interp, 2, objv, "command");
    return TCL_ERROR;
  }
  
  if (Tcl_GetIndexFromObj(interp, objv[2], 
                          (CONST char **)commands, "command", 0, &index) != TCL_OK) {
    return TCL_ERROR;
  }

  switch ((enum commands)(index)) {

    case cmDirectory:
      if (objc > 4) {
        Tcl_WrongNumArgs(interp, 3, objv, "?directory?");
        return TCL_ERROR;
      }
      if (objc == 4) {
        xbase->SetLogDirectory(Tcl_GetString(objv[3]));
      }
      Tcl_AppendResult(interp, xbase->GetLogDirectory().Str(), NULL);
      break;

    case cmName:
      if (objc > 4) {
        Tcl_WrongNumArgs(interp, 3, objv, "?filename?");
        return TCL_ERROR;
      }
      if (objc == 4) {
        xbase->SetLogFileName(Tcl_GetString(objv[3]));
      }
      Tcl_AppendResult(interp, xbase->GetLogFileName().Str(), NULL);
      break;

    case cmSize:
      if (objc > 4) {
        Tcl_WrongNumArgs(interp, 3, objv, "?size?");
        return TCL_ERROR;
      }
      if (objc == 4) {
        long size;
        if (Tcl_GetLongFromObj(interp, objv[3], &size)) {
          return TCL_ERROR;
        }
        xbase->SetLogSize(size);
      }
      Tcl_SetObjResult(interp, Tcl_NewLongObj(xbase->GetLogSize()));
      break;

    case cmEnabled:
      if (objc > 4) {
        Tcl_WrongNumArgs(interp, 3, objv, "?enabled?");
        return TCL_ERROR;
      }
      if (objc == 4) {
        int enabled;
        if (Tcl_GetBooleanFromObj(interp, objv[3], &enabled)) {
          return TCL_ERROR;
        }
        if (enabled)
          xbase->EnableMsgLogging();
        else
          xbase->DisableMsgLogging();
      }
      Tcl_SetObjResult(interp, Tcl_NewLongObj(xbase->GetLogStatus()));
      break;

    case cmString:
      if (objc != 4) {
        Tcl_WrongNumArgs(interp, 3, objv, "message");
        return TCL_ERROR;
      }
      xbase->WriteLogMessage(Tcl_GetString(objv[3]));
      break;

    case cmBytes:
      if (objc != 4) {
        Tcl_WrongNumArgs(interp, 3, objv, "bytes");
        return TCL_ERROR;
      }
      {
        int length;
        const unsigned char * bytes = Tcl_GetByteArrayFromObj(objv[3], &length);
        xbase->WriteLogBytes((unsigned)length, (const char *)bytes);
      }
      break;

    case cmFlush:
      if (objc != 3) {
        Tcl_WrongNumArgs(interp, 3, objv, NULL);
        return TCL_ERROR;
      }
      xbase->FlushLog();
      break;

    default:
      return TCL_ERROR;

  } // switch index

  return TCL_OK;
}