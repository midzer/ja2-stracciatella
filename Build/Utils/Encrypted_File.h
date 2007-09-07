#ifndef ENCRYPTED_FILE_H
#define ENCRYPTED_FILE_H

#include "Types.h"

BOOLEAN LoadEncryptedData(HWFILE File, wchar_t* DestString, UINT32 seek_chars, UINT32 read_chars);
BOOLEAN LoadEncryptedDataFromFile(const char* Filename, wchar_t DestString[], UINT32 seek_chars, UINT32 read_chars);

#endif
