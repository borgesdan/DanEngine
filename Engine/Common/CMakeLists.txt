set(CUR_PROJECT_NAME dan-core)

set(PROJECT_SRC_FILES 
"src/Log.cpp"
"src/Math.cpp"
"src/EngineCore.cpp"
"src/Window.cpp"
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
# SDL
find_package(SDL3 CONFIG REQUIRED)

target_link_libraries(${CUR_PROJECT_NAME}
    PUBLIC
        spdlog::spdlog
        glm::glm
    PRIVATE
        SDL3::SDL3
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