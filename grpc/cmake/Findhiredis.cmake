find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h PATHS /usr/local/include)
find_library(HIREDIS_LIBRARY NAMES hiredis PATHS /usr/local/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(hiredis DEFAULT_MSG HIREDIS_LIBRARY HIREDIS_INCLUDE_DIR)

mark_as_advanced(HIREDIS_INCLUDE_DIR HIREDIS_LIBRARY)
