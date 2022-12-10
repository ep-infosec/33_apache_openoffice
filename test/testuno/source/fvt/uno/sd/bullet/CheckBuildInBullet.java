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
/**
 * There are 8 build-in bullets. Verify those bullets can be applied successfully.
 * insert text into a SD
 * apply the 8 bullets one by one, and check
 */
package fvt.uno.sd.bullet;

import static org.junit.Assert.assertEquals;
import static testlib.uno.PageUtil.getDrawPageByIndex;
import static testlib.uno.ShapeUtil.addPortion;
import static testlib.uno.ShapeUtil.getPortion;

import java.io.File;
import java.util.Arrays;
import java.util.Collection;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import testlib.uno.SDUtil;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexReplace;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.style.NumberingType;
import com.sun.star.uno.UnoRuntime;


/**
 * @author LouQL
 *
 */
@RunWith(Parameterized.class)
public class CheckBuildInBullet {

	private static final UnoApp app = new UnoApp();	
	private XComponent m_xSDComponent = null;
	private String m_filePath = null;
	private XPropertySet m_xtextProps = null;
	private String m_BulletChar = null;
	private String m_expectedBulletChar = null;
	/**
	 * @throws java.lang.Exception
	 */
	
	public CheckBuildInBullet(String BulletChar, String expected) {
        this.m_BulletChar = BulletChar;
        m_expectedBulletChar = expected;
    }
	@Parameters
    public static Collection<String[]> data() {
        String[][] bulletChar = new String[][] {{"\u25cf","\u25cf"}, {"\u2022","\u2022"}, {"\ue00c","\ue00c"},{"\ue00a","\ue00a"},{"\u2794","\u2794"}, {"\u27a2","\u27a2"}, {"\u2717","\u2717"},{"\u2714","\u2714"}};
        return Arrays.asList(bulletChar);
    }
	
	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		app.start();
		File temp = new File(Testspace.getPath("temp"));
		temp.mkdirs();
	}

	/**
	 * @throws java.lang.Exception
	 */
	@AfterClass
	public static void tearDownAfterClass() throws Exception {
		app.close();
	}

	/**
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		m_filePath = Testspace.getPath("temp/CheckBuildInBullet.odp");
//		m_filePath = "F:/aa.odp";
		if(FileUtil.fileExists(m_filePath))
		{	//load
			m_xtextProps = load();	  		
		}
		else{
			//create a sd
			m_xSDComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, app.newDocument("simpress"));
			Object firstPage = getDrawPageByIndex(m_xSDComponent, 0);
			Object firstTextBox = SDUtil.getShapeOfPageByIndex(firstPage, 0);
			XShape xfirstTextBox = (XShape)UnoRuntime.queryInterface(XShape.class, firstTextBox);
			m_xtextProps = addPortion(xfirstTextBox, "test Build-in Bullet", false);
		}			
	}
	
	/**
	 * @throws java.lang.Exception
	 */
	@After
	public void tearDown() throws Exception {	
		app.closeDocument(m_xSDComponent);		
		FileUtil.deleteFile(Testspace.getPath("temp"));
	}
	private XPropertySet load() throws Exception{
		m_xSDComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, 
				app.loadDocument(m_filePath));
		Object firstPage = getDrawPageByIndex(m_xSDComponent, 0);
		Object firstTextBox = SDUtil.getShapeOfPageByIndex(firstPage, 0);
		XShape xfirstTextBox = (XShape)UnoRuntime.queryInterface(XShape.class, firstTextBox);
		return getPortion(xfirstTextBox, 0);
	}
			
	@Test
	public void testBuildInBullet() throws Exception {		
		    
		Object numberingrules = m_xtextProps.getPropertyValue("NumberingRules");
					
		XIndexReplace xReplace = (XIndexReplace) UnoRuntime.queryInterface(
	             XIndexReplace.class, numberingrules);    
						
		PropertyValue[] props = new PropertyValue[2];
	    props[0] = new PropertyValue();
	    props[0].Name = "NumberingType";
	    props[0].Value = new Short(NumberingType.CHAR_SPECIAL );
	    
	    props[1] = new PropertyValue();
	    props[1].Name = "BulletChar";
	    props[1].Value = this.m_BulletChar;
		
	    //set numberingType
	    xReplace.replaceByIndex(0, props);
	    m_xtextProps.setPropertyValue("NumberingRules", numberingrules);
	  //set numbering level to 0			
	    m_xtextProps.setPropertyValue("NumberingLevel", new Short((short)0));

		app.saveDocument(m_xSDComponent, m_filePath);
		app.closeDocument(m_xSDComponent);
		//reopen
		m_xtextProps = load();
				    
		Object numberingrules2 = m_xtextProps.getPropertyValue("NumberingRules");
					
		XIndexReplace xReplace2 = (XIndexReplace) UnoRuntime.queryInterface(
	             XIndexReplace.class, numberingrules2);
		
		PropertyValue[] proValues2 = (PropertyValue[])xReplace2.getByIndex(0);  
		assertEquals("NumberingType should be CHAR_SPECIAL", NumberingType.CHAR_SPECIAL, proValues2[0].Value);
		assertEquals("BulletChar should be"+m_expectedBulletChar, m_expectedBulletChar, proValues2[4].Value);
	}
}
