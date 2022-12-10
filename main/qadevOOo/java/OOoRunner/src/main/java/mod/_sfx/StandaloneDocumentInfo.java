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



package mod._sfx;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.document.XStandaloneDocumentInfo;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.document.StandaloneDocumentInfo</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::document::DocumentInfo</code></li>
 *  <li> <code>com::sun::star::document::XDocumentInfo</code></li>
 *  <li> <code>com::sun::star::document::XStandaloneDocumentInfo</code></li>
 *  <li> <code>com::sun::star::beans::XFastPropertySet</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 * </ul> <p>
 * The following files used by this test :
 * <ul>
 *  <li><b> SfxStandaloneDocInfoObject.sdw </b> : is copied
 *   to SOffice temporary directory and is used to load and
 *   save its info. </li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.document.DocumentInfo
 * @see com.sun.star.document.XDocumentInfo
 * @see com.sun.star.document.XStandaloneDocumentInfo
 * @see com.sun.star.beans.XFastPropertySet
 * @see com.sun.star.beans.XPropertySet
 * @see ifc.lang._XComponent
 * @see ifc.document._DocumentInfo
 * @see ifc.document._XDocumentInfo
 * @see ifc.document._XStandaloneDocumentInfo
 * @see ifc.beans._XFastPropertySet
 * @see ifc.beans._XPropertySet
 */
public class StandaloneDocumentInfo extends TestCase {

    String destUrl = null ;

    /**
     * Copies file 'SfxStandaloneDocInfoObject.sdw' to tempopary
     * location (overwriting the old file if exists).
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        String srcUrl = utils.getFullTestURL("SfxStandaloneDocInfoObject.sdw") ;
        destUrl = utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) +
            "SfxStandaloneDocInfoObject.sdw";

        utils.doOverwriteFile((XMultiServiceFactory)tParam.getMSF(), srcUrl, destUrl) ;
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.document.StandaloneDocumentInfo</code>.
     *     Object relations created :
     * <ul>
     *  <li> <code>'DOCURL'</code> for
     *      {@link ifc.document._XStandaloneDocumentInfo} :
     *      the Writer file URL in temporary location. </li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        XInterface oObj = null;

        try {
            oObj = (XInterface)xMSF.createInstance
                ("com.sun.star.document.StandaloneDocumentInfo");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        String Iname = util.utils.getImplName(oObj);
        log.println("Implementation Name: "+Iname);
        TestEnvironment tEnv = new TestEnvironment(oObj);
        XStandaloneDocumentInfo the_info = (XStandaloneDocumentInfo)
                UnoRuntime.queryInterface(XStandaloneDocumentInfo.class, oObj);
        try {
            the_info.loadFromURL(destUrl);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't load document: " + destUrl,e);
        }
        tEnv.addObjRelation("DOCURL",destUrl);
        return tEnv;
    }

}    // finish class TestCase

