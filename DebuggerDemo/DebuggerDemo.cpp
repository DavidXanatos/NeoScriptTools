#include "DebuggerDemo.h"

#include <QTextDocument>
#include <QToolButton>
#include <QMessageBox>
#include <QFileDialog>

#include "V4Engine.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "QSEngine.h"
#endif

#include "../NeoScriptTools/JSDebugging/JSScriptDebugger.h"
#include "../NeoScriptTools/JSDebugging/JSScriptDebuggerFrontend.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "../NeoScriptTools/debugging/qscriptenginedebugger.h"
#endif
//#include "../V8ScriptDebugger/V8ScriptDebuggerFrontend.h"

DebuggerDemo::DebuggerDemo(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.txtProgram->setTabStopDistance(16);
	ui.txtProgram->setPlainText(
		"function Debug() {\n"
		"\tvar my_obj = {a:1,b:2,c:{x:'a',y:'b'}};\n"
		"\tvar my_arr = [1,2,3];\n"
		"x2 = 1;\n"
		"var y2 = 2;\n"
		"let z2 = 3;\n"
		"\talert('debugger invoked, open the debugger to debug');\n"
		"\t_debugger();\n"
		"\tfor(var i=0; i < 10; i++){\n"
		"\t\tsleep(1000);\n"
		"\t\talert(i + '\\n');\n"
		"\t}\n"
		"\talert('+++');\n"
		"}\n"
		"\n"
		"\n"
		"function func1() {\n"
		"\tvar a = 'a in func 1';\n"
		"\tvar b = 'b in func 1';\n"
		"\tfunc2();\n"
		"}\n"
		"\n"
		"function func2() {\n"
		"\tvar a = 'a in func 2';\n"
		"\tvar b = 'b in func 2';\n"
		"\tfunc3();\n"
		"}\n"
		"\n"
		"function func3() {\n"
		"\tvar a = 'a in func 3';\n"
		"\tvar b = 'b in func 3';\n"
		"\tfunc4();\n"
		"}\n"
		"\n"
		"function func4() {\n"
		"\n"
		"\tvar a = 'a in func 4';\n"
		"\t{\n"
		"\t\tvar b = 'b in func 4';\n"
		"\t\t{\n"
		"\t\t\tDebug();\n"
		"\t\t}\n"
		"\t}\n"
		"}\n"
		"\n"
		"function Test() {\n"
		"\tfunc1();\n"
		"}\n"
		"\n"
		"const calculator = {\n"
		"\toperand1: 10,\n"
		"\toperand2 : 5,\n"
		"\tsum : function() {\n"
		"\t\tDebug()\n"
		"\t\treturn this.operand1 + this.operand2;\n"
		"\t},\n"
		"};\n"
		"\n"
		"function TestThis() {\n"
		"\talert(calculator.sum());\n"
		"}\n"
		"\n"
		"x1 = 1;\n"
		"var y1 = 2;\n"
		"let z1 = 3;\n"
		"\n"
		"\n"
		"if(typeof _debugger == 'undefined') _debugger = function() { debugger; }\n"
		"\n"
		//"alert('open debugger and evaluate Debug(), Test(), or TestThis()');\n"
		"Debug();\n"
		"");

	QTextDocument* document = ui.txtOutput->document();
	document->setHtml("For <b>COMERTIAL USE</b> please reach out for a apropriate license to: XanatosDavid@gmil.com<br /><br />");

	ui.txtOutput->appendPlainText(tr("Welcome to the Debuger Demo, review the test script and press the runn botton to RUN it in the V4 engine, "
		"then press the DEBUG button to open the apropriate debugger.\n"));


	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(OnOpen()));
	connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(OnSave()));

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));

	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(OnAbout()));
	connect(ui.actionAboutQt, SIGNAL(triggered()), this, SLOT(OnAboutQt()));

	connect(ui.actionRun, SIGNAL(triggered()), this, SLOT(OnRunV4()));
	QMenu* pRunMenu = new QMenu();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	pRunMenu->addAction(tr("Run in old QS engine"), this, SLOT(OnRunQS()));
	ui.actionRun->setText(tr("Run in new V4 engine"));
