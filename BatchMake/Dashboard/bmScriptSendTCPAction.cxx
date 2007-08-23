/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptSendTCPAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:49 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptSendTCPAction.h"

namespace bm {

/** */
ScriptSendTCPAction::ScriptSendTCPAction()
: ScriptAction()
{
}

/** */
ScriptSendTCPAction::~ScriptSendTCPAction()
{
}

/** */
bool ScriptSendTCPAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <2)
    {
    error->SetError(MString("No enough parameter for SendTCP"),linenumber);
    return false;
    }
  if (m_Parameters.size() >2)
    {
    error->SetError(MString("Too much parameters for SendTCP"),linenumber);
    return false;
    }

  return true;
}

/** */
MString ScriptSendTCPAction::Help()
{
  return "SendTCP(SocketName Message)";
}

/** */
void ScriptSendTCPAction::Execute()
{
  MString m_value = m_Manager->Convert(m_Parameters[1]).removeChar('\'').latin1();

  TCPSocket* socket = m_Manager->GetVariableSocket(m_Parameters[0]);
  int err = socket->SendMessage(m_value.toChar());
  
  if( err == -1 )
    {
    std::cout<<"error sending the message"<<std::endl;
    }
}

} // end namespace bm
