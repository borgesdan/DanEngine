#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>

#include "../pch.hpp"

#include "../SplashScreen.hpp"
#include "../EditorApp.hpp"

int main(int argc, char* argv[]) {
	//Inicializa os subsistemas base(Log antes de qualquer UI)
	Dan::Log::Init();
	DAN_ENGINE_INFO("Iniciando Editor...");	

	//Inicialização Nativa do Qt (Substitui QMfcApp e AfxWinInit do CryEngine 5.7)
	Dan::EditorApp app(argc, argv);

	//Parse de Argumentos e Configurações Modernas	
	qsizetype waitForSeconds = 0;

	//Usar o parse de argumentos embutido do Qt
	QStringList arglist = app.arguments();

	int waitIndex = arglist.indexOf("-waitfordebugger");
	if (waitIndex >= 0 && arglist.size() > static_cast<qsizetype>(waitIndex) + 1)
	{
		waitForSeconds = std::max(0, arglist[waitIndex + 1].toInt());
	}

	int dpiIndex = arglist.indexOf("-nodpiscaling");
	if (dpiIndex == -1)
	{
		QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	}

	QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
	QDir::setSearchPaths("icons", QStringList(":/icons"));

	if (waitForSeconds > 0)
	{
		DAN_ENGINE_WARN("Aguardando depurador por {0} segundos...", waitForSeconds);
		// CWaitingForDebuggerDialog timeoutDialog;
		// timeoutDialog.Execute(waitForSeconds);
	}

	// Splash Screen
	DAN_ENGINE_INFO("Exibe SplashScreen");
	QPixmap pixmap(":/splash.png");
	Dan::SplashScreen splash(pixmap);
	splash.show();
	app.processEvents();	

	if (!app.Initialize())
	{
		DAN_ENGINE_CRITICAL("Falha crítica na inicialização da Engine. Abortando.");

		// Retornamos 1 (erro) em vez de 0 (sucesso)[cite: 3].
		// Não é necessário chamar ExitInstance() ou AfxWinTerm() aqui. 
		// Quando a função retornar, o destrutor ~EditorApp() será chamado automaticamente 
		// e liberará toda a memória do CGameEngine e CEditorImpl graças aos unique_ptr.
		return 1;
	}

	// 7. Instanciação da Janela Principal
	// CEditorMainFrame* mainFrame = new CEditorMainFrame(nullptr);

	// Reload stylesheet per CryEngine bugfix comment
	// SetVisualStyle(); 

	// Melhoria: Passar 'app' como parente gerencia a memória automaticamente
	QFileSystemWatcher* watcher = new QFileSystemWatcher(&app);
	/*
	if (!watcher->addPath("caminho_do_seu_estilo.qss")) {
		DAN_ENGINE_ERROR("Falha ao adicionar stylesheet no watcher");
	} else {
		QObject::connect(watcher, &QFileSystemWatcher::fileChanged, UpdateStyleSheet);
	}
	*/

	// mainFrame->PostLoad();
	// splash.close();
	// mainFrame->show();

	DAN_ENGINE_INFO("Entrando no loop de eventos do Qt.");

	// 8. Loop Principal Nativo (Substitui QMfcApp::run)
	int execReturn = app.exec();

	// 9. Limpeza final
	// delete mainFrame;
	DAN_ENGINE_INFO("Editor encerrado com código {0}.", execReturn);

	return execReturn;
}