#endif
	if (!pRunMenu->actions().isEmpty()) {
		((QToolButton*)ui.mainToolBar->widgetForAction(ui.actionRun))->setPopupMode(QToolButton::MenuButtonPopup);
		((QToolButton*)ui.mainToolBar->widgetForAction(ui.actionRun))->setMenu(pRunMenu);
	}

	connect(ui.actionDebug, SIGNAL(triggered()), this, SLOT(OnDebugV4()));
	QMenu* pDebugMenu = new QMenu();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	pDebugMenu->addAction(tr("Debug old QS engine"), this, SLOT(OnDebugQS()));
	ui.actionDebug->setText(tr("Debug new V4 engine"));
#endif
	//pDebugMenu->addAction(tr("Connect to V8 engine"), this, SLOT(OnDebugV8()));
	if (!pDebugMenu->actions().isEmpty()) {
		((QToolButton*)ui.mainToolBar->widgetForAction(ui.actionDebug))->setPopupMode(QToolButton::MenuButtonPopup);
		((QToolButton*)ui.mainToolBar->widgetForAction(ui.actionDebug))->setMenu(pDebugMenu);
	}
}

DebuggerDemo::~DebuggerDemo()
{
}

void DebuggerDemo::OnOpen() 
{
	QString Path = QFileDialog::getOpenFileName(this, tr("Select script file to open."), "", tr("Script Files (*.js *.qs *.qml)"));
	if (Path.isEmpty())
		return;

	QFile File(Path);
	if (File.open(QFile::ReadOnly))
		ui.txtProgram->setPlainText(QString::fromUtf8(File.readAll()));
}

void DebuggerDemo::OnSave() 
{
	QString Path = QFileDialog::getSaveFileName(this, tr("Select script file to save as."), "", tr("Script Files (*.js *.qs *.qml)"));
	if (Path.isEmpty())
		return;

	QFile File(Path);
	if (File.open(QFile::WriteOnly))
		File.write(ui.txtProgram->toPlainText().toUtf8());
}

void DebuggerDemo::OnAbout() 
{
	QString AboutCaption = tr(
		"<h3>About Neo Script Tools</h3>"
		"<p>Copyright (C) 2012-2023 by DavidXanatos</p>"
	);

	QString AboutText = tr(
		"Neo Script Tools is an a Qt6 fork of Qt's Script Tools<br />Copyright (C) 2012 Nokia Corporation.<br />"
		"Neo Script Tools adds ability to create custom debugger front/back-ends and comunicates all objects using easy to serialize QVariant objects for easy remote debugging.<br />"
		"<br />"
		"This Demo incluses also the V4ScriptDebugger which allows to debug the QV4 engine using GUI from the script tools.<br />"
		"V4ScriptDebugger - Copyright (C) 2023 by DavidXanatos<br />"
		"<br />"
		"For <b>COMERTIAL USE</b> please reach out for a apropriate license to: XanatosDavid@gmil.com<br />"
	);

	QMessageBox* msgBox = new QMessageBox(this);
	msgBox->setAttribute(Qt::WA_DeleteOnClose);
	msgBox->setWindowTitle(tr("About Neo Script Tools"));
	msgBox->setText(AboutCaption);
	msgBox->setInformativeText(AboutText);

	QIcon ico(QLatin1String(":/DebuggerDemo/Debug.png"));
	msgBox->setIconPixmap(ico.pixmap(128, 128));

	msgBox->exec();
}

void DebuggerDemo::OnAboutQt() 
{
	QMessageBox::aboutQt(this);
}

void DebuggerDemo::OnRunV4()
{
	if (m_pV4Thread == NULL) {
		CV4Engine* pEngine = new CV4Engine();
		m_pV4Thread = new CEngineThread(pEngine);
		connect(pEngine, SIGNAL(LogMessage(const QString&)), this, SLOT(OnLogMessage(const QString&)));
		connect(pEngine, SIGNAL(EvalFinished(const QVariant&)), this, SLOT(OnEvalFinished(const QVariant&)));
	}

	m_pV4Thread->RunScript(ui.txtProgram->toPlainText(), "Demo Script");
}

