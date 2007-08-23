CONFIGURE_FILE(
  ${BatchMake_SOURCE_DIR}/Documentation/Doxygen/doxygen.config.in
  ${BatchMake_BINARY_DIR}/doxygen.config
  )

FILE(GLOB Doxygen ${BatchMake_SOURCE_DIR}/Documentation/Doxygen/?*.*)
INSTALL(FILES ${Doxygen} DESTINATION source/Documentation/Doxygen)
