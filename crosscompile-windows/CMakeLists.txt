# CMake file for crosscompiling Maxima/wxMaxima for Windows
# Copyright (C) by Wolfgang Dautermann
# License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>
# This is free software: you are free to change and redistribute it.
# There is NO WARRANTY, to the extent permitted by law.

set(CMAKE_SYSTEM_NAME Windows)

set(HOST i686-w64-mingw32)
set(CMAKE_C_COMPILER   ${HOST}-gcc)
set(CMAKE_CXX_COMPILER ${HOST}-g++)
set(CMAKE_RC_COMPILER  ${HOST}-windres)
project(maxima)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/)

option(USE_WXMAXIMA_GIT "Use wxMaxima development snapshot." OFF)
option(USE_VTK "Use VTK." OFF)


# If you want to use a updated version of a program,
# update the version number and the checksum.
# If no further patches are needed, you should get a
# updated setup-file automatically.

# current program versions
set(CLISPVERSION     2.49)
set(WXMAXIMAVERSION  15.08.2)
set(WXWIDGETSVERSION 3.0.2)
set(GNUPLOTVERSION   5.0.1)
set(TCLVERSION       8.6.4)
set(TKVERSION        8.6.4)
set(VTKVERSION       6.3.0)


# MD5 checksums for the current program versions.
set(CLISP_MD5     "8d58d64f83dd7ad01c054aecc16c1f52")
set(WXMAXIMA_MD5  "57f8ad6376bfc8ca92c69e613dcf0d99")
set(WXWIDGETS_MD5 "ba4cd1f3853d0cd49134c5ae028ad080")
set(GNUPLOT_MD5   "4da2089339d9dd2b834f68b39cc876e6")
set(TCL_MD5       "d7cbb91f1ded1919370a30edd1534304")
set(TK_MD5        "261754d7dc2a582f00e35547777e1fea")
set(TEXFONTS_MD5  "8101a6feca2c43a6b9218195df9e7c35")
set(VTK_MD5       "0231ca4840408e9dd60af48b314c5b6d")

# Get Maxima version from configure.ac
file(STRINGS "${CMAKE_SOURCE_DIR}/../configure.ac" CONFIGUREAC REGEX "AC_INIT\\(.*\\)")
string(REGEX REPLACE "AC_INIT\\(\\[maxima\\], \\[(.*)\\]\\)" "\\1" MAXIMAVERSION ${CONFIGUREAC})
message(STATUS "Maxima version (from configure.ac): ${MAXIMAVERSION}")

string(REPLACE "." "" GNUPLOTVERSIONWITHOUTDOTS "${GNUPLOTVERSION}")

# download URLs
set(CLISP_URL     "http://downloads.sourceforge.net/project/clisp/clisp/${CLISPVERSION}/clisp-${CLISPVERSION}-win32-mingw-big.zip")
set(WXMAXIMA_URL  "http://sourceforge.net/projects/wxmaxima/files/wxMaxima/${WXMAXIMAVERSION}/wxmaxima-${WXMAXIMAVERSION}.tar.gz")
set(WXWIDGETS_URL "https://sourceforge.net/projects/wxwindows/files/${WXWIDGETSVERSION}/wxWidgets-${WXWIDGETSVERSION}.tar.bz2")
set(GNUPLOT_URL   "http://sourceforge.net/projects/gnuplot/files/gnuplot/${GNUPLOTVERSION}/gp${GNUPLOTVERSIONWITHOUTDOTS}-win32-mingw.zip")
set(TCL_URL       "http://prdownloads.sourceforge.net/tcl/tcl${TCLVERSION}-src.tar.gz")
set(TK_URL        "http://prdownloads.sourceforge.net/tcl/tk${TKVERSION}-src.tar.gz")
set(VTK_URL       "http://www.vtk.org/files/release/6.3/VTK-${VTKVERSION}.tar.gz")

# No license at the homepage, but the TeX fonts are public domain according to:
# http://archive.debian.net/changelogs/pool/main/t/ttf-jsmath/ttf-jsmath_0.01-2/copyright
# https://www.mail-archive.com/sage-devel@googlegroups.com/msg09750.html
# https://www.mail-archive.com/sage-devel@googlegroups.com/msg09748.html
set(TEXFONTS_URL  "http://www.math.union.edu/~dpvc/jsMath/download/TeX-fonts-25.zip")

set(WXMAXIMA_GIT "https://github.com/andrejv/wxmaxima.git")


