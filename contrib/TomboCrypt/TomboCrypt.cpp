// TomboEncoder.cpp : Defines the entry point for the console application.
//

#include "wintypes.h"
#include <stdlib.h>
#include <string.h>
#include <string>
#include "CryptManager.h"
#include <iostream>

void usage()
{
	fputs("TomboCrypt encrypts and decrypts files, created by Tombo\n", stderr);
	fputs("     (http://tombo.sourceforge.jp/En/)\n", stderr);
	fputs("Usage: TomboCrypt enc|dec <file> <password>", stderr);
}

int main(int argc, char* argv[])
{
	if(argc != 4)
	{
		usage();
		return 0;
	}

	CryptManager cm;
	cm.Init(argv[3]);

	if(strcmp(argv[1], "enc") == 0)
	{
		// read data from stdin
		LPBYTE data = 0;
		DWORD size;
		data = cm.LoadStdIn(&size);

		// encode
		cm.EncryptAndStore(data, size, argv[2]);
	}
	else
	if(strcmp(argv[1], "dec") == 0)
	{
		// decode
		LPBYTE data = 0;
		DWORD size;
		data = cm.LoadAndDecrypt(&size, argv[2]);
		// write data to stdout
		fwrite(data, size, 1, stdout);
	}
	else
	{
		usage();
	}

	return 0;
}
