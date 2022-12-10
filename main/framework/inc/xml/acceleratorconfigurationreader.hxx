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



#ifndef _FRAMEWORK_XML_ACCELERATORCONFIGURATIONREADER_HXX_
#define _FRAMEWORK_XML_ACCELERATORCONFIGURATIONREADER_HXX_

//_______________________________________________
// own includes

#include <accelerators/acceleratorcache.hxx>
#include <accelerators/keymapping.hxx>
#include <macros/xinterface.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <general.h>

//_______________________________________________
// interface includes

#ifndef __COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef __COM_SUN_STAR_XML_SAX_XLOCATOR_HPP_
#include <com/sun/star/xml/sax/XLocator.hpp>
#endif

//_______________________________________________
// other includes
#include <salhelper/singletonref.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

namespace framework{

class AcceleratorConfigurationReader : public  css::xml::sax::XDocumentHandler
                                     , private ThreadHelpBase
                                     , public  ::cppu::OWeakObject
{
    //-------------------------------------------
    // const, types
    
    private:
    
        //---------------------------------------
        /** @short  classification of XML elements. */
		enum EXMLElement
		{
			E_ELEMENT_ACCELERATORLIST,
			E_ELEMENT_ITEM
        };
        
        //---------------------------------------
        /** @short  classification of XML attributes. */
        enum EXMLAttribute
        {
			E_ATTRIBUTE_KEYCODE,
			E_ATTRIBUTE_MOD_SHIFT,
			E_ATTRIBUTE_MOD_MOD1,
			E_ATTRIBUTE_MOD_MOD2,
                        E_ATTRIBUTE_MOD_MOD3,
			E_ATTRIBUTE_URL
		};
        
        //---------------------------------------
        /** @short  some namespace defines */
		enum EAcceleratorXMLNamespace
		{
			E_NAMESPACE_ACCEL,
			E_NAMESPACE_XLINK
		};
        
    //-------------------------------------------
    // member
    
    private:
    
        //---------------------------------------
        /** @short  needed to read the xml configuration. */
        css::uno::Reference< css::xml::sax::XDocumentHandler > m_xReader;
        
        //---------------------------------------
        /** @short  reference to the outside container, where this
                    reader/writer must work on. */
        AcceleratorCache& m_rContainer;
    
        //---------------------------------------
        /** @short  used to detect if an accelerator list
                    occurs recursive inside xml. */
		sal_Bool m_bInsideAcceleratorList;
        
        //---------------------------------------
        /** @short  used to detect if an accelerator item
                    occurs recursive inside xml. */
		sal_Bool m_bInsideAcceleratorItem;
        
        //---------------------------------------
        /** @short  is used to map key codes to its
                    string representation.
                    
            @descr  To perform this operatio is
                    created only one times and holded
                    alive forever ...*/
        ::salhelper::SingletonRef< KeyMapping > m_rKeyMapping;
        
        //---------------------------------------
        /** @short  provide informations abou the parsing state.
        
            @descr  We use it to find out the line and column, where
                    an error occur.
          */
		css::uno::Reference< css::xml::sax::XLocator > m_xLocator;
        
/*        SfxAcceleratorItemList& m_aReadAcceleratorList;
*/        
        
    //-------------------------------------------
    // interface
    
	public:

        //---------------------------------------
        /** @short  connect this new reader/writer instance
                    to an outside container, which should be used
                    flushed to the underlying XML configuration or
                    filled from there.
        
            @param  rContainer
                    a reference to the outside container.            
          */
        AcceleratorConfigurationReader(AcceleratorCache& rContainer);
        
        //---------------------------------------
        /** @short  does nothing real ... */
		virtual ~AcceleratorConfigurationReader();

        //---------------------------------------
        // XInterface
        FWK_DECLARE_XINTERFACE

        //---------------------------------------
		// XDocumentHandler
		virtual void SAL_CALL startDocument()
			throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );
            
		virtual void SAL_CALL endDocument()
			throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );
                  
		virtual void SAL_CALL startElement(const ::rtl::OUString&                                      sElement      ,
			                               const css::uno::Reference< css::xml::sax::XAttributeList >& xAttributeList)
			throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );
                  
		virtual void SAL_CALL endElement(const ::rtl::OUString& sElement)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );
                  
		virtual void SAL_CALL characters(const ::rtl::OUString& sChars)
			throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );
                  
		virtual void SAL_CALL ignorableWhitespace(const ::rtl::OUString& sWhitespaces)
			throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );
                  
		virtual void SAL_CALL processingInstruction(const ::rtl::OUString& sTarget,
                                                    const ::rtl::OUString& sData  )
			throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );
                  
		virtual void SAL_CALL setDocumentLocator(const css::uno::Reference< css::xml::sax::XLocator >& xLocator)
			throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

    //-------------------------------------------
    // helper
    
    private:
    
        //---------------------------------------
        /** TODO document me */
		static EXMLElement implst_classifyElement(const ::rtl::OUString& sElement);
        
        //---------------------------------------
        /** TODO document me */
		static EXMLAttribute implst_classifyAttribute(const ::rtl::OUString& sAttribute);
        
        //---------------------------------------
        /** TODO document me */
		::rtl::OUString implts_getErrorLineString();
};

} // namespace framework

#endif // _FRAMEWORK_XML_ACCELERATORCONFIGURATIONREADER_HXX_
