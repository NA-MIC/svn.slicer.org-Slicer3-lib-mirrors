ADD_LIBRARY(alan
coreAlan.c
enumsAlan.c
methodsAlan.c
)


TARGET_LINK_LIBRARIES(alan air biff ell nrrd)
INSTALL_TARGETS(${TEEM_INSTALL_LIB_DIR} alan)
