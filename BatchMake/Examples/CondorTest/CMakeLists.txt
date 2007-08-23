PROJECT(bmCondorTest)

SET(bmCondorTest_SRCS
  bmCondorTest.cxx
)

IF(WIN32)
LINK_LIBRARIES(
wsock32
)
ENDIF(WIN32)

ADD_EXECUTABLE(bmCondorTest ${bmCondorTest_SRCS})

# Installation file
INSTALL(TARGETS bmCondorTest 
        RUNTIME DESTINATION bin)
INSTALL(FILES bmCondorTest.cxx DESTINATION source/Examples/CondorTest)
INSTALL(FILES CMakeLists.txt DESTINATION source/Examples/CondorTest)
