set(CUR_PROJECT_NAME dan-cls-tst)

set(PROJECT_SRC_FILES 
"main.cpp"
)

add_executable(${CUR_PROJECT_NAME} ${PROJECT_SRC_FILES})

set_target_properties(${CUR_PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_link_libraries(${CUR_PROJECT_NAME}
    PUBLIC
        dan-core
)