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
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.TableAutoFormatField</code>.
* In StarCalc application there is a collection of autoformats
* for tables (you can select a predefined format for a
* table or create your own). Each autoformat has a number
* of fields with definitions of font parameters, number
* formats etc. for different parts of a table (column and
* row names, footers, data). This object represents the
* field of the same kind. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::TableAutoFormatField</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.sheet.TableAutoFormatField
* @see com.sun.star.beans.XPropertySet
* @see ifc.sheet._TableAutoFormatField
* @see ifc.beans._XPropertySet
*/
public class ScAutoFormatFieldObj extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Using SOffice ServiceManager an instance of
    * <code>com.sun.star.sheet.TableAutoFormatField</code> service
    * is created. From this collection one Format is retrieved
    * and then from this format one of its compound fields is
    * retrieved.
    */
    public synchronized TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log ) throws StatusException {

        XInterface oObj = null;

        try {

                  log.println ("create Object ...") ;
            // creation of testobject here
            XInterface formats = (XInterface)((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.sheet.TableAutoFormats");
            XIndexAccess formatsIndex = (XIndexAccess)UnoRuntime.queryInterface
                (XIndexAccess.class, formats);
            XInterface format = (XInterface) AnyConverter.toObject(
                        new Type(XInterface.class),formatsIndex.getByIndex(0));
            XIndexAccess formatIndex = (XIndexAccess) UnoRuntime.queryInterface
                (XIndexAccess.class, format);
            oObj = (XInterface) AnyConverter.toObject(
                        new Type(XInterface.class),formatIndex.getByIndex(0));

        } catch (com.sun.star.uno.Exception e) {
            log.println ("Exception occurred while creating test Object.") ;
            e.printStackTrace(log) ;
        }

        TestEnvironment tEnv = new TestEnvironment(oObj) ;

        return tEnv;
    }

}    // finish class ScAutoFormatFieldObj

