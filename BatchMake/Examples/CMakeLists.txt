SUBDIRS(
  PixelCounter
  )

IF(GRID_SUPPORT)
  SUBDIRS(
    CondorTest
    Calculator
    )
ENDIF(GRID_SUPPORT)

INSTALL(FILES CMakeLists.txt DESTINATION source/Examples)

FILE(GLOB Scripts Scripts/?*.*)
INSTALL(FILES ${Scripts} DESTINATION source/Examples/Scripts)

FILE(GLOB WrappedApplications WrappedApplications/?*.*)
INSTALL(FILES ${WrappedApplications} DESTINATION 
        source/Examples/WrappedApplications)

