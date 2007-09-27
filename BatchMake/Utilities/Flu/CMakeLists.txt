# Main CMakeLists.txt to build the FLU project using CMake (www.cmake.org)
# Written by Matthieu Jomier

PROJECT(FLU)

FIND_PACKAGE(FLTK)
IF(FLTK_FOUND)
   INCLUDE_DIRECTORIES(${FLTK_INCLUDE_DIR})
ENDIF(FLTK_FOUND)

SET(FLU_SRCS
Flu_Button.cpp
Flu_Chat_Buffer.cpp
Flu_Choice_Group.cpp
Flu_Collapsable_Group.cpp
Flu_Combo_Box.cpp
Flu_Combo_List.cpp
Flu_Combo_Tree.cpp
Flu_DND.cpp
Flu_Dual_Progress_Meter.cpp
Flu_Dual_Slider.cpp
Flu_File_Chooser.cpp
Flu_Float_Input.cpp
Flu_Helpers.cpp
Flu_Int_Input.cpp
Flu_Label.cpp
Flu_Link_Button.cpp
Flu_Output.cpp
flu_file_chooser_pixmaps.cpp
flu_pixmaps.cpp
Flu_Progress.cpp
Flu_Progress_Meter.cpp
Flu_Return_Button.cpp
Flu_Separator.cpp
Flu_Simple_Group.cpp
FluSimpleString.cpp
Flu_Spinner.cpp
Flu_Toggle_Group.cpp
Flu_Tree_Browser.cpp
Flu_Wrap_Group.cpp
)


LINK_LIBRARIES(FLU
 ${FLTK_LIBRARIES}
)

ADD_LIBRARY(FLU ${FLU_SRCS})

FILE(GLOB FLU_HDRS FLU/*.h)

# Installation file
INSTALL(TARGETS FLU
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib/BatchMake)
INSTALL(FILES ${FLU_HDRS} 
        DESTINATION include/BatchMake/Utilities/Flu/FLU)
INSTALL(FILES ${FLU_SRCS} ${CondorWatcherGUI_SRCS} 
        DESTINATION source/BatchMake/Utilities/Flu)
INSTALL(FILES CMakeLists.txt 
        DESTINATION source/BatchMake/Utilities/Flu)
