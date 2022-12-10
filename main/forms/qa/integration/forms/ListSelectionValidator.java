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



package integration.forms;

/**
 *
 * @author  fs@openoffice.org
 */
public class ListSelectionValidator extends integration.forms.ControlValidator
{
    /** Creates a new instance of ListSelectionValidator */
    public ListSelectionValidator()
    {
    }
    
    public String explainInvalid( Object Value )
    {
        try
        {
            short[] selectionIndexes = (short[])Value;
            if ( selectionIndexes.length > 2 )
                return "please 2 entries, at most";
        }
        catch( java.lang.Exception e )
        {
            return "oops. What's this?";
        }
        return "";
    }
    
    public boolean isValid( Object Value )
    {
        try
        {
            short[] selectionIndexes = (short[])Value;
            if ( selectionIndexes.length > 2 )
                return false;
            return true;
        }
        catch( java.lang.Exception e )
        {
        }
        return false;
    }
    
}
