/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkGeometricalQuadEdgeTest1.cxx,v $
  Language:  C++
  Date:      $Date: 2007/02/27 14:13:25 $
  Version:   $Revision: 1.5 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "itkGeometricalQuadEdge.h"
#include <iostream>

class itkGeometricalQuadEdgeTest1Helper
{
public:
  typedef unsigned int PointIdentifier;
  typedef unsigned int FaceIdentifier;
  typedef float        PointData;
  typedef std::string  FaceData;

  typedef itk::GeometricalQuadEdge<
     PointIdentifier, FaceIdentifier, 
     PointData, FaceData, true >        PrimalQuadEdgeType;


  typedef PrimalQuadEdgeType::DualType  DualQuadEdgeType;

 
  static PrimalQuadEdgeType * MakeQuadEdges()
    {
    PrimalQuadEdgeType * e1 = new PrimalQuadEdgeType();
    DualQuadEdgeType   * e2 = new DualQuadEdgeType();
    PrimalQuadEdgeType * e3 = new PrimalQuadEdgeType();
    DualQuadEdgeType   * e4 = new DualQuadEdgeType();
    
    e1->SetRot( e2 );
    e2->SetRot( e3 );
    e3->SetRot( e4 );
    e4->SetRot( e1 );
    
    e1->SetOnext( e1 );
    e2->SetOnext( e4 );
    e3->SetOnext( e3 );
    e4->SetOnext( e4 );

    return e1;
    }
};


int itkGeometricalQuadEdgeTest1( int , char* [] )
{

  typedef itkGeometricalQuadEdgeTest1Helper  HelperType;

  typedef HelperType::PrimalQuadEdgeType     PrimalQuadEdgeType;
  typedef HelperType::DualQuadEdgeType       DualQuadEdgeType;


    { // Define a local scope for testing constructors

    DualQuadEdgeType     dummyQuadEdge1;  // test constructor
    PrimalQuadEdgeType   dummyQuadEdge2;  // test constructor

    dummyQuadEdge1.SetRot( &dummyQuadEdge2 );  // Test SetRot()
    }

  HelperType::MakeQuadEdges();
 
  return EXIT_SUCCESS;
}
