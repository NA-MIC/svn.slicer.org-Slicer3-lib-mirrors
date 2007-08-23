SET(BATCHMAKE_TESTS ${CXX_TEST_PATH}/BatchMakeTests)

ADD_EXECUTABLE(BatchMakeMemoryTest BatchMakeMemoryTest.cxx)

IF(DASHBOARD_SUPPORT)
SET(BatchMakeTest_SRCS
bmDashboardSendTest.cxx
bmSystemInfoTest.cxx
)

ADD_TEST(DashboardSendTest ${BATCHMAKE_TESTS} DashboardSendTest)
ADD_TEST(SystemInfoTest ${BATCHMAKE_TESTS} SystemInfoTest)

ADD_EXECUTABLE(BatchMakeTests BatchMakeTests.cxx ${BatchMakeTest_SRCS})
TARGET_LINK_LIBRARIES(BatchMakeTests BatchMakeDashboard BatchMakeUtilities)

ADD_DEPENDENCIES(BatchMakeTests BatchMakeDashboard)

ENDIF(DASHBOARD_SUPPORT)