# Where are libgcc_s_sjlj-1.dll and libstdc++-6.dll?
#
execute_process(COMMAND "${CMAKE_CXX_COMPILER}" -print-file-name=libgcc_s_sjlj-1.dll OUTPUT_VARIABLE MINGW_LIBGCC    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "${CMAKE_CXX_COMPILER}" -print-file-name=libstdc++-6.dll     OUTPUT_VARIABLE MINGW_LIBSTDCPP OUTPUT_STRIP_TRAILING_WHITESPACE)
# That does work on Debian 7 and Ubuntu 14.04, but not on Opensuse 13.1 (don't know why)
# There the libraries are in /usr/i686-w64-mingw32/sys-root/mingw/bin
# (with repository: http://download.opensuse.org/repositories/windows:/mingw:/win32/openSUSE_13.1/)
# so try that...

if("${MINGW_LIBGCC}" STREQUAL "libgcc_s_sjlj-1.dll")
    message(STATUS "The command ${CMAKE_CXX_COMPILER} -print-file-name=libgcc_s_sjlj-1.dll   did not work (perhaps on Opensuse 13.1).")
    find_path(MINGW_LIBSTDCPP_DIR "libstdc++-6.dll" "/usr/i686-w64-mingw32/sys-root/mingw/bin")
    set(MINGW_LIBSTDCPP "${MINGW_LIBSTDCPP_DIR}/${MINGW_LIBSTDCPP}")
    message(STATUS "Dont worry. Found libstdc++-6.dll at ${MINGW_LIBSTDCPP}")
    find_path(MINGW_LIBGCC_DIR "libgcc_s_sjlj-1.dll" "/usr/i686-w64-mingw32/sys-root/mingw/bin")
    set(MINGW_LIBGCC "${MINGW_LIBGCC_DIR}/${MINGW_LIBGCC}")
    message(STATUS "Dont worry. Found libgcc_s_sjlj-1.dll at ${MINGW_LIBGCC}")
else()
    message(STATUS "Found libstdc++-6.dll at ${MINGW_LIBSTDCPP}")
    message(STATUS "Found libgcc_s_sjlj-1.dll at ${MINGW_LIBGCC}")
endif()


include(ExternalProject)

# Build wxWidgets static library (required for wxMaxima)
externalproject_add(wxWidgets
  URL "${WXWIDGETS_URL}"
  DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/downloads
  URL_MD5 ${WXWIDGETS_MD5}
  CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/wxWidgets-prefix/src/wxWidgets/configure --disable-shared --disable-compat28 --with-zlib=builtin --with-libjpeg=builtin --without-libtiff --with-libpng=builtin --with-expat=builtin --host=${HOST}
  BUILD_COMMAND $(MAKE)
        COMMAND ${CMAKE_SOURCE_DIR}/wxwidgets/debian-crosscompile-correction.sh ${CMAKE_BINARY_DIR}/wxWidgets-prefix/src/wxWidgets-build/lib
  INSTALL_COMMAND ""
)

# build tcl/tk (required for xmaxima)
externalproject_add(tcl
  URL "${TCL_URL}"
  DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/downloads
  URL_MD5 ${TCL_MD5}
  CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/tcl-prefix/src/tcl/win/configure --host=${HOST} --prefix=${CMAKE_BINARY_DIR}/tcltk-root
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install
)

externalproject_add(tk
  URL "${TK_URL}"
  DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/downloads
  URL_MD5 ${TK_MD5}
  DEPENDS tcl
  CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/tk-prefix/src/tk/win/configure --host=${HOST} --with-tcl=${CMAKE_BINARY_DIR}/tcltk-root/lib --prefix=${CMAKE_BINARY_DIR}/tcltk-root
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install
)
install(DIRECTORY ${CMAKE_BINARY_DIR}/tcltk-root/ DESTINATION .)

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
install(DIRECTORY ${CMAKE_BINARY_DIR}/clisp-prefix/src/clisp/ DESTINATION clisp-${CLISPVERSION})


