/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: main.cxx,v $
  Language:  C++
  Date:      $Date: 2006/09/07 02:18:47 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "CondorWatcher.h"

int main()
{
  CondorWatcher gui;
  gui.Window->show();
  gui.Watch();

  Fl::run();
  return 0;
}
