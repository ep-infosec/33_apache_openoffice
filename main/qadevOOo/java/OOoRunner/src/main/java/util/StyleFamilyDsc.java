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

/**
 * the class StyleFamilyDsc
 */
public class StyleFamilyDsc extends InstDescr {

 	private String name = null;
	final String ifcName = "com.sun.star.style.XStyle";
 	String service = "com.sun.star.style.CharacterStyle";

	public StyleFamilyDsc( String kind ) {
        service = "com.sun.star.style." + kind;
		initStyleFamily();
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

	private void initStyleFamily() {
		try {
	       	ifcClass = Class.forName( ifcName );
	    }
	    catch( ClassNotFoundException cnfE ) {
	    }
	}
	public XInterface createInstance( XMultiServiceFactory docMSF ) {


		Object SrvObj = null;
		try {
			SrvObj = docMSF.createInstance( service );
		}
		catch( com.sun.star.uno.Exception cssuE ){
		}

		XInterface StyleFamily = (XInterface)UnoRuntime.queryInterface(ifcClass, SrvObj );

		return StyleFamily;

	}
}
