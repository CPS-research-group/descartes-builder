cmake_minimum_required(VERSION 3.8)

project(
  DescartesBuilder
  VERSION 0.1
  LANGUAGES CXX)

option(BUILD_TESTS "Build the tests" ON)
option(WIN_DEPLOY "Enable deployment of Qt dependencies for Windows" OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

add_definitions("-DQT_MESSAGELOGCONTEXT")

find_package(Python3 REQUIRED)
if (WIN32 AND WIN_DEPLOY)
  include_directories(${PYTHON3_INCLUDE_DIRS})
  link_libraries(${PYTHON3_LIBRARIES})
  find_package(ZLIB REQUIRED)
  include_directories(${ZLIB_INCLUDE_DIR})
  link_libraries(${ZLIB_LIBRARY})
  find_package(BZIP2 REQUIRED)
  include_directories(${BZIP2_INCLUDE_DIR})
  link_libraries(${BZIP2_LIBRARY})
endif()
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Test OpenGL
                                                       Core5Compat)

add_subdirectory(external/qtnodes)
add_subdirectory(external/qtutility)
add_subdirectory(external/quazip)

include_directories(include)

file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp")
file(GLOB_RECURSE PROJECT_HEADERS "include/*.hpp")
file(GLOB_RECURSE PROJECT_RESOURCES "resources/*.qrc")

add_library(${PROJECT_NAME}_lib SHARED ${PROJECT_SOURCES} ${PROJECT_HEADERS}
                                       ${PROJECT_RESOURCES})

target_include_directories(${PROJECT_NAME}_lib
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
  ${PROJECT_NAME}_lib
  PUBLIC Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::OpenGL QtNodes
         QtUtility QuaZip::QuaZip)

qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)

target_compile_options(
  ${PROJECT_NAME}
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4
          /wd4127
          /EHsc
          /utf-8>
          $<$<CXX_COMPILER_ID:GNU>:-Wall
          -Wextra>
          $<$<CXX_COMPILER_ID:AppleClang>:-Wall
          -Wextra
          -Werror>)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES ${BUNDLE_ID_OPTION} MACOSX_BUNDLE_BUNDLE_VERSION
             ${PROJECT_VERSION} MACOSX_BUNDLE_SHORT_VERSION_STRING
             ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} 
             MACOSX_BUNDLE TRUE WIN32_EXECUTABLE OFF)

include(GNUInstallDirs)
install(
  TARGETS ${PROJECT_NAME}
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(QT_VERSION_MAJOR EQUAL 6)
  qt_finalize_executable(${PROJECT_NAME})
endif()

function(deploy_qt_target target)
  if(WIN32 AND WIN_DEPLOY)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND "${CMAKE_PREFIX_PATH}/bin/windeployqt.exe"
      "$<TARGET_FILE:${target}>")
    add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${CMAKE_PREFIX_PATH}/bin/Qt6Core5Compat.dll" 
    "$<TARGET_FILE_DIR:${target}>")
  endif()
endfunction()

deploy_qt_target(${PROJECT_NAME})

if(BUILD_TESTS)
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
  add_subdirectory(external/googletest)
  include(GoogleTest)
  include(CTest)

  enable_testing()

  file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

  add_executable(unitTests ${TEST_SOURCES})

  target_link_libraries(unitTests 
  PRIVATE 
  gtest 
  ${PROJECT_NAME}_lib 
  Qt${QT_VERSION_MAJOR}::Test 
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Widgets)

  add_test(NAME unitTests COMMAND unitTests)
  set_tests_properties(unitTests PROPERTIES ENVIRONMENT "TEST_MODE=1")
  deploy_qt_target(unitTests)
endif()

# Pack
include(CPack)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Function + Data Flow Digital Twin Builder")
set(CPACK_PACKAGE_VENDOR "CNRS@CREATE")
set(CPACK_PACKAGE_CONTACT "contact@cnrsatcreate.sg")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

# Set installation directories
set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${PROJECT_NAME};ALL;/")

# Define the generator to use (e.g., ZIP, TGZ, DEB, RPM, etc.)
set(CPACK_GENERATOR "ZIP;TGZ;DEB;RPM")
