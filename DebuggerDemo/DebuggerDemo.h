#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QPushButton>
#include "ui_DebuggerDemo.h"
#include "EngineThread.h"

class CJSScriptDebugger;

class DebuggerDemo : public QMainWindow
{
    Q_OBJECT

public:
    DebuggerDemo(QWidget* parent = nullptr);
    ~DebuggerDemo();

public slots:
    void OnLogMessage(const QString& Message);
    void OnEvalFinished(const QVariant& Result);

private slots:
    void OnOpen();
    void OnSave();
    void OnAbout();
    void OnAboutQt();
    void OnRunV4();
    void OnRunQS();
    void OnDebugV4();
    void OnDebugV8();
    void OnDebugQS();

protected:
    CEngineThread* m_pV4Thread = NULL;
    CJSScriptDebugger* m_pV4Debugger = NULL;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    CEngineThread* m_pQSThread = NULL;
    CJSScriptDebugger* m_pQSDebugger = NULL;
#endif

private:
    Ui::DebuggerDemoClass ui;
};