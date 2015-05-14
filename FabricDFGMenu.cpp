#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_plugin.h>
#include <xsi_menu.h>
#include <xsi_menuitem.h>
#include <xsi_desktop.h>
#include <xsi_layout.h>
#include <xsi_view.h>
#include <xsi_comapihandler.h>
#include <xsi_project.h>
#include <xsi_uitoolkit.h>

using namespace XSI;

XSIPLUGINCALLBACK CStatus FabricDFG_Init( CRef& in_ctxt )
{
  Menu menu = Context(in_ctxt).GetSource();
  MenuItem item;

  menu.AddCallbackItem("Online Help", "FabricDFG_Menu_OnlineHelp", item);

  return CStatus::OK;
}

SICALLBACK FabricDFG_Menu_OnlineHelp( XSI::CRef& )
{
  CValue returnVal;
  CValueArray args(3);
  args[0] = "http://docs.fabric-engine.com/FabricEngine/latest/HTML/";
  args[1] = true;
  args[2] = (LONG)1l;
  Application().ExecuteCommand(L"OpenNetView", args, returnVal);
  return true;
}
