#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "QXlsx::QXlsx" for configuration "Release"
set_property(TARGET QXlsx::QXlsx APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(QXlsx::QXlsx PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/QXlsxQt5.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/QXlsxQt5.dll"
  )

list(APPEND _cmake_import_check_targets QXlsx::QXlsx )
list(APPEND _cmake_import_check_files_for_QXlsx::QXlsx "${_IMPORT_PREFIX}/lib/QXlsxQt5.lib" "${_IMPORT_PREFIX}/bin/QXlsxQt5.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
