set(CUR_PROJECT_NAME dan-core)

set(PROJECT_SRC_FILES "src/Log.cpp")

add_library(${CUR_PROJECT_NAME} ${PROJECT_SRC_FILES})

find_package(spdlog CONFIG REQUIRED)

target_link_libraries(${CUR_PROJECT_NAME} PUBLIC spdlog::spdlog)

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