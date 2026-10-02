# DanEngine - Módulo Math (Core)

A biblioteca `Math` da DanEngine foi completamente reescrita do zero, substituindo implementações manuais obsoletas dos anos 2000 da CryEngine 5.7 por uma arquitetura enxuta baseada em **C++20** e **GLM (OpenGL Mathematics)**. O foco do novo módulo é garantir abstrações de custo zero (*zero-cost abstractions*), type-safety absoluto e separação estrita de responsabilidades, isolando a matemática pura de qualquer sistema de UI, reflexão ou renderização.

## Mapeamento de Refatoração (Legado $\rightarrow$ Moderno)

| Estrutura Legada (CryEngine 5.7) | Estrutura Moderna (DanEngine) | Resumo Arquitetural |
| :--- | :--- | :--- |
| `PlaneH` | `Dan::Math::Plane` | Eliminação do abuso de `union` e sobrecarga obscura de operadores bit a bit (`operator|`). Implementação enxuta representando $Ax + By + Cz + D = 0$ com operações vetoriais delegadas à GLM. |
| `CAngle` / `CClampedAngle` | `Dan::Math::Angle` | Abandono de macros globais (`DEG2RAD`). Adoção de `std::numbers::pi_v` e do operador nativo C++20 Spaceship (`<=>`), eliminando dezenas de comparações condicionais manuais. |
| `CAngles3` / `Ang3` / `Ang3_tpl` | `Dan::Math::Vec3` (via GLM) | Estruturas redundantes para Ângulos de Euler foram sumariamente deletadas. O Core agora armazena *pitch*, *yaw* e *roll* utilizando o tipo vetor padrão de radianos e a extensão `<glm/gtx/euler_angles.hpp>`. |
| `CRotation` | `Dan::Math::Quat` (`glm::quat`) | Remoção da classe *wrapper* que acoplava matemática de quatérnios com macros de UI (`Schematyc`). O Core agora consome primitivas da GLM diretamente. |
| `OBB_tpl` (Aninhado em `CCamera`) | `Dan::Math::OBB` | Extração do *Oriented Bounding Box* de dentro da *God Class* da Câmera. Agora atua como um POD (*Plain Old Data*) puramente descritivo, separando a representação geométrica dos algoritmos de intersecção (*Frustum Culling*). |

## Princípios Adotados na Modernização

* **Isolamento de Domínio (SRP):** A matemática da engine foi totalmente higienizada. Funções de reflexão de variáveis (`ReflectType`), macros de serialização visual de Editor e dependências diretas de instâncias de Câmeras foram removidas das classes matemáticas fundamentais. A matemática agora apenas computa; quem a consome é responsável pela renderização e interface.
* **Fim da Trigonometria Manual:** Blocos gigantescos de extração de raízes e funções trigonométricas obsoletas (`asin_tpl`, `atan2_tpl`) foram substituídos por chamadas em linha aos pacotes otimizados da GLM, garantindo alinhamento de memória nativo e vetorização SIMD imediata.
* **Namespaces e User-Defined Literals (UDL):** Atalhos visuais para desenvolvedores (como `90.0_deg` ou `1.5_rad`) foram movidos para um sub-namespace seguro (`Dan::Math::Literals`), evitando poluição do escopo global de compilação.
* **Constexpr por Padrão:** Onde matematicamente viável, construtores e operações vetoriais simples foram marcados como `constexpr`, permitindo que o compilador do C++20 resolva expressões matemáticas estáticas em tempo de compilação, em vez de onerar o tempo de execução (*runtime*).