#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_desktop.h>
#include <xsi_status.h>

#include "FabricDFGPlugin.h"
#include "FabricDFGWidget.h"
#include "FabricDFGOperators.h"

#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QBoxLayout>
#include <QtGui/QPalette>

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>

#include <DFG/DFGCombinedWidget.h>

using namespace XSI;

class FabricDFGWidget : public DFG::DFGCombinedWidget
{
 public:

  FabricDFGWidget(QWidget *parent) : DFGCombinedWidget(parent)
  {
  }

  virtual void onRecompilation()
  {
    std::string s = "Recompiling";
    log(NULL, s.c_str(), s.length());
  }

  static void log(void *userData, const char *message, unsigned int length)
  {
    feLog(userData, message, length);
  }
};

struct _windowData
{
  QBoxLayout      *qtLayout;
  FabricDFGWidget *qtDFGWidget;
  QDialog         *qtDialog;

  _windowData(void)
  {
    qtDialog      = NULL;
    qtDFGWidget   = NULL;
    qtLayout      = NULL;
  }
};

QApplication *g_qtApp         = NULL;   // global pointer at Qt application.
bool          g_canvasIsOpen  = false;  // global flag to ensure that not more than one Canvas is open.

void OpenCanvas(_opUserData *pud, const char *winTitle, bool windowIsTopMost)
{
  // if necessary allocate g_qtApp.
  if (!g_qtApp)
  {
    int argc = 0;
    g_qtApp = new QApplication(argc, NULL);
  }

  // check.
  if ( g_canvasIsOpen)          return;
  if (!g_qtApp)                 return;
  if (!pud)                     return;
  if (!pud->GetBaseInterface()) return;

  // set global flag to block any further canvas.
  g_canvasIsOpen = true;

  // declare and fill window data structure.
  _windowData winData;
  try
  {
    winData.qtDialog      = new QDialog         (NULL);
    winData.qtDFGWidget   = new FabricDFGWidget (winData.qtDialog);
    winData.qtLayout      = new QVBoxLayout     (winData.qtDialog);

    winData.qtDialog->setWindowTitle(winTitle ? winTitle : "Canvas");
    winData.qtLayout->addWidget(winData.qtDFGWidget); 
    winData.qtLayout->setContentsMargins(0, 0, 0, 0);

    // for some reason setting the qtDialog to modal doesn't work.
    /*winData.qtDialog->setWindowModality(Qt::WindowModal);*/

    // parenting the qtDialog to the Softimage main window results in a weird mouse position offset
    // => as a temporary workaround, we set the qtDialog to top most.
    if (windowIsTopMost)
      winData.qtDialog->setWindowFlags(Qt::WindowStaysOnTopHint); // SetParent((HWND)winData.qtDialog->winId(), (HWND)Application().GetDesktop().GetApplicationWindowHandle());

    // init the DFG widget.
    DFG::DFGConfig config;
    float f = 0.8f / 255.0f;
    config.graphConfig.useOpenGL                = false;
    config.graphConfig.headerBackgroundColor    . setRgbF(f * 113, f * 112, f * 111);
    config.graphConfig.mainPanelBackgroundColor . setRgbF(f * 127, f * 127, f * 127);
    config.graphConfig.sidePanelBackgroundColor . setRgbF(f * 171, f * 168, f * 166);
    FabricCore::DFGExec exec = pud->GetBaseInterface()->getBinding().getExec();
    winData.qtDFGWidget->init(*pud->GetBaseInterface()->getClient(),
                               pud->GetBaseInterface()->getManager(),
                               pud->GetBaseInterface()->getHost(),
                               pud->GetBaseInterface()->getBinding(),
                               exec,
                               pud->GetBaseInterface()->getStack(),
                               false,
                               config
                             );
  }
  catch(FabricCore::Exception e)
  {
    feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
  }

  // show/execute Qt dialog.
  {
    winData.qtDialog->exec();

    //winData.qtDialog->show();
    //while (winData.qtDialog->isVisible())
    //{
    //  winData.qtApp->processEvents();
    //}
  }

  // clean up.
  //if (winData.qtDFGWidget)   delete winData.qtDFGWidget;

  // done.
  g_canvasIsOpen = false;
  return;
}
