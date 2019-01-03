# CMake file for crosscompiling Maxima/wxMaxima for Windows
# Copyright (C) by Wolfgang Dautermann
# License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>
# This is free software: you are free to change and redistribute it.
# There is NO WARRANTY, to the extent permitted by law.

set(CMAKE_SYSTEM_NAME Windows)

option(BUILD_64BIT "Build a 64 bit installer." NO)

if(BUILD_64BIT)
    set(HOST x86_64-w64-mingw32)
    set(INSTALLERSUFFIX "win64")
else()
    set(HOST i686-w64-mingw32)
    set(INSTALLERSUFFIX "win32")
endif()

set(CMAKE_C_COMPILER   ${HOST}-gcc)
set(CMAKE_CXX_COMPILER ${HOST}-g++)
set(CMAKE_RC_COMPILER  ${HOST}-windres)
project(maxima)
cmake_minimum_required(VERSION 3.3)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/ ${CMAKE_BINARY_DIR}/)


# On which windows drive (C:, D:, ...) will be the default installation path?
set(WINDOWS_DRIVELETTER "C")


# What is the build system?
execute_process(COMMAND gcc -dumpmachine OUTPUT_VARIABLE BUILDHOST OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Build system: ${BUILDHOST}")

# Output the host system, for which we are compiling
message(STATUS "Host system: ${HOST}")


file(READ ${CMAKE_ROOT}/Modules/NSIS.template.in MYNSISTEMPLATE)
string(REPLACE "RequestExecutionLevel admin" "RequestExecutionLevel user
!include \"${CMAKE_SOURCE_DIR}\\\\\\\\FileAssociation.nsh\"
!include FileFunc.nsh
!include LogicLib.nsh
" MYNSISTEMPLATE "${MYNSISTEMPLATE}")

string(REPLACE "Function .onInit" "
Function .onInit
\${GetParameters} \$0
\${GetOptions} \$0 \"/no_vtk\" \$1
\${IfNot} \${Errors}
    !insertmacro UnselectSection \${vtk}
\${EndIf}
\${GetOptions} \$0 \"/no_gnuplot\" \$1
\${IfNot} \${Errors}
    !insertmacro UnselectSection \${gnuplot}
\${EndIf}
\${GetOptions} \$0 \"/no_wxmaxima\" \$1
\${IfNot} \${Errors}
    !insertmacro UnselectSection \${wxmaxima}
\${EndIf}
" MYNSISTEMPLATE "${MYNSISTEMPLATE}")
file(WRITE ${CMAKE_BINARY_DIR}/NSIS.template.in "${MYNSISTEMPLATE}")
message(STATUS "Copied and patched ${CMAKE_ROOT}/Modules/NSIS.template.in to ${CMAKE_BINARY_DIR}/NSIS.template.in")

option(USE_WXMAXIMA_GIT "Use wxMaxima development snapshot." NO)
option(USE_VTK "Use VTK." YES)
# the following option is used for nightly builds of the installer.
# it builds the current Maxima/wxMaxima Git versions and also
# sets the installation directory to ${WINDOWS_DRIVELETTER}:\maxima-current
option(BUILD_CURRENT "Build a installer for current Maxima/wxMaxima." NO)
# SBCL (64 Bit) does currently not support Wine, so disable that option:
# option(SBCL_64BIT "Build a installer with a 64 bit SBCL." NO)

option(WITH_ABCL "Include ABCL in the installer." NO)

# another option (option() can not be used, it is only boolean...)
#
# SBCL and CLISP have both advantages and disadvantages:
#  - SBCL is fast, but sometimes runs out of memory in places it really shouldn't
#    (Loading LAPACK on IA32, for example) and isn't too happy with changing to
#    non-ascii-non-unicode pathnames (which wxMaxima works around by now, though,
#    and which Xmaxima doesn't evven try to do)
#  - CLISP is slow. And version 2.49 only sends valid output over the network
#    if the front-end is slow enough. Test cases: Running a demo, all commands
#    with loads of compiler output (load("draw"); for the first time) or just
#    receiving a batch of new autocompletion symbols. Since the libraries the
#    front-ends use have evolved in the last years it has become easy to
#    trigger this => Setting SBCL to be the default.
if(WITH_ABCL)
    set(WITH_DEFAULT_LISP "sbcl" CACHE STRING "Which Lisp should be used by default (clisp/sbcl/abcl)?")
    set_property(CACHE WITH_DEFAULT_LISP PROPERTY STRINGS "clisp" "sbcl" "abcl")
    set(ABCL_LICENSE_INFORMATION "<li><a href=\"http://www.abcl.org/\">ABCL Website</a>, <a href=\"https://abcl.org/faq.shtml#license\">ABCL license</a></li>")
    set(ABCL_ENABLED 1)
else()
    set(WITH_DEFAULT_LISP "sbcl" CACHE STRING "Which Lisp should be used by default (clisp/sbcl)?")
    set_property(CACHE WITH_DEFAULT_LISP PROPERTY STRINGS "clisp" "sbcl")
    set(ABCL_ENABLED 0)
endif()

# If you want to use a updated version of a program,
# update the version number and the checksum.
# If no further patches are needed, you should get a
# updated setup-file automatically.

# current program versions
set(CLISPVERSION     2.49)
set(WXMAXIMAVERSION  16.04.2)
set(WXWIDGETSVERSION 3.0.2)
set(GNUPLOTVERSION   5.0.4)
set(TCLVERSION       8.6.5)
set(TKVERSION        8.6.5)
set(VTKVERSION       6.3.0)
set(SBCLVERSION      1.3.7)


# MD5 checksums for the current program versions.
set(CLISP_MD5       "8d58d64f83dd7ad01c054aecc16c1f52")

if(BUILD_CURRENT)
    set(USE_WXMAXIMA_GIT YES)
    set(MAXIMAVERSION "current")
else()
    # Get Maxima version from configure.ac
    file(STRINGS "${CMAKE_SOURCE_DIR}/../configure.ac" CONFIGUREAC REGEX "AC_INIT\\(.*\\)")
    string(REGEX REPLACE "AC_INIT\\(\\[maxima\\], \\[(.*)\\]\\)" "\\1" MAXIMAVERSION ${CONFIGUREAC})
    message(STATUS "Maxima version (from configure.ac): ${MAXIMAVERSION}")
endif()



string(REPLACE "." "" GNUPLOTVERSIONWITHOUTDOTS "${GNUPLOTVERSION}")

# download URLs
set(CLISP_URL     "http://downloads.sourceforge.net/project/clisp/clisp/${CLISPVERSION}/clisp-${CLISPVERSION}-win32-mingw-big.zip")
set(WXMAXIMA_URL  "http://sourceforge.net/projects/wxmaxima/files/wxMaxima/${WXMAXIMAVERSION}/wxmaxima-${WXMAXIMAVERSION}.tar.gz")
set(WXWIDGETS_URL "https://sourceforge.net/projects/wxwindows/files/${WXWIDGETSVERSION}/wxWidgets-${WXWIDGETSVERSION}.tar.bz2")
set(GNUPLOT_URL   "http://sourceforge.net/projects/gnuplot/files/gnuplot/${GNUPLOTVERSION}/gp${GNUPLOTVERSIONWITHOUTDOTS}-win32-mingw.zip")
set(TCL_URL       "http://prdownloads.sourceforge.net/tcl/tcl${TCLVERSION}-src.tar.gz")
set(TK_URL        "http://prdownloads.sourceforge.net/tcl/tk${TKVERSION}-src.tar.gz")
set(VTK_URL       "http://www.vtk.org/files/release/6.3/vtkpython-${VTKVERSION}-Windows-32bit.exe")
# SBCL (64 Bit) does currently not support Wine, so disable that option:
# if(SBCL_64BIT)
#    set(SBCL_INSTALLER "sbcl-${SBCLVERSION}-x86-64-windows-binary.msi")
#    set(SBCL_MD5      "f8cd3044c74beb84bba4aa9df5bb804e")
# else()
    set(SBCL_INSTALLER "sbcl-${SBCLVERSION}-x86-windows-binary.msi")
    set(SBCL_MD5      "a49b6f18ca9adf374b6ec4cbf90c698e")
# endif()
set(SBCL_URL      "http://prdownloads.sourceforge.net/sbcl/${SBCL_INSTALLER}")


# Where are libgcc_s_sjlj-1.dll and libstdc++-6.dll?
#
execute_process(COMMAND "${CMAKE_CXX_COMPILER}" -print-file-name=libgcc_s_sjlj-1.dll OUTPUT_VARIABLE MINGW_LIBGCC    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "${CMAKE_CXX_COMPILER}" -print-file-name=libstdc++-6.dll     OUTPUT_VARIABLE MINGW_LIBSTDCPP OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "Found libstdc++-6.dll at ${MINGW_LIBSTDCPP}")
message(STATUS "Found libgcc_s_sjlj-1.dll at ${MINGW_LIBGCC}")

if(SBCL_64BIT)
find_program(WINE_EXE NAMES wine64)
else()
find_program(WINE_EXE NAMES wine)
endif()

if(WINE_EXE)
    message(STATUS "Found wine at ${WINE_EXE}")
else()
    message(FATAL_ERROR "Wine not found. It is required for crosscompiling Maxima.")
endif()

find_program(SEVENZIP_EXE NAMES 7z)

if(SEVENZIP_EXE)
    message(STATUS "Found 7z at ${SEVENZIP_EXE}")
else()
    message(FATAL_ERROR "7z not found. It is required for crosscompiling Maxima.")
endif()

include(ExternalProject)

# Build wxWidgets static library (required for wxMaxima)
# --host=${HOST} should suffice, but then wxwidgets creates libs with wrong names, so add --build=${HOST}
# (that should mean, the build system is Windows - wrong. Perhaps a bug in the wxwidgets build system.)
externalproject_add(wxWidgets
  URL "${WXWIDGETS_URL}"
  DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/downloads
  URL_MD5 ${WXWIDGETS_MD5}
  CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/wxWidgets-prefix/src/wxWidgets/configure --disable-shared --disable-compat28 --with-zlib=builtin --with-libjpeg=builtin --without-libtiff --with-libpng=builtin --with-expat=builtin --host=${HOST} --build=${HOST}
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND ""
)

# build tcl/tk (required for xmaxima and the Lisp selection tool)
externalproject_add(tcl
  URL "${TCL_URL}"
  DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/downloads
  URL_MD5 ${TCL_MD5}
  CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/tcl-prefix/src/tcl/win/configure --host=${HOST} --prefix=C:/maxima-${MAXIMAVERSION}
  BUILD_COMMAND $(MAKE)
        COMMAND $(MAKE) install
)
install(DIRECTORY ${CMAKE_BINARY_DIR}/tcl-prefix/src/tcl-build/C:/maxima-${MAXIMAVERSION}/ DESTINATION .)

# Clisp (already a binary package just extract it...)
#
# What is the difference between clisp-2.49-win32-mingw-big.zip and clisp-2.49-win32-mingw-small.zip?
# What should be used for running Maxima?
externalproject_add(clisp
  URL "${CLISP_URL}"
  DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/downloads
  URL_MD5 ${CLISP_MD5}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
install(DIRECTORY ${CMAKE_BINARY_DIR}/clisp-prefix/src/clisp/ DESTINATION clisp-${CLISPVERSION} COMPONENT Clisp)

add_subdirectory("sbcl/")

add_subdirectory("gnuplot/")

add_subdirectory("tcltk/")

add_subdirectory("wxwidgets/")

install(DIRECTORY ${CMAKE_BINARY_DIR}/sbcl/ DESTINATION bin)

if(USE_VTK)
    add_subdirectory("vtk/")
endif()

if(WITH_ABCL)
    add_subdirectory("abcl/")
endif()

set(CLISPROOT "${CMAKE_BINARY_DIR}/clisp-prefix/src/clisp")
configure_file("${CMAKE_SOURCE_DIR}/wine-clisp.sh.tmpl" "${CMAKE_BINARY_DIR}/wine-clisp.sh")
configure_file("${CMAKE_SOURCE_DIR}/wine-lisp.sh.tmpl" "${CMAKE_BINARY_DIR}/wine-lisp.sh")
configure_file("${CMAKE_SOURCE_DIR}/wine-sbcl.sh.tmpl" "${CMAKE_BINARY_DIR}/wine-sbcl.sh")

externalproject_add(maxima
  # first copy the maxima source to a build directory, since out-of-source-builds do not work
  DOWNLOAD_COMMAND rsync -a --exclude=crosscompile-windows/build/ "${CMAKE_SOURCE_DIR}/../"  "${CMAKE_BINARY_DIR}/maxima-prefix/src/maxima"
  DEPENDS clisp sbcl tcl tk
  BUILD_IN_SOURCE 1
  #PATCH_COMMAND sh -c "test \"${BUILD_64BIT}\" != \"OFF\" && patch --binary -p1 < ${CMAKE_SOURCE_DIR}/maxima.bat.in-patch || exit 0"
  CONFIGURE_COMMAND sh -c "test -x ./bootstrap && ./bootstrap || return 0"
            COMMAND ./configure --host=${HOST} --enable-clisp --with-clisp=${CMAKE_BINARY_DIR}/wine-clisp.sh --with-clisp-runtime=${CMAKE_BINARY_DIR}/wine-lisp.sh --enable-sbcl --with-sbcl=${CMAKE_BINARY_DIR}/wine-sbcl.sh --with-default-lisp=${WITH_DEFAULT_LISP} --prefix=${WINDOWS_DRIVELETTER}:/maxima-${MAXIMAVERSION}
  # clisp-depends.mk is generated with "\" as path seperator, not "/" - correct that...
  BUILD_COMMAND LANG=C $(MAKE) -C src clisp-depends.mk
        COMMAND sed -i s+\\\\+/+g src/clisp-depends.mk
        COMMAND LANG=C $(MAKE)
        COMMAND LANG=C $(MAKE) pdf
  INSTALL_COMMAND LANG=C $(MAKE) install DESTDIR=${CMAKE_BINARY_DIR}/maxima-installroot/
)

# crosscompiling winkill with autoconf does not work, compile it using CMake.
add_library(winkill_lib SHARED ${CMAKE_SOURCE_DIR}/../interfaces/xmaxima/win32/winkill_lib.c)
add_executable(winkill ${CMAKE_SOURCE_DIR}/../interfaces/xmaxima/win32/winkill.c)
set_target_properties(winkill_lib PROPERTIES PREFIX "") # remove 'lib' prefix from libwinkill_lib.dll
target_link_libraries(winkill winkill_lib)
install(TARGETS winkill winkill_lib COMPONENT Maxima RUNTIME DESTINATION bin LIBRARY DESTINATION bin/)
install(FILES "${CMAKE_SOURCE_DIR}/../interfaces/xmaxima/win32/win_signals.lisp" DESTINATION bin/ COMPONENT Maxima)

# convert windows short names to long names (e.g.
# "C:/Users/TESTTE~1/AppData/Local/Temp" to "C:/Users/test test/AppData/Local/Temp")
# CLISP can (currently not handle short names.
add_executable(maxima_longnames maxima_longnames.c)
install(FILES "${CMAKE_BINARY_DIR}/maxima_longnames.exe" DESTINATION bin/ COMPONENT Maxima)


install(DIRECTORY ${CMAKE_BINARY_DIR}/maxima-installroot/${WINDOWS_DRIVELETTER}:/maxima-${MAXIMAVERSION}/ DESTINATION . COMPONENT Maxima)
install(FILES "${CMAKE_SOURCE_DIR}/xmaxima.bat" DESTINATION bin/ COMPONENT Maxima)
install(FILES "${CMAKE_BINARY_DIR}/maxima-prefix/src/maxima/doc/info/maxima.pdf" DESTINATION share/doc/ COMPONENT Maxima)
configure_file("${CMAKE_SOURCE_DIR}/lispselector.tcl" "${CMAKE_BINARY_DIR}/lispselector.tcl")
install(FILES "${CMAKE_BINARY_DIR}/lispselector.tcl" DESTINATION bin/ COMPONENT Maxima)
install(FILES "${CMAKE_SOURCE_DIR}/lispselector.bat" DESTINATION bin/ COMPONENT Maxima)

if(USE_WXMAXIMA_GIT)
    externalproject_add(wxMaxima-git
        GIT_REPOSITORY ${WXMAXIMA_GIT}
        DEPENDS wxWidgets
        PATCH_COMMAND  ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/wxWidgets-prefix/src/wxWidgets/wxwin.m4 ${CMAKE_BINARY_DIR}/wxMaxima-git-prefix/src/wxMaxima-git/acinclude.m4
              COMMAND ${CMAKE_BINARY_DIR}/wxMaxima-git-prefix/src/wxMaxima-git/bootstrap
        CONFIGURE_COMMAND  ${CMAKE_BINARY_DIR}/wxMaxima-git-prefix/src/wxMaxima-git/configure --host=${HOST} --with-wx-config=${CMAKE_BINARY_DIR}/wxWidgets-prefix/src/wxWidgets-build/lib/wx/config/inplace-msw-unicode-static-3.0 --prefix=C:/maxima-${MAXIMAVERSION}
        BUILD_COMMAND $(MAKE)
              COMMAND $(MAKE) allmo
              COMMAND $(MAKE) wxMaxima.win
       INSTALL_COMMAND ""
    )
    install(DIRECTORY ${CMAKE_BINARY_DIR}/wxMaxima-git-prefix/src/wxMaxima-git-build/wxMaxima/ DESTINATION wxMaxima)
else()
    externalproject_add(wxMaxima
        URL "${WXMAXIMA_URL}"
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/downloads
        URL_MD5 ${WXMAXIMA_MD5}
        PATCH_COMMAND patch -p0 < ${CMAKE_SOURCE_DIR}/wxmaxima-16.04.2-patch
        DEPENDS wxWidgets
        CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/wxMaxima-prefix/src/wxMaxima/configure --host=${HOST} --with-wx-config=${CMAKE_BINARY_DIR}/wxWidgets-prefix/src/wxWidgets-build/lib/wx/config/inplace-msw-unicode-static-3.0 --prefix=C:/maxima-${MAXIMAVERSION}
        BUILD_COMMAND $(MAKE)
              COMMAND $(MAKE) allmo
              COMMAND $(MAKE) wxMaxima.win
        INSTALL_COMMAND ""
    )
    install(DIRECTORY ${CMAKE_BINARY_DIR}/wxMaxima-prefix/src/wxMaxima-build/wxMaxima/ DESTINATION wxMaxima)
endif()

if(USE_VTK)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/vtk)
    file(WRITE ${CMAKE_BINARY_DIR}/download-vtk.cmake "file(DOWNLOAD ${VTK_URL} ${CMAKE_SOURCE_DIR}/downloads/vtkpython-${VTKVERSION}-Windows-32bit.exe SHOW_PROGRESS EXPECTED_HASH MD5=${VTK_MD5})")
    add_custom_target(vtk ALL
        WORKING_DIRECTORY vtk
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/download-vtk.cmake
        COMMAND ${SEVENZIP_EXE} x -y ${CMAKE_SOURCE_DIR}/downloads/vtkpython-${VTKVERSION}-Windows-32bit.exe
        
    )
    install(DIRECTORY ${CMAKE_BINARY_DIR}/vtk/\$_OUTDIR/bin DESTINATION .)
endif()

# Include DLLs from the current Mingw environment.
install(FILES "${MINGW_LIBSTDCPP}" "${MINGW_LIBGCC}" DESTINATION wxMaxima/ COMPONENT wxMaxima)
install(FILES "${MINGW_LIBSTDCPP}" "${MINGW_LIBGCC}" DESTINATION bin/ COMPONENT Maxima)

configure_file("${CMAKE_SOURCE_DIR}/licenses.html" "${CMAKE_BINARY_DIR}/licenses.html")
install(FILES "${CMAKE_BINARY_DIR}/licenses.html" DESTINATION share/doc/ COMPONENT Maxima)


set(CPACK_GENERATOR "NSIS")
set(CPACK_PACKAGE_VERSION "${MAXIMAVERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Maxima")
set(CPACK_PACKAGE_VENDOR "Maxima Team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/../README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/../COPYING")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "maxima-${CPACK_PACKAGE_VERSION}")
set(CPACK_NSIS_INSTALL_ROOT "${WINDOWS_DRIVELETTER}:")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/maxima-icon.bmp")
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/../interfaces/xmaxima\\\\maxima-icon.ico")
set(CPACK_NSIS_MUI_UNIICON "${CPACK_NSIS_MUI_ICON}")
set(CPACK_PACKAGE_EXECUTABLES "..\\\\wxMaxima\\\\wxmaxima" "wxMaxima (GUI for Maxima)")
set(CPACK_NSIS_URL_INFO_ABOUT "http://maxima.sourceforge.net")
set(CPACK_NSIS_MENU_LINKS "bin/maxima.bat" "Maxima (command line)" "bin/xmaxima.bat" "XMaxima (simple GUI)" "http://maxima.sourceforge.net" "About Maxima" "http://wxmaxima-developers.github.io/wxmaxima/" "About wxMaxima" "share/doc/maxima.pdf" "Maxima documentation"  "share/doc/licenses.html" "Maxima and included software licenses" "bin/lispselector.bat" "Configure default LISP for Maxima")
set(CPACK_PACKAGE_FILE_NAME "maxima-clisp-sbcl-${CPACK_PACKAGE_VERSION}-${INSTALLERSUFFIX}")
set(CPACK_NSIS_COMPRESSOR "/SOLID lzma")

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "wxMaxima")
    #File associations:
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    \\\${registerExtension} \\\"\\\$INSTDIR\\\\wxMaxima\\\\wxmaxima.exe\\\" \\\".wxm\\\" \\\"wxMaxima Document\\\"
    \\\${registerExtension} \\\"\\\$INSTDIR\\\\wxMaxima\\\\wxmaxima.exe\\\" \\\".wxmx\\\" \\\"wxMaxima XML Document\\\"
    \\\${registerExtension} \\\"\\\$INSTDIR\\\\wxMaxima\\\\wxmaxima.exe\\\" \\\".mac\\\" \\\"Maxima Batch file\\\"
    WriteRegStr HKCR \\\".wxmx\\\\ShellNew\\\" \\\"NullFile\\\" \\\"\\\"
    ")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    \\\${unregisterExtension} \\\".wxm\\\" \\\"wxMaxima Document\\\"
    \\\${unregisterExtension} \\\".wxmx\\\" \\\"wxMaxima XML Document\\\"
    \\\${unregisterExtension} \\\".mac\\\" \\\"Maxima Batch File\\\"
    DeleteRegKey HKCR \\\".wxmx\\\\ShellNew\\\"
    ")
endif()

set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "
  !define MUI_WELCOMEFINISHPAGE_BITMAP \\\"${CMAKE_SOURCE_DIR}/maxima-installerimage.bmp\\\"")
include(CPack)

cpack_add_component(Maxima
                    DESCRIPTION "The Maxima computer algebra system.\nRequired."
                    REQUIRED)

if(USE_VTK)
    cpack_add_component(VTK
                        DESCRIPTION "VTK is an advanced graphic toolkit for draw().\nOptional.")
endif()

cpack_add_component(Gnuplot
                    DESCRIPTION "Gnuplot is the main graphic toolkit.\nInstallation is strongly recommended.")

cpack_add_component(wxMaxima
                    DESCRIPTION "wxMaxima is an advanced GUI for Maxima based on wxWidgets.\nInstallation is strongly recommended.")

cpack_add_component(Clisp
                    DESCRIPTION "The CLISP Lisp compiler.\nRequired."
                    REQUIRED)

cpack_add_component(SBCL
                    DESCRIPTION "The SBCL Lisp compiler.\nRequired."
                    REQUIRED)

cpack_add_component(TclTk
                    DISPLAY_NAME "Tcl/Tk"
                    DESCRIPTION "The TCL/TK scripting language.\nRequired."
                    REQUIRED)
if(WITH_ABCL)
    cpack_add_component(ABCL
                        DESCRIPTION "The ABCL Lisp compiler.\nA Java installation is required.\nOptional.")
endif()
