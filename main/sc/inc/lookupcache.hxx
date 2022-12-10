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



#ifndef INCLUDED_SC_LOOKUPCACHE_HXX
#define INCLUDED_SC_LOOKUPCACHE_HXX

#include "address.hxx"
#include "global.hxx"
#include "formula/token.hxx"
#include <svl/listener.hxx>
#include <tools/string.hxx>

#include <hash_map>

class ScDocument;


/** Lookup cache for one range used with interpreter functions such as VLOOKUP
    and MATCH. Caches query for a specific row and the resulting address looked
    up, in case other lookups of the same query in the same row are to be
    performed, which usually occur to obtain a different offset column of the
    same query.
 */

class ScLookupCache : public SvtListener
{
public:

    enum Result
    {
        NOT_CACHED,         /// Query not found in cache.
        CRITERIA_DIFFERENT, /// Different criteria for same query position exists.
        NOT_AVAILABLE,      /// Criteria not available in lookup range.
        FOUND               /// Criteria found.
    };

    enum QueryOp
    {
        UNKNOWN,
        EQUAL,
        LESS_EQUAL,
        GREATER_EQUAL
    };

    class QueryCriteria
    {
        union
        {
            double          mfVal;
            const String *  mpStr;
        };
        bool                mbAlloc  : 1;
        bool                mbString : 1;
        QueryOp             meOp     : 2;

        void deleteString()
        {
            if (mbAlloc && mbString)
                delete mpStr;
        }

        // prevent usage
        QueryCriteria();
        QueryCriteria & operator=( const QueryCriteria & r );

    public:

        explicit QueryCriteria( const ScQueryEntry & rEntry ) :
            mfVal(0.0), mbAlloc(false), mbString(false)
        {
            switch (rEntry.eOp)
            {
                case SC_EQUAL :
                    meOp = EQUAL;
                    break;
                case SC_LESS_EQUAL :
                    meOp = LESS_EQUAL;
                    break;
                case SC_GREATER_EQUAL :
                    meOp = GREATER_EQUAL;
                    break;
                default:
                    meOp = UNKNOWN;
                    DBG_ERRORFILE( "ScLookupCache::QueryCriteria not prepared for this ScQueryOp");
            }
            if (rEntry.bQueryByString)
                setString( rEntry.pStr);
            else
                setDouble( rEntry.nVal);
        }
        QueryCriteria( const QueryCriteria & r ) :
            mfVal( r.mfVal),
            mbAlloc( false),
            mbString( false),
            meOp( r.meOp)
        {
            if (r.mbString && r.mpStr)
            {
                mpStr = new String( *r.mpStr);
                mbAlloc = mbString = true;
            }
        }
        ~QueryCriteria()
        {
            deleteString();
        }

        QueryOp getQueryOp() const { return meOp; }

        void setDouble( double fVal )
        {
            deleteString();
            mbAlloc = mbString = false;
            mfVal = fVal;
        }

        void setString( const String * pStr )
        {
            deleteString();
            mbAlloc = false;
            mbString = true;
            mpStr = pStr;
        }

        void setString( const String & rStr )
        {
            deleteString();
            mbAlloc = mbString = true;
            mpStr = new String( rStr);
        }

        bool operator==( const QueryCriteria & r ) const
        {
            return meOp == r.meOp && mbString == r.mbString &&
                (mbString ? (*mpStr == *r.mpStr) : (mfVal == r.mfVal));
        }

    };

    /// MUST be new'd because Notify() deletes.
                            ScLookupCache( ScDocument * pDoc, const ScRange & rRange );
    virtual                 ~ScLookupCache();
    /// Remove from document structure and delete (!) cache on modify hint.
    virtual void            Notify( SvtBroadcaster & rBC, const SfxHint &  rHint );

    /// @returns document address in o_rAddress if Result==FOUND
            Result          lookup( ScAddress & o_rResultAddress,
                                    const QueryCriteria & rCriteria,
                                    const ScAddress & rQueryAddress ) const;

    /** Insert query and result.
        @param bAvailable
            Pass sal_False if the search didn't deliver a result. A subsequent
            lookup() then will return Result::NOT_AVAILABLE.
        @returns successful insertion.
      */
            bool            insert( const ScAddress & rResultAddress,
                                    const QueryCriteria & rCriteria,
                                    const ScAddress & rQueryAddress,
                                    const bool bAvailable );

    inline  const ScRange&  getRange() const { return maRange; }

    struct Hash
    {
        size_t operator()( const ScRange & rRange ) const
        {
            // Lookups are performed on the first column.
            return rRange.hashStartColumn();
        }
    };

private:

    struct QueryKey
    {
        SCROW           mnRow;
        SCTAB           mnTab;
        QueryOp         meOp : 2;

        QueryKey( const ScAddress & rAddress, const QueryOp eOp ) :
            mnRow( rAddress.Row()),
            mnTab( rAddress.Tab()),
            meOp( eOp)
        {
        }

        bool operator==( const QueryKey & r ) const
        {
            return mnRow == r.mnRow && mnTab == r.mnTab && meOp == r.meOp && meOp != UNKNOWN;
        }

        struct Hash
        {
            size_t operator()( const QueryKey & r ) const
            {
                return (static_cast<size_t>(r.mnTab) << 24) ^
                    (static_cast<size_t>(r.meOp) << 22) ^
                    static_cast<size_t>(r.mnRow);
            }
        };
    };

    struct QueryCriteriaAndResult
    {
        QueryCriteria   maCriteria;
        ScAddress       maAddress;

        QueryCriteriaAndResult( const QueryCriteria & rCriteria, const ScAddress & rAddress ) :
            maCriteria( rCriteria),
            maAddress( rAddress)
        {
        }
        ~QueryCriteriaAndResult()
        {
        }
    };

    typedef ::std::hash_map< QueryKey, QueryCriteriaAndResult, QueryKey::Hash, ::std::equal_to< QueryKey > > QueryMap;
    QueryMap        maQueryMap;
    ScRange         maRange;
    ScDocument *    mpDoc;

    // prevent usage
    ScLookupCache( const ScLookupCache & );
    ScLookupCache & operator=( const ScLookupCache & );

};


typedef ::std::hash_map< ScRange, ScLookupCache*, ScLookupCache::Hash, ::std::equal_to< ScRange > > ScLookupCacheMap;

#endif
