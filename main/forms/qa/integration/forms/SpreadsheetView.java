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


/*
 * SpreadsheetView.java
 *
 * Created on 2. Oktober 2003, 14:02
 */

package integration.forms;

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.frame.*;
import com.sun.star.sheet.*;
import com.sun.star.container.*;

import integration.forms.DocumentViewHelper;
import integration.forms.DocumentHelper;

/**
 *
 * @author  fs93730
 */
public class SpreadsheetView extends integration.forms.DocumentViewHelper
{
    
    /** Creates a new instance of SpreadsheetView */
    public SpreadsheetView( XMultiServiceFactory orb, DocumentHelper document, XController controller )
    {
        super( orb, document, controller );
    }

    /** activates the sheet with the given index
     */
    void activateSheet( int sheetIndex )
    {
        try
        {
            // get the sheet to activate
            XSpreadsheetDocument doc = (XSpreadsheetDocument)UnoRuntime.queryInterface(
                XSpreadsheetDocument.class, getDocument().getDocument() );
            XIndexAccess sheets = (XIndexAccess)UnoRuntime.queryInterface(
                XIndexAccess.class, doc.getSheets() );

            XSpreadsheet sheet = (XSpreadsheet)UnoRuntime.queryInterface(
                XSpreadsheet.class, sheets.getByIndex( sheetIndex ) );

            // activate
            XSpreadsheetView view = (XSpreadsheetView)UnoRuntime.queryInterface(
                XSpreadsheetView.class, getController() );
            view.setActiveSheet( sheet );
        }
        catch( com.sun.star.uno.Exception e )
        {
        }
    }
}
