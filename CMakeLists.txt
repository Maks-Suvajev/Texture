add_library(TextureLib 
    source/TextureManager.cpp
)

find_package(Qt6 REQUIRED COMPONENTS OpenGL)

target_link_libraries(TextureLib 
    PUBLIC 
        Qt6::OpenGL
        TypeLib
        AssetsLib
)

target_include_directories(TextureLib 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        ${DEPS_INCLUDE_DIR}/stb    
)