# --------------------------------------------------------------------------
#                   OpenMS -- Open-Source Mass Spectrometry
# --------------------------------------------------------------------------
# Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
# ETH Zurich, and Freie Universitaet Berlin 2002-2012.
#
# This software is released under a three-clause BSD license:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of any author or any participating institution
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# For a full list of authors, refer to the file AUTHORS.
# --------------------------------------------------------------------------
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING
# INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# --------------------------------------------------------------------------
# $Maintainer: Stephan Aiche, Chris Bielow, Hannes Röst $
# $Authors: Hannes Röst, Stephan Aiche $
# --------------------------------------------------------------------------

project("PeakInvestigator")
cmake_minimum_required(VERSION 2.8.3 FATAL_ERROR)

#------------------------------------------------------------------------------
# Find Boost lib
#------------------------------------------------------------------------------
find_boost()

if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost was not found!")
endif()

#------------------------------------------------------------------------------
# Find Qt
#------------------------------------------------------------------------------
find_package(Qt4 REQUIRED QtGui QtOpenGL QtSVG QtWebKit QtNetwork ${phonon_extra})
if (NOT QT4_FOUND)
  message(STATUS "Qt4 not found!")
  message(FATAL_ERROR "To find a custom Qt installation use: cmake <..more options..> -D QT_QMAKE_EXECUTABLE='<path_to_qmake(.exe)' <src-dir>")
endif()
# required to indicate we use qtopengl
set(QT_USE_QTOPENGL TRUE)
include(${QT_USE_FILE})

#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Find LibSSH2 on Mac/Linux
#------------------------------------------------------------------------------
if(UNIX AND NOT CYGWIN)
  find_package(LibSSH2 REQUIRED)
  if(NOT LIBSSH2_FOUND)
    message(FATAL_ERROR "LibSSH2 was not found, which is required for Mac/Linux!")
  endif(NOT LIBSSH2_FOUND)
endif()
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Find QJSON
#------------------------------------------------------------------------------
find_package(QJSON REQUIRED)

# get information regarding the openswath files
include(source/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PeakInvestigator.cmake)

# get visual (dialogs)
#include(source/VISUAL/PEAKINVESTIGATOR/sources.cmake)
#include(include/OpenMS/VISUAL/PEAKINVESTIGATOR/sources.cmake)
#include(include/OpenMS/VISUAL/PEAKINVESTIGATOR/UIC/sources.cmake)

# --------------------------------------------------------------------------
# collect link dependencies
set(OpenMS_GUI_DEP_LIBRARIES OpenMS
                             ${QT_LIBRARIES})

# --------------------------------------------------------------------------
# handle OpenGL on macosx systems
if(APPLE)
  find_package(OpenGL REQUIRED)
  set(OpenMS_GUI_DEP_LIBRARIES ${OpenMS_GUI_DEP_LIBRARIES}
                               ${OPENGL_gl_LIBRARY}
                               ${OPENGL_glu_LIBRARY})
endif()

# --------------------------------------------------------------------------
# additional linker flags required by openms_gui
if (MSVC)
  set (GUI_lnk_flags "/FORCE:MULTIPLE /INCREMENTAL:NO /ignore:4006 /ignore:4088")
  set_target_properties(OpenMS_GUI PROPERTIES LINK_FLAGS ${GUI_lnk_flags}) ## allow multiple definitions of symbols (e.g. from template instanciations or STL-derived classes)
endif()

#------------------------------------------------------------------------------
# add the library
openms_add_library(TARGET_NAME PeakInvestigatorImpl
                   SOURCE_FILES ${PeakInvestigatorFiles}
                   HEADER_FILES ${PeakInvestigatorHeaders}
                   INTERNAL_INCLUDES ${PROJECT_SOURCE_DIR}/include 
                                     ${PROJECT_BINARY_DIR}/include
                   EXTERNAL_INCLUDES ${Boost_INCLUDE_DIRS} ${OpenMS_INCLUDE_DIRECTORIES}
                   LINK_LIBRARIES OpenMS ${LIBSSH2_LIBRARIES} ${OpenMS_GUI_DEP_LIBRARIES} qjson
                   DLL_EXPORT_PATH "OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/")

openms_doc_path("${PROJECT_SOURCE_DIR}/include")
