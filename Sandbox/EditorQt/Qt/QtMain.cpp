#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>

#include "../pch.hpp"
#include "../EditorApp.hpp"

int main(int argc, char* argv[]) {
	//Inicializa os subsistemas base(Log antes de qualquer UI)
	Dan::Log::Init();
	DAN_ENGINE_INFO("Iniciando Editor...");	

	//Inicialização Nativa do Qt (Substitui QMfcApp e AfxWinInit do CryEngine 5.7)
	Dan::EditorApp app(argc, argv);	

	//Removido parse de parametros e splashscreen

	if (!app.Initialize())
	{
		DAN_ENGINE_CRITICAL("Falha crítica na inicialização da Engine. Abortando.");

		// Retornamos 1 (erro) em vez de 0 (sucesso)[cite: 3].
		// Não é necessário chamar ExitInstance() ou AfxWinTerm() aqui. 
		// Quando a função retornar, o destrutor ~EditorApp() será chamado automaticamente 
		// e liberará toda a memória do CGameEngine e CEditorImpl graças aos unique_ptr.
		return 1;
	}

	DAN_ENGINE_INFO("Entrando no loop de eventos do Qt.");

	// 8. Loop Principal Nativo (Substitui QMfcApp::run)
	int execReturn = app.exec();

	// 9. Limpeza final
	// delete mainFrame;
	DAN_ENGINE_INFO("Editor encerrado com código {0}.", execReturn);

	return execReturn;
}