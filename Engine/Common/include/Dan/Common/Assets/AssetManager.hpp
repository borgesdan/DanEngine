#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include "../IO/FileSystem.hpp"

namespace Dan {
	// Classe base vazia para podermos armazenar diferentes tipos no mesmo mapa de cache
	class Asset {
	public:
		virtual ~Asset() = default;

		virtual const std::string GetVirtualPath() const noexcept { return {}; }
	};

	class AssetManager
	{
	public:
		// Injeção de dependência: O AssetManager precisa do FileSystem para ler os bytes
		explicit AssetManager(FileSystem* fileSystem) : m_fileSystem(fileSystem) {}

		// O famoso Content.Load<T> do XNA em C++20
		template<typename T>
		std::shared_ptr<T> Load(const std::string& path)
		{
			// 1. Verifica se o asset já está no cache (evita carregar a mesma textura duas vezes)
			auto cacheKey = path + typeid(T).name();
			if (m_assetCache.contains(cacheKey))
			{
				// Retorna a instância já carregada em memória
				return std::static_pointer_cast<T>(m_assetCache[cacheKey]);
			}

			// 2. Solicita os bytes brutos ao FileSystem
			auto rawBytes = m_fileSystem->ReadBinaryFile(path);

			// 3. Invoca o "ContentReader" específico para o tipo T.
			// Em C++, delegamos isso para um método estático da própria classe do Asset.
			std::shared_ptr<T> newAsset = T::CreateFromBytes(rawBytes, path);

			// 4. Salva no cache e retorna
			m_assetCache[cacheKey] = newAsset;
			return newAsset;
		}

		void ClearCache() noexcept {
			m_assetCache.clear();
		}

	private:
		FileSystem* m_fileSystem = nullptr;
		std::unordered_map<std::string, std::shared_ptr<Asset>> m_assetCache;
	};

} // namespace Dan

