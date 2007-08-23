/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmApplicationListGUIBase.h,v $
  Language:  C++
  Date:      $Date: 2007/03/07 21:35:32 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __bmApplicationListGUIBase_h_
#define __bmApplicationListGUIBase_h_

namespace bm
{

class ApplicationListGUIBase
{
public:
  virtual void OnAccept() = 0;
  virtual void OnNew() = 0;
  virtual void OnEdit() = 0;
  virtual void OnRemove() = 0;
  virtual void OnSelect() = 0;
  virtual void OnChangePath() = 0;
  virtual void OnSelectDir() = 0;
};

} // end namespace bm

#endif
