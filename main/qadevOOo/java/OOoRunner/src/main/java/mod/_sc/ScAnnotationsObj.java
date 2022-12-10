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



package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSheetAnnotation;
import com.sun.star.sheet.XSheetAnnotationAnchor;
import com.sun.star.sheet.XSheetAnnotationsSupplier;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XSimpleText;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which represents a collection of annotations
* for a spreadsheet document (implements
* <code>com.sun.star.sheet.CellAnnotations</code>). <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::sheet::XSheetAnnotations</code></li>
* </ul> <p>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.sheet.CellAnnotations
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.sheet.XSheetAnnotations
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.sheet._XSheetAnnotations
*/
public class ScAnnotationsObj extends TestCase {
    static XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occured.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * From a document collection of spreadsheets a single one is
    * retrieved and one annotation is added to it. Then a collection
    * of annotations is retrieved using spreadsheet's
    * <code>com.sun.star.sheet.XSheetAnnotationsSupplier</code> interface.
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        log.println("Getting test object ") ;

        XSpreadsheetDocument xSpreadsheetDoc = (XSpreadsheetDocument)
            UnoRuntime.queryInterface(XSpreadsheetDocument.class, xSheetDoc);
        XSpreadsheets sheets = (XSpreadsheets) xSpreadsheetDoc.getSheets();

        XNameAccess oNames = (XNameAccess)
            UnoRuntime.queryInterface( XNameAccess.class, sheets );
        XCell oCell = null;
        XSpreadsheet oSheet  = null;
        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),
                        oNames.getByName(oNames.getElementNames()[0]));
            // adding an annotation...
            XCellRange oCRange = (XCellRange)
                UnoRuntime.queryInterface(XCellRange.class, oSheet);
            oCell = oCRange.getCellByPosition(10,10);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Error getting test object from spreadsheet document",e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting test object from spreadsheet document",e) ;
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting test object from spreadsheet document",e) ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting test object from spreadsheet document",e) ;
        }

        XSheetAnnotationAnchor oAnnoA = (XSheetAnnotationAnchor)
            UnoRuntime.queryInterface(XSheetAnnotationAnchor.class, oCell);
        XSheetAnnotation oAnno = oAnnoA.getAnnotation();
        XSimpleText sText = ((XSimpleText)
            UnoRuntime.queryInterface(XSimpleText.class, oAnno));
        sText.setString("ScAnnotationsObj");

        XSheetAnnotationsSupplier supp = (XSheetAnnotationsSupplier)
            UnoRuntime.queryInterface(
                XSheetAnnotationsSupplier.class, oSheet);
        oObj = supp.getAnnotations();

        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    }

}    // finish class ScAnnotationsObj


