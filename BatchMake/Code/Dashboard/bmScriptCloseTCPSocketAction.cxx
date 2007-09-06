/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptCloseTCPSocketAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptCloseTCPSocketAction.h"

namespace bm {

/** */
ScriptCloseTCPSocketAction::ScriptCloseTCPSocketAction()
: ScriptAction()
{
}

/** */
ScriptCloseTCPSocketAction::~ScriptCloseTCPSocketAction()
{
}

/** */
bool ScriptCloseTCPSocketAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <1)
    {
    error->SetError(MString("No enough parameter for SendTCP"),linenumber);
    return false;
    }
  if (m_Parameters.size() >1)
    {
    error->SetError(MString("Too much parameters for SendTCP"),linenumber);
    return false;
    }

  return true;
}

/** */
MString ScriptCloseTCPSocketAction::Help()
{
  return "CloseTCPSocket(SocketName)";
}

/** */
void ScriptCloseTCPSocketAction::Execute()
{
  TCPSocket* socket = m_Manager->GetVariableSocket(m_Parameters[0]);
  socket->CloseSocket();
  bool err = m_Manager->RemoveSocket(m_Parameters[0]);
  
  if( err == false )
    {
    std::cout<<"socket cannot be removed"<<std::endl;
    }
}

} // end namespace bm
