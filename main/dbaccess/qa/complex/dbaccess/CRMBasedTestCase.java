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


package complex.dbaccess;

import com.sun.star.sdb.XSingleSelectQueryComposer;
import connectivity.tools.CRMDatabase;
import java.util.logging.Level;
import java.util.logging.Logger;

// ---------- junit imports -----------------
import org.junit.After;
import org.junit.Before;
import static org.junit.Assert.*;
// ------------------------------------------

public abstract class CRMBasedTestCase extends TestCase
{
    protected   CRMDatabase m_database;

    // --------------------------------------------------------------------------------------------------------
    protected void createTestCase()
    {
        try
        {
            m_database = new CRMDatabase( getMSF(), false );
        }
        catch ( Exception e )
        {
            e.printStackTrace( System.err );
            fail( "caught an exception (" + e.getMessage() + ") while creating the test case");
        }
    }

    // --------------------------------------------------------------------------------------------------------
    @Before
    @Override
    public void before()
    {
        createTestCase();
    }

    // --------------------------------------------------------------------------------------------------------
    @After
    @Override
    public void after()
    {
        try
        {
            if ( m_database != null )
            {
                m_database.saveAndClose();
            }
        }
        catch ( Exception ex )
        {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, null, ex );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    /** creates a SingleSelectQueryComposer for our connection
     */
    protected final XSingleSelectQueryComposer createQueryComposer() throws com.sun.star.uno.Exception
    {
        return m_database.getConnection().createSingleSelectQueryComposer();
    }
}
