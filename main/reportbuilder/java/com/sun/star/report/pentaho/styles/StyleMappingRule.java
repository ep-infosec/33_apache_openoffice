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


package com.sun.star.report.pentaho.styles;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 12.03.2007
 */
public class StyleMappingRule
{

    private final StyleMapperKey key;
    private final String family;
    private final boolean listOfValues;

    public StyleMappingRule(final StyleMapperKey key, final String family,
            final boolean listOfValues)
    {
        this.key = key;
        this.family = family;
        this.listOfValues = listOfValues;
    }

    public StyleMapperKey getKey()
    {
        return key;
    }

    public String getFamily()
    {
        return family;
    }

    public boolean isListOfValues()
    {
        return listOfValues;
    }
}
