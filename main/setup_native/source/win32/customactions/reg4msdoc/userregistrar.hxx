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

// UserRegistrar.h: Schnittstelle f�r die Klasse UserRegistrar.
//
//////////////////////////////////////////////////////////////////////

#ifndef _USERREGISTRAR_HXX_
#define _USERREGISTRAR_HXX_

#include "registrar.hxx"

class UserRegistrar : public Registrar  
{
public:
	
	//###################################
	// Creation
	//###################################

	UserRegistrar(const RegistrationContextInformation& RegContext);

	//###################################
	// Command 
	//###################################
    	
	virtual void UnregisterAsHtmlEditorForInternetExplorer() const;
    
	virtual void RegisterAsDefaultShellHtmlEditor() const;
	virtual void UnregisterAsDefaultShellHtmlEditor() const;
    
protected:
    virtual void UnregisterForMsOfficeApplication(
        const std::wstring& FileExtension) const;
        
    virtual RegistryKey GetRootKeyForDefHtmlEditorForIERegistration() const;
    
private:
    
    /** Delete the privately created file associations 
          for htm files if the keys are empty  
    */
    void DeleteHtmFileAssociationKeys() const;
};

#endif 
