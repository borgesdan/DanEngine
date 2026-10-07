#include <Dan/Common/GameFramework.hpp>
#include <Dan/Common/Math/Transform.hpp>
#include <Dan/Common/Log.hpp>

namespace Dan {

	// Componente temporário de exemplo para representar algo desenhável
	struct MeshComponent {
		uint32_t meshId;
		uint32_t materialId;
	};

	GameFramework::GameFramework(AssetManager* assetManager) : m_assetManager(assetManager)
	{
		DAN_ENGINE_INFO("GameFramework inicializado.");

		// ==========================================================
		// Mock: Criando uma entidade de teste na cena
		// O Editor Qt fará isso dinamicamente no futuro[cite: 5].
		// ==========================================================
		auto entity = m_registry.create();

		Math::Transform transform;
		transform.translation = Math::Vec3(0.0f, 0.0f, -5.0f); // 5 unidades para frente
		m_registry.emplace<Math::Transform>(entity, transform);

		m_registry.emplace<MeshComponent>(entity, MeshComponent{ 1, 1 });
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
		// Aqui você atualizaria scripts, animações e a física do jogo.
		// Exemplo: Rotacionar todas as entidades de teste a cada frame
		auto view = m_registry.view<Math::Transform>();
		for (auto entity : view)
		{
			auto& transform = view.get<Math::Transform>(entity);
			// Rotaciona lentamente no eixo Y
			transform.rotation *= glm::angleAxis(deltaTime, Math::Vec3(0, 1, 0));
		}
	}

	void GameFramework::BuildRenderData()
	{
		// Limpa o pacote do frame anterior
		m_currentRenderData.drawCommands.clear();

		// Configura a câmera base (Mock - Depois virá de um CameraComponent)
		m_currentRenderData.viewMatrix = Math::Mat4(1.0f); // Na origem
		m_currentRenderData.projectionMatrix = glm::perspective(glm::radians(60.0f), 1280.0f / 720.0f, 0.1f, 1000.0f);

		// Extrai todas as entidades que possuem Transform e Mesh
		auto view = m_registry.view<const Math::Transform, const MeshComponent>();

		// O método .each() resolve o problema do iterador complexo e 
		// desempacota a entidade e seus componentes diretamente nas variáveis.
		for (auto [entity, transform, mesh] : view.each())
		{
			DrawCommand cmd;

			// Agora você usa as variáveis 'transform' e 'mesh' diretamente, sem precisar de view.get()
			cmd.transform = transform.GetMatrix();
			cmd.meshId = mesh.meshId;
			cmd.materialId = mesh.materialId;

			m_currentRenderData.drawCommands.push_back(cmd);
		}
	}

	const RenderData& GameFramework::GetRenderData() const noexcept
	{
		return m_currentRenderData;
	}

} // namespace Dan