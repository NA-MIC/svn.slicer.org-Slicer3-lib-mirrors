PROJECT(PixelCounter)

ADD_EXECUTABLE(PixelCounter main.cxx)
TARGET_LINK_LIBRARIES(PixelCounter BatchMakeUtilities)

# Installation file
INSTALL(TARGETS PixelCounter
        RUNTIME DESTINATION bin)
INSTALL(FILES main.cxx DESTINATION source/Examples/PixelCounter)
INSTALL(FILES CMakeLists.txt DESTINATION source/Examples/PixelCounter)
