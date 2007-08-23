PROJECT(Zip)

INCLUDE_DIRECTORIES(
  ${Zip_SOURCE_DIR}
  ${Zip_BINARY_DIR}
)

SET(ZLib_HDRS   
  inftrees.h 
  infblock.h  
  infutil.h   
  zutil.h    
  infcodes.h 
  deflate.h  
  inffast.h   
  trees.h 
  zip.h
  unzip.h
  zconf.h
  itk_zlib_mangle.h
  zlib.h
  inffixed.h
)

SET(ZLib_SRCS
  adler32.c   
  gzio.c      
  inftrees.c  
  uncompr.c
  compress.c  
  infblock.c  
  infutil.c   
  zutil.c
  crc32.c     
  infcodes.c  
  deflate.c   
  inffast.c   
  inflate.c   
  trees.c 
)

SET(Zip_SRCS
  zip.c
  zipcmd.cxx
)

SET(Unzip_SRCS
  unzip.c
  unzipcmd.cxx
)


CONFIGURE_FILE(${Zip_SOURCE_DIR}/zlibDllConfig.h.in
               ${Zip_BINARY_DIR}/zlibDllConfig.h)

ADD_LIBRARY(BatchMakeZLib ${ZLib_SRCS})

ADD_EXECUTABLE(Zip ${Zip_SRCS})
TARGET_LINK_LIBRARIES(Zip BatchMakeZLib)

ADD_EXECUTABLE(Unzip ${Unzip_SRCS})
TARGET_LINK_LIBRARIES(Unzip BatchMakeZLib)

# Installation file
INSTALL(TARGETS BatchMakeZLib
        RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib/BatchMake
        ARCHIVE DESTINATION lib/BatchMake)
INSTALL(FILES ${ZLib_HDRS} DESTINATION include/BatchMake/Utilities/Zip)
INSTALL(FILES ${ZLib_SRCS} ${Zip_SRCS} ${Unzip_SRCS} DESTINATION source/Utilities/Zip)
INSTALL(FILES CMakeLists.txt zlibDllConfig.h.in DESTINATION source/Utilities/Zip)
