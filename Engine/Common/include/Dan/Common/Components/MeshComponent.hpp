#ifndef DAN_MESHCOMPONENT_HPP
#define DAN_MESHCOMPONENT_HPP

#include <memory>
#include "../Assets/Mesh.hpp"

namespace Dan {
	// Componente temporário de exemplo para representar algo desenhável
	struct MeshComponent {
		std::shared_ptr<Mesh> meshAsset;
		uint32_t materialId;
	};
}

#endif