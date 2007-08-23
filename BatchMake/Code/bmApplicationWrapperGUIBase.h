/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmApplicationWrapperGUIBase.h,v $
  Language:  C++
  Date:      $Date: 2007/03/07 21:35:32 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __bmApplicationWrapperGUIBase_h_
#define __bmApplicationWrapperGUIBase_h_

namespace bm {

class ApplicationWrapperGUIBase
{
public:
  virtual void OnReject() = 0;
  virtual void OnSelectPath() = 0;
  virtual void OnSelectType() = 0;
  virtual void OnAddEnum() = 0;
  virtual void OnSelectEnumList() = 0;
  virtual void OnSelectParameters() = 0;
  virtual void OnAddParameters() = 0;
  virtual void OnSaveModule() = 0;
  virtual void OnLoadModule() = 0;
  virtual void OnDeleteParameters() = 0;
  virtual void OnUpParam() = 0;
  virtual void OnDownParam() = 0;
};

} // end namespace bm

#endif
