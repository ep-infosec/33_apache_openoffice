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
#include "precompiled_cui.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <Winreg.h>
#include <Shlwapi.h>
#include <stdio.h>


#define SO_PATH_SIZE		4096 
#define MOZ_PLUGIN_DLL_NAME "npsopluginmi.dll"
extern "C" {
int lc_isInstalled(const  char* realFilePath)
{
	HKEY hKeySoftware;
	HKEY hMozillaPlugins;
	HKEY hStarOffice;
	char sSoPath[SO_PATH_SIZE];
	char sPluginPath[SO_PATH_SIZE];

	LONG ret;
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,  "SOFTWARE",  0,  KEY_READ, &hKeySoftware);
	if(ret != ERROR_SUCCESS){
		ret = RegOpenKeyEx(HKEY_CURRENT_USER,  "SOFTWARE",  0,  KEY_READ, &hKeySoftware);
		if(ret != ERROR_SUCCESS){
			return -1;
		}
	}
	ret = RegOpenKeyEx(hKeySoftware,  "MozillaPlugins",  0,  KEY_READ, &hMozillaPlugins);
	if(ret != ERROR_SUCCESS){
		RegCloseKey(hKeySoftware);
		if( ret == ERROR_FILE_NOT_FOUND)
			return 1;
		else 
			return -1;
	}
	ret = RegOpenKeyEx(hMozillaPlugins,  "@sun.com/npsopluginmi;version=1.0",  0,  KEY_READ, &hStarOffice);
	if(ret != ERROR_SUCCESS){
		RegCloseKey(hKeySoftware);
		RegCloseKey(hMozillaPlugins);
		if( ret == ERROR_FILE_NOT_FOUND)
			return 1;
		else 
			return -1;
	}
	
	if((realFilePath == NULL) || (strlen(realFilePath) == 0) || (strlen(realFilePath) >= SO_PATH_SIZE)) 
	    ret = -1;	    
	else{
	    sprintf(sSoPath,"%s", realFilePath);    
	    ret = 0;
	} 
	//ret =  GetCurrentDirectory( SO_PATH_SIZE, sSoPath);
	//ret = GetEnvironmentVariable("prog", sSoPath, SO_PATH_SIZE);
	// GetCurrentDirectory return the char number of the string
	if(ret == 0){
		DWORD  dType = REG_SZ;
		DWORD  dSize = SO_PATH_SIZE;
		ret = RegQueryValueEx (hStarOffice, "Path", NULL,  &dType , (LPBYTE) sPluginPath, &dSize);
		if(ret == ERROR_SUCCESS){
			if(strcmp(sPluginPath, sSoPath) == 0)
				ret = 0;
			else
				ret = 1;
		}
		else
			ret = -1;
	}
	else 
		ret = -1;
	RegCloseKey(hStarOffice);	
	RegCloseKey(hMozillaPlugins);	
	RegCloseKey(hKeySoftware);
	return ret;
}

int lc_uninstallPlugin(const  char*)
{
	HKEY hKeySoftware;
	HKEY hMozillaPlugins;
	HKEY hStarOffice;

	LONG ret;
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,  "SOFTWARE",  0,  KEY_READ|KEY_WRITE, &hKeySoftware);
	if(ret != ERROR_SUCCESS){
		ret = RegOpenKeyEx(HKEY_CURRENT_USER,  "SOFTWARE",  0,  KEY_READ|KEY_WRITE, &hKeySoftware);
		if(ret != ERROR_SUCCESS){
			return -1;
		}
	}
	ret = RegOpenKeyEx(hKeySoftware,  "MozillaPlugins",  0,  KEY_READ|KEY_WRITE, &hMozillaPlugins);
	if(ret != ERROR_SUCCESS){
		RegCloseKey(hKeySoftware);
		if( ret == ERROR_FILE_NOT_FOUND)
			return 0;
		else 
			return -1;
	}

	ret = RegOpenKeyEx(hMozillaPlugins,  "@sun.com/npsopluginmi;version=1.0",  0,  KEY_READ|KEY_WRITE, &hStarOffice);
	if(ret != ERROR_SUCCESS){
		RegCloseKey(hKeySoftware);
		RegCloseKey(hMozillaPlugins);
		if( ret == ERROR_FILE_NOT_FOUND)
			return 0;
		else 
			return -1;
	}
	RegCloseKey(hStarOffice);
	ret = SHDeleteKey(hMozillaPlugins,  "@sun.com/npsopluginmi;version=1.0");
	if(ret != ERROR_SUCCESS){
		ret = -1;
	}
	RegFlushKey(hMozillaPlugins);
	RegCloseKey(hMozillaPlugins);
	RegCloseKey(hKeySoftware);
	return ret;
}

int lc_installPlugin(const  char* realFilePath)
{
	HKEY hKeySoftware;
	HKEY hMozillaPlugins;
	HKEY hStarOffice;
	char sSoPath[SO_PATH_SIZE];
	DWORD  sState;


	LONG ret;
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,  "SOFTWARE",  0,  KEY_READ|KEY_WRITE, &hKeySoftware);
	if(ret != ERROR_SUCCESS){
		ret = RegOpenKeyEx(HKEY_CURRENT_USER,  "SOFTWARE",  0,  KEY_READ|KEY_WRITE, &hKeySoftware);
		if(ret != ERROR_SUCCESS){
			return -1;
		}
	}
	ret = RegOpenKeyEx(hKeySoftware,  "MozillaPlugins",  0,  KEY_READ|KEY_WRITE, &hMozillaPlugins);
	if(ret != ERROR_SUCCESS){
		RegCreateKeyEx(hKeySoftware,  
			"MozillaPlugins",  
			0,  
			NULL,  
			REG_OPTION_NON_VOLATILE, 
			KEY_READ|KEY_WRITE, 
			NULL,
			&hMozillaPlugins,
			&sState);
	}

	ret = RegCreateKeyEx(hMozillaPlugins,  
		"@sun.com/npsopluginmi;version=1.0",  
		0,  
		NULL,  
		REG_OPTION_NON_VOLATILE, 
		KEY_READ|KEY_WRITE, 
		NULL,
        &hStarOffice, 
		&sState);
	if(ret != ERROR_SUCCESS){
		RegCloseKey(hKeySoftware);
		RegCloseKey(hMozillaPlugins);
		return -1;
	}
	
	RegFlushKey(hStarOffice);
    RegFlushKey(hMozillaPlugins);
	
	
	if((realFilePath == NULL) || (strlen(realFilePath) == 0) || (strlen(realFilePath) >= SO_PATH_SIZE)) 
	    ret = -1;	    
	else{
	    sprintf(sSoPath,"%s", realFilePath); 
	    ret = 0; 
	}
	   
	//ret =  GetCurrentDirectory( SO_PATH_SIZE, sSoPath);
	// GetCurrentDirectory return the char number of the string
	if(ret == 0){
		ret = RegSetValueEx( hStarOffice, "Path", 0, REG_SZ,  (LPBYTE) sSoPath, strlen(sSoPath) + 1);
		if(ret == ERROR_SUCCESS)
			ret = 0;
		else
			ret = -1;
	}
	else 
		ret = -1;
    RegFlushKey(hStarOffice);
    RegFlushKey(hMozillaPlugins);
    RegCloseKey(hStarOffice);
    RegCloseKey(hMozillaPlugins);
	RegCloseKey(hKeySoftware);
	RegFlushKey(HKEY_LOCAL_MACHINE);

	return ret;
}
}
