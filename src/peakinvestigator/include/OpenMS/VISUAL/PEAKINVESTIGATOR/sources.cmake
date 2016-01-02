### the directory name
set(directory include/OpenMS/VISUAL/DIALOGS)

### list all MOC filenames of the directory here
set(sources_list
PeakInvestigatorInitDialog.ui
MassRangesDialog.ui
)

### add path to the filenames
set(sources)
foreach(i ${sources_list})
  list(APPEND sources ${directory}/${i})
endforeach(i)

### Apply MOC compiler
qt4_wrap_cpp(mocced_sources ${sources} OPTIONS ${BOOST_MOC_ARGS})

### pass source file list to the upper instance
set(OpenMSVisual_sources ${OpenMSVisual_sources} ${mocced_sources})

source_group("Source Files\\OpenMS\\VISUAL\\PEAKINVESTIGATOR" FILES ${mocced_sources})

### list all header files of the directory here
set(sources_list_h
PeakInvestigatorInitDialog.h
MassRangesDialog.h
)

### add path to the filenames
set(sources_h)
foreach(i ${sources_list_h})
	list(APPEND sources_h ${directory}/${i})
endforeach(i)

### source group definition
source_group("Header Files\\OpenMS\\VISUAL\\PEAKINVESTIGATOR" FILES ${sources_h})

set(OpenMSVisual_sources_h ${OpenMSVisual_sources_h} ${sources_h})

