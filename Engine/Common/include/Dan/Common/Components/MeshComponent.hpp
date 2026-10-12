#pragma once

#include <memory>
#include "../Assets/Mesh.hpp"
#include "../Assets/Material.hpp"

namespace Dan {
	struct MeshComponent {
		std::shared_ptr<Mesh> meshAsset;
		std::shared_ptr<Material> material;
	};
}