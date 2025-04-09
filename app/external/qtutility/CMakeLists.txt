cmake_minimum_required(VERSION 3.8)
project(QtUtility)

option(BUILD_TESTS_QTUTILITY "BUILD TESTS" OFF)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Test)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp")
file(GLOB_RECURSE PROJECT_HEADERS "include/*.hpp")
file(GLOB_RECURSE PROJECT_RESOURCES "resources/*.qrc")

add_library(QtUtility SHARED ${PROJECT_SOURCES} ${PROJECT_HEADERS}
                             ${PROJECT_RESOURCES})

include(GNUInstallDirs)

target_include_directories(
  QtUtility
  PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_link_libraries(QtUtility PUBLIC Qt${QT_VERSION_MAJOR}::Core
                                       Qt${QT_VERSION_MAJOR}::Widgets)

target_compile_definitions(QtUtility PRIVATE QTUTILITY_LIBRARY)

qt_wrap_cpp(utility_moc ${PROJECT_HEADERS} TARGET QtUtility OPTIONS --no-notes)

target_sources(QtUtility PRIVATE ${utility_moc})

install(
  TARGETS QtUtility
  EXPORT QtUtilityConfig
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINARYDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
  EXPORT QtUtilityConfig
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtUtility/
  NAMESPACE QtUtility::)
export(EXPORT QtUtilityConfig NAMESPACE QtUtility::)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(BUILD_TESTS_QTUTILITY)
  message("BUILDING TESTS")
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
  add_subdirectory(external/googletest)
  include(GoogleTest)
  include(CTest)

  enable_testing()

  file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

  add_executable(unitTests ${TEST_SOURCES})

  target_link_libraries(
    unitTests PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Test
                      gtest QtUtility)

  add_test(NAME unitTests COMMAND unitTests)
endif()
