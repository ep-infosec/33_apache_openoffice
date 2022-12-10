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

package fvt.gui.sc.validity;

import static org.junit.Assert.*;
import static org.openoffice.test.common.Testspace.*;
import static testlib.gui.AppTool.*;
import static testlib.gui.UIMap.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.Logger;

import testlib.gui.SCTool;

public class ValiditySampleFile {

	@Rule
	public Logger log = Logger.getLogger(this);
	
	@Before
	public void setUp() throws Exception {
		app.start(true);
	}

	@After
	public void tearDown() throws Exception {
		app.stop();

	}

	/**
	 * Test open MS 2003 spreadsheet with ignore blank validity.
	 * 
	 * @throws Exception
	 */
	@Test
	public void testFFCIgnoreBlank() throws Exception {
		// Open sample file
		String file = prepareData("sc/FFC252FFCSC_XML_Datarange0235.xls");
		open(file);
		calc.waitForExistence(10, 2);

		SCTool.selectRange("D5");
		scInputBarInput.activate();
		for (int i = 1; i <= 10; i++)
			typeKeys("<backspace>");
		typeKeys("<enter>");

		assertEquals("", SCTool.getCellText("D5"));
	}

	/**
	 * Test open MS 2003 spreadsheet with ignore blank validity.
	 * 
	 * @throws Exception
	 */
	@Test
	public void testFFCNotIgnoreBlank() throws Exception {
		// open sample file
		String file = prepareData("sc/FFC252FFCSC_XML_Datarange0205.xls");
		open(file);
		calc.waitForExistence(10, 2);

		SCTool.selectRange("F5");
		scInputBarInput.activate();
		typeKeys("<backspace><enter>");

		activeMsgBox.ok();
		assertEquals("8", SCTool.getCellText("F5"));
	}

	/**
	 * test Cell is not locked after switch from validity cell to source cells
	 */
	@Test
	public void testNotLockCellFromValidityCell() {
		// open sample file on data path
		String file = prepareData("sc/sampledata.ods");
		open(file);
		calc.waitForExistence(10, 2);

		SCTool.selectRange("F19");
		typeKeys("d<enter>");
		SCTool.selectRange("F17");
		typeKeys("Test<enter>");

		assertEquals("Test", SCTool.getCellText("F17"));
	}
}
