# Copies given runtime library from Conan's full_deploy directory structure to
# ${CMAKE_BINARY_DIR}/runtime_libs for packaging.

function(copy_runtime_libs package libname)
  set(package_root "${CMAKE_BINARY_DIR}/full_deploy/host/${package}")
  file(GLOB_RECURSE lib_files
    "${package_root}/*/lib${libname}.so.*"
    "${package_root}/*/lib${libname}.dylib"
    "${package_root}/*/${libname}.dll"
  )
  # We want to avoid symlinks because they get dereferenced when packaged as a Python wheel.
  # Copy only e.g. libtbb.so.12, not libtbb.so.12.10 or libtbb.so.
  # Only the major soname is used by the dynamic linker.
  list(FILTER lib_files EXCLUDE REGEX ".+\\.so\\.[0-9]+\\.[0-9]+$")
  # Copy to runtime_libs.
  # Can't use `file(COPY ...)` because it does not dereference symlinks.
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/runtime_libs")
  foreach(file ${lib_files})
    get_filename_component(name "${file}" NAME)
    file(COPY_FILE "${file}" "${CMAKE_BINARY_DIR}/runtime_libs/${name}")
  endforeach()
endfunction()
