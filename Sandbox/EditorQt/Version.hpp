#ifndef DAN_EDITORQT_VERSION_HPP
#define DAN_EDITORQT_VERSION_HPP

#include <string>
#include <format>
#include <compare>
#include <sstream>

namespace Dan {

    //Padronizar o armazenamento, a leitura e, principalmente, a comparação matemática de números de versão dentro da engine.
    struct Version
    {
        int major = 0;
        int minor = 0;
        int patch = 0;
        int build = 0;

        Version() = default;

        Version(int maj, int min, int pat, int bld = 1)
            : major(maj), minor(min), patch(pat), build(bld) {}

        // Modernização da leitura de string usando sstream e stoi (type-safe)
        explicit Version(const std::string& s)
        {
            std::stringstream ss(s);
            std::string item;

            if (std::getline(ss, item, '.')) major = std::stoi(item);
            if (std::getline(ss, item, '.')) minor = std::stoi(item);
            if (std::getline(ss, item, '.')) patch = std::stoi(item);
            if (std::getline(ss, item, '.')) build = std::stoi(item);
        }

        // C++20: Operador Spaceship (<=>)
        // Isso substitui AUTOMATICAMENTE todos os operadores <, >, <=, >= e ==
        // O compilador fará a comparação membro a membro na ordem em que foram declarados.
        auto operator<=>(const Version&) const = default;
        
        std::string ToString() const
        {
            return std::format("{}.{}.{}", major, minor, patch);
        }

        std::string ToFullString() const
        {
            return std::format("{}.{}.{}.{}", major, minor, patch, build);
        }
    };

} 

#endif