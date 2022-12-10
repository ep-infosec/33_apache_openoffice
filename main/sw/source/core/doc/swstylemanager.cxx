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
#include "precompiled_sw.hxx"


#include "swstylemanager.hxx"
#include <hash_map>
#include <svl/stylepool.hxx>
#include <doc.hxx>
#include <charfmt.hxx>
#include <docary.hxx>
#include <swtypes.hxx>
#include <istyleaccess.hxx>

typedef ::std::hash_map< const ::rtl::OUString,
                         StylePool::SfxItemSet_Pointer_t,
                         ::rtl::OUStringHash,
                         ::std::equal_to< ::rtl::OUString > > SwStyleNameCache;

class SwStyleCache
{
    SwStyleNameCache mMap;
public:
    SwStyleCache() {}
    void addStyleName( StylePool::SfxItemSet_Pointer_t pStyle )
        { mMap[ StylePool::nameOf(pStyle) ] = pStyle; }
    void addCompletePool( StylePool& rPool );
	StylePool::SfxItemSet_Pointer_t getByName( const rtl::OUString& rName ) { return mMap[rName]; }
};

void SwStyleCache::addCompletePool( StylePool& rPool )
{
    IStylePoolIteratorAccess *pIter = rPool.createIterator();
    StylePool::SfxItemSet_Pointer_t pStyle = pIter->getNext();
    while( pStyle.get() )
    {
        rtl::OUString aName( StylePool::nameOf(pStyle) );
        mMap[ aName ] = pStyle;
        pStyle = pIter->getNext();
    }
    delete pIter;
}

class SwStyleManager : public IStyleAccess
{
	StylePool aAutoCharPool;
    StylePool aAutoParaPool;
    SwStyleCache *mpCharCache;
    SwStyleCache *mpParaCache;

public:
    // --> OD 2008-03-07 #refactorlists#
    // accept empty item set for ignorable paragraph items.
    SwStyleManager( SfxItemSet* pIgnorableParagraphItems )
        : aAutoCharPool(),
          aAutoParaPool( pIgnorableParagraphItems ),
          mpCharCache(0),
          mpParaCache(0)
    {}
    // <--
    virtual ~SwStyleManager();
	virtual StylePool::SfxItemSet_Pointer_t getAutomaticStyle( const SfxItemSet& rSet,
                                                               IStyleAccess::SwAutoStyleFamily eFamily );
	virtual StylePool::SfxItemSet_Pointer_t getByName( const rtl::OUString& rName,
                                                               IStyleAccess::SwAutoStyleFamily eFamily );
    virtual void getAllStyles( std::vector<StylePool::SfxItemSet_Pointer_t> &rStyles,
                                                               IStyleAccess::SwAutoStyleFamily eFamily );
	virtual StylePool::SfxItemSet_Pointer_t cacheAutomaticStyle( const SfxItemSet& rSet,
                                                               SwAutoStyleFamily eFamily );
	virtual void clearCaches();
};

IStyleAccess *createStyleManager( SfxItemSet* pIgnorableParagraphItems )
{
    return new SwStyleManager( pIgnorableParagraphItems );
}

SwStyleManager::~SwStyleManager()
{
    delete mpCharCache;
    delete mpParaCache;
}

void SwStyleManager::clearCaches()
{
    delete mpCharCache;
    mpCharCache = 0;
    delete mpParaCache;
    mpParaCache = 0;
}

StylePool::SfxItemSet_Pointer_t SwStyleManager::getAutomaticStyle( const SfxItemSet& rSet,
                                                                   IStyleAccess::SwAutoStyleFamily eFamily )
{
    StylePool& rAutoPool = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? aAutoCharPool : aAutoParaPool;
    return rAutoPool.insertItemSet( rSet );
}

StylePool::SfxItemSet_Pointer_t SwStyleManager::cacheAutomaticStyle( const SfxItemSet& rSet,
                                                                   IStyleAccess::SwAutoStyleFamily eFamily )
{
    StylePool& rAutoPool = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? aAutoCharPool : aAutoParaPool;
    StylePool::SfxItemSet_Pointer_t pStyle = rAutoPool.insertItemSet( rSet );
    SwStyleCache* &rpCache = eFamily == IStyleAccess::AUTO_STYLE_CHAR ?
                             mpCharCache : mpParaCache;
    if( !rpCache )
        rpCache = new SwStyleCache();
    rpCache->addStyleName( pStyle );
    return pStyle;
}

StylePool::SfxItemSet_Pointer_t SwStyleManager::getByName( const rtl::OUString& rName,
                                                           IStyleAccess::SwAutoStyleFamily eFamily )
{
    StylePool& rAutoPool = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? aAutoCharPool : aAutoParaPool;
    SwStyleCache* &rpCache = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? mpCharCache : mpParaCache;
    if( !rpCache )
        rpCache = new SwStyleCache();
    StylePool::SfxItemSet_Pointer_t pStyle = rpCache->getByName( rName );
    if( !pStyle.get() )
    {
        // Ok, ok, it's allowed to ask for uncached styles (from UNO) but it should not be done
        // during loading a document
        ASSERT( false, "Don't ask for uncached styles" );
        rpCache->addCompletePool( rAutoPool );
        pStyle = rpCache->getByName( rName );
    }
    return pStyle;
}

void SwStyleManager::getAllStyles( std::vector<StylePool::SfxItemSet_Pointer_t> &rStyles,
                                   IStyleAccess::SwAutoStyleFamily eFamily )
{
    StylePool& rAutoPool = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? aAutoCharPool : aAutoParaPool;
    // --> OD 2008-03-07 #refactorlists#
    // setup <StylePool> iterator, which skips unused styles and ignorable items
    IStylePoolIteratorAccess *pIter = rAutoPool.createIterator( true, true );
    // <--
    StylePool::SfxItemSet_Pointer_t pStyle = pIter->getNext();
    while( pStyle.get() )
    {
        rStyles.push_back( pStyle );

        pStyle = pIter->getNext();
    }
    delete pIter;
}
