ADD_LIBRARY(hoover
  defaultsHoover.c
  methodsHoover.c
  rays.c
  stub.c
)

TARGET_LINK_LIBRARIES(hoover limn ell nrrd biff air)
INSTALL_TARGETS(${TEEM_INSTALL_LIB_DIR} hoover)
