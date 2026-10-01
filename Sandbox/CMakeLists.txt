# ============================================================
# Recursos Qt
# CMAKE_AUTORCC habilita a compilação automática de recursos do Qt
# AUTOMOC processará as macros Q_OBJECT e AUTOUIC processará arquivos .ui
# ============================================================
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# /EditorQt
set(EDITOR_QT_SRC 
    "EditorQt/SplashScreen.cpp"
    "EditorQt/EditorApp.cpp"
	"EditorQt/Qt/QtMain.cpp"
	"EditorQt/Resources/resources.qrc"
)

# ============================================================
# Configurações gerais
# ============================================================

add_executable (DanEngine ${EDITOR_QT_SRC})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET DanEngine PROPERTY CXX_STANDARD 20)
endif()

# -- Pacotes --
find_package(Qt6 COMPONENTS Widgets REQUIRED)

target_link_libraries(DanEngine 
	PRIVATE dan-core Qt6::Widgets)

# ============================================================
# Qt deployment
# windeployqt é necessário para analisar o executável e copiar automaticamente todas as DLLs necessárias
# ============================================================

if(WIN32)

    set(QT_TOOLS_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/Qt6/bin")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(WINDEPLOYQT_EXECUTABLE "${QT_TOOLS_DIR}/windeployqt.debug.bat")
    else()
        set(WINDEPLOYQT_EXECUTABLE "${QT_TOOLS_DIR}/windeployqt.exe")
    endif()

    if(EXISTS "${WINDEPLOYQT_EXECUTABLE}")
        message(STATUS "Qt deployment tool: ${WINDEPLOYQT_EXECUTABLE}")

        add_custom_command(TARGET DanEngine POST_BUILD
            COMMAND
                "${WINDEPLOYQT_EXECUTABLE}"
                "$<TARGET_FILE:DanEngine>"
            COMMENT
                "Executando windeployqt para fazer deploy das dependencias do Qt..."
            VERBATIM
        )

    else()
        message(WARNING
            "Qt deployment tool nao encontrado: "
            "${WINDEPLOYQT_EXECUTABLE}"
        )
    endif()
endif()
