# SPDX-License-Identifier: LGPL-2.1-or-later

EXECUTE_PROCESS(COMMAND ${Python3_EXECUTABLE} -c
    "from sysconfig import get_path; print(get_path('purelib'))"
    OUTPUT_VARIABLE python_libs OUTPUT_STRIP_TRAILING_WHITESPACE )

SET(PYTHON_MAIN_DIR ${python_libs})

set(NAMESPACE_DIR "${CMAKE_BINARY_DIR}/Ext/freecad")
set(NAMESPACE_INIT "${NAMESPACE_DIR}/__init__.py")
if (WIN32)
    get_filename_component(FREECAD_LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}"
                       REALPATH BASE_DIR "${CMAKE_INSTALL_PREFIX}")
    set( ${CMAKE_INSTALL_BINDIR})
else()
    get_filename_component(FREECAD_LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}"
                       REALPATH BASE_DIR "${CMAKE_INSTALL_PREFIX}")
endif()

configure_file(__init__.py.template ${NAMESPACE_INIT})

set(EXT_FILES
    freecad_doc.py
    module_io.py
    app/__init__.pyi
    part/__init__.py
    part/__init__.pyi
    partdesign.py
    project_utility.py
    sketcher.py
    UiTools.py
    utils.py
    utils_zip.py
)

foreach (it ${EXT_FILES})
    configure_file(${it} ${NAMESPACE_DIR}/${it})
endforeach()

file(GLOB PART_PYI_FILES "${CMAKE_SOURCE_DIR}/src/Mod/Part/App/*.pyi")

file(MAKE_DIRECTORY "${NAMESPACE_DIR}/part/internal")

foreach(pyifile ${PART_PYI_FILES})
    get_filename_component(basename ${pyifile} NAME)
    configure_file(${pyifile} "${NAMESPACE_DIR}/part/internal/${basename}")
endforeach()

file(GLOB BASE_PYI_FILES "${CMAKE_SOURCE_DIR}/src/Base/*.pyi")
file(GLOB APP_PYI_FILES "${CMAKE_SOURCE_DIR}/src/App/*.pyi")

file(MAKE_DIRECTORY "${NAMESPACE_DIR}/app/internal/Base")
foreach(pyifile ${BASE_PYI_FILES})
    get_filename_component(basename ${pyifile} NAME)
    configure_file(${pyifile} "${NAMESPACE_DIR}/app/internal/Base/${basename}")
endforeach()

file(MAKE_DIRECTORY "${NAMESPACE_DIR}/app/internal/App")
foreach(pyifile ${APP_PYI_FILES})
    get_filename_component(basename ${pyifile} NAME)
    configure_file(${pyifile} "${NAMESPACE_DIR}/app/internal/App/${basename}")
endforeach()

if (INSTALL_TO_SITEPACKAGES)
    SET(SITE_PACKAGE_DIR ${PYTHON_MAIN_DIR}/freecad)
else ()
    SET(SITE_PACKAGE_DIR Ext/freecad)
endif()

INSTALL(
    FILES
        ${NAMESPACE_INIT}
        ${EXT_FILES}
    DESTINATION
        ${SITE_PACKAGE_DIR}
)

INSTALL(
    FILES ${PYI_FILES}
    DESTINATION ${SITE_PACKAGE_DIR}/part/internal
)

INSTALL(
    FILES ${BASE_PYI_FILES}
    DESTINATION ${SITE_PACKAGE_DIR}/app/internal/Base
)

INSTALL(
    FILES ${APP_PYI_FILES}
    DESTINATION ${SITE_PACKAGE_DIR}/app/internal/App
)
