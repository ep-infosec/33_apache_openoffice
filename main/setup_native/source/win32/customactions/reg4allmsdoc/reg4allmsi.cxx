/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include <string>
#include <strsafe.h>

//----------------------------------------------------------
static const CHAR* g_Extensions[] =
{
    ".doc",     // Microsoft Word Text [0]
    ".dot",     // Microsoft Word Template
    ".rtf",     // RTF text
    ".docx",    // Office Word 2007 XML document
    ".docm",    // Office Word 2007 XML macro-enabled document
    ".dotx",    // Office Word 2007 XML template
    ".dotm",    // Office Word 2007 XML macro-enabled template
    ".xlw",     // Microsoft Excel
    ".xls",     // Microsoft Excel
    ".xlt",     // Microsoft Excel Template
    ".xlsx",    // Office Excel 2007 XML workbook
    ".xlsm",    // Office Excel 2007 XML macro-enabled workbook
    ".xltx",    // Office Excel 2007 XML template
    ".xltm",    // Office Excel 2007 XML macro-enabled template
    ".xlsb",    // Office Excel 2007 binary workbook (BIFF12)
    ".ppt",     // Microsoft PowerPoint
    ".pps",     // Microsoft PowerPoint
    ".pot",     // Microsoft PowerPoint Template
    ".pptx",    // Office PowerPoint 2007 XML presentation
    ".pptm",    // Office PowerPoint 2007 macro-enabled XML presentation
    ".potx",    // Office PowerPoint 2007 XML template
    ".potm",    // Office PowerPoint 2007 macro-enabled XML template
    ".ppsx",    // Office PowerPoint 2007 XML show
    0
};

static const int WORD_START = 0;
static const int EXCEL_START = 7;
static const int POWERPOINT_START = 15;
static const int POWERPOINT_END = 23;

//    ".xlam",    // Office Excel 2007 XML macro-enabled add-in
//    ".ppam",    // Office PowerPoint 2007 macro-enabled XML add-in
//    ".ppsm",    // Office PowerPoint 2007 macro-enabled XML show

