SET(CURL_TESTS
    ftpget
    ftpgetresp
    ftpupload
    getinmemory
    persistant
    sepheaders
    simple
   )

CONFIGURE_FILE(${SLICERLIBCURL_SOURCE_DIR}/Testing/testconfig.h.in
               ${SLICERLIBCURL_BINARY_DIR}/Testing/testconfig.h)

INCLUDE_DIRECTORIES(${SLICERLIBCURL_BINARY_DIR}/Testing)

FOREACH(TEST ${CURL_TESTS})
  ADD_EXECUTABLE(${TEST} ${TEST}.c)
  TARGET_LINK_LIBRARIES(${TEST} slicerlibcurl)
ENDFOREACH(TEST)
