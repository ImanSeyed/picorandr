cmake_minimum_required(VERSION 3.25)
project(picorandr LANGUAGES C)

include(FindPkgConfig)

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

pkg_check_modules(LIBDRM REQUIRED libdrm)
pkg_check_modules(LIBPCI REQUIRED libpci)

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${LIBDRM_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${LIBDRM_LIBRARIES} ${LIBPCI_LIBRARIES})

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
)
