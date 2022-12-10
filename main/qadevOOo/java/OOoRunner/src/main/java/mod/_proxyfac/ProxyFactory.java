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



package mod._proxyfac;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;


/**
* Test for object which is represented by service
* <code>com.sun.star.reflection.ProxyFactory</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::reflection::XProxyFactory</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.reflection.XProxyFactory
* @see com.sun.star.reflection.ProxyFactory
* @see ifc.reflection._XProxyFactory
*/
public class ProxyFactory extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.reflection.ProxyFactory</code>.
    */
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
            PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        try {
            XInterface xInt = (XInterface)xMSF.createInstance(
                    "com.sun.star.reflection.ProxyFactory");

            TestEnvironment tEnv = new TestEnvironment(xInt);

            return tEnv;
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

    }
}
