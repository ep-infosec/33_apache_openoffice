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



package util;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.text.XTextContent;

/**
 * describes a Footnote to be inserted in a container
 */
public class FootnoteDsc extends InstDescr {

	final String service = "com.sun.star.text.Footnote";
	String ifcName = "com.sun.star.text.XTextContent";
	private String name = null;


	public FootnoteDsc() {
		initFootnote();
	}

	public FootnoteDsc( String name ) {
		this.name = name;
		initFootnote();
	}

	public String getName() {
		return name;
	}

	public String getIfcName() {
		return ifcName;
	}

	public String getService() {
		return service;
	}

	private void initFootnote() {
		try {
	       	ifcClass = Class.forName( ifcName );
	    }
	    catch( ClassNotFoundException cnfE ) {
	    }
	}
	public XInterface createInstance( XMultiServiceFactory docMSF ) {
		Object ServiceObj = null;

		try {
			ServiceObj = docMSF.createInstance( service );
		}
		catch( com.sun.star.uno.Exception cssuE ){
		}
		XTextContent FN = (XTextContent)UnoRuntime.queryInterface( ifcClass,
																ServiceObj );
		return FN;
	}
}