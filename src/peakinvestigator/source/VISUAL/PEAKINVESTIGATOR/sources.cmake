### the directory name
set(directory source/VISUAL/PEAKINVESTIGATOR)

### list all filenames of the directory here
set(sources_list
PeakInvestigatorInitDialog.cpp
MassRangeDialog.cpp
)

### add path to the filenames
set(sources)
foreach(i ${sources_list})
	list(APPEND sources ${directory}/${i})
endforeach(i)

### pass source file list to the upper instance
set(OpenMSVisual_sources ${OpenMSVisual_sources} ${sources})

### source group definition
source_group("Source Files\\VISUAL\\PEAKINVESTIGATOR" FILES ${sources})

