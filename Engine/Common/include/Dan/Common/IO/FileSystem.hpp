#ifndef DAN_IO_FILESYSTEM_HPP
#define DAN_IO_FILESYSTEM_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>

namespace Dan {

    class FileSystem
    {
    public:
        FileSystem();
        ~FileSystem() = default;

        // Define a pasta raiz de onde os assets serão lidos (ex: "DanEngine/Data")
        void MountRoot(const std::filesystem::path& rootPath){}

        // Lê um arquivo binário inteiro para a memória
        [[nodiscard]] std::vector<uint8_t> ReadBinaryFile(const std::string& virtualPath) const { return {}; }

        // Lê um arquivo de texto (útil para Shaders GLSL/HLSL ou JSONs)
        [[nodiscard]] std::string ReadTextFile(const std::string& virtualPath) const { return {}; }

    private:
        std::filesystem::path m_rootPath;
    };

} // namespace Dan

#endif