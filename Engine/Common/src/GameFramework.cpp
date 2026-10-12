#include <Dan/Common/GameFramework.hpp>
#include <Dan/Common/Math/Transform.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>
#include <Dan/Common/Components/CameraComponent.hpp>
#include <SDL3/SDL_keyboard.h>

namespace Dan {	

	GameFramework::GameFramework(AssetManager* assetManager) // Removido : m_assetManager(assetManager) se não for guardar na classe
	{
		DAN_ENGINE_INFO("GameFramework inicializado.");

		// ========================================================
		// 1. CRIAÇÃO DO OBJETO 3D (CUBO)
		// ========================================================
		auto entity = m_registry.create();

		Math::Transform transform;
		transform.translation = Math::Vec3(0.0f, 0.0f, -5.0f); // 5 unidades para frente
		m_registry.emplace<Math::Transform>(entity, transform);

		// Carrega a malha (Geometria) do disco via Assimp
		auto cubeMesh = assetManager->Load<Mesh>("box.obj");
		DAN_CORE_ASSERT(cubeMesh, "Falha ao carregar box.obj");

		// Carrega os shaders binários compilados pelo shaderc do bgfx
		// Lembre-se de colocar os arquivos .bin na pasta Assets/Shaders
		auto vsMesh = assetManager->Load<Shader>("Shaders/vs_mesh.bin");
		DAN_CORE_ASSERT(vsMesh, "Falha ao carregar Vertex Shader");

		auto fsMesh = assetManager->Load<Shader>("Shaders/fs_mesh.bin");
		DAN_CORE_ASSERT(fsMesh, "Falha ao carregar Fragment Shader");

		// Cria o material agrupando os dois shaders
		auto cubeMaterial = std::make_shared<Material>(vsMesh, fsMesh);

		// Atribui o componente modernizado à entidade, passando a Malha e o Material reais
		m_registry.emplace<MeshComponent>(entity, MeshComponent{ cubeMesh, cubeMaterial });

		// ========================================================
		// 2. CRIAÇÃO DA CÂMERA (SEU CÓDIGO INTACTO)
		// ========================================================
		auto cameraEntity = m_registry.create();
		Math::Transform camTransform;
		camTransform.translation = Math::Vec3(0.0f, 0.0f, 5.0f); // Coloca a câmera 5 unidades para trás
		m_registry.emplace<Math::Transform>(cameraEntity, camTransform);
		m_registry.emplace<CameraComponent>(cameraEntity);
	}

	GameFramework::~GameFramework()
	{
		DAN_ENGINE_TRACE("GameFramework desligado. Entidades destruídas.");
	}

	void GameFramework::Update(const EngineContext& context)
	{
		UpdateGameLogic(context.time.elapsedRealTime);
		BuildRenderData();
	}

	void GameFramework::UpdateGameLogic(float deltaTime)
	{
		// Velocidade de movimento (ex: 10 unidades por segundo)
		const float moveSpeed = 10.0f * deltaTime;

		// Pega o estado atual de todas as teclas pressionadas na SDL3
		int numKeys;
		const bool* keyState = SDL_GetKeyboardState(&numKeys);

		// Itera apenas pelas entidades que possuem Transform e são Câmeras
		auto view = m_registry.view<Math::Transform, CameraComponent>();

		for (auto [entity, transform, camera] : view.each())
		{
			if (!camera.isPrimary) continue;

			// Movimentação básica (Nota: para um FPS real, você multiplicaria pela rotação do Transform)
			if (keyState[SDL_SCANCODE_W]) transform.translation.z -= moveSpeed; // Frente
			if (keyState[SDL_SCANCODE_S]) transform.translation.z += moveSpeed; // Trás
			if (keyState[SDL_SCANCODE_A]) transform.translation.x -= moveSpeed; // Esquerda
			if (keyState[SDL_SCANCODE_D]) transform.translation.x += moveSpeed; // Direita
			if (keyState[SDL_SCANCODE_Q]) transform.translation.y -= moveSpeed; // Desce
			if (keyState[SDL_SCANCODE_E]) transform.translation.y += moveSpeed; // Sobe

			// 1. Atualiza a Matriz de Projeção (Lente da Câmera)
			// Usamos a proporção 16:9 (1280/720) temporariamente. No futuro, isso virá da Window.
			camera.projectionMatrix = glm::perspective(
				glm::radians(camera.fov),
				1280.0f / 720.0f,
				camera.nearClip,
				camera.farClip
			);

			// 2. Atualiza a Matriz de Visão (Onde a câmera está e para onde olha)
			// A ViewMatrix é matematicamente a Inversa do Transform da câmera no mundo[cite: 2]
			camera.viewMatrix = glm::inverse(transform.GetMatrix());
		}
	}

	void GameFramework::BuildRenderData()
	{
		// Limpa o pacote do frame anterior
		m_currentRenderData.drawCommands.clear();

		// 1. Busca a Câmera Primária no ECS para extrair as matrizes
		auto cameraView = m_registry.view<const CameraComponent>();
		for (auto entity : cameraView)
		{
			const auto& camera = cameraView.get<const CameraComponent>(entity);
			if (camera.isPrimary) {
				// Repassa as matrizes calculadas no frame atual para a GPU[cite: 8]
				m_currentRenderData.viewMatrix = camera.viewMatrix;
				m_currentRenderData.projectionMatrix = camera.projectionMatrix;
				break; // Só precisamos de uma câmera
			}
		}

		// Extrai todas as entidades que possuem Transform e Mesh
		auto view = m_registry.view<const Math::Transform, const MeshComponent>();

		// O método .each() resolve o problema do iterador complexo e 
		// desempacota a entidade e seus componentes diretamente nas variáveis.
		// Itera usando Structured Bindings otimizado do C++20[cite: 2]
		for (auto [entity, transform, meshComp] : view.each())
		{
			if (!meshComp.meshAsset) continue;

			DrawCommand cmd{};
			cmd.transform = transform.GetMatrix();

			// Passamos os handles da GPU (BGFX) gerados pela Assimp diretamente para o RenderData
			cmd.vbh = meshComp.meshAsset->GetVertexBuffer().idx; // Ajuste na struct DrawCommand para receber VBH/IBH numéricos
			cmd.ibh = meshComp.meshAsset->GetIndexBuffer().idx;
			cmd.programId = meshComp.material->GetProgram().idx;

			m_currentRenderData.drawCommands.push_back(cmd);
		}
	}

	const RenderData& GameFramework::GetRenderData() const noexcept
	{
		return m_currentRenderData;
	}

} // namespace Dan