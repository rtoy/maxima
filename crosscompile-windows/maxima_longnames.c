/*
    Clisp has problems with generated Windows 'short names' e.g.
    echo %TMP%
    C:\Users\TESTTE~1\AppData\Local\Temp
    Windows generates a short name for the temporary dir, when the
    username contains spaces and is longer than 8 chars.
    This program converts that path name to the 'long version', so that Clisp has
    no problems with long user names and plotting commands.
*/
#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#define BUFSIZE 4096

int main(int argc, char *argv[])
{
    DWORD  retval=0;
    TCHAR  buffer[BUFSIZE];

   if( argc != 2 )
   {
      _tprintf(TEXT("Usage: %s [file]\n"), argv[0]);
      return 1;
   }

// Retrieve the long path name.  

    retval = GetLongPathName(argv[1],
              buffer,
              BUFSIZE);

    if (retval == 0) 
    {
        // Handle an error condition.
         printf ("GetLongPathName failed (%d)\n", GetLastError());
         return 1;
    }
    else printf("%s", buffer);
    return 0;
}


