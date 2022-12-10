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

#include "precompiled_sfx2.hxx"

#include "Context.hxx"

#define AnyApplicationName "any"
#define AnyContextName "any"

namespace sfx2 { namespace sidebar {

const sal_Int32 Context::NoMatch = 4;
const sal_Int32 Context::ApplicationWildcardMatch = 1;
const sal_Int32 Context::ContextWildcardMatch = 2;
const sal_Int32 Context::OptimalMatch = 0;  // Neither application nor context name is "any".

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

Context::Context (void)
    : msApplication(A2S(AnyApplicationName)),
      msContext(A2S(AnyContextName))
{
}




Context::Context (
    const ::rtl::OUString& rsApplication,
    const ::rtl::OUString& rsContext)
    : msApplication(rsApplication),
      msContext(rsContext)
{
}




sal_Int32 Context::EvaluateMatch (
    const Context& rOther) const
{
    const bool bApplicationNameIsAny (rOther.msApplication.equalsAscii(AnyApplicationName));
    if (rOther.msApplication.equals(msApplication) || bApplicationNameIsAny)
    {
        // Application name matches.
        const bool bContextNameIsAny (rOther.msContext.equalsAscii(AnyContextName));
        if (rOther.msContext.equals(msContext) || bContextNameIsAny)
        {
            // Context name matches.
            return (bApplicationNameIsAny ? ApplicationWildcardMatch : 0)
                + (bContextNameIsAny ? ContextWildcardMatch : 0);
        }
    }
    return NoMatch;
}




sal_Int32 Context::EvaluateMatch (const ::std::vector<Context>& rOthers) const
{
    sal_Int32 nBestMatch (NoMatch);
    
    for (::std::vector<Context>::const_iterator
             iContext(rOthers.begin()),
             iEnd(rOthers.end());
         iContext!=iEnd;
         ++iContext)
    {
        const sal_Int32 nMatch (EvaluateMatch(*iContext));
        if (nMatch < nBestMatch)
        {
            if (nMatch == OptimalMatch)
            {
                // We will find no better match so stop searching.
                return OptimalMatch;
            }
            nBestMatch = nMatch;
        }
    }
    return nBestMatch;
}




bool Context::operator== (const Context& rOther) const
{
    return msApplication.equals(rOther.msApplication)
        && msContext.equals(rOther.msContext);
}




bool Context::operator!= (const Context& rOther) const
{
    return ( ! msApplication.equals(rOther.msApplication))
        || ( ! msContext.equals(rOther.msContext));
}


} } // end of namespace sfx2::sidebar
