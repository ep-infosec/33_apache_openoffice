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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sal/types.h>

#include "gtest/gtest.h"
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <rtl/process.h>
#include <osl/process.h>
#include <osl/module.hxx>

#include "rtl_Process_Const.h"

using namespace osl;
using namespace rtl;

/** print a UNI_CODE String. And also print some comments of the string.
*/
inline void printUString( const ::rtl::OUString & str, const sal_Char * msg = NULL )
{
    if ( msg != NULL )
    {
    	printf("#%s #printUString_u# ", msg );
    }
    rtl::OString aString;
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf("%s\n", (char *)aString.getStr( ) );
}

// -----------------------------------------------------------------------------
inline ::rtl::OUString getModulePath( void )
{
    ::rtl::OUString suDirPath;
    ::osl::Module::getUrlFromAddress(
        reinterpret_cast< oslGenericFunction >(getModulePath), suDirPath );

    printUString(suDirPath, "modulePath:");
    suDirPath = suDirPath.copy( 0, suDirPath.lastIndexOf('/') );
    suDirPath = suDirPath.copy( 0, suDirPath.lastIndexOf('/') + 1);
    suDirPath += rtl::OUString::createFromAscii("bin");
    return suDirPath;
}

// -----------------------------------------------------------------------------

namespace rtl_Process
{
class getAppCommandArg : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class getAppCommandArg

TEST_F(getAppCommandArg, getAppCommandArg_001)
{
#if defined(WNT) || defined(OS2)
    const rtl::OUString EXECUTABLE_NAME = rtl::OUString::createFromAscii("child_process.exe");
#else
    const rtl::OUString EXECUTABLE_NAME = rtl::OUString::createFromAscii("child_process");
#endif
    rtl::OUString suCWD = getModulePath();
    // rtl::OUString suCWD2 = getExecutableDirectory();

    printUString(suCWD, "path to the current module");
    // printUString(suCWD2, "suCWD2");

    oslProcess hProcess = NULL;

    const int nParameterCount = 4;
    rtl_uString* pParameters[ nParameterCount ];

    pParameters[0] = suParam0.pData;
    pParameters[1] = suParam1.pData;
    pParameters[2] = suParam2.pData;
    pParameters[3] = suParam3.pData;

    rtl::OUString suFileURL = suCWD;
    suFileURL += rtl::OUString::createFromAscii("/");
    suFileURL += EXECUTABLE_NAME;

    oslProcessError osl_error = osl_executeProcess(
        suFileURL.pData,
        pParameters,
        nParameterCount,
        osl_Process_WAIT,
        0, /* osl_getCurrentSecurity() */
        suCWD.pData,
        NULL,
        0,
        &hProcess );

    ASSERT_TRUE(osl_error == osl_Process_E_None) <<
        "osl_createProcess failed";
    //we could get return value only after the process terminated
    osl_joinProcess(hProcess);
    // ASSERT_TRUE(osl_Process_E_None == osl_error)
    //     << "osl_joinProcess returned with failure";
    oslProcessInfo* pInfo = new oslProcessInfo;
    //please pay attention to initial the Size to sizeof(oslProcessInfo), or else
    //you will get unknow error when call osl_getProcessInfo
    pInfo->Size = sizeof(oslProcessInfo);
    osl_error = osl_getProcessInfo( hProcess, osl_Process_EXITCODE, pInfo );
    ASSERT_TRUE(osl_Process_E_None == osl_error)
        << "osl_getProcessInfo returned with failure";

    printf("the exit code is %d.\n", pInfo->Code );
    ASSERT_TRUE(pInfo->Code == 2) << "rtl_getAppCommandArg or rtl_getAppCommandArgCount error.";
    delete pInfo;
}


/************************************************************************
 * For diagnostics( from sal/test/testuuid.cxx )
 ************************************************************************/
void printUuid( sal_uInt8 *pNode )
{
    printf("# UUID is: ");
    for( sal_Int32 i1 = 0 ; i1 < 4 ; i1++ )
    {
        for( sal_Int32 i2 = 0 ; i2 < 4 ; i2++ )
        {
            sal_uInt8 nValue = pNode[i1*4 +i2];
            if (nValue < 16)
            {
                printf( "0");
            }
            printf( "%02x" ,nValue );
        }
        if( i1 == 3 )
            break;
        printf( "-" );
    }
    printf("\n");
}

/**************************************************************************
 *  output UUID to a string
 **************************************************************************/
void printUuidtoBuffer( sal_uInt8 *pNode, sal_Char * pBuffer )
{
    sal_Int8 nPtr = 0;
    for( sal_Int32 i1 = 0 ; i1 < 16 ; i1++ )
    {
        sal_uInt8 nValue = pNode[i1];
        if (nValue < 16)
        {
             sprintf( (sal_Char *)(pBuffer + nPtr), "0");
             nPtr++;
        }
        sprintf( (sal_Char *)(pBuffer + nPtr), "%02x", nValue );
        nPtr += 2 ;
    }
}

class getGlobalProcessId : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class getGlobalProcessId

//gets a 16-byte fixed size identifier which is guaranteed not to change	during the current process.
TEST_F(getGlobalProcessId, getGlobalProcessId_001)
{
    sal_uInt8 pTargetUUID1[16];
    sal_uInt8 pTargetUUID2[16];
    rtl_getGlobalProcessId( pTargetUUID1 );
    rtl_getGlobalProcessId( pTargetUUID2 );
    ASSERT_TRUE( !memcmp( pTargetUUID1 , pTargetUUID2 , 16 ) )
        << "getGlobalProcessId: got two same ProcessIds.";
}

//different processes different pids
TEST_F(getGlobalProcessId, getGlobalProcessId_002)
{
#if defined(WNT) || defined(OS2)
    const rtl::OUString EXEC_NAME = rtl::OUString::createFromAscii("child_process_id.exe");
#else
    const rtl::OUString EXEC_NAME = rtl::OUString::createFromAscii("child_process_id");
#endif
    sal_uInt8 pTargetUUID1[16];
    rtl_getGlobalProcessId( pTargetUUID1 );
    printUuid( pTargetUUID1 );
    sal_Char pUUID1[32];
    printUuidtoBuffer( pTargetUUID1, pUUID1 );
    printf("# UUID to String is %s\n", pUUID1);

    rtl::OUString suCWD = getModulePath();
    oslProcess hProcess = NULL;
    rtl::OUString suFileURL = suCWD;
    suFileURL += rtl::OUString::createFromAscii("/");
    suFileURL += EXEC_NAME;
    oslFileHandle* pChildOutputRead = new oslFileHandle();
    oslProcessError osl_error = osl_executeProcess_WithRedirectedIO(
        suFileURL.pData,
        NULL,
        0,
        osl_Process_WAIT,
        0,
        suCWD.pData,
        NULL,
        0,
        &hProcess,
        NULL,
        pChildOutputRead,
        NULL);

    ASSERT_TRUE(osl_error == osl_Process_E_None)
        << "osl_createProcess failed";
    //we could get return value only after the process terminated
    osl_joinProcess(hProcess);

    sal_Char pUUID2[33];
    pUUID2[32] = '\0';
    sal_uInt64 nRead = 0;
    osl_readFile( *pChildOutputRead, pUUID2, 32, &nRead );
    printf("read buffer is %s, nRead is %lu \n", pUUID2, nRead );
    OUString suUUID2 = OUString::createFromAscii( pUUID2 );
    ASSERT_TRUE( suUUID2.equalsAsciiL( pUUID1, 32) == sal_False ) << "getGlobalProcessId: got two same ProcessIds.";
}

} // namespace rtl_Process

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
