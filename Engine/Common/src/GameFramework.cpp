#include <Dan/Common/GameFramework.hpp>
#include <Dan/Common/Math/Transform.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Components/MeshComponent.hpp>

namespace Dan {	

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

		m_registry.emplace<MeshComponent>(entity, MeshComponent{ nullptr, 1 });
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
		// Itera usando Structured Bindings otimizado do C++20[cite: 2]
		for (auto [entity, transform, meshComp] : view.each())
		{
			if (!meshComp.meshAsset) continue;

			DrawCommand cmd{};
			cmd.transform = transform.GetMatrix();

			// Passamos os handles da GPU (BGFX) gerados pela Assimp diretamente para o RenderData
			cmd.vbh = meshComp.meshAsset->GetVertexBuffer().idx; // Ajuste na struct DrawCommand para receber VBH/IBH numéricos
			cmd.ibh = meshComp.meshAsset->GetIndexBuffer().idx;
			cmd.materialId = meshComp.materialId;

			m_currentRenderData.drawCommands.push_back(cmd);
		}
	}

	const RenderData& GameFramework::GetRenderData() const noexcept
	{
		return m_currentRenderData;
	}

} // namespace Dan