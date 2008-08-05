// this file defines the SegmentationExamples for the test driver
// and all it expects is that you have a function called RegisterTests
#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif
#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include <iostream>
#include "itkTestMain.h" 


void RegisterTests()
{
REGISTER_TEST(GeodesicActiveContourShapePriorLevelSetImageFilterTest);
}

#undef main
#define main GeodesicActiveContourShapePriorLevelSetImageFilterTest
#include "GeodesicActiveContourShapePriorLevelSetImageFilter.cxx"
