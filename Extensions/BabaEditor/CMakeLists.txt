# Target name
set(target BabaEditor)

# Packages
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h")

# Sources
set(sources
    LevelFile.hpp
    LevelEditor.cpp
    main.cpp)

add_executable(${target}
    ${sources})

# Project options
set_target_properties(${target}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
)

# Compile options
target_compile_options(${target}
    PRIVATE
    ${DEFAULT_COMPILE_OPTIONS}
)
target_compile_definitions(${target}
    PRIVATE
    $<$<PLATFORM_ID:Darwin>:GL_SILENCE_DEPRECATION>
    BABA_EDITOR_MAPS_DIR="${PROJECT_SOURCE_DIR}/Resources/Maps/"
    BABA_EDITOR_SPRITES_DIR="${PROJECT_SOURCE_DIR}/Extensions/BabaGUI/sprites/")

# Include directories
target_include_directories(${target}
    PRIVATE
    ${STB_INCLUDE_DIRS})

# Linker options
target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LINKER_OPTIONS}
    baba-is-auto
    glfw
    imgui::imgui
    OpenGL::GL)
