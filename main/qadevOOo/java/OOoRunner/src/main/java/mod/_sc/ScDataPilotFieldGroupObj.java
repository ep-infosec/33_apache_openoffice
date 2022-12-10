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

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.DataPilotFieldGroupInfo;
import com.sun.star.sheet.XDataPilotDescriptor;
import com.sun.star.sheet.XDataPilotFieldGrouping;
import com.sun.star.sheet.XDataPilotTables;
import com.sun.star.sheet.XDataPilotTablesSupplier;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;



public class ScDataPilotFieldGroupObj extends TestCase
{
    static XSpreadsheetDocument xSheetDoc = null;
    
    /**
     * A field is filled some values. This integer determines the size of the
     * field in x and y direction.
     */
    private int mMaxFieldIndex = 6;
    
    /**
     * Creates Spreadsheet document.
     */
    protected void initialize (TestParameters tParam, PrintWriter log)
    {
        SOfficeFactory SOF = SOfficeFactory.getFactory (
            (XMultiServiceFactory) tParam.getMSF ());
        
        try
        {
            log.println ("creating a Spreadsheet document");
            xSheetDoc = SOF.createCalcDoc (null);
        }
        catch (com.sun.star.uno.Exception e)
        {
            // Some exception occured.FAILED
            e.printStackTrace (log);
            throw new StatusException ("Couldn't create document", e);
        }
    }
    
    /**
     * Disposes Spreadsheet document.
     */
    protected void cleanup (TestParameters tParam, PrintWriter log)
    {
        log.println ("    disposing xSheetDoc ");
        
        XComponent oComp = (XComponent) UnoRuntime.queryInterface (
            XComponent.class, xSheetDoc);
        util.DesktopTools.closeDoc (oComp);
    }
    
