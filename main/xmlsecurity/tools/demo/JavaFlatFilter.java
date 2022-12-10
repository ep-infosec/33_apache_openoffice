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



package com.sun.star.xml.security.eval;

import com.sun.star.registry.XRegistryKey;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.xml.sax.InputSource;
import com.sun.star.xml.sax.XDocumentHandler;
import com.sun.star.xml.sax.XParser;
import com.sun.star.xml.sax.XDTDHandler;
import com.sun.star.xml.sax.XEntityResolver;
import com.sun.star.xml.sax.XErrorHandler;
import com.sun.star.xml.sax.XAttributeList;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.Locale;

/*
 * the JavaFlatFilter class is a pure java filter, which does nothing
 * but forwarding the SAX events to the next document handler.
 * The purpose of this class is to calculate the time consumed by
 * the UNO C++/Java bridge during exporting/importing.
 */
public class JavaFlatFilter extends Object
		implements XDocumentHandler, XParser, XTypeProvider, XServiceInfo
{
	XDocumentHandler m_xDocumentHandler;
	
	/* XDocumentHandler */
	public void startDocument() 
		throws com.sun.star.xml.sax.SAXException 
	{
		m_xDocumentHandler.startDocument();
	}
	
	public void endDocument()
		throws com.sun.star.xml.sax.SAXException 
	{
		m_xDocumentHandler.endDocument();
	}
	
	public void startElement (String aName, com.sun.star.xml.sax.XAttributeList xAttribs ) 
		throws com.sun.star.xml.sax.SAXException 
	{
		m_xDocumentHandler.startElement(aName, xAttribs);
	}
	
	public void endElement ( String aName ) 
		throws com.sun.star.xml.sax.SAXException 
	{
		m_xDocumentHandler.endElement(aName);
	}
	
	public void characters ( String aChars ) 
		throws com.sun.star.xml.sax.SAXException 
	{
		m_xDocumentHandler.characters(aChars);
	}
	
	public void ignorableWhitespace ( String aWhitespaces ) 
		throws com.sun.star.xml.sax.SAXException 
	{
		m_xDocumentHandler.ignorableWhitespace(aWhitespaces);
	}
	
	public void processingInstruction ( String aTarget, String aData ) 
		throws com.sun.star.xml.sax.SAXException 
	{
		m_xDocumentHandler.processingInstruction(aTarget, aData);
	}
	
	public void setDocumentLocator (com.sun.star.xml.sax.XLocator xLocator ) 
		throws com.sun.star.xml.sax.SAXException
	{
		m_xDocumentHandler.setDocumentLocator(xLocator);
	}
	
	/* XParser */
	public void parseStream(InputSource strucInputSource)
	{
	}
	
	public void setDocumentHandler(XDocumentHandler xDocumentHandler)
	{
		m_xDocumentHandler = xDocumentHandler;
	}
	
	public void setDTDHandler(XDTDHandler xHandler)
	{
	}
	
	public void setEntityResolver(XEntityResolver xResolver)
	{
	}
	
	public void setErrorHandler(XErrorHandler xHandler)
	{
	}
	
	public void setLocale(Locale locale)
	{
	}
	
	/*
	 * XTypeProvider implementation
	 * maintain a static implementation id for all instances of JavaFlatFilter
	 * initialized by the first call to getImplementationId()
	 */
	protected static byte[] _implementationId;
	public com.sun.star.uno.Type[] getTypes() 
	{
		com.sun.star.uno.Type[] retValue = new com.sun.star.uno.Type[4];
		
		/*
		 * instantiate Type instances for each interface you support and add them to Type[] array
		 * this object implements XServiceInfo, XTypeProvider and XSignFilter
		 */
		retValue[0]= new com.sun.star.uno.Type( XServiceInfo.class);
		retValue[1]= new com.sun.star.uno.Type( XTypeProvider.class);
		retValue[2]= new com.sun.star.uno.Type( XDocumentHandler.class);
		retValue[3]= new com.sun.star.uno.Type( XParser.class);
		
		/*
		 * XInterface is not needed for Java components, the UnoRuntime does its job
		 */
		 
		return retValue;
	}
	
	synchronized public byte[] getImplementationId() 
	{
		if (_implementationId == null) {
		_implementationId= new byte[16];
		int hash = hashCode(); // hashDode of this object
		_implementationId[0] = (byte)(hash & 0xff);
		_implementationId[1] = (byte)((hash >>> 8) & 0xff);
		_implementationId[2] = (byte)((hash >>> 16) & 0xff);
		_implementationId[3] = (byte)((hash >>>24) & 0xff);
		}
		return _implementationId;
	}	
	
	
	/*
	 * XServiceInfo implementation
	 * hold the service name in a private static member variable of the class
	 */
	protected static final String __serviceName = "com.sun.star.xml.crypto.eval.JavaFlatFilter";
	public String getImplementationName( ) 
	{
		return getClass().getName();
	}
	
	public boolean supportsService(String serviceName) 
	{
		boolean rc = false;
		
		if ( serviceName.equals( __serviceName))
		{
			rc = true;
		}
		
		return rc;
	}
	
	public String[] getSupportedServiceNames( ) 
	{
		String[] retValue= new String[0];
		retValue[0]= __serviceName;
		return retValue;
	}
	
	/* static __getServiceFactory() implementation */
	public static XSingleServiceFactory __getServiceFactory(String implName,
		XMultiServiceFactory multiFactory,
		com.sun.star.registry.XRegistryKey regKey) 
	{
		com.sun.star.lang.XSingleServiceFactory xSingleServiceFactory = null;
		if (implName.equals( JavaFlatFilter.class.getName()) )
		{
			xSingleServiceFactory = FactoryHelper.getServiceFactory( JavaFlatFilter.class,
				JavaFlatFilter.__serviceName,
				multiFactory,
				regKey);
		}
		
		return xSingleServiceFactory;
	}	
	
	/* static __writeRegistryServiceInfo implementation */
	public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) 
	{
		return FactoryHelper.writeRegistryServiceInfo( JavaFlatFilter.class.getName(),
								__serviceName,
								regKey);
	}	
}
