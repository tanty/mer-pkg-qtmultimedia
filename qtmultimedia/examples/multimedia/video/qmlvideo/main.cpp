/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/QStandardPaths>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickView>
#include "trace.h"

#ifdef PERFORMANCEMONITOR_SUPPORT
#include "performancemonitordeclarative.h"
#endif

static const QString DefaultFileName1 = "";
static const QString DefaultFileName2 = "";

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

#ifdef PERFORMANCEMONITOR_SUPPORT
    PerformanceMonitor::qmlRegisterTypes();
#endif

    QString source1, source2;
    qreal volume = 0.5;
    QStringList args = app.arguments();
#ifdef PERFORMANCEMONITOR_SUPPORT
    PerformanceMonitor::State performanceMonitorState;
#endif
    bool sourceIsUrl = false;
    for (int i = 1; i < args.size(); ++i) {
        const QByteArray arg = args.at(i).toUtf8();
        if (arg.startsWith('-')) {
            if ("-volume" == arg) {
                if (i+1 < args.count())
                    volume = 0.01 * args.at(++i).toInt();
                else
                    qtTrace() << "Option \"-volume\" takes a value";
            }
#ifdef PERFORMANCEMONITOR_SUPPORT
            else if (performanceMonitorState.parseArgument(arg)) {
                // Do nothing
            }
#endif
            else if ("-url" == arg) {
                sourceIsUrl = true;
            } else {
                qtTrace() << "Option" << arg << "ignored";
            }
        } else {
            if (source1.isEmpty())
                source1 = arg;
            else if (source2.isEmpty())
                source2 = arg;
            else
                qtTrace() << "Argument" << arg << "ignored";
        }
    }

    QUrl url1, url2;
    if (sourceIsUrl) {
        url1 = source1;
        url2 = source2;
    } else {
        if (!source1.isEmpty())
            url1 = QUrl::fromLocalFile(source1);
        if (!source2.isEmpty())
            url2 = QUrl::fromLocalFile(source2);
    }

    QQuickView viewer;
    viewer.setSource(QUrl("qrc:///qml/qmlvideo/main.qml"));
    QObject::connect(viewer.engine(), SIGNAL(quit()), &viewer, SLOT(close()));

    QQuickItem *rootObject = viewer.rootObject();
    rootObject->setProperty("source1", url1);
    rootObject->setProperty("source2", url2);
    rootObject->setProperty("volume", volume);

#ifdef PERFORMANCEMONITOR_SUPPORT
    if (performanceMonitorState.valid) {
        rootObject->setProperty("perfMonitorsLogging", performanceMonitorState.logging);
        rootObject->setProperty("perfMonitorsVisible", performanceMonitorState.visible);
    }
    QObject::connect(&viewer, SIGNAL(afterRendering()),
                     rootObject, SLOT(qmlFramePainted()));
#endif

    QUrl videoPath;
    const QStringList moviesLocation = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation);
    if (moviesLocation.isEmpty()) {
        QUrl appPath(QString("file://%1").arg(app.applicationDirPath()));
        videoPath = appPath.resolved(QUrl("./"));
    } else
        videoPath = QString("file://%1").arg(moviesLocation.first());
    viewer.rootContext()->setContextProperty("videoPath", videoPath);

    QMetaObject::invokeMethod(rootObject, "init");

    viewer.show();

    return app.exec();
}

