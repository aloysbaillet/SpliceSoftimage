#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_menu.h>
#include <xsi_model.h>
#include <xsi_customoperator.h>
#include <xsi_x3dobject.h>
#include <xsi_uitoolkit.h>
#include <xsi_null.h>
#include <xsi_vector3.h>
#include <xsi_selection.h>
#include <xsi_string.h>

#include "FabricDFGTools.h"

using namespace XSI;

XSIPLUGINCALLBACK CStatus Fabric_Init( CRef &in_ctxt )
{
  Menu menu = Context(in_ctxt).GetSource();
  MenuItem item;

  menu.AddCallbackItem("Create Graph",                        "FabricCanvas_Menu_CreateDFGOp",            item);
  menu.AddSeparatorItem();
  menu.AddCallbackItem("Create Null with Graph",              "FabricCanvas_Menu_CreateNullWithOp",       item);
  menu.AddCallbackItem("Create Polymesh with Graph",          "FabricCanvas_Menu_CreatePolymeshWithOp",   item);
  menu.AddSeparatorItem();
  menu.AddCallbackItem("Inspect Canvas Op",                   "FabricCanvas_Menu_InspectCanvasOp",        item);
  menu.AddSeparatorItem();
  menu.AddCallbackItem("Load Graph",                          "FabricCanvas_Menu_ImportGraph",            item);
  menu.AddCallbackItem("Save Graph",                          "FabricCanvas_Menu_ExportGraph",            item);
  menu.AddSeparatorItem();
  menu.AddCallbackItem("Splice Editor (legacy)",              "FabricSplice_Menu_Editor",                 item);
  menu.AddCallbackItem("Load Splice Preset (legacy)",         "FabricSplice_Menu_LoadSplice",             item);
  menu.AddSeparatorItem();
  menu.AddCallbackItem("Toggle Manipulation",                 "FabricSplice_Menu_Manipulation",           item);
  menu.AddSeparatorItem();
  menu.AddCallbackItem("Toggle Renderer",                     "FabricSplice_Menu_Renderer",               item);
  menu.AddSeparatorItem();
  menu.AddCallbackItem("Online Help",                         "FabricCanvas_Menu_OnlineHelp",             item);
  menu.AddCallbackItem("ThirdParty Licenses",                 "FabricSplice_Menu_ThirdPartyLicenses",     item);
  menu.AddCallbackItem("Log Status",                          "FabricCanvas_Menu_LogStatus",              item);

  return CStatus::OK;
}

SICALLBACK FabricCanvas_Menu_CreateDFGOp(XSI::CRef&)
{
  // init and fill array of target objects.
  CStringArray targetObjects;
  {
    Selection sel = Application().GetSelection();
    if (sel.GetCount() <= 0)
    {
      // nothing is selected
      // => fill targetObjects from picking session.
      CValueArray args(7);
      args[0] = siGenericObjectFilter;
      args[1] = CString(L"Pick Object");
      args[2] = CString(L"Pick Object");
      args[5] = 0;

      CString command(L"PickElement");
      CValue value;

      while (true)
      {
        // pick session failed?
        if (Application().ExecuteCommand(command, args, value) == CStatus::Fail)
          break;

        // right button?
        if ((LONG)args[4] == 0)
          break;

        // add CRef of picked object to targetObjects.
        CRef ref = args[3];
        targetObjects.Add(ref.GetAsText());
      }
    }
    else
    {
      // there are selected objects
      // => fill targetObjects from selection.
      for (int i=0;i<sel.GetCount();i++)
        targetObjects.Add(sel[i].GetAsText());
    }
  }

  // execute command for all elements in targetObjects.
  for (int i=0;i<targetObjects.GetCount();i++)
  {
    CValueArray args;
    args.Add(targetObjects[i]);
    args.Add(L"");
    args.Add(true);
    Application().ExecuteCommand(L"FabricCanvasOpApply", args, CValue());
  }

  // done.
  dfgTools::ClearUndoHistory();
  return CStatus::OK;
}

SICALLBACK FabricCanvas_Menu_CreateNullWithOp(XSI::CRef&)
{
  // create a null.
  Null obj;
  if (Application().GetActiveSceneRoot().AddNull(L"null", obj) != CStatus::OK)
  { Application().LogMessage(L"failed to create a null", siErrorMsg);
  return CStatus::OK; }

  // select it.
  Selection sel = Application().GetSelection();
  sel.Clear();
  sel.Add(obj.GetRef());

  // execute command.
  CValueArray args;
  args.Add(obj.GetFullName());
  args.Add(L"");
  args.Add(true);
  Application().ExecuteCommand(L"FabricCanvasOpApply", args, CValue());

  // done.
  dfgTools::ClearUndoHistory();
  return CStatus::OK;
}

SICALLBACK FabricCanvas_Menu_CreatePolymeshWithOp(XSI::CRef&)
{
  // create an empty polygon mesh.
  X3DObject obj;
  if (Application().GetActiveSceneRoot().AddPolygonMesh(MATH::CVector3Array(), CLongArray(), L"polymsh", obj) != CStatus::OK)
  { Application().LogMessage(L"failed to create an empty polygon mesh", siErrorMsg);
  return CStatus::OK; }

  // select it.
  Selection sel = Application().GetSelection();
  sel.Clear();
  sel.Add(obj.GetRef());

  // execute command.
  CValueArray args;
  args.Add(obj.GetFullName());
  args.Add(L"");
  args.Add(true);
  Application().ExecuteCommand(L"FabricCanvasOpApply", args, CValue());

  // done.
  dfgTools::ClearUndoHistory();
  return CStatus::OK;
}

