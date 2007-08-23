#include "zlib.h"
#include <iostream>

#if defined(MSDOS) || defined(OS2) || defined(WIN32)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
# include <iostream>
#  define SET_BINARY_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#ifdef RISCOS
#  define unlink remove
#  define GZ_SUFFIX "-gz"
#  define fileno(file) file->__file
#endif
#if defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
   #include <unistd.h>
#endif

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (sizeof(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local static
   /* Needed for systems with limitation on stack size. */
#else
#  define local
#endif


bool IsCompressed(const char* name)
{
  std::string m_string(name);
  if (m_string.rfind(".gz") == m_string.length()-3)
   return true;
  else
   return false;
}

void gz_uncompress(gzFile in,FILE* out)
{
    local char buf[BUFLEN];
    int len;
    int err;

    for (;;) {
        len = gzread(in, buf, sizeof(buf));
        if (len < 0) 
          std::cerr << gzerror(in, &err) <<std::endl;
        if (len == 0) break;

        if ((int)fwrite(buf, 1, (unsigned)len, out) != len) {
          std::cerr << "failed fwrite" << std::endl;
  }
    }
    if (fclose(out)) 
      std::cerr << "failed fclose" << std::endl;

    if (gzclose(in) != Z_OK) 
       std::cerr << "failed gzclose" << std::endl;
}

int gz_uncompressfile(char* file)
{
   if (!IsCompressed(file))
   {
     std::cerr << "File not compressed or bad format !" << std::endl;
     return -1;
   }

    local char buf[MAX_NAME_LEN];
    char *infile, *outfile;
    FILE  *out;
    gzFile in;
    int len = strlen(file);

    strcpy(buf, file);

    if (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } else {
        outfile = file;
        infile = buf;
        strcat(infile, GZ_SUFFIX);
    }
    in = gzopen(infile, "rb");
    if (in == NULL) {
      std::cerr << "Can't gzopen: " << infile << std::endl;
        return -1;
    }
    out = fopen(outfile, "wb");
    if (out == NULL) {
        perror(file);
        exit(1);
    }

    gz_uncompress(in, out);

    unlink(infile);

    return 0;
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cerr << "Please specify a file to uncompress !" << std::endl;
    return -1;
  }

   char outmode[20];
   sprintf(outmode,"wb%i ",6);
   int m_result = 0; 
   for (int i=1;i<argc;i++)
     {
     if (gz_uncompressfile(argv[i]) != 0)
       {
       m_result = -1;
       }
     }
  return m_result; 
}





