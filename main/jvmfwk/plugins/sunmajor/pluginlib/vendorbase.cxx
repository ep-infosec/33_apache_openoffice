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
#include "precompiled_plugin.hxx"

#include "osl/file.hxx"
#include "osl/diagnose.h"
#include "osl/module.hxx"
#include "osl/thread.hxx"

#include "vendorbase.hxx"
#include "util.hxx"
#include "sunjre.hxx"

using namespace std;
using namespace rtl;
using namespace osl;

namespace jfw_plugin
{
rtl::Reference<VendorBase> createInstance(createInstance_func pFunc,
                                          vector<pair<OUString, OUString> > properties);







//##############################################################################

MalformedVersionException::MalformedVersionException()
{}
MalformedVersionException::MalformedVersionException(
    const MalformedVersionException & )
{}
MalformedVersionException::~MalformedVersionException()
{}
MalformedVersionException &
MalformedVersionException::operator =(
    const MalformedVersionException &)
{
    return *this;
}
//##############################################################################


VendorBase::VendorBase(): m_bAccessibility(false)
{
}

char const* const * VendorBase::getJavaExePaths(int* size)
{
    static char const * ar[] = {
#if defined(WNT) || defined(OS2)
        "java.exe",
        "bin/java.exe"
#elif UNX
        "java",
        "bin/java"
#endif
    };
    *size = sizeof(ar) / sizeof(char*);
    return ar;
}


rtl::Reference<VendorBase> VendorBase::createInstance()
{
    VendorBase *pBase = new VendorBase();
    return rtl::Reference<VendorBase>(pBase);
}

bool VendorBase::initialize(vector<pair<OUString, OUString> > props)
{
    //get java.vendor, java.version, java.home,
    //javax.accessibility.assistive_technologies from system properties
    
    OUString sVendor;
    typedef vector<pair<OUString, OUString> >::const_iterator it_prop;
    OUString sVendorProperty(
        RTL_CONSTASCII_USTRINGPARAM("java.vendor"));
    OUString sVersionProperty(
        RTL_CONSTASCII_USTRINGPARAM("java.version"));
    OUString sHomeProperty(
        RTL_CONSTASCII_USTRINGPARAM("java.home"));
    OUString sAccessProperty(
        RTL_CONSTASCII_USTRINGPARAM("javax.accessibility.assistive_technologies"));

    bool bVersion = false;
    bool bVendor = false;
    bool bHome = false;
    bool bAccess = false;
    
    typedef vector<pair<OUString, OUString> >::const_iterator it_prop;
    for (it_prop i = props.begin(); i != props.end(); i++)
    {
        if(! bVendor && sVendorProperty.equals(i->first))
        {
            m_sVendor = i->second;
            bVendor = true;
        }
        else if (!bVersion && sVersionProperty.equals(i->first))
        {
            m_sVersion = i->second;
            bVersion = true;
        }
        else if (!bHome && sHomeProperty.equals(i->first))
        {
           OUString fileURL;
           if (osl_getFileURLFromSystemPath(i->second.pData,& fileURL.pData) ==
               osl_File_E_None)
           {
               //make sure that the drive letter have all the same case
               //otherwise file:///c:/jre and file:///C:/jre produce two
               //different objects!!!
               if (makeDriveLetterSame( & fileURL))
               {
                   m_sHome = fileURL;
                   bHome = true;
               }
           }
        }
        else if (!bAccess && sAccessProperty.equals(i->first))
        {
            if (i->second.getLength() > 0)
            {
                m_bAccessibility = true;
                bAccess = true;
            }
        }
        // the javax.accessibility.xxx property may not be set. Therefore we
        //must search through all properties.

    }
    if (!bVersion || !bVendor || !bHome)
        return false;
    
    // init m_sRuntimeLibrary
    OSL_ASSERT(m_sHome.getLength());
    //call virtual function to get the possible paths to the runtime library.
    
    int size = 0;
    char const* const* arRtPaths = getRuntimePaths( & size);
    vector<OUString> libpaths = getVectorFromCharArray(arRtPaths, size);
    
    bool bRt = false;
    typedef vector<OUString>::const_iterator i_path;
    for(i_path ip = libpaths.begin(); ip != libpaths.end(); ip++)
    {
        //Construct an absolute path to the possible runtime
        OUString usRt= m_sHome + *ip;
        DirectoryItem item;
        if(DirectoryItem::get(usRt, item) == File::E_None)
        {
            //found runtime lib
            m_sRuntimeLibrary = usRt;
            bRt = true;
            break;
        }
    }
    if (!bRt)
        return false;

#if defined(WNT)
    oslModule moduleRt = 0;
    rtl::OUString sRuntimeLib;
    if( File::getSystemPathFromFileURL( m_sRuntimeLibrary, sRuntimeLib ) == File::E_None )
    {
        if ( ( moduleRt = osl_loadModule( sRuntimeLib.pData, SAL_LOADMODULE_DEFAULT ) ) == 0 )
        {
            OSL_TRACE( "jfw_plugin::VendorBase::initialize - cannot load library %s",
			           rtl::OUStringToOString( sRuntimeLib, osl_getThreadTextEncoding() ).getStr() );
            return false;
        }
        else
        {
            // do not leave the module loaded!
            osl_unloadModule( moduleRt );
        }
    }
#endif

    // init m_sLD_LIBRARY_PATH
    OSL_ASSERT(m_sHome.getLength());
    size = 0;
    char const * const * arLDPaths = getLibraryPaths( & size);
    vector<OUString> ld_paths = getVectorFromCharArray(arLDPaths, size);

    char arSep[]= {SAL_PATHSEPARATOR, 0};
    OUString sPathSep= OUString::createFromAscii(arSep);
    bool bLdPath = true;
    int c = 0;
    for(i_path il = ld_paths.begin(); il != ld_paths.end(); il ++, c++)
    {
        OUString usAbsUrl= m_sHome + *il;
        // convert to system path
        OUString usSysPath;
        if(File::getSystemPathFromFileURL(usAbsUrl, usSysPath) == File::E_None)
        {

            if(c > 0)
                m_sLD_LIBRARY_PATH+= sPathSep;
            m_sLD_LIBRARY_PATH+= usSysPath;
        }
        else
        {
            bLdPath = false;
            break;
        }
    }
    if (bLdPath == false)
        return false;
    
    return true;
}

char const* const* VendorBase::getRuntimePaths(int* /*size*/)
{
    return NULL;
}

char const* const* VendorBase::getLibraryPaths(int* /*size*/)
{
    return NULL;
}

const OUString & VendorBase::getVendor() const
{
    return m_sVendor;
}
const OUString & VendorBase::getVersion() const
{
    return m_sVersion;
}

const OUString & VendorBase::getHome() const
{
    return m_sHome;
}

const OUString & VendorBase::getLibraryPaths() const
{
    return m_sLD_LIBRARY_PATH;
}

const OUString & VendorBase::getRuntimeLibrary() const
{
    return m_sRuntimeLibrary;
}
bool VendorBase::supportsAccessibility() const
{
    return m_bAccessibility;
}

bool VendorBase::needsRestart() const
{
    if (getLibraryPaths().getLength() > 0)
        return true;
    return false;
}

int VendorBase::compareVersions(const rtl::OUString& /*sSecond*/) const
{
    OSL_ENSURE(0, "[Java framework] VendorBase::compareVersions must be "
               "overridden in derived class.");
    return 0;
}


        

}
