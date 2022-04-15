cmake_minimum_required(VERSION 3.13)
project(triggeralgs VERSION 1.1.0)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
set(CMAKE_INSTALL_CMAKEDIR   ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}/cmake )

find_package(nlohmann_json REQUIRED)
find_package(TRACE REQUIRED)
find_package(detdataformats REQUIRED)

# We follow the daq-cmake convention of building one main library for
# the package. In our case, we include all of the available trigger
# implementations in the library (rather than, say, splitting them out
# into individual libraries).
#
# As in daq-cmake, headers that form part of the package's interface
# (ie, which are to be used by other packages) live in
# include/triggeralgs, while headers which are private to triggeralgs
# live in src/triggeralgs.

# We don't depend on any plugin library, so we don't have any
# plugins. If plugins are needed, they will be provided by the code
# which uses triggeralgs (eg, for dunedaq plugins, the plugin will be
# provided by the trigger package)
add_library(triggeralgs SHARED
  src/TriggerActivityMakerADCSimpleWindow.cpp
  src/TriggerCandidateMakerADCSimpleWindow.cpp
  src/TriggerActivityMakerHorizontalMuon.cpp
  src/TriggerCandidateMakerHorizontalMuon.cpp
  src/TriggerActivityMakerPrescale.cpp
  src/TriggerCandidateMakerPrescale.cpp
  src/TriggerActivityMakerSupernova.cpp
  src/TriggerCandidateMakerSupernova.cpp
  src/TriggerDecisionMakerSupernova.cpp)


target_include_directories(triggeralgs PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
target_link_libraries(triggeralgs PRIVATE nlohmann_json::nlohmann_json detdataformats::detdataformats)
install(TARGETS triggeralgs EXPORT triggeralgsTargets)

CONFIGURE_PACKAGE_CONFIG_FILE(cmake/triggeralgsConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/triggeralgsConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
  )

# TODO PAR 2021-04-15: What is in autogen? Is it actually used?
add_subdirectory(autogen)

install(EXPORT triggeralgsTargets DESTINATION ${CMAKE_INSTALL_CMAKEDIR} NAMESPACE triggeralgs::)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/triggeralgsConfig.cmake
        DESTINATION ${CMAKE_INSTALL_CMAKEDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/triggeralgsConfig.cmake
        DESTINATION ${CMAKE_INSTALL_CMAKEDIR})
install(DIRECTORY include/${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h??")
#install(DIRECTORY ${CMAKE_CODEGEN_BINARY_DIR}/include/${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h??")
