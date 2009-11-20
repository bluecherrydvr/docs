/****************************************************************************\
*  Copyright C 2007 Stretch, Inc. All rights reserved. Stretch products are  *
*  protected under numerous U.S. and foreign patents, maskwork rights,       *
*  copyrights and other intellectual property laws.                          *
*                                                                            *
*  This source code and the related tools, software code and documentation,  *
*  and your use thereof, are subject to and governed by the terms and        *
*  conditions of the applicable Stretch IDE or SDK and RDK License Agreement *
*  (either as agreed by you or found at www.stretchinc.com). By using these  *
*  items, you indicate your acceptance of such terms and conditions between  *
*  you and Stretch, Inc. In the event that you do not agree with such terms  *
*  and conditions, you may not use any of these items and must immediately   *
*  destroy any copies you have made.                                         *
\****************************************************************************/

#include <QApplication>
#include <QMessageBox>

//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
//#include <crtdbg.h>

#include "mainwindow.h"
#include "sdvr.h"

int main(int argc, char *argv[])
{
    int rc;
    Q_INIT_RESOURCE(dvrcp);
//    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
//    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );

    QApplication app(argc, argv);

    g_App = &app;
    g_MainWindow = new MainWindow;
    g_MainWindow->show();

    rc = app.exec();
    delete g_MainWindow;

    return rc;
}
