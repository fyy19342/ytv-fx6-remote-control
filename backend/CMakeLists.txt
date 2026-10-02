cmake_minimum_required(VERSION 3.20)
project(ytv_fx6_remote_control_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "Minimum macOS deployment target" FORCE)
endif()

set(BUILD_INFO_JSON "${CMAKE_CURRENT_SOURCE_DIR}/../build_info.json")
if(NOT EXISTS "${BUILD_INFO_JSON}")
  message(FATAL_ERROR "build_info.json was not found at ${BUILD_INFO_JSON}")
endif()

file(READ "${BUILD_INFO_JSON}" BUILD_INFO_RAW)
string(JSON FX6_BUILD_ID GET "${BUILD_INFO_RAW}" build_id)
string(JSON FX6_VERSION GET "${BUILD_INFO_RAW}" version)
string(JSON FX6_APP_NAME GET "${BUILD_INFO_RAW}" app_name)
string(JSON FX6_BACKEND_PORT GET "${BUILD_INFO_RAW}" backend_port)

option(FX6_BUILD_SDK_BACKEND "Build the macOS backend (requires locally imported Sony SDK)" ON)
if(FX6_BUILD_SDK_BACKEND)
set(SONY_VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/sony")
set(SONY_INCLUDE_DIR "${SONY_VENDOR_DIR}/include")
set(SONY_LIB_DIR "${SONY_VENDOR_DIR}/lib")

if(NOT EXISTS "${SONY_INCLUDE_DIR}/CameraRemote_SDK.h")
  message(FATAL_ERROR "Sony SDK headers were not found. Run scripts/import_sdk.sh first.")
endif()

if(NOT EXISTS "${SONY_LIB_DIR}/libCr_Core.dylib")
  message(FATAL_ERROR "Sony SDK library libCr_Core.dylib was not found. Run scripts/import_sdk.sh first.")
endif()

find_package(Threads REQUIRED)
find_library(CR_CORE_LIB Cr_Core HINTS "${SONY_LIB_DIR}" REQUIRED NO_DEFAULT_PATH)

add_executable(fx6d
  src/nd_controller.cpp
  src/nd_state_machine.cpp
  src/main.cpp
  src/logger.cpp
  src/sdk_service.cpp
  src/sony_diagnostics.cpp
)

target_include_directories(fx6d PRIVATE
  src
  third_party
  "${SONY_INCLUDE_DIR}"
)

target_link_libraries(fx6d PRIVATE
  "${CR_CORE_LIB}"
  Threads::Threads
)

target_compile_definitions(fx6d PRIVATE
  FX6_APP_NAME="${FX6_APP_NAME}"
  FX6_BUILD_ID="${FX6_BUILD_ID}"
  FX6_VERSION="${FX6_VERSION}"
  FX6_BACKEND_PORT=${FX6_BACKEND_PORT}
)

if(APPLE)
  set_target_properties(fx6d PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    BUILD_RPATH "@executable_path;@executable_path/Contents/Frameworks/CrAdapter"
    INSTALL_RPATH "@executable_path;@executable_path/Contents/Frameworks/CrAdapter"
  )
  add_custom_command(TARGET fx6d PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:fx6d>/Contents/Frameworks/CrAdapter
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SONY_LIB_DIR}/libCr_Core.dylib $<TARGET_FILE_DIR:fx6d>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SONY_LIB_DIR}/libmonitor_protocol.dylib $<TARGET_FILE_DIR:fx6d>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SONY_LIB_DIR}/libmonitor_protocol_pf.dylib $<TARGET_FILE_DIR:fx6d>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SONY_LIB_DIR}/CrAdapter $<TARGET_FILE_DIR:fx6d>/Contents/Frameworks/CrAdapter
  )
endif()

endif()

enable_testing()

add_executable(nd_state_machine_test
  src/nd_state_machine.cpp
  tests/nd_state_machine_test.cpp
)

target_include_directories(nd_state_machine_test PRIVATE
  src
)

target_compile_definitions(nd_state_machine_test PRIVATE
  FX6_BUILD_ID="${FX6_BUILD_ID}"
  FX6_VERSION="${FX6_VERSION}"
)

add_test(NAME nd_state_machine_test COMMAND nd_state_machine_test)

add_executable(nd_controller_test src/nd_controller.cpp src/nd_state_machine.cpp tests/nd_controller_test.cpp)
target_include_directories(nd_controller_test PRIVATE src)
add_test(NAME nd_controller_test COMMAND nd_controller_test)
add_executable(exposure_steps_test tests/exposure_steps_test.cpp)
target_include_directories(exposure_steps_test PRIVATE src)
add_test(NAME exposure_steps_test COMMAND exposure_steps_test)
add_executable(network_target_test tests/network_target_test.cpp)
target_include_directories(network_target_test PRIVATE src)
add_test(NAME network_target_test COMMAND network_target_test)
add_executable(shutter_controller_test tests/shutter_controller_test.cpp)
target_include_directories(shutter_controller_test PRIVATE src)
add_test(NAME shutter_controller_test COMMAND shutter_controller_test)
add_executable(awb_controller_test tests/awb_controller_test.cpp)
target_include_directories(awb_controller_test PRIVATE src)
add_test(NAME awb_controller_test COMMAND awb_controller_test)

# These tests use assert; keep the assertions active in Release builds too.
foreach(test_target nd_state_machine_test nd_controller_test exposure_steps_test network_target_test shutter_controller_test awb_controller_test)
  if(MSVC)
    target_compile_options(${test_target} PRIVATE /UNDEBUG)
  else()
    target_compile_options(${test_target} PRIVATE -UNDEBUG)
  endif()
endforeach()