//----------------------------------------------------------
#ifdef DEBUG
inline void OutputDebugStringFormat( LPCSTR pFormat, ... )
{
	CHAR    buffer[1024];
	va_list args;

	va_start( args, pFormat );
	StringCchVPrintfA( buffer, sizeof(buffer), pFormat, args );
	OutputDebugStringA( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCSTR, ... )
{
}
#endif

//----------------------------------------------------------
static BOOL CheckExtensionInRegistry( LPCSTR lpSubKey )
{
    BOOL    bRet = false;
    HKEY    hKey = NULL;
    LONG    lResult = RegOpenKeyExA( HKEY_CLASSES_ROOT, lpSubKey, 0, KEY_QUERY_VALUE, &hKey );

	if ( ERROR_SUCCESS == lResult )
	{
        CHAR    szBuffer[1024];
        DWORD   nSize = sizeof( szBuffer );

        lResult = RegQueryValueExA( hKey, "", NULL, NULL, (LPBYTE)szBuffer, &nSize );
        if ( ERROR_SUCCESS == lResult )
        {
            szBuffer[nSize] = '\0';
            OutputDebugStringFormat( "Found value [%s] for key [%s].\n", szBuffer, lpSubKey );

            if ( strncmp( szBuffer, "WordPad.Document.1", 18 ) == 0 )
            {   // We will replace registration for WordPad
                bRet = true;
            }
            else if ( strncmp( szBuffer, "OpenOffice.org.", 15 ) == 0 )
            {   // We will replace registration for our (former) own types, too
                bRet = true;
            }
            else if ( strncmp( szBuffer, "OpenOffice.", 11 ) == 0 )
            {   // We will replace registration for our own types, too
                bRet = true;
            }
            else if ( strncmp( szBuffer, "ooostub.", 8 ) == 0 )
            {   // We will replace registration for ooostub, too
                bRet = true;
            }
            else
            {
                OutputDebugStringFormat( "  Checking OpenWithList of [%s].\n", lpSubKey );
                HKEY hSubKey;
                lResult = RegOpenKeyExA( hKey, "OpenWithList", 0, KEY_ENUMERATE_SUB_KEYS, &hSubKey );
                if ( ERROR_SUCCESS == lResult )
                {
                    DWORD nIndex = 0;
                    while ( ERROR_SUCCESS == lResult )
                    {
                        nSize = sizeof( szBuffer );
                        lResult = RegEnumKeyExA( hSubKey, nIndex++, szBuffer, &nSize, NULL, NULL, NULL, NULL );
                        if ( ERROR_SUCCESS == lResult )
                        {
                            OutputDebugStringFormat( "    Found value [%s] in OpenWithList of [%s].\n", szBuffer, lpSubKey );
                            if ( strncmp( szBuffer, "WordPad.exe", 11 ) == 0 )
                            {   // We will replace registration for WordPad
                                bRet = true;
                            }
                            else if ( nSize > 0 )
                                bRet = false;
                        }
                    }
                }
                else
                {
                    OutputDebugStringFormat( "  No OpenWithList found!\n" );
                }
            }
        }
        else    // no default value found -> return TRUE to register for that key
            bRet = true;

		RegCloseKey( hKey );
	}
    else // no key found -> return TRUE to register for that key
        bRet = true;

    return bRet;
}

//----------------------------------------------------------
static LONG DeleteSubKeyTree( HKEY RootKey, LPCSTR lpKey )
{
	HKEY hKey;
	LONG rc = RegOpenKeyExA( RootKey, lpKey, 0, KEY_READ | DELETE, &hKey );

	if (ERROR_SUCCESS == rc)
	{
		LPCSTR    lpSubKey;
		DWORD     nMaxSubKeyLen;

		rc = RegQueryInfoKeyA( hKey, 0, 0, 0, 0, &nMaxSubKeyLen, 0, 0, 0, 0, 0, 0 );
		nMaxSubKeyLen++; // space for trailing '\0'
		lpSubKey = reinterpret_cast<CHAR*>( _alloca( nMaxSubKeyLen*sizeof(CHAR) ) );

		while (ERROR_SUCCESS == rc)
        {
			DWORD nLen = nMaxSubKeyLen;
			rc = RegEnumKeyExA( hKey, 0, (LPSTR)lpSubKey, &nLen, 0, 0, 0, 0); // always index zero

            if ( ERROR_NO_MORE_ITEMS == rc )
            {
				rc = RegDeleteKeyA( RootKey, lpKey );
                if ( rc == ERROR_SUCCESS )
                    OutputDebugStringFormat( "deleted key [%s] from registry.\n", lpKey );
                else
                    OutputDebugStringFormat( "RegDeleteKeyA %s returned %ld.\n", lpKey, rc );
                break;
            }
            else if ( rc == ERROR_SUCCESS )
			{
				rc = DeleteSubKeyTree( hKey, lpSubKey );
                if ( ERROR_SUCCESS != rc )
                    OutputDebugStringFormat( "RegDeleteKeyA %s returned %ld.\n", lpSubKey, rc );
			}

		}
        RegCloseKey(hKey);
	}
    else
    {
        OutputDebugStringFormat( "RegOpenKeyExA %s returned %ld.\n", lpKey, rc );
    }

	return rc;
}

//----------------------------------------------------------
static BOOL RemoveExtensionInRegistry( LPCSTR lpSubKey )
{
    CHAR    szBuffer[4096];
    DWORD   nSize = sizeof( szBuffer );
    HKEY    hKey = NULL;
    HKEY    hSubKey = NULL;
    LONG    lResult = RegOpenKeyExA( HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes", 0, KEY_QUERY_VALUE, &hKey );

	if ( ERROR_SUCCESS == lResult )
    {
		lResult = RegOpenKeyExA( hKey, lpSubKey, 0, KEY_QUERY_VALUE, &hSubKey );

        if ( ERROR_SUCCESS == lResult )
        {
            DWORD nSubKeys = 1;
            szBuffer[0] = '\0';

            // we get the value of the default key first and while we are on querying,
            // we ask for the subkey count, too
            lResult = RegQueryValueExA( hSubKey, "", NULL, NULL, (LPBYTE)szBuffer, &nSize );
            if ( ERROR_SUCCESS == lResult )
                RegQueryInfoKeyA( hSubKey, 0, 0, 0, &nSubKeys, 0, 0, 0, 0, 0, 0, 0 );
            RegCloseKey( hSubKey );

            // we will remove all key with a default value starting with ooostub but
            // we have to be careful about MSO keys
            if ( strncmp( szBuffer, "opendocument.", 13 ) == 0 )
            {
                if ( nSubKeys == 0 )
                {
                    DeleteSubKeyTree( hKey, lpSubKey );
                }
                else
                {
                    lResult = RegOpenKeyExA( hKey, lpSubKey, 0, KEY_SET_VALUE, &hSubKey );
                    if ( ERROR_SUCCESS == lResult )
                        RegDeleteValueA( hSubKey, "" );
                    else
                        OutputDebugStringFormat( "Could not open key %s for deleting: RegOpenKeyEx returned %ld.\n", lpSubKey, lResult );
                }
            }
        }

        RegCloseKey( hKey );
    }

    return ( ERROR_SUCCESS == lResult );
}

//----------------------------------------------------------
bool GetMsiProp( MSIHANDLE handle, LPCSTR name, /*out*/std::string& value )
{
    DWORD sz = 0;
    LPSTR dummy = "";
    if (MsiGetPropertyA(handle, name, dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++;
        DWORD nbytes = sz * sizeof(TCHAR);
        LPSTR buff = reinterpret_cast<LPSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetPropertyA(handle, name, buff, &sz);
        value = buff;
        return true;
    }
    return false;
}

//----------------------------------------------------------
bool IsSetMsiProp( MSIHANDLE handle, LPCSTR name )
{
    std::string val;
    GetMsiProp( handle, name, val );
    return (val == "1");
}

//----------------------------------------------------------
static void registerForExtension( MSIHANDLE handle, const int nIndex, bool bRegister )
{
    CHAR sPropName[256];
    StringCchCopyA( sPropName, 256, "REGISTER_" );
    StringCchCatA( sPropName, 256, (g_Extensions[nIndex])+1 );
    CharUpperBuffA( sPropName+9, 4 );

    if ( bRegister ) {
        MsiSetPropertyA( handle, sPropName, "1" );
        OutputDebugStringFormat( "Set MSI property %s.\n", sPropName );
    } else {
        MsiSetPropertyA( handle, sPropName, "0" );
        OutputDebugStringFormat( "Unset MSI property %s.\n", sPropName );
    }
}

//----------------------------------------------------------
static void registerForExtensions( MSIHANDLE handle, BOOL bRegisterAll )
{ // Check all file extensions
    int nIndex = 0;
    while ( g_Extensions[nIndex] != 0 )
    {
        BOOL bRegister = bRegisterAll || CheckExtensionInRegistry( g_Extensions[nIndex] );
        if ( bRegister )
            registerForExtension( handle, nIndex, true );
        ++nIndex;
    }
}

//----------------------------------------------------------
static bool checkSomeExtensionInRegistry( const int nStart, const int nEnd )
{ // Check all file extensions
    int nIndex = nStart;
    bool bFound = false;

    while ( !bFound && ( g_Extensions[nIndex] != 0 ) && ( nIndex < nEnd ) )
    {
        bFound = ! CheckExtensionInRegistry( g_Extensions[nIndex] );

        if ( bFound )
            OutputDebugStringFormat( "Found registration for [%s].\n", g_Extensions[nIndex] );

        ++nIndex;
    }
    return bFound;
}

//----------------------------------------------------------
static void registerSomeExtensions( MSIHANDLE handle, const int nStart, const int nEnd, bool bRegister )
{ // Check all file extensions
    int nIndex = nStart;

    while ( ( g_Extensions[nIndex] != 0 ) && ( nIndex < nEnd ) )
    {
        registerForExtension( handle, nIndex++, bRegister );
    }
}

//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
extern "C" UINT __stdcall LookForRegisteredExtensions( MSIHANDLE handle )
{
    OutputDebugStringFormat( "LookForRegisteredExtensions: " );

    INSTALLSTATE current_state;
    INSTALLSTATE future_state;

    bool bWriterEnabled = false;
    bool bCalcEnabled = false;
    bool bImpressEnabled = false;
    bool bRegisterNone = IsSetMsiProp( handle, "REGISTER_NO_MSO_TYPES" );

    if ( ( ERROR_SUCCESS == MsiGetFeatureState( handle, L"gm_p_Wrt", &current_state, &future_state ) ) &&
         ( (future_state == INSTALLSTATE_LOCAL) || ((current_state == INSTALLSTATE_LOCAL) && (future_state == INSTALLSTATE_UNKNOWN) ) ) )
        bWriterEnabled = true;

    OutputDebugStringFormat( "LookForRegisteredExtensions: Install state Writer is [%d], will be [%d]", current_state, future_state );
    if ( bWriterEnabled )
        OutputDebugStringFormat( "LookForRegisteredExtensions: Writer is enabled" );
    else
        OutputDebugStringFormat( "LookForRegisteredExtensions: Writer is NOT enabled" );

    if ( ( ERROR_SUCCESS == MsiGetFeatureState( handle, L"gm_p_Calc", &current_state, &future_state ) ) &&
         ( (future_state == INSTALLSTATE_LOCAL) || ((current_state == INSTALLSTATE_LOCAL) && (future_state == INSTALLSTATE_UNKNOWN) ) ) )
        bCalcEnabled = true;

    OutputDebugStringFormat( "LookForRegisteredExtensions: Install state Calc is [%d], will be [%d]", current_state, future_state );
    if ( bCalcEnabled )
        OutputDebugStringFormat( "LookForRegisteredExtensions: Calc is enabled" );
    else
        OutputDebugStringFormat( "LookForRegisteredExtensions: Calc is NOT enabled" );

    if ( ( ERROR_SUCCESS == MsiGetFeatureState( handle, L"gm_p_Impress", &current_state, &future_state ) ) &&
         ( (future_state == INSTALLSTATE_LOCAL) || ((current_state == INSTALLSTATE_LOCAL) && (future_state == INSTALLSTATE_UNKNOWN) ) ) )
        bImpressEnabled = true;

    OutputDebugStringFormat( "LookForRegisteredExtensions: Install state Impress is [%d], will be [%d]", current_state, future_state );
    if ( bImpressEnabled )
        OutputDebugStringFormat( "LookForRegisteredExtensions: Impress is enabled" );
    else
        OutputDebugStringFormat( "LookForRegisteredExtensions: Impress is NOT enabled" );

    MsiSetPropertyA( handle, "SELECT_WORD", "" );
    MsiSetPropertyA( handle, "SELECT_EXCEL", "" );
    MsiSetPropertyA( handle, "SELECT_POWERPOINT", "" );

    if ( ! bRegisterNone )
    {
        if ( IsSetMsiProp( handle, "REGISTER_ALL_MSO_TYPES" ) )
        {
            if ( bWriterEnabled )
                MsiSetPropertyA( handle, "SELECT_WORD", "1" );
            if ( bCalcEnabled )
                MsiSetPropertyA( handle, "SELECT_EXCEL", "1" );
            if ( bImpressEnabled )
                MsiSetPropertyA( handle, "SELECT_POWERPOINT", "1" );
        }
        else
        {
            if ( bWriterEnabled && ! checkSomeExtensionInRegistry( WORD_START, EXCEL_START ) )
            {
                MsiSetPropertyA( handle, "SELECT_WORD", "1" );
                OutputDebugStringFormat( "LookForRegisteredExtensions: Register for Microsoft Word" );
            }
            if ( bCalcEnabled && ! checkSomeExtensionInRegistry( EXCEL_START, POWERPOINT_START ) )
            {
                MsiSetPropertyA( handle, "SELECT_EXCEL", "1" );
                OutputDebugStringFormat( "LookForRegisteredExtensions: Register for Microsoft Excel" );
            }
            if ( bImpressEnabled && ! checkSomeExtensionInRegistry( POWERPOINT_START, POWERPOINT_END ) )
            {
                MsiSetPropertyA( handle, "SELECT_POWERPOINT", "1" );
                OutputDebugStringFormat( "LookForRegisteredExtensions: Register for Microsoft PowerPoint" );
            }
        }
    }

    MsiSetPropertyA( handle, "FILETYPEDIALOGUSED", "1" );

    return ERROR_SUCCESS;
}

//----------------------------------------------------------
extern "C" UINT __stdcall RegisterSomeExtensions( MSIHANDLE handle )
{
    OutputDebugStringFormat( "RegisterSomeExtensions: " );

    if ( IsSetMsiProp( handle, "SELECT_WORD" ) )
    {
        registerSomeExtensions( handle, WORD_START, EXCEL_START, true );
        MsiSetFeatureState( handle, L"gm_p_Wrt_MSO_Reg", INSTALLSTATE_LOCAL );
        OutputDebugStringFormat( "RegisterSomeExtensions: Register for Microsoft Word" );
    }
    else
    {
        registerSomeExtensions( handle, WORD_START, EXCEL_START, false );
        MsiSetFeatureState( handle, L"gm_p_Wrt_MSO_Reg", INSTALLSTATE_ABSENT );
    }

    if ( IsSetMsiProp( handle, "SELECT_EXCEL" ) )
    {
        registerSomeExtensions( handle, EXCEL_START, POWERPOINT_START, true );
        MsiSetFeatureState( handle, L"gm_p_Calc_MSO_Reg", INSTALLSTATE_LOCAL );
        OutputDebugStringFormat( "RegisterSomeExtensions: Register for Microsoft Excel" );
    }
    else
    {
        registerSomeExtensions( handle, EXCEL_START, POWERPOINT_START, false );
        MsiSetFeatureState( handle, L"gm_p_Calc_MSO_Reg", INSTALLSTATE_ABSENT );
    }

    if ( IsSetMsiProp( handle, "SELECT_POWERPOINT" ) )
    {
        registerSomeExtensions( handle, POWERPOINT_START, POWERPOINT_END, true );
        MsiSetFeatureState( handle, L"gm_p_Impress_MSO_Reg", INSTALLSTATE_LOCAL );
        OutputDebugStringFormat( "RegisterSomeExtensions: Register for Microsoft PowerPoint" );
    }
    else
    {
        registerSomeExtensions( handle, POWERPOINT_START, POWERPOINT_END, false );
        MsiSetFeatureState( handle, L"gm_p_Impress_MSO_Reg", INSTALLSTATE_ABSENT );
    }

    return ERROR_SUCCESS;
}

//----------------------------------------------------------
extern "C" UINT __stdcall FindRegisteredExtensions( MSIHANDLE handle )
{
    if ( IsSetMsiProp( handle, "FILETYPEDIALOGUSED" ) )
    {
        OutputDebugStringFormat( "FindRegisteredExtensions: FILETYPEDIALOGUSED!" );
        return ERROR_SUCCESS;
    }

    OutputDebugStringFormat( "FindRegisteredExtensions:" );

    bool bRegisterAll = IsSetMsiProp( handle, "REGISTER_ALL_MSO_TYPES" );

    if ( IsSetMsiProp( handle, "REGISTER_NO_MSO_TYPES" ) )
    {
        OutputDebugStringFormat( "FindRegisteredExtensions: Register none!" );
        return ERROR_SUCCESS;
    }
    else if ( bRegisterAll )
        OutputDebugStringFormat( "FindRegisteredExtensions: Force all on" );
    else
        OutputDebugStringFormat( "FindRegisteredExtensions: " );

    // setting the msi properties SELECT_* will force registering for all corresponding
    // file types
    if ( IsSetMsiProp( handle, "SELECT_WORD" ) )
        registerSomeExtensions( handle, WORD_START, EXCEL_START, true );
    if ( IsSetMsiProp( handle, "SELECT_EXCEL" ) )
        registerSomeExtensions( handle, EXCEL_START, POWERPOINT_START, true );
    if ( IsSetMsiProp( handle, "SELECT_POWERPOINT" ) )
        registerSomeExtensions( handle, POWERPOINT_START, POWERPOINT_END, true );

    registerForExtensions( handle, bRegisterAll );

    return ERROR_SUCCESS;
}

//----------------------------------------------------------
extern "C" UINT __stdcall DeleteRegisteredExtensions( MSIHANDLE /*handle*/ )
{
    OutputDebugStringFormat( "DeleteRegisteredExtensions\n" );

    // remove all file extensions
    int nIndex = 0;
    while ( g_Extensions[nIndex] != 0 )
    {
        RemoveExtensionInRegistry( g_Extensions[nIndex] );
        ++nIndex;
    }

    return ERROR_SUCCESS;
}