SICALLBACK FabricCanvas_Menu_InspectCanvasOp(XSI::CRef&)
{
  // execute command.
  CValueArray args;
  Application().ExecuteCommand(L"FabricCanvasInspectOp", args, CValue());

  // done.
  return CStatus::OK;
}

SICALLBACK FabricCanvas_Menu_OnlineHelp(XSI::CRef&)
{
  CValueArray args;
  args.Add(L"http://docs.fabric-engine.com/FabricEngine/latest/HTML/");
  args.Add(true);
  args.Add((LONG)1);
  Application().ExecuteCommand(L"OpenNetView", args, CValue());
  return CStatus::OK;
}

SICALLBACK FabricCanvas_Menu_ImportGraph(XSI::CRef&)
{
  LONG ret;
  UIToolkit toolkit = Application().GetUIToolkit();
  CString msgBoxtitle = L"Import graph";

  // get/check current selection.
  CRef selRef;
  {
    Selection sel = Application().GetSelection();
    if (sel.GetCount() < 1)
    { toolkit.MsgBox(L"Nothing selected.\nPlease select an object or operator and try again.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
    if (sel.GetCount() > 1)
    { toolkit.MsgBox(L"More than one object selected.\nPlease select a single object or operator and try again.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
    selRef = sel.GetArray()[0];
  }

  // get operator from selection.
  CustomOperator op;
  if (selRef.GetClassID() == siCustomOperatorID)
  {
    op.SetObject(selRef);
    if (!op.IsValid())
    { toolkit.MsgBox(L"The selection cannot be used for this operation.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
  }
  else
  {
    X3DObject obj(selRef);
    if (!obj.IsValid())
    { toolkit.MsgBox(L"The selection cannot be used for this operation.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
    CRefArray opRefs;
    int numOps = dfgTools::GetRefsAtOps(obj, CString(L"CanvasOp"), opRefs);
    if (numOps < 1)
    { toolkit.MsgBox(L"No valid custom operator found to perform this operation.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
    if (numOps > 1)
    { toolkit.MsgBox(L"The selection contains more than one custom operator.\nPlease select a single operator and try again.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
    op.SetObject(opRefs[0]);
    if (!op.IsValid())
    { toolkit.MsgBox(L"Failed to get custom operator.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
  }

  // open file browser.
  CString fileName;
  if (!dfgTools::FileBrowserJSON(false, fileName))
  { Application().LogMessage(L"aborted by user.", siWarningMsg);
    return CStatus::OK; }

  // execute command.
  CValueArray args;
  args.Add(op.GetUniqueName());
  args.Add(fileName);
  Application().ExecuteCommand(L"FabricCanvasImportGraph", args, CValue());

  // done.
  dfgTools::ClearUndoHistory();
  return CStatus::OK;
}

SICALLBACK FabricCanvas_Menu_ExportGraph(XSI::CRef&)
{
  LONG ret;
  UIToolkit toolkit = Application().GetUIToolkit();
  CString msgBoxtitle = L"Export Graph";

  // get/check current selection.
  CRef selRef;
  {
    Selection sel = Application().GetSelection();
    if (sel.GetCount() < 1)
    { toolkit.MsgBox(L"Nothing selected.\nPlease select an object or operator and try again.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
    if (sel.GetCount() > 1)
    { toolkit.MsgBox(L"More than one object selected.\nPlease select a single object or operator and try again.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
    selRef = sel.GetArray()[0];
  }

  // get operator from selection.
  CustomOperator op;
  if (selRef.GetClassID() == siCustomOperatorID)
  {
    op.SetObject(selRef);
    if (!op.IsValid())
    { toolkit.MsgBox(L"The selection cannot be used for this operation.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
  }
  else
  {
    X3DObject obj(selRef);
    if (!obj.IsValid())
    { toolkit.MsgBox(L"The selection cannot be used for this operation.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
    CRefArray opRefs;
    int numOps = dfgTools::GetRefsAtOps(obj, CString(L"CanvasOp"), opRefs);
    if (numOps < 1)
    { toolkit.MsgBox(L"No valid custom operator found to perform this operation.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
    if (numOps > 1)
    { toolkit.MsgBox(L"The selection contains more than one custom operator.\nPlease select a single operator and try again.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
    op.SetObject(opRefs[0]);
    if (!op.IsValid())
    { toolkit.MsgBox(L"Failed to get custom operator.", siMsgOkOnly | siMsgInformation, msgBoxtitle, ret);
      return CStatus::OK; }
  }

  // open file browser.
  CString fileName;
  if (!dfgTools::FileBrowserJSON(true, fileName))
  { Application().LogMessage(L"aborted by user.", siWarningMsg);
    return CStatus::OK; }

  // execute command.
  CValueArray args;
  args.Add(op.GetUniqueName());
  args.Add(fileName);
  Application().ExecuteCommand(L"FabricCanvasExportGraph", args, CValue());

  // done.
  return CStatus::OK;
}

SICALLBACK FabricCanvas_Menu_LogStatus(XSI::CRef&)
{
  CValueArray args;
  Application().ExecuteCommand(L"FabricCanvasLogStatus", args, CValue());
  return CStatus::OK;
}


