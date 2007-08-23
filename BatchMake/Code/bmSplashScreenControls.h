/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmSplashScreenControls.h,v $
  Language:  C++
  Date:      $Date: 2006/09/07 02:22:15 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __BatchMakeSplashScreenControls_h_
#define __BatchMakeSplashScreenControls_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "bmSplashScreen.h"

namespace bm {

class SplashScreenControls : public SplashScreen
{
public:
  SplashScreenControls();
  ~SplashScreenControls();

  void Show();

};

}

#endif
