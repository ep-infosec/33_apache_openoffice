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



package mod._ucpchelp;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class CHelpContentProvider extends TestCase {

    public TestEnvironment createTestEnvironment
            ( TestParameters Param,PrintWriter log ) {

        XInterface oObj = null;
        Object oInterface = null;
        Object aUCB = null;

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance
                ( "com.sun.star.ucb.HelpContentProvider" );
            aUCB = xMSF.createInstance
                ( "com.sun.star.comp.ucb.UniversalContentBroker" );
        } catch( com.sun.star.uno.Exception e ) {
            log.println("Can't create an object." );
            throw new StatusException( "Can't create an object", e );
        }

        oObj = (XInterface) oInterface;

        XContentIdentifierFactory CIF = (XContentIdentifierFactory)
                UnoRuntime.queryInterface(XContentIdentifierFactory.class,aUCB);

        System.out.println("ImplementationName: "+util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment( oObj );

        //Adding ObjRelation for XContentProvider
        tEnv.addObjRelation("FACTORY",CIF);
        tEnv.addObjRelation("CONTENT1", "vnd.sun.star.help://swriter?System=WIN&Language=de&Query=text&HitCount=120&Scope=Heading");            
        tEnv.addObjRelation("CONTENT2", 
            util.utils.getFullTestURL("SwXTextEmbeddedObject.sdw"));

        return tEnv;

    } // finish method getTestEnvironment

}    // finish class FileProvider

