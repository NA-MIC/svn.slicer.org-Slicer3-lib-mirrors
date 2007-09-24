SUBDIRS(
  PixelCounter
  )

IF(GRID_SUPPORT)
  SUBDIRS(
    CondorTest
    Calculator
    )
ENDIF(GRID_SUPPORT)

INSTALL(FILES CMakeLists.txt 
        DESTINATION source/BatchMake/Examples)

FILE(GLOB Scripts Scripts/?*.*)
INSTALL(FILES ${Scripts} 
        DESTINATION source/BatchMake/Examples/Scripts)
