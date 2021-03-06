#include <xsi_application.h>
#include <xsi_customoperator.h>
#include <xsi_projectitem.h>
#include <xsi_pluginregistrar.h>
#include <xsi_uitoolkit.h>
#include <xsi_plugin.h>

#include "plugin.h"
#include "FabricDFGBaseInterface.h"
#include "FabricDFGPlugin.h"
#include "FabricDFGOperators.h"
#include "FabricDFGTools.h"

#include "FabricSplicePlugin.h"
#include "FabricSpliceICENodes.h"

#include <FabricUI/DFG/DFGUICmd/DFGUICmds.h>

#include <QtGui/QApplication>

using namespace XSI;

// for registering the DFGUICmdHandler custom commands.
#define REGISTER_DFGUICMD(inreg, CMD)                           \
  {                                                             \
    std::wstring cmdName(                                       \
      FabricUI::DFG::DFGUICmd_##CMD::CmdName().begin(),         \
      FabricUI::DFG::DFGUICmd_##CMD::CmdName().end()            \
      );                                                        \
    in_reg.RegisterCommand( cmdName.c_str(), cmdName.c_str() ); \
    ccnames.Add( cmdName.c_str() );                             \
  }

// load plugin.
SICALLBACK XSILoadPlugin(PluginRegistrar& in_reg)
{
  // check if the Fabric environment variables are set.
  const int numEnvVars = 3;
  std::string envVarNames   [numEnvVars];
  std::string envVarExamples[numEnvVars];
  envVarNames   [0] = "FABRIC_DIR";
  envVarNames   [1] = "FABRIC_DFG_PATH";
  envVarNames   [2] = "FABRIC_EXTS_PATH";
  #ifdef _WIN32
    envVarExamples[0] = "<Fabric-Installation-Path>";
    envVarExamples[1] = "<Fabric-Installation-Path>\\Presets\\DFG";
    envVarExamples[2] = "<Fabric-Installation-Path>\\Exts";
  #else
    envVarExamples[0] = "<Fabric-Installation-Path>";
    envVarExamples[1] = "<Fabric-Installation-Path>/Presets/DFG";
    envVarExamples[2] = "<Fabric-Installation-Path>/Exts";
  #endif
  for (int i=0;i<numEnvVars;i++)
  {
    // get the environment variable's value.
    char *envVarValue  = getenv(envVarNames[i].c_str());

    // no value found?
    if (!envVarValue || envVarValue[0] == '\0')
    {
      // log error.
      std::string t = "The environment variable " + envVarNames[i] + " is not set. Please make sure that " + envVarNames[i] + " is set and points to \"" + envVarExamples[i] + "\".";
      Application().LogMessage(L"[Fabric]: " + CString(t.c_str()), siErrorMsg);
    }
  }

  // set plugin's name, version and author.
  in_reg.PutAuthor(L"Fabric Engine");
  in_reg.PutName  (L"Fabric Engine Plugin");
  in_reg.PutVersion(FabricCore::GetVersionMaj(), FabricCore::GetVersionMin());

  // register the legacy Fabric Splice.
  {
    // rendering.
    in_reg.RegisterDisplayCallback(L"SpliceRenderPass");

    // properties.
    in_reg.RegisterProperty(L"SpliceInfo");
  
    // dialogs.
    in_reg.RegisterProperty(L"SpliceEditor");
    in_reg.RegisterProperty(L"ImportSpliceDialog");

    // operators.
    in_reg.RegisterOperator(L"SpliceOp");

    // commands.
    in_reg.RegisterCommand(L"fabricSplice",             L"fabricSplice");
    in_reg.RegisterCommand(L"fabricSpliceManipulation", L"fabricSpliceManipulation");
    in_reg.RegisterCommand(L"proceedToNextScene",       L"proceedToNextScene");

    // tools.
    in_reg.RegisterTool(L"fabricSpliceTool");

    // events.
    in_reg.RegisterEvent(L"FabricSpliceNewScene",       siOnEndNewScene);
    in_reg.RegisterEvent(L"FabricSpliceCloseScene",     siOnCloseScene);
    in_reg.RegisterEvent(L"FabricSpliceOpenBeginScene", siOnBeginSceneOpen);
    in_reg.RegisterEvent(L"FabricSpliceOpenEndScene",   siOnEndSceneOpen);
    in_reg.RegisterEvent(L"FabricSpliceSaveScene",      siOnBeginSceneSave);
    in_reg.RegisterEvent(L"FabricSpliceSaveAsScene",    siOnBeginSceneSaveAs);
    in_reg.RegisterEvent(L"FabricSpliceTerminate",      siOnTerminate);
    in_reg.RegisterEvent(L"FabricSpliceSaveScene2",     siOnBeginSceneSave2);
    in_reg.RegisterEvent(L"FabricSpliceImport",         siOnEndFileImport);
    in_reg.RegisterEvent(L"FabricSpliceBeginExport",    siOnBeginFileExport);
    in_reg.RegisterEvent(L"FabricSpliceEndExport",      siOnEndFileExport);
    in_reg.RegisterEvent(L"FabricSpliceValueChange",    siOnValueChange);

    // ice nodes.
    Register_spliceGetData(in_reg);
  }

  // array of custom command names.
  CStringArray ccnames;

  // register the new Fabric Canvas.
  {
    // operators.
    in_reg.RegisterOperator(L"CanvasOp");

    // commands.
    CString cmdName;
    cmdName = L"FabricCanvasOpApply";           in_reg.RegisterCommand(cmdName, cmdName);   ccnames.Add(cmdName);
    cmdName = L"FabricCanvasOpPortMapDefine";   in_reg.RegisterCommand(cmdName, cmdName);   ccnames.Add(cmdName);
    cmdName = L"FabricCanvasOpConnectPort";     in_reg.RegisterCommand(cmdName, cmdName);   ccnames.Add(cmdName);
    cmdName = L"FabricCanvasOpPortMapQuery";    in_reg.RegisterCommand(cmdName, cmdName);   ccnames.Add(cmdName);
    cmdName = L"FabricCanvasImportGraph";       in_reg.RegisterCommand(cmdName, cmdName);   ccnames.Add(cmdName);
    cmdName = L"FabricCanvasExportGraph";       in_reg.RegisterCommand(cmdName, cmdName);   ccnames.Add(cmdName);
    cmdName = L"FabricCanvasSelectConnected";   in_reg.RegisterCommand(cmdName, cmdName);   ccnames.Add(cmdName);
    cmdName = L"FabricCanvasLogStatus";         in_reg.RegisterCommand(cmdName, cmdName);   ccnames.Add(cmdName);
    cmdName = L"FabricCanvasGetContextID";      in_reg.RegisterCommand(cmdName, cmdName);   ccnames.Add(cmdName);
    cmdName = L"FabricCanvasGetBindingID";      in_reg.RegisterCommand(cmdName, cmdName);   ccnames.Add(cmdName);

    // commands for DFGUICmdHandler.
    REGISTER_DFGUICMD( in_reg, AddBackDrop );
    REGISTER_DFGUICMD( in_reg, AddFunc );
    REGISTER_DFGUICMD( in_reg, AddGet );
    REGISTER_DFGUICMD( in_reg, AddGraph );
    REGISTER_DFGUICMD( in_reg, AddPort );
    REGISTER_DFGUICMD( in_reg, AddSet );
    REGISTER_DFGUICMD( in_reg, AddVar );
    REGISTER_DFGUICMD( in_reg, Connect );
    REGISTER_DFGUICMD( in_reg, CreatePreset );
    REGISTER_DFGUICMD( in_reg, Disconnect );
    REGISTER_DFGUICMD( in_reg, EditNode );
    REGISTER_DFGUICMD( in_reg, EditPort );
    REGISTER_DFGUICMD( in_reg, ExplodeNode );
    REGISTER_DFGUICMD( in_reg, ImplodeNodes );
    REGISTER_DFGUICMD( in_reg, InstPreset );
    REGISTER_DFGUICMD( in_reg, MoveNodes );
    REGISTER_DFGUICMD( in_reg, Paste );
    REGISTER_DFGUICMD( in_reg, RemoveNodes );
    REGISTER_DFGUICMD( in_reg, RemovePort );
    REGISTER_DFGUICMD( in_reg, RenamePort );
    REGISTER_DFGUICMD( in_reg, ReorderPorts );
    REGISTER_DFGUICMD( in_reg, ResizeBackDrop );
    REGISTER_DFGUICMD( in_reg, SetArgType );
    REGISTER_DFGUICMD( in_reg, SetArgValue );
    REGISTER_DFGUICMD( in_reg, SetCode );
    REGISTER_DFGUICMD( in_reg, SetExtDeps );
    REGISTER_DFGUICMD( in_reg, SetNodeComment );
    REGISTER_DFGUICMD( in_reg, SetPortDefaultValue );
    REGISTER_DFGUICMD( in_reg, SetRefVarPath );
    REGISTER_DFGUICMD( in_reg, SetTitle );
    REGISTER_DFGUICMD( in_reg, SplitFromPreset );

    // menu.
    in_reg.RegisterMenu(siMenuMainTopLevelID,       L"Fabric", true, true);

    // events.
    in_reg.RegisterEvent(L"FabricCanvasOnStartup",    siOnStartup);
    in_reg.RegisterEvent(L"FabricCanvasOnEndCommand", siOnEndCommand);
  }

  // sort the list of custom command names and log the result.
  for (LONG i=ccnames.GetCount();i>1;i--) // simple bubble sort.
    for (LONG j=0;j<i-1;j++)
      if (ccnames[j] > ccnames[j + 1])
      {
        CString mem = ccnames[j];
        ccnames[j] = ccnames[j + 1];
        ccnames[j + 1] = mem;
      }
  for (LONG i=0;i<ccnames.GetCount();i++)
    Application().LogMessage(L"    " + ccnames[i]);

  // done.
  return CStatus::OK;
}

