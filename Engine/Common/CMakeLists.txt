set(CUR_PROJECT_NAME dan-core)

set(PROJECT_SRC_FILES 
"src/Log.cpp"
"src/Math.cpp"
"src/EngineCore.cpp"
"src/FileSystem.cpp"
"src/Window.cpp"
"src/Renderer.cpp"
"src/Clock.cpp"
"src/Mesh.cpp"
"src/GameFramework.cpp"
"src/Shader.cpp"
"src/Texture2D.cpp"
"src/SceneSerializer.cpp"
"src/PhysicsSystem.cpp"
)

add_library(${CUR_PROJECT_NAME} ${PROJECT_SRC_FILES})

set_target_properties(${CUR_PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Logs do sistema
find_package(spdlog CONFIG REQUIRED)
# Biblioteca matemática
find_package(glm CONFIG REQUIRED)
# Gerenciamento de janelas e sistemas
find_package(SDL3 CONFIG REQUIRED)
# Gerenciamento de gráficos
find_package(bgfx CONFIG REQUIRED)
# Gerenciamento de entidades
find_package(EnTT CONFIG REQUIRED)
# Leitura de modelos 3D primitivos
find_package(assimp CONFIG REQUIRED)
# Gerenciamento de arquivos json
find_package(nlohmann_json CONFIG REQUIRED)
# Física e colisão
find_package(Jolt CONFIG REQUIRED)
# Leitura e carregamento de imagens
find_path(STB_INCLUDE_DIRS "stb_image.h")
target_include_directories(dan-core PRIVATE ${STB_INCLUDE_DIRS})

target_link_libraries(${CUR_PROJECT_NAME}
    PUBLIC
        spdlog::spdlog
        glm::glm
        EnTT::EnTT    
        SDL3::SDL3
        bgfx::bgfx
        bgfx::bx
        bgfx::bimg  
        assimp::assimp
        nlohmann_json::nlohmann_json
        Jolt::Jolt
)

# --- Exportação da lib ---

# Lógica mágica de exportação/importação:
if(BUILD_SHARED_LIBS)
    # Se for DLL, dizemos apenas para a lib exportar os símbolos.
    # Os consumidores não recebem essa macro e caem no 'dllimport'.
    target_compile_definitions(${CUR_PROJECT_NAME} PRIVATE DAN_EXPORT_DLL)
else()
    # Se for ESTÁTICO, nós desligamos o import/export.
    # Usamos PUBLIC para que qualquer target que faça link com a lib.
    target_compile_definitions(${CUR_PROJECT_NAME} PUBLIC DAN_STATIC)
endif()

target_include_directories(${CUR_PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>   
)

install(
    TARGETS ${CUR_PROJECT_NAME}
    EXPORT ${CUR_PROJECT_NAME}Targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(
    DIRECTORY include/
    DESTINATION include
)