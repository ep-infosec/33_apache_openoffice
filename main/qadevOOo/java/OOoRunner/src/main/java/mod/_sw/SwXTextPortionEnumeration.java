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



package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 *
 * initial description
 * @see com.sun.star.text.TextPortion
 *
 */
public class SwXTextPortionEnumeration extends TestCase {

    XTextDocument xTextDoc;

    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
                log.println( "creating a textdocument" );
                xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
                // Some exception occured.FAILED
                e.printStackTrace( log );
                throw new StatusException( "Couldn't create document", e );
        }
    }

    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     *    creating a Testenvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *    @see PrintWriter
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface param = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        // create testobject here

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println( "inserting Strings" );
        log.println( "inserting ControlCharacter" );


        try{
            for (int i =0; i < 5; i++){
                oText.insertString( oCursor,"Paragraph Number: " + i, false);
                oText.insertControlCharacter(
                    oCursor, ControlCharacter.LINE_BREAK, false );
                oText.insertString( oCursor,
                    "The quick brown fox jumps over the lazy Dog: SwXParagraph\n",
                    false);
                oText.insertControlCharacter(
                    oCursor, ControlCharacter.LINE_BREAK, false );
                oText.insertString( oCursor,
                    "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG: SwXParagraph",
                    false);
                oText.insertControlCharacter(
                    oCursor, ControlCharacter.PARAGRAPH_BREAK, false );
            }
        }catch(Exception e){
            log.println("Couldn't insert Text");
            e.printStackTrace();
            throw new StatusException( "Couldn't insert Text", e );
        }

        // Enumeration
        XEnumerationAccess oEnumA = (XEnumerationAccess)
            UnoRuntime.queryInterface( XEnumerationAccess.class, oText );
        XEnumeration oEnum = oEnumA.createEnumeration();

        int n = 0;
        while ( (oEnum.hasMoreElements()) ) {
            try {
                    param = (XInterface) AnyConverter.toObject(
                        new Type(XInterface.class),oEnum.nextElement());
            } catch ( Exception e) {
                log.println("Couldn't get Paragraph");
                e.printStackTrace();
                throw new StatusException( "Couldn't get Paragraph", e );
            }
             n++;
        }

        XEnumerationAccess oEnumP = (XEnumerationAccess)
            UnoRuntime.queryInterface( XEnumerationAccess.class, param );
        XEnumeration oEnum2 = oEnumP.createEnumeration();

        log.println( "creating a new environment for TextPortionEnumeration object" );
        TestEnvironment tEnv = new TestEnvironment( oEnum2 );

        log.println("adding ObjRelation ENUM for XEnumeration");
        tEnv.addObjRelation("ENUM", oEnumP);

        return tEnv;
    } // finish method getTestEnvironment


}    // finish class SwXTextPortionEnumeration

