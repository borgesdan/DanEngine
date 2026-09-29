# CMakeList.txt: projeto de CMake para DanEngine, incluir origem e definir
# lógica específica a um projeto aqui.
#

# Adicione a origem ao executável deste projeto.
add_executable (DanEngine "DanEngine.cpp" "DanEngine.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET DanEngine PROPERTY CXX_STANDARD 20)
endif()

# TODO: Adicione testes e instale destinos, se necessário.
find_package(Qt6 COMPONENTS Widgets REQUIRED)

target_link_libraries(DanEngine PRIVATE Qt6::Widgets)