    protected synchronized TestEnvironment createTestEnvironment (TestParameters Param,
        PrintWriter log)
    {
        XInterface oObj = null;
        XInterface datapilotfield = null;
        XInterface groups = null;
        
        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println ("Creating a test environment");
        
        // the cell range
        CellRangeAddress sCellRangeAdress = new CellRangeAddress ();
        sCellRangeAdress.Sheet = 0;
        sCellRangeAdress.StartColumn = 1;
        sCellRangeAdress.StartRow = 0;
        sCellRangeAdress.EndColumn = mMaxFieldIndex - 1;
        sCellRangeAdress.EndRow = mMaxFieldIndex - 1;
        
        // position of the data pilot table
        CellAddress sCellAdress = new CellAddress ();
        sCellAdress.Sheet = 0;
        sCellAdress.Column = 7;
        sCellAdress.Row = 8;
        
        log.println ("Getting a sheet");
        
        XSpreadsheets xSpreadsheets = (XSpreadsheets) xSheetDoc.getSheets ();
        XSpreadsheet oSheet = null;
        XSpreadsheet oSheet2 = null;
        XIndexAccess oIndexAccess = (XIndexAccess) UnoRuntime.queryInterface (
            XIndexAccess.class, xSpreadsheets);
        
        try
        {
            oSheet = (XSpreadsheet) AnyConverter.toObject (
                new Type (XSpreadsheet.class),
                oIndexAccess.getByIndex (0));
            oSheet2 = (XSpreadsheet) AnyConverter.toObject (
                new Type (XSpreadsheet.class),
                oIndexAccess.getByIndex (1));
        }
        catch (com.sun.star.lang.WrappedTargetException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't get a spreadsheet", e);
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't get a spreadsheet", e);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't get a spreadsheet", e);
        }
        
        try
        {
            log.println ("Filling a table");
            
            for (int i = 1; i < mMaxFieldIndex; i++)
            {
                oSheet.getCellByPosition (i, 0).setFormula ("Col" + i);
                oSheet.getCellByPosition (0, i).setFormula ("Row" + i);
                oSheet2.getCellByPosition (i, 0).setFormula ("Col" + i);
                oSheet2.getCellByPosition (0, i).setFormula ("Row" + i);
            }
            
            for (int i = 1; i < mMaxFieldIndex; i++)
            {
                for (int j = 1; j < mMaxFieldIndex; j++)
                {
                    oSheet.getCellByPosition (i, j).setValue (i * (j + 1));
                    oSheet2.getCellByPosition (i, j).setValue (i * (j + 2));
                }
            }
            
            oSheet.getCellByPosition (1, 1).setFormula ("aName");
            oSheet.getCellByPosition (1, 2).setFormula ("otherName");
            oSheet.getCellByPosition (1, 3).setFormula ("una");
            oSheet.getCellByPosition (1, 4).setFormula ("otherName");
            oSheet.getCellByPosition (1, 5).setFormula ("somethingelse");
            
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't fill some cells", e);
        }
        
        // change a value of a cell and check the change in the data pilot
        // (for the XDataPilotTable.refresh() test)
        Object oChangeCell = null;
        Object oCheckCell = null;
        Integer aChangeValue = null;
        
        try
        {
            // cell of data
            oChangeCell = oSheet.getCellByPosition (1, 5);
            
            int x = sCellAdress.Column;
            int y = sCellAdress.Row + 3;
            
            
            // cell of the data pilot output
            oCheckCell = oSheet.getCellByPosition (x, y);
            aChangeValue = new Integer (27);
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't get cells for changeing.", e);
        }
        
        
        // create the test objects
        log.println ("Getting test objects");
        
        XDataPilotTablesSupplier DPTS = (XDataPilotTablesSupplier) UnoRuntime.queryInterface (
            XDataPilotTablesSupplier.class,
            oSheet);
        XDataPilotTables DPT = DPTS.getDataPilotTables ();
        XDataPilotDescriptor DPDsc = DPT.createDataPilotDescriptor ();
        DPDsc.setSourceRange (sCellRangeAdress);
        
        XPropertySet fieldPropSet = null;        
        
        try
        {
            Object oDataPilotField = DPDsc.getDataPilotFields ().getByIndex (0);
            fieldPropSet = (XPropertySet) UnoRuntime.queryInterface (
                XPropertySet.class, oDataPilotField);
            fieldPropSet.setPropertyValue ("Orientation",
                com.sun.star.sheet.DataPilotFieldOrientation.ROW);
            oDataPilotField = DPDsc.getDataPilotFields ().getByIndex (1);
            fieldPropSet = (XPropertySet) UnoRuntime.queryInterface (
                XPropertySet.class, oDataPilotField);
            fieldPropSet.setPropertyValue ("Function",
                com.sun.star.sheet.GeneralFunction.SUM);
            fieldPropSet.setPropertyValue ("Orientation",
                com.sun.star.sheet.DataPilotFieldOrientation.DATA);
            oDataPilotField = DPDsc.getDataPilotFields ().getByIndex (2);
            fieldPropSet = (XPropertySet) UnoRuntime.queryInterface (
                XPropertySet.class, oDataPilotField);
            fieldPropSet.setPropertyValue ("Orientation",
                com.sun.star.sheet.DataPilotFieldOrientation.COLUMN);
        }
        catch (com.sun.star.lang.WrappedTargetException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't create a test environment", e);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't create a test environment", e);
        }
        catch (com.sun.star.beans.PropertyVetoException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't create a test environment", e);
        }
        catch (com.sun.star.beans.UnknownPropertyException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't create a test environment", e);
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't create a test environment", e);
        }
        
        log.println ("Insert the DataPilotTable");
        
        if (DPT.hasByName ("DataPilotTable"))
        {
            DPT.removeByName ("DataPilotTable");
        }
        
        DPT.insertNewByName ("DataPilotTable", sCellAdress, DPDsc);
        XIndexAccess xIA = (XIndexAccess) UnoRuntime.queryInterface (XIndexAccess.class,DPTS.getDataPilotTables ());
        XIndexAccess IA = null;
        try
        {
            XDataPilotDescriptor xDPT = (XDataPilotDescriptor) UnoRuntime.queryInterface (XDataPilotDescriptor.class,xIA.getByIndex (0));
            IA = xDPT.getRowFields ();
            //getSRange(IA);
            System.out.println ("COUNT: "+IA.getCount ());
            datapilotfield = (XInterface) AnyConverter.toObject (
                new Type (XInterface.class), IA.getByIndex (0));
        }
        catch (com.sun.star.lang.WrappedTargetException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't get data pilot field", e);
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't get data pilot field", e);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't get data pilot field", e);
        }
        
        try
        {
            XDataPilotFieldGrouping  dpfg = (XDataPilotFieldGrouping) UnoRuntime.queryInterface (XDataPilotFieldGrouping.class, datapilotfield);
            String[] elements = new String[]{"aName","otherName"};
            dpfg.createNameGroup (elements);
            DataPilotFieldGroupInfo dpgi=null;
            xIA = (XIndexAccess) UnoRuntime.queryInterface (XIndexAccess.class,DPTS.getDataPilotTables ());
        try
        {
            XDataPilotDescriptor xDPT = (XDataPilotDescriptor) UnoRuntime.queryInterface (XDataPilotDescriptor.class,xIA.getByIndex (0));
            IA = xDPT.getRowFields ();
        }
        catch (com.sun.star.lang.WrappedTargetException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't get data pilot field", e);
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            e.printStackTrace ();
            throw new StatusException ("Couldn't get data pilot field", e);
        }       
            for (int i=0;i<IA.getCount ();i++)
            {
                datapilotfield = (XInterface) AnyConverter.toObject (
                    new Type (XInterface.class), IA.getByIndex (i));
                XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface (XPropertySet.class, IA.getByIndex (i));
                if (((Boolean)xPropertySet.getPropertyValue ("IsGroupField")).booleanValue ())
                {
                    xPropertySet = (XPropertySet) UnoRuntime.queryInterface (XPropertySet.class, datapilotfield);
                    XNamed xNamed = (XNamed) UnoRuntime.queryInterface (XNamed.class, IA.getByIndex (i));
                    System.out.println ("name: "+xNamed.getName ());
                    dpgi = (DataPilotFieldGroupInfo) xPropertySet.getPropertyValue ("GroupInfo");
                }
            }
            groups = dpgi.Groups;
            XIndexAccess groupAccess = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, groups);
            oObj = (XInterface) UnoRuntime.queryInterface(XInterface.class, groupAccess.getByIndex(0));
        }
        catch (Exception e)
        {
            e.printStackTrace ();
        }
        
        log.println ("Creating object - " +
            ((oObj == null) ? "FAILED" : "OK"));
        
        TestEnvironment tEnv = new TestEnvironment (oObj);
        
        log.println ("Implementationname: " + util.utils.getImplName (oObj));
        
        // Other parameters required for interface tests
        return tEnv;
    }
    
}
