#include <Dan/Common/GameFramework.hpp>
#include <Dan/Common/Math/Transform.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>
#include <Dan/Common/Components/CameraComponent.hpp>
#include <SDL3/SDL_keyboard.h>

namespace Dan {	

	GameFramework::GameFramework(AssetManager* assetManager) 
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

		auto texture = assetManager->Load<Texture2D>("Textures/texture.jpg");
		DAN_CORE_ASSERT(texture, "Falha ao carregar a textura");

		// Cria o material agrupando os dois shaders
		auto cubeMaterial = std::make_shared<Material>(vsMesh, fsMesh, texture);

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
		UpdateGameLogic(context);
		BuildRenderData();
	}

	void GameFramework::UpdateGameLogic(const EngineContext& context)
	{
		// Velocidade de movimento (ex: 10 unidades por segundo)
		const float moveSpeed = 10.0f * context.time.elapsedGameTime;
		const auto& input = context.input;

		// Pega o estado atual de todas as teclas pressionadas na SDL3
		int numKeys;
		const bool* keyState = SDL_GetKeyboardState(&numKeys);

		auto view = m_registry.view<Math::Transform, CameraComponent>();

		for (auto [entity, transform, camera] : view.each())
		{
			if (!camera.isPrimary) continue;

			//// 1. Rotação (Mouse Yaw/Pitch)
			// Verifica se o Botão Esquerdo está pressionado
			if (input.IsMouseButtonDown(SDL_BUTTON_LEFT))
			{
				camera.yaw += input.mouseDeltaX * camera.mouseSensitivity;
				camera.pitch -= input.mouseDeltaY * camera.mouseSensitivity;
				camera.pitch = glm::clamp(camera.pitch, -89.0f, 89.0f);					
			}

			transform.rotation = glm::quat(glm::vec3(
				glm::radians(camera.pitch),
				glm::radians(camera.yaw),
				0.0f
			));


			// 2. Extração dos vetores direcionais (Frente, Direita, Cima) a partir da rotação
			Math::Vec3 forward = transform.rotation * Math::Vec3(0.0f, 0.0f, -1.0f);
			Math::Vec3 right = transform.rotation * Math::Vec3(1.0f, 0.0f, 0.0f);
			Math::Vec3 up = transform.rotation * Math::Vec3(0.0f, 1.0f, 0.0f);

			// 3. Movimentação (Teclado) relativa à direção do olhar
			if (input.IsKeyDown(SDL_SCANCODE_W)) transform.translation += forward * moveSpeed;
			if (input.IsKeyDown(SDL_SCANCODE_S)) transform.translation -= forward * moveSpeed;
			if (input.IsKeyDown(SDL_SCANCODE_A)) transform.translation -= right * moveSpeed;
			if (input.IsKeyDown(SDL_SCANCODE_D)) transform.translation += right * moveSpeed;
			if (input.IsKeyDown(SDL_SCANCODE_E)) transform.translation += up * moveSpeed;
			if (input.IsKeyDown(SDL_SCANCODE_Q)) transform.translation -= up * moveSpeed;

			// 4. Atualização das Matrizes
			camera.projectionMatrix = glm::perspective(
				glm::radians(camera.fov),
				1280.0f / 720.0f,
				camera.nearClip,
				camera.farClip
			);

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
			if (!meshComp.meshAsset || !meshComp.material) continue;

			DrawCommand cmd{};
			cmd.transform = transform.GetMatrix();
			cmd.vbh = meshComp.meshAsset->GetVertexBuffer().idx;
			cmd.ibh = meshComp.meshAsset->GetIndexBuffer().idx;

			// IDs do Material
			cmd.programId = meshComp.material->GetProgram().idx;
			cmd.textureId = meshComp.material->GetTextureHandle().idx;
			cmd.samplerId = meshComp.material->GetSamplerHandle().idx;

			m_currentRenderData.drawCommands.push_back(cmd);
		}
	}

	const RenderData& GameFramework::GetRenderData() const noexcept
	{
		return m_currentRenderData;
	}

} // namespace Dan