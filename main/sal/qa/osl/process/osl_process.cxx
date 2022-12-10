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

#include "gtest/gtest.h"
#include <osl/process.h>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/ustring.hxx>
#include <unistd.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <osl/module.hxx>

#ifdef WNT // Windows
#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <string>

#if defined(WNT) || defined(OS2)
	const rtl::OUString EXECUTABLE_NAME = rtl::OUString::createFromAscii("osl_process_child.exe");
#else
	const rtl::OUString EXECUTABLE_NAME = rtl::OUString::createFromAscii("osl_process_child");
#endif


//########################################
std::string OUString_to_std_string(const rtl::OUString& oustr)
{
    rtl::OString ostr = rtl::OUStringToOString(oustr, osl_getThreadTextEncoding());
    return std::string(ostr.getStr());
}

//########################################
using namespace osl;
using namespace rtl;

/** print a UNI_CODE String.
*/
inline void printUString( const ::rtl::OUString & str )
{
	rtl::OString aString;

	printf("#printUString_u# " );
	aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
	printf("%s\n", aString.getStr( ) );
}

/** get binary Path.
*/
inline ::rtl::OUString getExecutablePath( void )
{
	::rtl::OUString dirPath;
	osl::Module::getUrlFromAddress( ( void* ) &getExecutablePath, dirPath );
	dirPath = dirPath.copy( 0, dirPath.lastIndexOf('/') );
	dirPath = dirPath.copy( 0, dirPath.lastIndexOf('/') + 1);
	dirPath += rtl::OUString::createFromAscii("bin");
	return dirPath;
}

//rtl::OUString CWD = getExecutablePath();

//########################################
class Test_osl_joinProcess : public ::testing::Test
{
protected:
    const OUString join_param_;
    const OUString wait_time_;
    OUString suCWD;
    OUString suExecutableFileURL;

    rtl_uString* parameters_[2];
    int          parameters_count_;

public:

    Test_osl_joinProcess() :
        join_param_(OUString::createFromAscii("-join")),
        wait_time_(OUString::createFromAscii("1")),
        parameters_count_(2)
    {
        parameters_[0] = join_param_.pData;
        parameters_[1] = wait_time_.pData;
        suCWD = getExecutablePath();
        suExecutableFileURL = suCWD;
        suExecutableFileURL += rtl::OUString::createFromAscii("/");
        suExecutableFileURL += EXECUTABLE_NAME;
    }
};

/*-------------------------------------
    Start a process and join with this
    process specify a timeout so that
    osl_joinProcessWithTimeout returns
    osl_Process_E_TimedOut
 -------------------------------------*/

TEST_F(Test_osl_joinProcess, osl_joinProcessWithTimeout_timeout_failure)
{
    oslProcess process;
    oslProcessError osl_error = osl_executeProcess(
        suExecutableFileURL.pData,
        parameters_,
        parameters_count_,
        osl_Process_NORMAL,
        osl_getCurrentSecurity(),
        suCWD.pData,
        NULL,
        0,
        &process);

    ASSERT_TRUE(osl_error == osl_Process_E_None) << "osl_createProcess failed";

    TimeValue timeout;
    timeout.Seconds = 1;
    timeout.Nanosec = 0;

    osl_error = osl_joinProcessWithTimeout(process, &timeout);

    ASSERT_TRUE(osl_Process_E_TimedOut == osl_error) << "osl_joinProcessWithTimeout returned without timeout failure";

    osl_error = osl_terminateProcess(process);

    ASSERT_TRUE(osl_error == osl_Process_E_None) << "osl_terminateProcess failed";

    osl_freeProcessHandle(process);
}

/*-------------------------------------
    Start a process and join with this
    process specify a timeout so that
    osl_joinProcessWithTimeout returns
    osl_Process_E_None
 -------------------------------------*/

