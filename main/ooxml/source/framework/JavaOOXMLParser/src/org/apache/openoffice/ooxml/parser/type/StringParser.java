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

package org.apache.openoffice.ooxml.parser.type;

import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

import org.apache.openoffice.ooxml.parser.NameMap;

public class StringParser
    implements ISimpleTypeParser
{

    public StringParser (final String[] aLine)
    {
        switch(aLine[5])
        {
            case "E":
                meRestrictionType = RestrictionType.Enumeration;
                maEnumeration = new HashSet<>();
                for (int nIndex=6; nIndex<aLine.length; ++nIndex)
                    maEnumeration.add(Integer.parseInt(aLine[nIndex]));
                break;
                
            case "P":
                meRestrictionType = RestrictionType.Pattern;
                maPattern = Pattern.compile(aLine[6].replace("\\p{Is", "\\p{In"));
                break;
                
            case "L":
                meRestrictionType = RestrictionType.Length;
                mnMinimumLength = Integer.parseInt(aLine[6]);
                mnMaximumLength = Integer.parseInt(aLine[7]);
                break;
                
            case "N":
                meRestrictionType = RestrictionType.None;
                break;

            default:
                throw new RuntimeException();
        }
    }

    
    
    
    @Override
    public Object Parse (
        final String sRawValue,
        final NameMap aAttributeValueMap)
    {
        switch(meRestrictionType)
        {
            case Enumeration:
                final int nId = aAttributeValueMap.GetIdForOptionalName(sRawValue);
                if ( ! maEnumeration.contains(nId))
                    return null;//throw new RuntimeException("value is not part of enumeration");
                else
                    return nId;

            case Pattern:
                if ( ! maPattern.matcher(sRawValue).matches())
                    return null;//throw new RuntimeException("value does not match pattern");
                else
                    return sRawValue;

            case Length:
                if (sRawValue.length()<mnMinimumLength || sRawValue.length()>mnMaximumLength)
                    return null;/*throw new RuntimeException(
                        String.format("value violates string length restriction: %s is not inside [%d,%d]",
                            sRawValue.length(),
                            mnMinimumLength,
                            mnMaximumLength));
                            */
                else
                    return sRawValue;
                
            case None:
                return sRawValue;

            default:
                throw new RuntimeException();
        }
    }
    
    
    
    
    enum RestrictionType
    {
        Enumeration,
        Pattern,
        Length,
        None
    }
    private final RestrictionType meRestrictionType;
    private Set<Integer> maEnumeration;
    private Pattern maPattern;
    private int mnMinimumLength;
    private int mnMaximumLength;
}
