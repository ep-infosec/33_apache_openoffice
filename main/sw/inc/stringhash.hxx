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


#ifndef _STRINGHASH_HXX
#define _STRINGHASH_HXX

#include <tools/string.hxx>

struct StringEq
{
	sal_Bool operator() ( const String *r1,
					  	  const String *r2) const
	{
		return r1->Equals(*r2);
	}
};

struct StringEqRef
{
    sal_Bool operator() (const String &r1, const String &r2) const
    {
        return r1.Equals(r2);
    }
};

struct StringHash
{
	size_t operator() ( const String *rString) const
	{
		sal_Int32 h, nLen; 
		h = nLen = rString->Len();
		const sal_Unicode *pStr = rString->GetBuffer();

		if ( nLen < 16 )
			while ( nLen-- > 0 )
				h = (h*37) + *(pStr++);
		else
		{
			sal_Int32               nSkip;
			const sal_Unicode* pEndStr = pStr+nLen-5;

			/* only sample some characters */
			/* the first 3, some characters between, and the last 5 */
			h = (h*39) + *(pStr++);
			h = (h*39) + *(pStr++);
			h = (h*39) + *(pStr++);

			nSkip = nLen / nLen < 32 ? 4 : 8;
			nLen -= 8;
			while ( nLen > 0 )
			{
				h = (h*39) + ( *pStr );
				pStr += nSkip;
				nLen -= nSkip;
			}

			h = (h*39) + *(pEndStr++);
			h = (h*39) + *(pEndStr++);
			h = (h*39) + *(pEndStr++);
			h = (h*39) + *(pEndStr++);
			h = (h*39) + *(pEndStr++);
		}
		return h;
	}

    size_t operator() (const String & rStr) const
    {
        return (*this)(&rStr);
    }
};

struct StringHashRef
{
    size_t operator () (const String &rStr) const 
    { 
        StringHash aStrHash;

        return aStrHash(&rStr); 
    }
};
#endif // _STRINGHASH_HXX
