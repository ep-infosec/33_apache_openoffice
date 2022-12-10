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



package mod._scripting;

import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;
import java.net.URLEncoder;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.frame.XModel;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XComponent;
import com.sun.star.frame.XDesktop;

import util.SOfficeFactory;

import drafts.com.sun.star.script.framework.provider.XFunction;
import drafts.com.sun.star.script.framework.provider.XFunctionProvider;

public class Function extends TestCase {
    private String script = "script://returns-Integer";
    private String doc = "doc_with_beanshell_scripts.sxw";

    public void initialize( TestParameters tParam, PrintWriter log ) {
    }

    public synchronized TestEnvironment createTestEnvironment(
        TestParameters tParam, PrintWriter log ) throws StatusException {
        XInterface oObj = null;
	XFunctionProvider provider = null;

        log.println("creating test environment");
        try {
      
            XMultiServiceFactory xMSF = tParam.getMSF();
            SOfficeFactory SOF = null;
            SOF = SOfficeFactory.getFactory( xMSF );
            String docPath = util.utils.getFullTestURL(doc); 
            XComponent doc = SOF.loadDocument( docPath );
            XModel model = ( XModel ) UnoRuntime.queryInterface( XModel.class,
                doc );
            oObj  = 
                (XInterface)xMSF.createInstanceWithArguments( "drafts.com.sun.star.script.framework.provider.FunctionProvider", new Object[]{ model } );
            provider = ( XFunctionProvider )UnoRuntime.queryInterface( XFunctionProvider.class, oObj );           
            oObj = provider.getFunction( script );
            
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
            throw new StatusException("Can't create object environment", e) ;
        }

        TestEnvironment tEnv = new TestEnvironment(oObj) ;
        tEnv.addObjRelation("provider", provider);
        TestDataLoader.setupData(tEnv, "Function");

        return tEnv ;
    }

    public synchronized void disposeTestEnvironment( TestEnvironment tEnv,
            TestParameters tParam) {
    }
}


