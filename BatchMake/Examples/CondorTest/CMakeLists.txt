PROJECT(bmCondorTest)

SET(bmCondorTest_SRCS
  bmCondorTest.cxx
  )

ADD_EXECUTABLE(bmCondorTest ${bmCondorTest_SRCS})

IF(WIN32)
  TARGET_LINK_LIBRARIES(bmCondorTest
    wsock32
  )
ENDIF(WIN32)


#----------------------------------------------------------------------
# Installation file

INSTALL(TARGETS bmCondorTest 
        RUNTIME DESTINATION bin)
INSTALL(FILES bmCondorTest.cxx DESTINATION source/Examples/CondorTest)
INSTALL(FILES CMakeLists.txt DESTINATION source/Examples/CondorTest)

