#ifndef _XMLINIIO_H
#define _XMLINIIO_H
  
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
  
#include "XMLWriter.h"
#include "XMLReader.h"


class XMLIniIO
{
public:
  XMLIniIO();
  ~XMLIniIO();
  void SetFileName(MString filename);
  int Read();
  int Write();
  MString Find(MString balise);
  void Update(MString balise,const char* value);
  void Update(MString balise,MString value);
  void Update(MString balise,bool value);
  void Remove(MString balise);

protected:

private:
  MString m_filename;
  struct balisestruct
  {
    MString balise;
    MString value;
  };
  
  std::vector<balisestruct*> m_list;
  XMLReader* m_reader;
  XMLWriter* m_writer;  
};

#endif
