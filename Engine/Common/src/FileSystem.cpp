#include <Dan/Common/IO/FileSystem.hpp>
#include <Dan/Common/Log.hpp>
#include <Dan/Common/Macros/CoreAssert.hpp>
#include <fstream>

namespace Dan {

    std::vector<uint8_t> FileSystem::ReadBinaryFile(const std::string& virtualPath) const
    {
        // Concatena o caminho raiz (ex: "Assets/") com o caminho do arquivo (ex: "models/player.obj")
        std::filesystem::path fullPath = m_rootPath / virtualPath;

        // Abre o arquivo em modo binário e move o cursor para o final (ate = at end) para descobrirmos o tamanho
        std::ifstream file(fullPath, std::ios::binary | std::ios::ate);

        // Aciona o breakpoint na IDE (se em modo debug) caso o arquivo falhe ao abrir
        DAN_CORE_ASSERT(file.is_open(), "Falha ao abrir o arquivo binário: {0}", fullPath.string());

        if (!file.is_open())
        {
            // Registra o erro no console de forma formatada para o motor
            DAN_ENGINE_ERROR("FileSystem não encontrou ou não conseguiu abrir o arquivo: {0}", fullPath.string());
            return {};
        }

        // Pega o tamanho total do arquivo baseado na posição do cursor no final
        std::streamsize size = file.tellg();

        // Volta o cursor para o início do arquivo para começar a leitura
        file.seekg(0, std::ios::beg);

        // Aloca o buffer inteiro de uma vez (evita realocações dinâmicas custosas)
        std::vector<uint8_t> buffer(size);

        // Lê os bytes brutos diretamente para o ponteiro do buffer[cite: 1, 2]
        if (file.read(reinterpret_cast<char*>(buffer.data()), size))
        {
            return buffer;
        }

        DAN_ENGINE_ERROR("Falha durante a leitura dos bytes do arquivo binário: {0}", fullPath.string());
        return {};
    }

    std::string FileSystem::ReadTextFile(const std::string& virtualPath) const
    {
        std::filesystem::path fullPath = m_rootPath / virtualPath;

        // Abre em modo texto, mas ainda vai para o final para checar o tamanho
        std::ifstream file(fullPath, std::ios::ate);

        DAN_CORE_ASSERT(file.is_open(), "Falha ao abrir o arquivo de texto: {0}", fullPath.string());

        if (!file.is_open())
        {
            DAN_ENGINE_ERROR("FileSystem não encontrou ou não conseguiu abrir o arquivo: {0}", fullPath.string());
            return {};
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string buffer;
        // Aloca o tamanho da string de forma precisa
        buffer.resize(size);

        // Lê os caracteres diretamente para a string[cite: 1]
        if (file.read(buffer.data(), size))
        {
            return buffer;
        }

        DAN_ENGINE_ERROR("Falha durante a leitura do texto do arquivo: {0}", fullPath.string());
        return {};
    }

} // namespace Dan