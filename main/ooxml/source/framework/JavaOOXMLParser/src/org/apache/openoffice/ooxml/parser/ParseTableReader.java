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

package org.apache.openoffice.ooxml.parser;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

/** A simple reader for the parse table data that allows simple filtering on the
 *  first word in each line.
 *  
 *  Lines that only contain comments or whitespace are ignored.
 *
 */
public class ParseTableReader
{
    public ParseTableReader (final File aFile)
    {
        maSections = new HashMap<>();
        
        try
        {
            final BufferedReader aReader = new BufferedReader(new FileReader(aFile));
            
            while (true)
            {
                final String sLine = aReader.readLine();
                if (sLine == null)
                    break;
                else if (sLine.startsWith("#"))
                    continue;
                else if (sLine.isEmpty())
                    continue;
            
                final String[] aLineParts = sLine.split("\\s+");
                for (int nIndex=0; nIndex<aLineParts.length; ++nIndex)
                {
                    final String sPart = aLineParts[nIndex];
                    if (sPart.isEmpty())
                    {
                        throw new RuntimeException();
                    }
                    else if (sPart.charAt(0) == '"')
                    {
                        // Remove leading and trailing quotes, unquote spaces.
                        aLineParts[nIndex] = sPart.substring(1, sPart.length()-1).replace("%20", " ").replace("&quot;", "\"");
                    }
                }
                GetSection(aLineParts[0]).add(aLineParts);
            }
            
            aReader.close();
        } 
        catch (final Exception aException)
        {
            throw new RuntimeException(aException);
        }
    }


    
    
    public Vector<String[]> GetSection (final String sSectionName)
    {
        Vector<String[]> aSection = maSections.get(sSectionName);
        if (aSection == null)
        {
            aSection = new Vector<>();
            maSections.put(sSectionName, aSection);
        }
        return aSection;
    }

    
    

    private final Map<String,Vector<String[]>> maSections;
}
