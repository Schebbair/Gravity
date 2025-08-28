cmake_minimum_required(VERSION 3.10)
project(Simulador3D)

# Estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Archivos fuente
set(COMMON_SOURCES
    src/glad.c
)

# Incluir directorios de cabeceras
include_directories(include)

# Crear ejecutable
add_executable(simulador main.cpp ${COMMON_SOURCES})

# Enlazar librerías (GLFW, OpenGL, etc.)
target_link_libraries(simulador glfw GL dl X11 pthread)

add_executable(simulador_verlet main_verlet.cpp ${COMMON_SOURCES})
target_link_libraries(simulador_verlet glfw GL dl X11 pthread)

