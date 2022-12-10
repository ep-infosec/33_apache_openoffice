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



#include "oox/drawingml/chart/datasourcecontext.hxx"

#include "oox/drawingml/chart/datasourcemodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;
using ::rtl::OUString;

// ============================================================================

DoubleSequenceContext::DoubleSequenceContext( ContextHandler2Helper& rParent, DataSequenceModel& rModel ) :
    DataSequenceContextBase( rParent, rModel ),
    mnPtIndex( -1 )
{
}

DoubleSequenceContext::~DoubleSequenceContext()
{
}

ContextHandlerRef DoubleSequenceContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( numRef ):
            switch( nElement )
            {
                case C_TOKEN( f ):
                case C_TOKEN( numCache ):
                    return this;
            }
        break;

        case C_TOKEN( numCache ):
        case C_TOKEN( numLit ):
            switch( nElement )
            {
                case C_TOKEN( formatCode ):
                    return this;
                case C_TOKEN( ptCount ):
                    mrModel.mnPointCount = rAttribs.getInteger( XML_val, -1 );
                    return 0;
                case C_TOKEN( pt ):
                    mnPtIndex = rAttribs.getInteger( XML_idx, -1 );
                    return this;
            }
        break;

        case C_TOKEN( pt ):
            switch( nElement )
            {
                case C_TOKEN( v ):
                    return this;
            }
        break;
    }
    return 0;
}

void DoubleSequenceContext::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( f ):
            mrModel.maFormula = rChars;
        break;
        case C_TOKEN( formatCode ):
            mrModel.maFormatCode = rChars;
        break;
        case C_TOKEN( v ):
            if( mnPtIndex >= 0 )
                mrModel.maData[ mnPtIndex ] <<= rChars.toDouble();
        break;
    }
}

// ============================================================================

StringSequenceContext::StringSequenceContext( ContextHandler2Helper& rParent, DataSequenceModel& rModel ) :
    DataSequenceContextBase( rParent, rModel )
{
}

StringSequenceContext::~StringSequenceContext()
{
}

ContextHandlerRef StringSequenceContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( multiLvlStrRef ):
            switch( nElement )
            {
                case C_TOKEN( f ):
                    return this;
            }
        break;

        case C_TOKEN( strRef ):
            switch( nElement )
            {
                case C_TOKEN( f ):
                case C_TOKEN( strCache ):
                    return this;
            }
        break;

        case C_TOKEN( strCache ):
        case C_TOKEN( strLit ):
            switch( nElement )
            {
                case C_TOKEN( ptCount ):
                    mrModel.mnPointCount = rAttribs.getInteger( XML_val, -1 );
                    return 0;
                case C_TOKEN( pt ):
                    mnPtIndex = rAttribs.getInteger( XML_idx, -1 );
                    return this;
            }
        break;

        case C_TOKEN( pt ):
            switch( nElement )
            {
                case C_TOKEN( v ):
                    return this;
            }
        break;
    }
    return 0;
}

void StringSequenceContext::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( f ):
            mrModel.maFormula = rChars;
        break;
        case C_TOKEN( v ):
            if( mnPtIndex >= 0 )
                mrModel.maData[ mnPtIndex ] <<= rChars;
        break;
    }
}

// ============================================================================

DataSourceContext::DataSourceContext( ContextHandler2Helper& rParent, DataSourceModel& rModel ) :
    ContextBase< DataSourceModel >( rParent, rModel )
{
}

DataSourceContext::~DataSourceContext()
{
}

ContextHandlerRef DataSourceContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( cat ):
        case C_TOKEN( xVal ):
            switch( nElement )
            {
                case C_TOKEN( multiLvlStrRef ):
                case C_TOKEN( strLit ):
                case C_TOKEN( strRef ):
                    OSL_ENSURE( !mrModel.mxDataSeq, "DataSourceContext::onCreateContext - multiple data sequences" );
                    return new StringSequenceContext( *this, mrModel.mxDataSeq.create() );

                case C_TOKEN( numLit ):
                case C_TOKEN( numRef ):
                    OSL_ENSURE( !mrModel.mxDataSeq, "DataSourceContext::onCreateContext - multiple data sequences" );
                    return new DoubleSequenceContext( *this, mrModel.mxDataSeq.create() );
            }
        break;

        case C_TOKEN( plus ):
        case C_TOKEN( minus ):
        case C_TOKEN( val ):
        case C_TOKEN( yVal ):
        case C_TOKEN( bubbleSize ):
            switch( nElement )
            {
                case C_TOKEN( numLit ):
                case C_TOKEN( numRef ):
                    OSL_ENSURE( !mrModel.mxDataSeq, "DataSourceContext::onCreateContext - multiple data sequences" );
                    return new DoubleSequenceContext( *this, mrModel.mxDataSeq.create() );
            }
        break;
    }
    return 0;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox
