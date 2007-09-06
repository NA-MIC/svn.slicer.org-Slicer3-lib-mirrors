/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptOpenTCPSocketAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptOpenTCPSocketAction.h"

namespace bm {

/** */
ScriptOpenTCPSocketAction::ScriptOpenTCPSocketAction()
: ScriptAction()
{
}

/** */
ScriptOpenTCPSocketAction::~ScriptOpenTCPSocketAction()
{
}

/** */
bool ScriptOpenTCPSocketAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <3)
    {
    error->SetError(MString("No enough parameter for OpenTCPSocket"),linenumber);
    return false;
    }
  if (m_Parameters.size() >3)
    {
    error->SetError(MString("Too much parameters for OpenTCPSocket"),linenumber);
    return false;
    }

  m_Manager->SetSocketVariable(m_Parameters[0]);

  return true;
}

/** */
MString ScriptOpenTCPSocketAction::Help()
{
  return "OpenTCPSocket(SocketName IPadresse TCPport)";
}

/** */
void ScriptOpenTCPSocketAction::Execute()
{
  TCPSocket* socket = m_Manager->GetVariableSocket(m_Parameters[0]);
  int err = socket->OpenSocket(m_Parameters[1].toChar(),m_Parameters[2].toInt());
  
  if( err == -1 )
    {
    std::cout<<"error creating the TCPsocket"<<std::endl;
    }
}

} // end namespace bm
