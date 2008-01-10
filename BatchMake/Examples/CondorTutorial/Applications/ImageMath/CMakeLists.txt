PROJECT(ImageMath)


INCLUDE_DIRECTORIES(
  ${ITK_SOURCE_DIR}/Utilities/zlib
  ${ITK_DIR}/Utilities/zlib
)

LINK_DIRECTORIES (
  ${ITK_LIBRARY_PATH}
)

LINK_LIBRARIES (
  ${ITK_LIBRARIES}
)

SET ( MAIN_SRCS
itkNJetImageFunction.h
itkCVTImageFilter.h
itkNJetImageFunction.txx
itkCVTImageFilter.txx
main.cxx
)
ADD_EXECUTABLE(ImageMath ${MAIN_SRCS})