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




#ifndef _CONFIGURATION_ACCESS_HXX_
#define _CONFIGURATION_ACCESS_HXX_
#include <vector>
#include "pppoptimizertoken.hxx"
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <map>


struct OptimizerSettings
{
	rtl::OUString	maName;
	sal_Bool		mbJPEGCompression;
	sal_Int32		mnJPEGQuality;
	sal_Bool		mbRemoveCropArea;
	sal_Int32		mnImageResolution;
	sal_Bool		mbEmbedLinkedGraphics;
	sal_Bool		mbOLEOptimization;
	sal_Int16		mnOLEOptimizationType;
	sal_Bool		mbDeleteUnusedMasterPages;
	sal_Bool		mbDeleteHiddenSlides;
	sal_Bool		mbDeleteNotesPages;
	rtl::OUString	maCustomShowName;
	sal_Bool		mbSaveAs;
	rtl::OUString	maSaveAsURL;
	rtl::OUString	maFilterName;
	sal_Bool		mbOpenNewDocument;
	sal_Int64		mnEstimatedFileSize;
    
	OptimizerSettings() :
		mbJPEGCompression( sal_False ),
		mnJPEGQuality( 90 ),
		mbRemoveCropArea( sal_False ),
		mnImageResolution( 0 ),
		mbEmbedLinkedGraphics( sal_False ),
		mbOLEOptimization( sal_False ),
		mnOLEOptimizationType( 0 ),
		mbDeleteUnusedMasterPages( sal_False ),
		mbDeleteHiddenSlides( sal_False ),
		mbDeleteNotesPages( sal_False ),
		mbSaveAs( sal_True ),
		mbOpenNewDocument( sal_True ),
		mnEstimatedFileSize( 0 ){};
		~OptimizerSettings(){};
	
		void LoadSettingsFromConfiguration( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& rSettings );
		void SaveSettingsToConfiguration( const com::sun::star::uno::Reference< com::sun::star::container::XNameReplace >& rSettings );

		sal_Bool operator==( const OptimizerSettings& rOptimizerSettings ) const;

}; 
class ConfigurationAccess
{
	public :

		ConfigurationAccess( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
								OptimizerSettings* pDefaultSettings = NULL );
		~ConfigurationAccess();
		void SaveConfiguration();

		static rtl::OUString getString( sal_Int32 nResId );

		// access to current OptimizerSettings (stored in the first entry of maSettings)
		com::sun::star::uno::Any GetConfigProperty( const PPPOptimizerTokenEnum ) const;
		void SetConfigProperty( const PPPOptimizerTokenEnum, const com::sun::star::uno::Any& aValue );

		sal_Bool GetConfigProperty( const PPPOptimizerTokenEnum, const sal_Bool bDefault ) const;
		sal_Int16 GetConfigProperty( const PPPOptimizerTokenEnum, const sal_Int16 nDefault ) const;
		sal_Int32 GetConfigProperty( const PPPOptimizerTokenEnum, const sal_Int32 nDefault ) const;

		com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > GetConfigurationSequence();
		
		// getting access to the OptimizerSettings list
		std::vector< OptimizerSettings >& GetOptimizerSettings() { return maSettings; };
		std::vector< OptimizerSettings >::iterator GetOptimizerSettingsByName( const rtl::OUString& rName );

	private :

        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > m_xContext;
		std::vector< OptimizerSettings > maSettings;
		std::vector< OptimizerSettings > maInitialSettings;

		void LoadStrings();
		void LoadConfiguration();
		com::sun::star::uno::Reference< com::sun::star::uno::XInterface > OpenConfiguration( bool bReadOnly );
		com::sun::star::uno::Reference< com::sun::star::uno::XInterface > GetConfigurationNode(
			const com::sun::star::uno::Reference< com::sun::star::uno::XInterface >& xRoot, const rtl::OUString& sPathToNode );
};

#endif	// _CONFIGURATION_ACCESS_HXX_