TEST_F(Test_osl_joinProcess, osl_joinProcessWithTimeout_without_timeout_failure)
{
    oslProcess process;
    oslProcessError osl_error = osl_executeProcess(
        suExecutableFileURL.pData,
        parameters_,
        parameters_count_,
        osl_Process_NORMAL,
        osl_getCurrentSecurity(),
        suCWD.pData,
        NULL,
        0,
        &process);

    ASSERT_TRUE(osl_error == osl_Process_E_None) << "osl_createProcess failed";

    TimeValue timeout;
    timeout.Seconds = 10;
    timeout.Nanosec = 0;

    osl_error = osl_joinProcessWithTimeout(process, &timeout);

    ASSERT_TRUE(osl_Process_E_None == osl_error) << "osl_joinProcessWithTimeout returned with failure";

    osl_freeProcessHandle(process);
}

 /*-------------------------------------
    Start a process and join with this
    process specify an infinite timeout
 -------------------------------------*/

TEST_F(Test_osl_joinProcess, osl_joinProcessWithTimeout_infinite)
{
    oslProcess process;
    oslProcessError osl_error = osl_executeProcess(
        suExecutableFileURL.pData,
        parameters_,
        parameters_count_,
        osl_Process_NORMAL,
        osl_getCurrentSecurity(),
        suCWD.pData,
        NULL,
        0,
        &process);

    ASSERT_TRUE(osl_error == osl_Process_E_None) << "osl_createProcess failed";

    osl_error = osl_joinProcessWithTimeout(process, NULL);

    ASSERT_TRUE(osl_Process_E_None == osl_error) << "osl_joinProcessWithTimeout returned with failure";

    osl_freeProcessHandle(process);
}

 /*-------------------------------------
    Start a process and join with this
    process using osl_joinProcess
 -------------------------------------*/

TEST_F(Test_osl_joinProcess, osl_joinProcess)
{
    oslProcess process;
    oslProcessError osl_error = osl_executeProcess(
        suExecutableFileURL.pData,
        parameters_,
        parameters_count_,
        osl_Process_NORMAL,
        osl_getCurrentSecurity(),
        suCWD.pData,
        NULL,
        0,
        &process);

    ASSERT_TRUE(osl_error == osl_Process_E_None) << "osl_createProcess failed";

    osl_error = ::osl_joinProcess(process);

    ASSERT_TRUE(osl_Process_E_None == osl_error) << "osl_joinProcess returned with failure";

    osl_freeProcessHandle(process);
}


//#########################################################

typedef std::vector<std::string>  string_container_t;
typedef string_container_t::const_iterator string_container_const_iter_t;
typedef string_container_t::iterator       string_container_iter_t;

//#########################################################
class exclude : public std::unary_function<std::string, bool>
{
public:
    //------------------------------------------------
    exclude(const string_container_t& exclude_list)
    {
        string_container_const_iter_t iter     = exclude_list.begin();
        string_container_const_iter_t iter_end = exclude_list.end();
        for (/**/; iter != iter_end; ++iter)
            exclude_list_.push_back(env_var_name(*iter));
    }

    //------------------------------------------------
    bool operator() (const std::string& env_var) const
    {
        return (exclude_list_.end() !=
                std::find(
                    exclude_list_.begin(),
                    exclude_list_.end(),
                    env_var_name(env_var)));
    }

private:
    //-------------------------------------------------
    // extract the name from an environment variable
    // that is given in the form "NAME=VALUE"
    std::string env_var_name(const std::string& env_var) const
    {
        std::string::size_type pos_equal_sign =
            env_var.find_first_of("=");

        if (std::string::npos != pos_equal_sign)
            return std::string(env_var, 0, pos_equal_sign);

        return std::string();
    }

private:
    string_container_t exclude_list_;
};

#ifdef WNT
    void read_parent_environment(string_container_t* env_container)
    {
        LPTSTR env = reinterpret_cast<LPTSTR>(GetEnvironmentStrings());
        LPTSTR p   = env;

        while (size_t l = _tcslen(p))
        {
            env_container->push_back(std::string(p));
            p += l + 1;
        }
        FreeEnvironmentStrings(env);
    }
#else
    extern char** environ;
    void read_parent_environment(string_container_t* env_container)
    {
        for (int i = 0; NULL != environ[i]; i++)
            env_container->push_back(std::string(environ[i]));
    }
#endif

