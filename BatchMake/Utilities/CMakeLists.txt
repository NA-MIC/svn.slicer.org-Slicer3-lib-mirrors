
#---------------------------------------------------------------------
# Use FLTK = FLU and CondorWatcher

IF(USE_FLTK)
  SUBDIRS(Flu)
  IF(GRID_SUPPORT)
    SUBDIRS(CondorWatcher)
  ENDIF(GRID_SUPPORT)
ENDIF(USE_FLTK)

#---------------------------------------------------------------------
#  ZIP

SUBDIRS(Zip)

#---------------------------------------------------------------------
#  Source

SET(BatchMakeUtilities_HDRS
  MString.h
  Timer.h
  XMLReader.h
  XMLWriter.h
  XMLIniIO.h
  TCPSocket.h
  XMLBufferReader.h
  SliceExtractor.h
  SystemInfo.h
  Whetstone.h
  )


SET(BatchMakeUtilities_SRCS
  MString.cxx
  Timer.cxx
  XMLReader.cxx
  XMLWriter.cxx
  XMLIniIO.cxx
  TCPSocket.cxx
  XMLBufferReader.cxx
  SliceExtractor.cxx
  SystemInfo.cxx
  )

#---------------------------------------------------------------------
#  Build BatchMakeUtilities Library

ADD_LIBRARY(BatchMakeUtilities ${BatchMakeUtilities_SRCS})


TARGET_LINK_LIBRARIES(BatchMakeUtilities ITKCommon)
IF(USE_FLTK)
  TARGET_LINK_LIBRARIES(BatchMakeUtilities ${FLTK_LIBRARIES} FLU)
  ADD_DEPENDENCIES(BatchMakeUtilities FLU)
  IF(GRID_SUPPORT)
    TARGET_LINK_LIBRARIES(BatchMakeUtilities CondorWatcher)
    ADD_DEPENDENCIES(BatchMakeUtilities CondorWatcher)
  ENDIF(GRID_SUPPORT)
ENDIF(USE_FLTK)

#---------------------------------------------------------------------
# Installation file

INSTALL(TARGETS BatchMakeUtilities 
        ARCHIVE DESTINATION lib/BatchMake
        LIBRARY DESTINATION lib/BatchMake
        RUNTIME DESTINATION bin)
INSTALL(FILES ${BatchMakeUtilities_HDRS} 
        DESTINATION include/BatchMake/Utilities)
INSTALL(FILES ${BatchMakeUtilities_SRCS} 
        DESTINATION source/Utilities)
INSTALL(FILES CMakeLists.txt 
        DESTINATION source/Utilities)
