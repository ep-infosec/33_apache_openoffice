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


#ifndef INCLUDED_DOMAIN_MAPPER_TABLE_HANDLER_HXX
#define INCLUDED_DOMAIN_MAPPER_TABLE_HANDLER_HXX

#include <resourcemodel/TableManager.hxx>
#include <PropertyMap.hxx>

#include <com/sun/star/text/XTextAppendAndConvert.hpp>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;

typedef text::XTextRange TextRange_t;
typedef uno::Reference< TextRange_t > Handle_t;
typedef uno::Sequence< Handle_t> CellSequence_t;
typedef boost::shared_ptr<CellSequence_t> CellSequencePointer_t;
typedef uno::Sequence< CellSequence_t > RowSequence_t;
typedef boost::shared_ptr<RowSequence_t> RowSequencePointer_t;
typedef uno::Sequence< RowSequence_t> TableSequence_t;
typedef boost::shared_ptr<TableSequence_t> TableSequencePointer_t;
typedef text::XTextAppendAndConvert Text_t;
typedef uno::Reference<Text_t> TextReference_t;

typedef beans::PropertyValues                     TablePropertyValues_t;
typedef uno::Sequence< TablePropertyValues_t >    RowPropertyValuesSeq_t;
typedef uno::Sequence< RowPropertyValuesSeq_t>    CellPropertyValuesSeq_t;

typedef std::vector<PropertyMapPtr>     PropertyMapVector1;
typedef std::vector<PropertyMapVector1> PropertyMapVector2;

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
XMLTag::Pointer_t handleToTag(Handle_t & rHandle);
void cellSequenceToTag(XMLTag::Pointer_t pTag, CellSequence_t & pCellSequence);
void rowSequenceToTag(XMLTag::Pointer_t pTag, RowSequence_t & pRowSequence);
XMLTag::Pointer_t tableSequenceToTag(TableSequence_t & rTableSequence);
#endif

class DomainMapper_Impl;
class TableStyleSheetEntry; 
struct TableInfo;   
class DomainMapperTableHandler : public TableDataHandler<Handle_t , TablePropertyMapPtr >
{
public:
    typedef boost::shared_ptr<DomainMapperTableHandler> Pointer_t;

    DomainMapperTableHandler(TextReference_t xText, DomainMapper_Impl& rDMapper_Impl);
    virtual ~DomainMapperTableHandler();

    virtual void startTable(
        unsigned int nRows,
        unsigned int nDepth,
        TablePropertyMapPtr pProps );

    virtual void endTable(
        const unsigned int nDepth );

    virtual void startRow(
        unsigned int nCells,
        TablePropertyMapPtr pProps );

    virtual void endRow();

    virtual void startCell(
        const Handle_t & start,
        TablePropertyMapPtr pProps );

    virtual void endCell( const Handle_t & end );

    virtual Handle_t* getTable( )
    {
        return &m_xTableRange;
    };

private:
    TextReference_t         m_xText;
    DomainMapper_Impl&      m_rDMapper_Impl;
    CellSequencePointer_t   m_pCellSeq;
    RowSequencePointer_t    m_pRowSeq;
    TableSequencePointer_t  m_pTableSeq;

    Handle_t               m_xTableRange;

    // properties
    PropertyMapVector2      m_aCellProperties;
    PropertyMapVector1      m_aRowProperties;
    TablePropertyMapPtr     m_aTableProperties;
    
    sal_Int32 m_nCellIndex;
    sal_Int32 m_nRowIndex;

    TableStyleSheetEntry * endTableGetTableStyle(
        TableInfo & rInfo,
        const bool bAdjustLeftMarginByDefaultValue );

    CellPropertyValuesSeq_t endTableGetCellProperties(TableInfo & rInfo);

    RowPropertyValuesSeq_t endTableGetRowProperties();
};

}}

#endif // INCLUDED_DOMAIN_MAPPER_TABLE_HANDLER_HXX
