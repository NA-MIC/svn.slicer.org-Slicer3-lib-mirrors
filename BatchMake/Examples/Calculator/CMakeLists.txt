PROJECT(Calculator)
ADD_EXECUTABLE(Calculator main.cxx)
TARGET_LINK_LIBRARIES(Calculator BatchMakeLib BatchMakeDashboard BatchMakeUtilities)

# Installation file
INSTALL(TARGETS Calculator 
        RUNTIME DESTINATION bin)
INSTALL(FILES main.cxx DESTINATION source/Examples/Calculator)
INSTALL(FILES CMakeLists.txt DESTINATION source/Examples/Calculator)
