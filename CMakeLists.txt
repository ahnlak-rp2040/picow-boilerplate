# CMakeLists.txt - part of the PicoW C++ Boilerplate Project
cmake_minimum_required(VERSION 3.12)

# Change your executable name to something creative!
set(NAME picow-boilerplate) # <-- Name your project/executable here!

# Explicitly define that the board is a PicoW
set(PICO_BOARD pico_w)

# We will ALWAYS need to import the Pico C/C++ SDK
include(pico_sdk_import.cmake)

# If you are using any Pimoroni devices, you will need to import these too.
# include(pimoroni_pico_import.cmake)

# Configure the project language options
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico C/C++ SDK
pico_sdk_init()

# Define the source files
add_executable(${NAME}
    main.cpp # <-- Add source files here!
)

# Include any required libraries; this makes their headers available to your code


# To include any Pimoroni specific ones, these includes assume 
# `pimoroni-pico` is stored alongside your project as per the guide.
#include(common/pimoroni_i2c)
#include(common/pimoroni_bus)
#include(libraries/bitmap_fonts/bitmap_fonts)
#include(libraries/hershey_fonts/hershey_fonts)
#include(libraries/pico_explorer/pico_explorer)

# We need to explicitly include any headers within our own project
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Define the libraries needed to link; we start with the ones we always
# need for any PicoW project
target_link_libraries(${NAME} 
    pico_stdlib pico_cyw43_arch_lwip_threadsafe_background
)

# And now any project specific requires, such as Pimoroni libraries
target_link_libraries(${NAME}
#    pimoroni_i2c
#    pico_explorer # <-- List libraries here!
)

# Ensure that we get a uf2 output
pico_add_extra_outputs(${NAME})

# Direct stdio to the USB port
pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 0)

# Define what goes into a release; at least the uf2, probably the README and LICENSE
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    ${CMAKE_CURRENT_LIST_DIR}/LICENSE
    DESTINATION .
)

# Configure the packager to make zip files
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP")
include(CPack)
