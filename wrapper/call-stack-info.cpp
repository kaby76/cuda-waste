/*
   Copyright 2010 Ken Domino

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1

#include "stdafx.h"
#include "call-stack-info.h"
#include <windows.h> // required by stupid dbghelp.h -- it doesn't know its own dependencies...
#include <dbghelp.h>
#include <stdio.h>
#include <iostream>

extern char * file_name_tail(char * file_name);
#define BUFFERSIZE 50000
char lpszPath[BUFFERSIZE];
char lpszPath_shorten[BUFFERSIZE];

CALL_STACK_INFO::CALL_STACK_INFO()
{
    DWORD symOptions = SymGetOptions();

    symOptions |= SYMOPT_LOAD_LINES; 
    symOptions &= ~SYMOPT_UNDNAME;
    SymSetOptions( symOptions );
    if (! GetEnvironmentVariableA( "PATH", lpszPath, BUFFERSIZE))
    {
        strcpy(lpszPath, ".");
    }
    // SymInitialize crashes on large search paths. FUCKING AWEINSPIRING!
    // Shorten the fucker.
    int num = 6;
    int stop = 0;
    size_t len = strlen(lpszPath);
    // Scan ahead for semicolon.
    for (int i = 0; i < len; ++i)
    {
        char c = lpszPath[i];
        if (c == ';')
            num--;
        if (num == 0)
        {
            stop = i;
            break;
        }
        if (c == 0)
        {
            stop = i;
            break;
        }
    }
    for (int i = 0; i < stop; ++i)
    {
        char c = lpszPath[i];
        lpszPath_shorten[i] = lpszPath[i];
    }
    lpszPath_shorten[stop] = 0;
    HANDLE h = GetCurrentProcess();
    SymInitialize(h, lpszPath_shorten, TRUE);
}
CALL_STACK_INFO * CALL_STACK_INFO::singleton = 0;

CALL_STACK_INFO * CALL_STACK_INFO::Singleton()
{
    CALL_STACK_INFO * s = CALL_STACK_INFO::singleton;
    if (s)
        return s;
    s = new CALL_STACK_INFO();
    CALL_STACK_INFO::singleton = s;
    return s;
}

// Find the module name from the ip
bool CALL_STACK_INFO::GetModuleNameFromAddress(void * address, char * lpszModule)
{
    bool              ret = false;
        strcpy( lpszModule, "?");
    return false;
//#if defined(_WIN64)
        // Not found :(
        strcpy( lpszModule, "?");
//#elif defined(_WIN32)
    IMAGEHLP_MODULE   moduleInfo;

    ::ZeroMemory( &moduleInfo, sizeof(moduleInfo) );
    moduleInfo.SizeOfStruct = sizeof(moduleInfo);

    if (SymGetModuleInfo( GetCurrentProcess(), (DWORD)address, &moduleInfo ) )
    {
        strncpy(lpszModule, moduleInfo.ModuleName, sizeof(moduleInfo.ModuleName) );
        ret = true;
    }
    else
        // Not found :(
        strcpy( lpszModule, "?");
//#endif    
    return ret;
}

// Get function prototype and parameter info from ip address and stack address
bool CALL_STACK_INFO::GetFunctionInfoFromAddresses( void * fnAddress, void * stackAddress, char * lpszSymbol )
{
    return FALSE;
#ifdef XXX
    BOOL              ret = FALSE;
    DWORD             dwDisp = 0;
    DWORD             dwSymSize = 10000;
   TCHAR             lpszUnDSymbol[BUFFERSIZE]=_T("?");
    CHAR              lpszNonUnicodeUnDSymbol[BUFFERSIZE]="?";
    LPTSTR            lpszParamSep = NULL;
    LPCTSTR           lpszParsed = lpszUnDSymbol;
    PIMAGEHLP_SYMBOL  pSym = (PIMAGEHLP_SYMBOL)GlobalAlloc( GMEM_FIXED, dwSymSize );

    ::ZeroMemory( pSym, dwSymSize );
    pSym->SizeOfStruct = dwSymSize;
    pSym->MaxNameLength = dwSymSize - sizeof(IMAGEHLP_SYMBOL);

   // Set the default to unknown
    _tcscpy( lpszSymbol, _T("?") );

    // Get symbol info for IP
    if ( SymGetSymFromAddr( GetCurrentProcess(), (ULONG)fnAddress, &dwDisp, pSym ) )
    {
       // Make the symbol readable for humans
        UnDecorateSymbolName( pSym->Name, lpszNonUnicodeUnDSymbol, BUFFERSIZE, 
            UNDNAME_COMPLETE | 
            UNDNAME_NO_THISTYPE |
            UNDNAME_NO_SPECIAL_SYMS |
            UNDNAME_NO_MEMBER_TYPE |
            UNDNAME_NO_MS_KEYWORDS |
            UNDNAME_NO_ACCESS_SPECIFIERS );

      // Symbol information is ANSI string
        PCSTR2LPTSTR( lpszNonUnicodeUnDSymbol, lpszUnDSymbol );

      // I am just smarter than the symbol file :)
        if ( _tcscmp(lpszUnDSymbol, _T("_WinMain@16")) == 0 )
            _tcscpy(lpszUnDSymbol, _T("WinMain(HINSTANCE,HINSTANCE,LPCTSTR,int)"));
        else
        if ( _tcscmp(lpszUnDSymbol, _T("_main")) == 0 )
            _tcscpy(lpszUnDSymbol, _T("main(int,TCHAR * *)"));
        else
        if ( _tcscmp(lpszUnDSymbol, _T("_mainCRTStartup")) == 0 )
            _tcscpy(lpszUnDSymbol, _T("mainCRTStartup()"));
        else
        if ( _tcscmp(lpszUnDSymbol, _T("_wmain")) == 0 )
            _tcscpy(lpszUnDSymbol, _T("wmain(int,TCHAR * *,TCHAR * *)"));
        else
        if ( _tcscmp(lpszUnDSymbol, _T("_wmainCRTStartup")) == 0 )
            _tcscpy(lpszUnDSymbol, _T("wmainCRTStartup()"));

        lpszSymbol[0] = _T('\0');

      // Let's go through the stack, and modify the function prototype, and insert the actual
      // parameter values from the stack
        if ( _tcsstr( lpszUnDSymbol, _T("(void)") ) == NULL && _tcsstr( lpszUnDSymbol, _T("()") ) == NULL)
        {
            ULONG index = 0;
            for( ; ; index++ )
            {
                lpszParamSep = _tcschr( lpszParsed, _T(',') );
                if ( lpszParamSep == NULL )
                    break;

                *lpszParamSep = _T('\0');

                _tcscat( lpszSymbol, lpszParsed );
                _stprintf( lpszSymbol + _tcslen(lpszSymbol), _T("=0x%08X,"), *((ULONG*)(stackAddress) + 2 + index) );

                lpszParsed = lpszParamSep + 1;
            }

            lpszParamSep = _tcschr( lpszParsed, _T(')') );
            if ( lpszParamSep != NULL )
            {
                *lpszParamSep = _T('\0');

                _tcscat( lpszSymbol, lpszParsed );
                _stprintf( lpszSymbol + _tcslen(lpszSymbol), _T("=0x%08X)"), *((ULONG*)(stackAddress) + 2 + index) );

                lpszParsed = lpszParamSep + 1;
            }
        }

        _tcscat( lpszSymbol, lpszParsed );
   
        ret = TRUE;
    }

    GlobalFree( pSym );

    return ret;
#endif
}

// Get source file name and line number from IP address
// The output format is: "sourcefile(linenumber)" or
//                       "modulename!address" or
//                       "address"
    char          lpszFileName[BUFFERSIZE];
    char          lpModuleInfo[BUFFERSIZE];
bool CALL_STACK_INFO::GetSourceInfoFromAddress( void * address, char * lpszSourceInfo, char * full_file_name )
{
    return false;
    bool           ret = FALSE;
    IMAGEHLP_LINE  lineInfo;
    DWORD          dwDisp;

    strcpy(lpszFileName, "");
    strcpy(lpModuleInfo, "");
    strcpy(lpszSourceInfo, "?(?)");

    ::ZeroMemory( &lineInfo, sizeof( lineInfo ) );
    lineInfo.SizeOfStruct = sizeof( lineInfo );

    if ( SymGetLineFromAddr( GetCurrentProcess(), (DWORD)address, &dwDisp, &lineInfo ) )
    {
        char * fn = file_name_tail(lineInfo.FileName);
        strncpy(full_file_name, lineInfo.FileName, BUFFERSIZE);
        sprintf(lpszSourceInfo, "file %s, line %d", fn, lineInfo.LineNumber);
        ret = true;
    }
    else
    {
        strncpy(full_file_name, "", BUFFERSIZE);
        GetModuleNameFromAddress( address, lpModuleInfo );
        if ( lpModuleInfo[0] == '?' || lpModuleInfo[0] == '\0')
            sprintf(lpszSourceInfo, "Address 0x%08X", address);
        else
            sprintf(lpszSourceInfo, "Module %s, Address 0x%08X", lpModuleInfo, address);
        ret = false;
    }
    return ret;
}

char buffer2[BUFFERSIZE];
char context_string[BUFFERSIZE];
char full_file_name[BUFFERSIZE];

char * CALL_STACK_INFO::Context(int lines)
{
    typedef USHORT (WINAPI *CaptureStackBackTraceType)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);
    CaptureStackBackTraceType func = (CaptureStackBackTraceType)(GetProcAddress(LoadLibraryA("kernel32.dll"), "RtlCaptureStackBackTrace"));
    const int kMaxCallers = 62; 
    void* callers[kMaxCallers];
    int count = (func)(0, kMaxCallers, callers, NULL);
    bool seen_prefix = false;
    int times = 0;
    for(int i = 0; i < count; i++)
    {
        GetSourceInfoFromAddress(callers[i], context_string, full_file_name);
        if (! seen_prefix)
        {
            bool present = false;
            for (std::list<char*>::iterator it = ignore_files.begin(); it != ignore_files.end(); it++)
            {
                // Remove prefix if the full_file_name matches a file on the list of files to ignore.
                if (strstr(full_file_name, *it))
                {
                    // present.
                    present = true;
                    break;
                }
            }
            if (! present)
            {
                seen_prefix = true;
            }
        }
        else
            seen_prefix = true;
        if (seen_prefix)
        {
            if (times)
                strcat(buffer2, "\n");
            strcat(buffer2, context_string);
            times++;
            if (times == lines)
                break;
        }
    }

    if (strcmp("", buffer2) == 0)
    {
        strcat(buffer2, "(no call stack available)");
    }
    return strdup(buffer2);
}

void * * CALL_STACK_INFO::AddressContext(size_t lines)
{
    typedef USHORT (WINAPI *CaptureStackBackTraceType)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);
    CaptureStackBackTraceType func = (CaptureStackBackTraceType)(GetProcAddress(LoadLibraryA("kernel32.dll"), "RtlCaptureStackBackTrace"));
    const int kMaxCallers = 62; 
    static void* callers[kMaxCallers];
    for (int i = 0; i < kMaxCallers; ++i)
        callers[i] = 0;
    int count = (func)(0, kMaxCallers, callers, NULL);
    return callers;
}

void CALL_STACK_INFO::ClassifyAsPrefix(char * file)
{
    // Add file name to list of files that you want to remove from call stack context string.
    ignore_files.push_back(file);
}

std::list<void*> * CALL_STACK_INFO::CallTree()
{
    std::list<void*> * result = new std::list<void*>();
    typedef USHORT (WINAPI *CaptureStackBackTraceType)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);
    CaptureStackBackTraceType func = (CaptureStackBackTraceType)(GetProcAddress(LoadLibraryA("kernel32.dll"), "RtlCaptureStackBackTrace"));
    const int kMaxCallers = 62; 
    void* callers[kMaxCallers];
    int count = (func)(0, kMaxCallers, callers, NULL);
    for(int i = 0; i < count; i++)
    {
        result->push_back(callers[i]);
    }
    return result;
}
