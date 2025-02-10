#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SPIRV-Tools-static" for configuration "Debug"
set_property(TARGET SPIRV-Tools-static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(SPIRV-Tools-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/libSPIRV-Tools.a"
  )

list(APPEND _cmake_import_check_targets SPIRV-Tools-static )
list(APPEND _cmake_import_check_files_for_SPIRV-Tools-static "${_IMPORT_PREFIX}/debug/lib/libSPIRV-Tools.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
