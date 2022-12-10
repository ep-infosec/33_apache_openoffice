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



package mod._dynamicloader;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.loader.Dynamic</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::loader::XImplementationLoader</code></li>
*  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.loader.XImplementationLoader
* @see com.sun.star.lang.XServiceInfo
* @see com.sun.star.loader.Dynamic
* @see ifc.loader._XImplementationLoader
* @see ifc.lang._XServiceInfo
*/
public class Dynamic extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates service
    * <code>com.sun.star.loader.Dynamic</code>.
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance( "com.sun.star.loader.Dynamic" );
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
            throw new StatusException("Can't create service", e) ;
        }

        if (oInterface == null)
            log.println("Service wasn't created") ;

        oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relations for XImplementationLoader

        String loader = "com.sun.star.loader.SharedLibrary";
        tEnv.addObjRelation("ImplementationLoader", loader) ;

        String url = "servicename=com.sun.star.loader.SharedLibrary,link=mylink";
        //util.utils.getFullTestURL("solibrary.jar");
        tEnv.addObjRelation("ImplementationUrl", url) ;

        tEnv.addObjRelation("ImplementationName", "com.sun.star.io.Pipe") ;

        return tEnv;
    } // finish method getTestEnvironment

}