//#########################################################
class Test_osl_executeProcess : public ::testing::Test
{
protected:
    const OUString env_param_;

    OUString     temp_file_path_;
    rtl_uString* parameters_[2];
    int          parameters_count_;
    OUString	suCWD;
    OUString	suExecutableFileURL;

public:

    //------------------------------------------------
    // ctor
    Test_osl_executeProcess() :
        env_param_(OUString::createFromAscii("-env")),
        parameters_count_(2)
    {
        parameters_[0] = env_param_.pData;
        suCWD = getExecutablePath();
        suExecutableFileURL = suCWD;
        suExecutableFileURL += rtl::OUString::createFromAscii("/");
        suExecutableFileURL += EXECUTABLE_NAME;
    }

    //------------------------------------------------
    virtual void SetUp()
    {
        temp_file_path_ = create_temp_file();
        parameters_[1]  = temp_file_path_.pData;
    }

    //------------------------------------------------
    OUString create_temp_file()
    {
        OUString temp_file_url;
        FileBase::RC rc = FileBase::createTempFile(0, 0, &temp_file_url);
        EXPECT_TRUE(FileBase::E_None == rc) << "createTempFile failed";

        OUString temp_file_path;
        rc = FileBase::getSystemPathFromFileURL(temp_file_url, temp_file_path);
        EXPECT_TRUE(FileBase::E_None == rc) << "getSystemPathFromFileURL failed";

        return temp_file_path;
    }

   //------------------------------------------------
    void read_child_environment(string_container_t* env_container)
    {
        OString temp_file_name = OUStringToOString(OUString(
            parameters_[1]), osl_getThreadTextEncoding());
        std::ifstream file(temp_file_name.getStr());

        ASSERT_TRUE(file.is_open()) << "I/O error, cannot open child environment file";

        std::string line;
        while (std::getline(file, line))
            env_container->push_back(line);
    }

    //------------------------------------------------
    void dump_env(const string_container_t& env, OUString file_name)
    {
        OString fname = OUStringToOString(file_name, osl_getThreadTextEncoding());
        std::ofstream file(fname.getStr());
        std::ostream_iterator<std::string> oi(file, "\n");
		std::copy(env.begin(), env.end(), oi);
    }

    //------------------------------------------------
    // environment of the child process that was
    // started. The child process writes his
    // environment into a file
    bool compare_environments()
    {
        string_container_t parent_env;
        read_parent_environment(&parent_env);

        string_container_t child_env;
		read_child_environment(&child_env);

		return ((parent_env.size() == child_env.size()) &&
		        (std::equal(child_env.begin(), child_env.end(), parent_env.begin())));
    }

    //------------------------------------------------
    // compare the equal environment parts and the
    // different part of the child environment
    bool compare_merged_environments(const string_container_t& different_env_vars)
    {
        string_container_t parent_env;
        read_parent_environment(&parent_env);

        //remove the environment variables that we have changed
        //in the child environment from the read parent environment
        parent_env.erase(
            std::remove_if(parent_env.begin(), parent_env.end(), exclude(different_env_vars)),
            parent_env.end());

        //read the child environment and exclude the variables that
        //are different
        string_container_t child_env;
        read_child_environment(&child_env);

        //partition the child environment into the variables that
        //are different to the parent environment (they come first)
        //and the variables that should be equal between parent
        //and child environment
        string_container_iter_t iter_logical_end =
            std::stable_partition(child_env.begin(), child_env.end(), exclude(different_env_vars));

        string_container_t different_child_env_vars(child_env.begin(), iter_logical_end);
        child_env.erase(child_env.begin(), iter_logical_end);

        bool common_env_size_equals    = (parent_env.size() == child_env.size());
        bool common_env_content_equals = std::equal(child_env.begin(), child_env.end(), parent_env.begin());

		bool different_env_size_equals    = (different_child_env_vars.size() == different_env_vars.size());
		bool different_env_content_equals =
		    std::equal(different_env_vars.begin(), different_env_vars.end(), different_child_env_vars.begin());

        return (common_env_size_equals && common_env_content_equals &&
                different_env_size_equals && different_env_content_equals);
    }
};

