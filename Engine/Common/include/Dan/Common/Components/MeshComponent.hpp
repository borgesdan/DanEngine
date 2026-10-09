#pragma once

#include <memory>
#include "../Assets/Mesh.hpp"

namespace Dan {
	// Componente temporário de exemplo para representar algo desenhável
	struct MeshComponent {
		std::shared_ptr<Mesh> meshAsset;
		uint32_t materialId;
	};
}