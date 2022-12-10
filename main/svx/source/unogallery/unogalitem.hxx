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



#ifndef _SVX_UNOGALITEM_HXX 
#define _SVX_UNOGALITEM_HXX 

#include <svx/unomodel.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/gallery/XGalleryItem.hpp>
#include <comphelper/propertysethelper.hxx>
#ifndef _COMPHELPER_PROPERTYSETINFO_HXX_
#include <comphelper/propertysetinfo.hxx>
#endif

class GalleryTheme;
struct GalleryObject;
namespace unogallery { class GalleryTheme; } 

namespace unogallery {

// ---------------
// - GalleryItem -
// ---------------

class GalleryItem : public ::cppu::OWeakAggObject,
					public ::com::sun::star::lang::XServiceInfo,
					public ::com::sun::star::lang::XTypeProvider,
					public ::com::sun::star::gallery::XGalleryItem,
					public ::comphelper::PropertySetHelper
{
	friend class ::unogallery::GalleryTheme;

public:
		
			GalleryItem( ::unogallery::GalleryTheme& rTheme, const GalleryObject& rObject );
			~GalleryItem() throw();

	bool	isValid() const;
	
    static ::rtl::OUString getImplementationName_Static() throw();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >  getSupportedServiceNames_Static() throw();
	
protected:

	// XInterface
	virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL acquire() throw();
	virtual void SAL_CALL release() throw();
	
	// XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

	// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
	
	// XGalleryItem
    virtual ::sal_Int8 SAL_CALL getType(  ) throw (::com::sun::star::uno::RuntimeException);
	
	// PropertySetHelper
    virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const ::com::sun::star::uno::Any* pValues ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, ::com::sun::star::uno::Any* pValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );

protected:
	
	::comphelper::PropertySetInfo* createPropertySetInfo();
	
private:

	::unogallery::GalleryTheme* mpTheme;
	const ::GalleryObject*		mpGalleryObject;

	const ::GalleryObject*		implGetObject() const;
	void 						implSetInvalid();
		
								// not available
								GalleryItem();
								GalleryItem( const GalleryItem& );
								GalleryItem& operator=( const GalleryItem& );
};

// -----------------------
// - GalleryDrawingModel -
// -----------------------

class GalleryDrawingModel : public SvxUnoDrawingModel
{
public:

				GalleryDrawingModel( SdrModel* pDoc ) throw();
	virtual 	~GalleryDrawingModel() throw();
	
				UNO3_GETIMPLEMENTATION_DECL( GalleryDrawingModel )
};

}

#endif 