//------------------------------------------------
// test that parent and child process have the
// same environment when osl_executeProcess will
// be called with out setting new environment
// variables
TEST_F(Test_osl_executeProcess, osl_execProc_parent_equals_child_environment)
{
    oslProcess process;
    oslProcessError osl_error = osl_executeProcess(
        suExecutableFileURL.pData,
        parameters_,
        parameters_count_,
        osl_Process_NORMAL,
        NULL,
        suCWD.pData,
        NULL,
        0,
        &process);

    ASSERT_TRUE(osl_error == osl_Process_E_None) << "osl_createProcess failed";

    osl_error = ::osl_joinProcess(process);

    ASSERT_TRUE(osl_Process_E_None == osl_error) << "osl_joinProcess returned with failure";

    osl_freeProcessHandle(process);

    ASSERT_TRUE(compare_environments()) << "Parent an child environment not equal";
}

//------------------------------------------------
#define ENV1 "PAT=a:\\"
#define ENV2 "PATHb=b:\\"
#define ENV3 "Patha=c:\\"
#define ENV4 "Patha=d:\\"

TEST_F(Test_osl_executeProcess, osl_execProc_merged_child_environment)
{
    rtl_uString* child_env[4];
    OUString env1 = OUString::createFromAscii(ENV1);
    OUString env2 = OUString::createFromAscii(ENV2);
    OUString env3 = OUString::createFromAscii(ENV3);
    OUString env4 = OUString::createFromAscii(ENV4);

    child_env[0] = env1.pData;
    child_env[1] = env2.pData;
    child_env[2] = env3.pData;
    child_env[3] = env4.pData;

    oslProcess process;
    oslProcessError osl_error = osl_executeProcess(
        suExecutableFileURL.pData,
        parameters_,
        parameters_count_,
        osl_Process_NORMAL,
        NULL,
        suCWD.pData,
        child_env,
        sizeof(child_env)/sizeof(child_env[0]),
        &process);

    ASSERT_TRUE(osl_error == osl_Process_E_None) << "osl_createProcess failed";

    osl_error = ::osl_joinProcess(process);

    ASSERT_TRUE(osl_Process_E_None == osl_error) << "osl_joinProcess returned with failure";

    osl_freeProcessHandle(process);

    string_container_t different_child_env_vars;
    different_child_env_vars.push_back(ENV1);
    different_child_env_vars.push_back(ENV2);
    different_child_env_vars.push_back(ENV4);

    ASSERT_TRUE(compare_merged_environments(different_child_env_vars)) << "osl_execProc_merged_child_environment";
}

TEST_F(Test_osl_executeProcess, osl_execProc_test_batch)
{
    oslProcess process;
    rtl::OUString suBatch = suCWD + rtl::OUString::createFromAscii("/") + rtl::OUString::createFromAscii("batch.bat");
    oslProcessError osl_error = osl_executeProcess(
        suBatch.pData,
        NULL,
        0,
        osl_Process_NORMAL,
        NULL,
        suCWD.pData,
        NULL,
        0,
        &process);

    ASSERT_TRUE(osl_error == osl_Process_E_None) << "osl_createProcess failed";

    osl_error = ::osl_joinProcess(process);

    ASSERT_TRUE(osl_Process_E_None == osl_error) << "osl_joinProcess returned with failure";

    osl_freeProcessHandle(process);
}

TEST_F(Test_osl_executeProcess, osl_execProc_exe_name_in_argument_list)
{
    rtl_uString* params[3];

    params[0] = suExecutableFileURL.pData;
    params[1] = env_param_.pData;
    params[2] = temp_file_path_.pData;
    oslProcess process;
    oslProcessError osl_error = osl_executeProcess(
        NULL,
        params,
        3,
        osl_Process_NORMAL,
        NULL,
        suCWD.pData,
        NULL,
        0,
        &process);

    ASSERT_TRUE(osl_error == osl_Process_E_None) << "osl_createProcess failed";

    osl_error = ::osl_joinProcess(process);

    ASSERT_TRUE(osl_Process_E_None == osl_error) << "osl_joinProcess returned with failure";

    osl_freeProcessHandle(process);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
