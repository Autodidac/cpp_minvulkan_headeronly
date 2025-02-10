#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::shaderc::shaderc" for configuration "Release"
set_property(TARGET unofficial::shaderc::shaderc APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(unofficial::shaderc::shaderc PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libshaderc.a"
  )

list(APPEND _cmake_import_check_targets unofficial::shaderc::shaderc )
list(APPEND _cmake_import_check_files_for_unofficial::shaderc::shaderc "${_IMPORT_PREFIX}/lib/libshaderc.a" )

# Import target "unofficial::shaderc::shaderc_util" for configuration "Release"
set_property(TARGET unofficial::shaderc::shaderc_util APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(unofficial::shaderc::shaderc_util PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libshaderc_util.a"
  )

list(APPEND _cmake_import_check_targets unofficial::shaderc::shaderc_util )
list(APPEND _cmake_import_check_files_for_unofficial::shaderc::shaderc_util "${_IMPORT_PREFIX}/lib/libshaderc_util.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
