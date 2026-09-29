# CMakeList.txt: projeto de CMake para DanEngine, incluir origem e definir
# lógica específica a um projeto aqui.
#

# EditorQt
set(EDITOR_QT_SRC "EditorQt/Qt/QtMain.cpp")

add_executable (DanEngine ${EDITOR_QT_SRC})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET DanEngine PROPERTY CXX_STANDARD 20)
endif()

# TODO: Adicione testes e instale destinos, se necessário.
find_package(Qt6 COMPONENTS Widgets REQUIRED)

target_link_libraries(DanEngine PRIVATE Qt6::Widgets)
