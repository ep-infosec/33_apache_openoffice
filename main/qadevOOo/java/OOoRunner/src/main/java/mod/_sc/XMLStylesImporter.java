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
import com.sun.star.document.XImporter;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.Calc.XMLStylesImporter</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li><code>com::sun::star::lang::XInitialization</code></li>
 *  <li><code>com::sun::star::document::XImporter</code></li>
 *  <li><code>com::sun::star::document::XFilter</code></li>
 *  <li><code>com::sun::star::document::ImportFilter</code></li>
 *  <li><code>com::sun::star::beans::XPropertySet</code></li>
 *  <li><code>com::sun::star::xml::sax::XDocumentHandler</code></li>

 * </ul>
 * @see com.sun.star.lang.XInitialization
 * @see com.sun.star.document.XImporter
 * @see com.sun.star.document.XFilter
 * @see com.sun.star.document.ImportFilter
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.xml.sax.XDocumentHandler
 * @see ifc.lang._XInitialization
 * @see ifc.document._XImporter
 * @see ifc.document._XFilter
 * @see ifc.document._XExporter
 * @see ifc.beans._XPropertySet
 * @see ifc.xml.sax._XDocumentHandler
 */
public class XMLStylesImporter extends TestCase {
    static XSpreadsheetDocument xSheetDoc;
    static XComponent comp ;

    /**
    * New spreadsheet document created.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc( null );
            comp = (XComponent) UnoRuntime.queryInterface
                (XComponent.class, xSheetDoc) ;
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occured.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Spreadsheet document destroyed.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing document " );
        util.DesktopTools.closeDoc(comp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Calc.XMLStylesImporter</code><p>
    *
    * The calc document is set as a target document for importer.
    * Imported XML-data contains only style tags including tag
    * with new style name.
    * After import style names getting from
    * target document is checked.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XDocumentHandler.XMLData'</code> for
    *      {@link ifc.xml.sax._XDocumentHandler} interface </li>
    *  <li> <code>'XDocumentHandler.ImportChecker'</code> for
    *      {@link ifc.xml.sax._XDocumentHandler} interface </li>
    *  <li> <code>'TargetDocument'</code> for
    *      {@link ifc.document._XImporter} interface </li>
    * </ul>
    */
    public synchronized TestEnvironment createTestEnvironment( TestParameters tParam,
                                                  PrintWriter log )
                                                    throws StatusException {

        XInterface oObj = null;
        Object oInt = null ;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF() ;

        try {
            oInt = xMSF.createInstance
                ("com.sun.star.comp.Calc.XMLStylesImporter") ;
            XImporter imp = (XImporter) UnoRuntime.queryInterface
                (XImporter.class, oInt) ;
            imp.setTargetDocument(comp) ;
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException("Can't create component.", e) ;
        }

        oObj = (XInterface) oInt ;

        // create testobject here
        log.println( "creating a new environment for Paragraph object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relation
        tEnv.addObjRelation("TargetDocument", comp) ;

        final String impStyleName = "XMLStylesImporter" ;

        // adding relation for XDocumentHandler
        String[][] xml = new String[][] {
            {"start", "office:document-styles",
                "xmlns:office", "CDATA", "http://openoffice.org/2000/office",
                "xmlns:style", "CDATA", "http://openoffice.org/2000/style",
                "xmlns:fo", "CDATA", "http://www.w3.org/1999/XSL/Format",
                "xmlns:table", "CDATA", "http://openoffice.org/2000/table"},
            {"start", "office:styles"},
            {"start", "style:style",
                "style:name", "CDATA", impStyleName,
                "style:family", "CDATA", "table-cell",
                "style:parent-style-name", "CDATA", "Default"},
            { "start", "style:properties",
                "fo:width", "CDATA", "12cm",
                "fo:background-color", "CDATA", "light-grey"},
            {"end", "style:properties"},
            {"end", "style:style"},
            {"end", "office:styles"},
            {"end", "office:document-styles"}} ;

        tEnv.addObjRelation("XDocumentHandler.XMLData", xml) ;

        XNameAccess styles = null ;
        try {
            XStyleFamiliesSupplier sup = (XStyleFamiliesSupplier)
                UnoRuntime.queryInterface
                (XStyleFamiliesSupplier.class, xSheetDoc);
            XNameAccess oStyleFamilies = sup.getStyleFamilies();
            Object family = oStyleFamilies.getByName("CellStyles") ;
            styles = (XNameAccess) UnoRuntime.queryInterface
                (XNameAccess.class, family) ;
            log.println("Styles before:") ;
            String[] names = styles.getElementNames() ;
            for (int i = 0; i < names.length; i++) {
                log.println("  " + names[i]) ;
            }
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
        }
        final XNameAccess stylesF = styles ;
        final PrintWriter logF = log ;

        tEnv.addObjRelation("XDocumentHandler.ImportChecker",
            new ifc.xml.sax._XDocumentHandler.ImportChecker() {
                public boolean checkImport() {
                    logF.println("Styles after:") ;
                    String[] names = stylesF.getElementNames() ;
                    for (int i = 0; i < names.length; i++) {
                        logF.println("  " + names[i]) ;
                    }
                    return stylesF.hasByName(impStyleName) ;
                }
            }) ;

        return tEnv ;
    }
}

