#include <Dan/Common/GameFramework.hpp>
#include <Dan/Common/Math/Transform.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>
#include <Dan/Common/Components/CameraComponent.hpp>
#include <Dan/Common/Components/LightComponents.hpp>
#include <SDL3/SDL_keyboard.h>
#include <Dan/Common/IO/FileSystem.hpp>
#include <Dan/Common/Scene/SceneSerializer.hpp>

namespace Dan {	

	GameFramework::GameFramework(AssetManager* assetManager, FileSystem* fileSystem)
	{
		DAN_ENGINE_INFO("GameFramework inicializado. Carregando Cena...");

		SceneSerializer serializer(m_registry, assetManager, fileSystem);

		// Substitui completamente a criação de entidades em C++
		bool loaded = serializer.Load("Levels/example.json");
		DAN_CORE_ASSERT(loaded, "Falha ao carregar o arquivo de nivel principal.");
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

		// Extração da Iluminação Global
		auto lightView = m_registry.view<const DirectionalLightComponent>();
		for (auto [entity, light] : lightView.each()) {
			m_currentRenderData.dirLight.direction = light.direction;
			m_currentRenderData.dirLight.color = light.color;
			m_currentRenderData.dirLight.intensity = light.intensity;
			break; // Assume 1 luz direcional principal para este exemplo
		}

		// Extrai todas as entidades que possuem Transform e Mesh
		auto view = m_registry.view<const Math::Transform, const MeshComponent>();

		// Extração das Malhas
		for (auto [entity, transform, meshComp] : view.each())
		{
			if (!meshComp.meshAsset || !meshComp.material) continue;

			DrawCommand cmd{};
			cmd.transform = transform.GetMatrix();
			cmd.vbh = meshComp.meshAsset->GetVertexBuffer().idx;
			cmd.ibh = meshComp.meshAsset->GetIndexBuffer().idx;

			cmd.programId = meshComp.material->GetProgram().idx;

			cmd.albedoTexId = meshComp.material->GetAlbedoHandle().idx;
			cmd.albedoSamplerId = meshComp.material->GetAlbedoSampler().idx;

			cmd.normalTexId = meshComp.material->GetNormalHandle().idx;
			cmd.normalSamplerId = meshComp.material->GetNormalSampler().idx;

			cmd.propsUniformId = meshComp.material->GetPropsUniform().idx;
			cmd.materialProps = meshComp.material->GetPackedProperties();

			m_currentRenderData.drawCommands.push_back(cmd);
		}
	}

	const RenderData& GameFramework::GetRenderData() const noexcept
	{
		return m_currentRenderData;
	}

} // namespace Dan