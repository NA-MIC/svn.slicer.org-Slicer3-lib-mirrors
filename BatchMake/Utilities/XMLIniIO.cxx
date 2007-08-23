#include "XMLIniIO.h"

XMLIniIO::XMLIniIO()
{
  m_reader = new XMLReader();
  m_writer = new XMLWriter();
}


XMLIniIO::~XMLIniIO()
{
}

void XMLIniIO::SetFileName(MString filename)
{
  m_filename = filename;
}


int XMLIniIO::Read()
{
  if (m_reader->Open(m_filename.toChar()) == -1)
     return -1;
  
  bool m_continue = true;
  while(m_continue)
  {
    balisestruct* m_value = new balisestruct;
    m_value->balise =  m_reader->GetBalise();
    m_value->value =  m_reader->GetValue();
    if (m_value->balise.length() != 0)
      m_list.push_back(m_value);
    else
      m_continue = false;
  }
  m_reader->Close();
  return 0;
}

int XMLIniIO::Write()
{
  m_writer->Open(m_filename.toChar());
  balisestruct* m_value;
  for (unsigned int i=0;i<m_list.size();i++)
  {
    m_value =  m_list[i];
    m_writer->Write(m_value->balise,m_value->value);
  }
  m_writer->Close();
  return 0;
}

MString XMLIniIO::Find(MString balise)
{
  for(vector<balisestruct*>::iterator itNum = m_list.begin(); itNum < m_list.end(); itNum++)
  {
    if ((*itNum)->balise == balise)
          return (*itNum)->value;
  }


  return "";
}

void XMLIniIO::Update(MString balise,const char* value)
{
  Update(balise,MString(value));
}


void XMLIniIO::Update(MString balise,MString value)
{
  bool m_append = true;
  for (unsigned int i=0;i<m_list.size();i++)
  {
    if (m_list[i]->balise == balise)
    {
      m_list[i]->value = value;
      m_append = false;
    }
  }

  if (m_append)
  {
     balisestruct* m_value = new balisestruct;
     m_value->balise = balise;
     m_value->value = value;
     m_list.push_back(m_value);
  }
}

void XMLIniIO::Update(MString balise,bool value)
{
  bool m_append = true;
  for (unsigned int i=0;i<m_list.size();i++)
  {
    if (m_list[i]->balise == balise)
    {
      m_list[i]->value = MString("%1").arg((int)value);
      m_append = false;
    }
  }

  if (m_append)
  {
     balisestruct* m_value = new balisestruct;
     m_value->balise = balise;
     m_value->value = MString("%1").arg((int)value);
     m_list.push_back(m_value);
  }
}

void XMLIniIO::Remove(MString balise)
{
  for(vector<balisestruct*>::iterator itNum = m_list.begin(); itNum < m_list.end(); itNum++)
      if ((*itNum)->balise == balise)
          m_list.erase(itNum);
}