# Gnuplot (already a binary package just extract it...)
externalproject_add(gnuplot
  URL "${GNUPLOT_URL}"
  DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/downloads
  URL_MD5 ${GNUPLOT_MD5}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
install(DIRECTORY ${CMAKE_BINARY_DIR}/gnuplot-prefix/src/gnuplot/ DESTINATION gnuplot)

# TeX Fonts (a binary package just extract it...)
externalproject_add(texfonts
  URL "${TEXFONTS_URL}"
  DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/downloads
  URL_MD5 ${TEXFONTS_MD5}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
find_program(WINE_EXE NAMES wine)
if(WINE_EXE)
    message(STATUS "Found wine at ${WINE_EXE}")
else()
    message(FATAL_ERROR "Wine not found. It is required for crosscompiling Maxima.")
endif()

set(CLISPROOT "${CMAKE_BINARY_DIR}/clisp-prefix/src/clisp")
configure_file("${CMAKE_SOURCE_DIR}/wine-clisp.sh.tmpl" "${CMAKE_BINARY_DIR}/wine-clisp.sh")
configure_file("${CMAKE_SOURCE_DIR}/wine-lisp.sh.tmpl" "${CMAKE_BINARY_DIR}/wine-lisp.sh")


# Build Maxima
externalproject_add(maxima
  # first copy the maxima source to a build directory, since out-of-source-builds do not work
  DOWNLOAD_COMMAND rsync -av --exclude=crosscompile-windows/build/ "${CMAKE_SOURCE_DIR}/../"  "${CMAKE_SOURCE_DIR}/build/maxima-prefix/src/maxima"
  DEPENDS clisp
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND sh -c "test -x configure || ./bootstrap"
            COMMAND ./configure --host=${HOST} --enable-clisp --with-clisp=${CMAKE_BINARY_DIR}/wine-clisp.sh --with-clisp-runtime=${CMAKE_BINARY_DIR}/wine-lisp.sh --prefix=C:/maxima-${MAXIMAVERSION}
  # clisp-depends.mk is generated with "\" as path seperator, not "/" - correct that...
  BUILD_COMMAND $(MAKE) -C src clisp-depends.mk
        COMMAND sh -c "tr '\\\\' / <src/clisp-depends.mk >src/clisp-depends.mk-new"
        COMMAND mv src/clisp-depends.mk-new src/clisp-depends.mk
        COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) -C ${CMAKE_BINARY_DIR}/maxima-prefix/src/maxima install DESTDIR=${CMAKE_BINARY_DIR}/maxima-installroot/
)


install(DIRECTORY ${CMAKE_BINARY_DIR}/maxima-installroot/C:/maxima-${MAXIMAVERSION}/ DESTINATION .)
install(FILES "${CMAKE_SOURCE_DIR}/xmaxima.bat" DESTINATION bin/)




if(USE_WXMAXIMA_GIT)
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/wxMaxima-git-prefix/src/wxMaxima-git/src")
    externalproject_add(wxMaxima-git
        GIT_REPOSITORY ${WXMAXIMA_GIT}
        DEPENDS wxWidgets
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND sh -c "cp ${CMAKE_BINARY_DIR}/wxWidgets-prefix/src/wxWidgets/wxwin.m4 ${CMAKE_BINARY_DIR}/wxMaxima-git-prefix/src/wxMaxima-git/acinclude.m4"
                  COMMAND ./bootstrap
                  COMMAND ./configure --host=${HOST} --with-wx-config=${CMAKE_BINARY_DIR}/wxWidgets-prefix/src/wxWidgets-build/lib/wx/config/inplace-${HOST}-msw-unicode-static-3.0 --prefix=C:/maxima-${MAXIMAVERSION}
        BUILD_COMMAND $(MAKE)
              COMMAND $(MAKE) -C locales allmo
        INSTALL_COMMAND $(MAKE) -C ${CMAKE_BINARY_DIR}/wxMaxima-git-prefix/src/wxMaxima-git wxMaxima.win
    )
    install(DIRECTORY ${CMAKE_BINARY_DIR}/wxMaxima-git-prefix/src/wxMaxima-git/wxMaxima/ DESTINATION wxMaxima)
else()
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/wxMaxima-prefix/src/wxMaxima/src")
    externalproject_add(wxMaxima
        URL "${WXMAXIMA_URL}"
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/downloads
        URL_MD5 ${WXMAXIMA_MD5}
        DEPENDS wxWidgets
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ./configure --host=${HOST} --with-wx-config=${CMAKE_BINARY_DIR}/wxWidgets-prefix/src/wxWidgets-build/lib/wx/config/inplace-${HOST}-msw-unicode-static-3.0 --prefix=C:/maxima-${MAXIMAVERSION}
        BUILD_COMMAND $(MAKE)
              COMMAND $(MAKE) -C locales allmo
        INSTALL_COMMAND $(MAKE) -C ${CMAKE_BINARY_DIR}/wxMaxima-prefix/src/wxMaxima wxMaxima.win
    )
    install(DIRECTORY ${CMAKE_BINARY_DIR}/wxMaxima-prefix/src/wxMaxima/wxMaxima/ DESTINATION wxMaxima)
endif()

if(USE_VTK)
    externalproject_add(vtk-native
        URL "${VTK_URL}"
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/downloads
        URL_MD5 ${VTK_MD5}
        CMAKE_ARGS -DVTK_WRAP_TCL=YES -DVTK_USE_TK=YES -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/vtk-native-root
        BUILD_COMMAND $(MAKE)
    )
    externalproject_add(vtk
        URL "${VTK_URL}"
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/downloads
        URL_MD5 ${VTK_MD5}
	PATCH_COMMAND patch -p0 < ${CMAKE_SOURCE_DIR}/vtk-6.3.patch
        DEPENDS tcl tk vtk-native
        CMAKE_ARGS -DVTK_WRAP_TCL=YES -DVTK_USE_TK=YES -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/Toolchain-mingw32.cmake -DTCL_LIBRARY=${CMAKE_BINARY_DIR}/tcltk-root/bin/tcl86.dll -DTCL_INCLUDE_PATH=${CMAKE_BINARY_DIR}/tcltk-root/include -DTCL_TCLSH=${CMAKE_BINARY_DIR}/tcltk-root/bin/tclsh86.exe -DTK_LIBRARY=${CMAKE_BINARY_DIR}/tcltk-root/bin/tk86.dll -DTK_INCLUDE_PATH=${CMAKE_BINARY_DIR}/tcltk-root/include -DCMAKE_CROSSCOMPILING_EMULATOR=/usr/bin/wine -DVTKCompileTools_DIR=${CMAKE_BINARY_DIR}/vtk-native-prefix/src/vtk-native-build -DTK_XLIB_PATH=${CMAKE_BINARY_DIR}/tcltk-root/include -DCMAKE_INSTALL_PREFIX=C:/maxima-${MAXIMAVERSION}
        BUILD_COMMAND $(MAKE)
    )
    install(DIRECTORY ${CMAKE_BINARY_DIR}/vtk-prefix/src/vtk-build/C:/maxima-${MAXIMAVERSION}/ DESTINATION .)
endif()

# Include DLLs from the current Mingw environment.
install(FILES "${MINGW_LIBSTDCPP}" "${MINGW_LIBGCC}" DESTINATION wxMaxima/)
install(FILES "${MINGW_LIBSTDCPP}" "${MINGW_LIBGCC}" DESTINATION bin/)



set(CPACK_GENERATOR "NSIS")
set(CPACK_PACKAGE_VERSION "${MAXIMAVERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Maxima")
set(CPACK_PACKAGE_VENDOR "Maxima Team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/../README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/../COPYING")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "maxima-${CPACK_PACKAGE_VERSION}")
set(CPACK_NSIS_INSTALL_ROOT "C:")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/maxima-icon.bmp")
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/../interfaces/xmaxima\\\\maxima-icon.ico")
set(CPACK_NSIS_MUI_UNIICON "${CPACK_NSIS_MUI_ICON}")
set(CPACK_PACKAGE_EXECUTABLES "..\\\\wxMaxima\\\\wxmaxima" "wxMaxima (GUI for Maxima)")
set(CPACK_NSIS_URL_INFO_ABOUT "http://maxima.sourceforge.net")
set(CPACK_NSIS_MENU_LINKS "bin/maxima.bat" "Maxima (command line)" "bin/xmaxima.bat" "XMaxima (simple GUI)" "http://maxima.sourceforge.net" "About Maxima" "https://andrejv.github.io/wxmaxima/" "About wxMaxima")
set(CPACK_PACKAGE_FILE_NAME "maxima-clisp-${CPACK_PACKAGE_VERSION}")

# File associations:
set(CPACK_NSIS_DEFINES "!include ${CMAKE_SOURCE_DIR}\\\\FileAssociation.nsh")
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
\\\${registerExtension} \\\"\\\$INSTDIR\\\\wxMaxima\\\\wxmaxima.exe\\\" \\\".wxm\\\" \\\"wxMaxima Document\\\"
\\\${registerExtension} \\\"\\\$INSTDIR\\\\wxMaxima\\\\wxmaxima.exe\\\" \\\".wxmx\\\" \\\"wxMaxima XML Document\\\"
\\\${registerExtension} \\\"\\\$INSTDIR\\\\wxMaxima\\\\wxmaxima.exe\\\" \\\".mac\\\" \\\"Maxima Batch file\\\"
")
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
\\\${unregisterExtension} \\\".wxm\\\" \\\"wxMaxima Document\\\"
\\\${unregisterExtension} \\\".wxmx\\\" \\\"wxMaxima XML Document\\\"
\\\${unregisterExtension} \\\".mac\\\" \\\"Maxima Batch File\\\"
")

include(CPack)