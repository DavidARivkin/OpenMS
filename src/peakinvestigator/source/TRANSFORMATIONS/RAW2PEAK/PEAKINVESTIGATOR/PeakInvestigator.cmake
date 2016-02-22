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
# $Maintainer: Adam Tenderholt $
# $Authors: Adam Tenderholt David Rivkin $
# --------------------------------------------------------------------------

### the directory names
set(directory source/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR)
set(header_directory include/OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR)
#set(dlg_directory source/VISUAL/PEAKINVESTIGATOR)
#set(dlg_header_directory include/OpenMS/VISUAL/PEAKINVESTIGATOR)

### list all files of the source directory here
set(sources_format_list
  FORMAT/TarFile.cpp
)

set(sources_system_list
  SYSTEM/AbstractSecureFileTransfer.cpp
  SYSTEM/PSCPSecureFileTransfer.cpp
)

set(sources_actions_list
  BaseAction.cpp
  DeleteAction.cpp
  InitAction.cpp
  PiVersionsAction.cpp
  PrepAction.cpp
  RunAction.cpp
  SftpAction.cpp
  StatusAction.cpp
)

if(WITH_GUI)
set(sources_dlg_list
  VersionDialog.cpp
  RtoDialog.cpp
)
endif(WITH_GUI)

if(${LIBSSH2_FOUND})
  list(APPEND sources_system_list
      SYSTEM/LibSSH2SecureFileTransfer.cpp
  )
endif(${LIBSSH2_FOUND})

### add path to the filenames
set(sources_format)
foreach(i ${sources_format_list})
	list(APPEND sources_format ${directory}/${i})
endforeach(i)

set(sources_system)
foreach(i ${sources_system_list})
	list(APPEND sources_system ${directory}/${i})
endforeach(i)

set(sources_actions)
foreach(i ${sources_actions_list})
        list(APPEND sources_actions ${directory}/${i})
endforeach(i)

set(peakinvestigator_dlg)
foreach(i ${sources_dlg_list})
       list(APPEND peakinvestigator_dlg ${directory}/${i})
endforeach(i)

### end add path

### Setup format internal source
set(sources_format_internal ${directory}/FORMAT/INTERNAL/tarball.cpp)

# header needs to be defined here, because it needs to be MOC'd
set(peakinvestigator_header ${header_directory}/PeakInvestigator.h)
set(peakinvestigator_source ${directory}/PeakInvestigator.cpp)

qt4_wrap_cpp(mocced_sources ${peakinvestigator_header} OPTIONS ${BOOST_MOC_ARGS})
#if(WITH_GUI)
#qt4_wrap_cpp(mocced_ui ${header_dialog} OPTIONS ${BOOST_MOC_ARGS})
#endif(WITH_GUI)

set(PeakInvestigatorFiles)
list(APPEND PeakInvestigatorFiles ${peakinvestigator_source})
list(APPEND PeakInvestigatorFiles ${mocced_sources})
list(APPEND PeakInvestigatorFiles ${sources_format})
list(APPEND PeakInvestigatorFiles ${sources_format_internal})
list(APPEND PeakInvestigatorFiles ${sources_system})
list(APPEND PeakInvestigatorFiles ${sources_actions})

if(WITH_GUI)
list(APPEND PeakInvestigatorFiles ${peakinvestigator_dlg})
endif(WITH_GUI)

source_group("Source Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR" FILES ${peakinvestigator_source})
source_group("Source Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR" FILES ${mocced_sources})
source_group("Source Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR\\FORMAT" FILES ${sources_format})
source_group("Source Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR\\FORMAT\\INTERNAL" FILES ${sources_format_internal})
source_group("Source Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR\\SYSTEM" FILES ${sources_system})
source_group("Source Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR" FILES ${sources_actions})
if(WITH_GUI)
source_group("Source Files\\VISUAL\\PEAKINVESTIGATOR" FILES ${peakinvestigator_dlg})
endif(WITH_GUI)

## add groups for headers
set(header_format_list
  FORMAT/TarFile.h
)
set(header_system_list
  SYSTEM/AbstractSecureFileTransfer.h
  SYSTEM/PSCPSecureFileTransfer.h
)
set(header_actions_list
  BaseAction.h
  DeleteAction.h
  InitAction.h
  PiVersionsAction.h
  PrepAction.h
  RunAction.h
  SftpAction.h
  StatusAction.h
)

if(WITH_GUI)
set(header_dlg_list
  VersionDialog.h
  RtoDialog.h
)
endif(WITH_GUI)

if(${LIBSSH2_FOUND})
  list(APPEND header_system_list
      SYSTEM/LibSSH2SecureFileTransfer.h
  )
endif(${LIBSSH2_FOUND})

### add path to the filenames
set(header_format)
foreach(i ${header_format_list})
	list(APPEND header_format ${header_directory}/${i})
endforeach(i)

set(header_system)
foreach(i ${header_system_list})
	list(APPEND header_system ${header_directory}/${i})
endforeach(i)

set(header_actions)
foreach(i ${header_actions_list})
        list(APPEND header_actions ${header_directory}/${i})
endforeach(i)

if(WITH_GUI)
set(header_dialog)
foreach(i ${header_dlg_list})
        list(APPEND header_dialog ${header_directory}/${i})
endforeach(i)
endif(WITH_GUI)

### end add path

### Setup format internal header
set(header_format_internal ${header_directory}/FORMAT/INTERNAL/tarball.h)


list(APPEND PeakInvestigatorFiles ${peakinvestigator_header})
list(APPEND PeakInvestigatorFiles ${header_format})
list(APPEND PeakInvestigatorFiles ${header_format_internal})
list(APPEND PeakInvestigatorFiles ${header_system})
list(APPEND PeakInvestigatorFiles ${header_actions})

if(WITH_GUI)
list(APPEND PeakInvestigatorFiles ${header_dialog})
endif(WITH_GUI)

# define list of headers related to peakinvestigator needed for
# installation and export
set(PeakInvestigatorHeaders ${peakinvestigator_header} ${header_format} ${header_format_internal} ${header_system} ${header_actions})

source_group("Header Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR" FILES ${peakinvestigator_header})
source_group("Header Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR\\FORMAT" FILES ${header_format})
source_group("Header Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR\\FORMAT\\INTERNAL" FILES ${header_format_internal})
source_group("Header Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR\\SYSTEM" FILES ${header_system})
source_group("Header Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR\\SYSTEM" FILES ${header_actions})
if(WITH_GUI)
source_group("Header Files\\VISUAL\\PEAKINVESTIGATOR" FILES ${header_dialog})
endif(WITH_GUI)