// unload plugin.
SICALLBACK XSIUnloadPlugin(const PluginRegistrar& in_reg)
{
  CString strPluginName;
  strPluginName = in_reg.GetName();
  Application().LogMessage(strPluginName + L" has been unloaded.", siVerboseMsg);

  // Qt.
  if (qApp)
    delete qApp;

  // done.
  return CStatus::OK;
}

// __________________________________
// siEvent + event  helper functions.
// ----------------------------------

CStatus helpFnct_siEventOpenSave(CRef &ctxt, int doWhat, CRef &modelRef)
{
  Context context(ctxt);

  /*
      doWhat == 0: store DFG JSON in op's persistenceData parameter (e.g. before saving a scene).
      doWhat == 1: set DFG JSON from op's persistenceData parameter (e.g. after loading a scene or importing a model).

      note: if doWhat == 1 and model.IsValid() then we are importing a model.
  */

  std::map <unsigned int, _opUserData *> &s_instances = *_opUserData::GetStaticMapOfInstances();
  for (std::map<unsigned int, _opUserData *>::iterator it = s_instances.begin(); it != s_instances.end(); it++)
  {
    // get pointer at _opUserData.
    _opUserData *pud = it->second;
    if (!pud) continue;

    // get the object ID of the CanvasOp operator to which the user data belongs to.
    unsigned int opObjID = it->first;

    // check if the user data has a base interface.
    if (!pud->GetBaseInterface())
    { Application().LogMessage(L"user data has no base interface", siWarningMsg);
      continue; }

    // get the operator from the object ID.
    ProjectItem pItem = Application().GetObjectFromID(opObjID);
    if (!pItem.IsValid())
    { Application().LogMessage(L"Application().GetObjectFromID(opObjID) failed", siWarningMsg);
      continue; }
    CustomOperator op(pItem.GetRef());
    if (!op.IsValid())
    { Application().LogMessage(L"CustomOperator op(pItem.GetRef()) failed", siWarningMsg);
      continue; }
    if (op.GetType() != L"CanvasOp")
    { Application().LogMessage(L"op.GetType() returned \"" + op.GetType() + L"\" instead of \"CanvasOp\"", siWarningMsg);
      continue; }

    // store JSON in parameter persistenceData.
    if (doWhat == 0)
    {
      // do it.
      Application().LogMessage(L"storing DFG JSON for CanvasOp(opObjID = " + CString(op.GetObjectID()) + L")");
      try
      {
        CString dfgJSON = pud->GetBaseInterface()->getJSON().c_str();
        if (op.PutParameterValue(L"persistenceData", dfgJSON) != CStatus::OK)
        { Application().LogMessage(L"op.PutParameterValue(L\"persistenceData\") failed!", siWarningMsg);
          continue; }
      }
      catch (FabricCore::Exception e)
      {
        std::string s = std::string("failed: ") + (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
        feLogError(s);
      }
    }

    // set DFG JSON from parameter persistenceData.
    else if (doWhat == 1)
    {
      // if we are importing a model then we need to check whether
      // the current operator is parented under the model or not.
      if (modelRef.IsValid())
      {
        X3DObject prnt = op.GetParent3DObject();
        bool isPartOfModelHierarchy = false;

        while (   prnt.IsValid()
               && prnt.GetName() != L"Scene_Root"
               && !isPartOfModelHierarchy)
        {
          if (prnt.GetRef() == modelRef)
            isPartOfModelHierarchy = true;
          else
            prnt = prnt.GetParent3DObject();
        }

        if (!isPartOfModelHierarchy)
          continue;
      }

      // do it.
      Application().LogMessage(L"setting DFG JSON from CanvasOp(opObjID = " + CString(op.GetObjectID()) + L")");
      try
      {
        CString dfgJSON = op.GetParameterValue(L"persistenceData");

        if (dfgJSON.IsEmpty())
        { Application().LogMessage(L"op.GetParameterValue(L\"persistenceData\") returned an empty value.", siWarningMsg);
          continue; }

        pud->GetBaseInterface()->setFromJSON(dfgJSON.GetAsciiString());
      }
      catch (FabricCore::Exception e)
      {
        std::string s = std::string("failed: ") + (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
        feLogError(s);
      }
      dfgTools::ClearUndoHistory();

      // when importing models: make sure that the DFG gets
      // properly evaluated by setting the according flag.
      if (modelRef.IsValid())
      {
        pud->execFabricStep12 = true;
      }
    }

    // do nothing.
    else
    {
      continue;
    }
  }

  // done.
  // /note: we return 1 (i.e. "true") instead of CStatus::OK or else the event gets aborted).
  return 1;
}

XSIPLUGINCALLBACK CStatus FabricCanvasOnStartup_OnEvent(CRef &ctxt)
{
  // set the license type.
  if (Application().IsInteractive())
    FabricSplice::SetLicenseType(FabricCore::ClientLicenseType_Interactive);
  else
    FabricSplice::SetLicenseType(FabricCore::ClientLicenseType_Compute);

  // done.
  // /note: we return 1 (i.e. "true") instead of CStatus::OK or else the event gets aborted).
  return 1;
}

bool g_clearSoftimageUndoHistory = false;
XSIPLUGINCALLBACK CStatus FabricCanvasOnEndCommand_OnEvent(CRef &ctxt)
{
  // if the global flag g_clearSoftimageUndoHistory is set then we clear Softimage's undo history and reset the flag.
  if (g_clearSoftimageUndoHistory)
  {
    if (!dfgTools::ClearUndoHistory())
      Application().LogMessage(L"failed to clear undo history", siWarningMsg);
    g_clearSoftimageUndoHistory = false;
  }

  // done.
  // /note: we return 1 (i.e. "true") instead of CStatus::OK or else the event gets aborted).
  return 1;
}