void DebuggerDemo::OnRunQS() 
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	if (m_pQSThread == NULL) {
		CQSEngine* pEngine = new CQSEngine();
		m_pQSThread = new CEngineThread(pEngine);
		connect(pEngine, SIGNAL(LogMessage(const QString&)), this, SLOT(OnLogMessage(const QString&)));
		connect(pEngine, SIGNAL(EvalFinished(const QVariant&)), this, SLOT(OnEvalFinished(const QVariant&)));
	}

	m_pQSThread->RunScript(ui.txtProgram->toPlainText(), "Demo Script");
#endif
}

void DebuggerDemo::OnDebugV4() 
{
	if (m_pV4Thread == NULL) {
		CV4Engine* pEngine = new CV4Engine();
		m_pV4Thread = new CEngineThread(pEngine);
		connect(pEngine, SIGNAL(LogMessage(const QString&)), this, SLOT(OnLogMessage(const QString&)));
		connect(pEngine, SIGNAL(EvalFinished(const QVariant&)), this, SLOT(OnEvalFinished(const QVariant&)));
	}

	if (m_pV4Debugger == NULL) 
	{
		CJSScriptDebuggerFrontend* pDebuggerFrontend = new CJSScriptDebuggerFrontend();
		QObject::connect(m_pV4Thread->GetDebuggerBackend(), SIGNAL(sendResponse(QVariant)), pDebuggerFrontend, SLOT(processResponse(QVariant)), Qt::QueuedConnection);
		QObject::connect(pDebuggerFrontend, SIGNAL(sendRequest(QVariant)), m_pV4Thread->GetDebuggerBackend(), SLOT(processRequest(QVariant)), Qt::QueuedConnection);

		m_pV4Debugger = new CJSScriptDebugger();
		connect(m_pV4Debugger, &CJSScriptDebugger::detach, this, [=]() {
			// todo: detach
		});
		m_pV4Debugger->resize(1024, 640);
		m_pV4Debugger->show();
		m_pV4Debugger->attachTo(pDebuggerFrontend);
	}
}

void DebuggerDemo::OnDebugV8() 
{
	// todo: Add V8ScriptDebugger and test if it works with modern V8 engines and not just those from 2012 LOL
}

void DebuggerDemo::OnDebugQS() 
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	if (m_pQSThread == NULL) {
		CQSEngine* pEngine = new CQSEngine();
		m_pQSThread = new CEngineThread(pEngine);
		connect(pEngine, SIGNAL(LogMessage(const QString&)), this, SLOT(OnLogMessage(const QString&)));
		connect(pEngine, SIGNAL(EvalFinished(const QVariant&)), this, SLOT(OnEvalFinished(const QVariant&)));
	}

	if (m_pQSDebugger == NULL) 
	{
		CJSScriptDebuggerFrontend* pDebuggerFrontend = new CJSScriptDebuggerFrontend();
		QObject::connect(m_pQSThread->GetDebuggerBackend(), SIGNAL(sendResponse(QVariant)), pDebuggerFrontend, SLOT(processResponse(QVariant)), Qt::QueuedConnection);
		QObject::connect(pDebuggerFrontend, SIGNAL(sendRequest(QVariant)), m_pQSThread->GetDebuggerBackend(), SLOT(processRequest(QVariant)), Qt::QueuedConnection);

		m_pQSDebugger = new CJSScriptDebugger();
		connect(m_pV4Debugger, &CJSScriptDebugger::detach, this, [=]() {
			// todo: detach
			});
		m_pQSDebugger->resize(1024, 640);
		m_pQSDebugger->show();
		m_pQSDebugger->attachTo(pDebuggerFrontend);
	}
#endif
}

void DebuggerDemo::OnLogMessage(const QString& Message)
{
	ui.txtOutput->appendPlainText(Message.trimmed());
}

void DebuggerDemo::OnEvalFinished(const QVariant& Result)
{
	ui.actionRun->setEnabled(true);
	ui.txtOutput->appendPlainText(Result.toString());
}