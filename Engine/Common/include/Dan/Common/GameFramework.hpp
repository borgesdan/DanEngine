#ifndef DAN_GAMEFRAMEWORK_HPP
#define DAN_GAMEFRAMEWORK_HPP

#include "System/EngineContext.hpp"
#include "Renderer/RenderData.hpp"

namespace Dan {
    class AssetManager;

    class GameFramework
    {
    public:
        GameFramework(AssetManager* assetManager);
        ~GameFramework();

        // O Framework não é copiável
        GameFramework(const GameFramework&) = delete;
        GameFramework& operator=(const GameFramework&) = delete;

        // Chamado a cada volta do Main Loop pelo EngineCore
        void Update(const EngineContext& context);

        // Chamado pelo EngineCore para extrair o que deve ser desenhado pelo bgfx
        [[nodiscard]] const RenderData& GetRenderData() const noexcept;

        // Expõe o registro para que o Editor Qt (via entt::meta) possa ler/editar a cena[cite: 5]
        //entt::registry& GetRegistry() { return m_registry; }

    private:
        void UpdateGameLogic(float deltaTime);
        void BuildRenderData();

    private:
        //entt::registry m_registry;
        RenderData m_currentRenderData;
        AssetManager* m_assetManager;
    };

} // namespace Dan

#endif