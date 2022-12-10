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
package testlib.uno;


import org.openoffice.test.common.FileUtil;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNamed;
import com.sun.star.document.XDocumentProperties;
import com.sun.star.document.XDocumentPropertiesSupplier;
import com.sun.star.frame.XStorable;
import com.sun.star.io.IOException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.BreakType;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XController;
import com.sun.star.uno.UnoRuntime;

public class SWUtil {
	



	public static void saveAsDoc(XTextDocument document, String url) throws IOException {
 		saveAs(document, "MS Word 97", url);
 		
 	}
 
	public static void saveAsDoc(XComponent component, String url) throws IOException{
		XTextDocument document = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, component);
		saveAs(document, "MS Word 97", url);		
	}

	public static void saveAsODT(XTextDocument document, String url) throws IOException {
 		saveAs(document, "writer8", url);
 	}

	public static void saveAs(XTextDocument document, String filterValue, String url) throws IOException {
		XStorable store = (XStorable) UnoRuntime.queryInterface(XStorable.class, document);
 		PropertyValue[] propsValue = new PropertyValue[1];
 		propsValue[0] = new PropertyValue();
 		propsValue[0].Name = "FilterName";
 		propsValue[0].Value = filterValue;
		store.storeAsURL(url, propsValue);
		
 	}
	
	public static void save(XTextDocument document) throws IOException {
 		XStorable store = (XStorable) UnoRuntime.queryInterface(XStorable.class, document);
		store.store();
	}
	
	public static XTextDocument saveAndReload(XTextDocument document, UnoApp app) throws Exception {
 		XStorable store = (XStorable) UnoRuntime.queryInterface(XStorable.class, document);
		store.store();
		String url = document.getURL();
		app.closeDocument(document);
		return openDocumentFromURL(url, app);
		
	}
	
	public static XTextDocument newDocument(UnoApp app) throws Exception {
		return (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
		
 	}
	
	public static XTextDocument openDocumentFromURL(String url, UnoApp app) throws Exception {
		return (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.loadDocumentFromURL(url));
		
	}
	public static XTextDocument openDocument(String filePath, UnoApp app) throws Exception {
		
		return (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(filePath));
		
	}

	public static void moveCuror2End(XTextDocument document) {
		XText xText = document.getText();
		XTextCursor xTextCursor = xText.createTextCursor();	
		xTextCursor.gotoEnd(false);	
	}
	
	public static void moveCuror2Start(XTextDocument document) {
		XText xText = document.getText();
		XTextCursor xTextCursor = xText.createTextCursor();	
		xTextCursor.gotoStart(false);	
	}
	
	/**
	 * Set document properties. Only supported: subject, title, author
	 * @param document - set document information on this document
	 * @param prop - document information, including "Subject" ,"Title", "Author"
	 * @param propValue - value you want to set for prop
	 * @throws Exception
	 */
	public static void setDocumentProperty(XTextDocument document, String prop, String propValue) throws Exception {
       XDocumentPropertiesSupplier docPropsSupplier = UnoRuntime.queryInterface(
            XDocumentPropertiesSupplier.class, document);
       XDocumentProperties docProps = docPropsSupplier.getDocumentProperties();
        if ( prop.equals("Title"))
            docProps.setTitle(propValue);
        else if ( prop.equals("Author"))
            docProps.setAuthor(propValue);
        else if ( prop.equals("Subject"))
            docProps.setSubject(propValue);
    }
	

	/**
	 * Insert a bookmark into text document
	 * @param document text document
	 * @param textCursor which part will be bookmarked
	 * @param bookmarkName bookmark name
	 * @throws Exception
	 */
	public static void insertBookmark(XTextDocument document, XTextCursor textCursor, String bookmarkName) throws Exception {
		XMultiServiceFactory xDocFactory = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
		Object xBookmark = xDocFactory.createInstance("com.sun.star.text.Bookmark");
		XTextContent xBookmarkAsTextContent = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xBookmark);
		XNamed xBookmarkAsNamed = (XNamed) UnoRuntime.queryInterface(XNamed.class, xBookmark);
		xBookmarkAsNamed.setName(bookmarkName);
		document.getText().insertTextContent(textCursor, xBookmarkAsTextContent, true);
	}
	
	/**
	 * insert column break in current cursor
	 * @param xText
	 * @param currentCursor
	 * @throws Exception
	 */
	public static void insertColumnBreak(XText xText, XTextCursor currentCursor) throws Exception
	{
		XPropertySet xCursorProps = (XPropertySet)UnoRuntime.queryInterface(
		        XPropertySet.class, currentCursor);
		xCursorProps.setPropertyValue("BreakType", BreakType.COLUMN_AFTER);		
	    xText.insertControlCharacter(currentCursor,ControlCharacter.PARAGRAPH_BREAK,false); 
	}
	
	/**
	 * insert page break in current cursor
	 * @param xText
	 * @param currentCursor
	 * @throws Exception
	 */
	public static void insertPageBreak(XText xText, XTextCursor currentCursor) throws Exception
	{
		XPropertySet xCursorProps = (XPropertySet)UnoRuntime.queryInterface(
		        XPropertySet.class, currentCursor);
		xCursorProps.setPropertyValue("BreakType", BreakType.PAGE_AFTER);		
	    xText.insertControlCharacter(currentCursor,ControlCharacter.PARAGRAPH_BREAK,false); 
	}	
	
	
	/**
	 * get page count
	 * @param document
	 * @return
	 * @throws Exception
	 */
	public static int getPageCount(XTextDocument document) throws Exception
	{
		XModel xmodel = (XModel)UnoRuntime.queryInterface(XModel.class, document);
		XController xcont = xmodel.getCurrentController();

		XPropertySet xps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xcont);
		Integer pageCount = (Integer) xps.getPropertyValue("PageCount"); 
		return pageCount.intValue();
	}	
	
	
	/**
	 * get specific property value of the default page style
	 * @param xComponent
	 * @param propertyName
	 * @return
	 * @throws Exception
	 */
	public static Object getDefaultPageStyleProperty(XComponent xComponent, String propertyName) throws Exception
	{
		XTextDocument textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, xComponent);
		XStyleFamiliesSupplier xSupplier = (XStyleFamiliesSupplier)UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, textDocument);	     
        XNameAccess xFamilies = (XNameAccess) UnoRuntime.queryInterface (XNameAccess.class, xSupplier.getStyleFamilies());        
        XNameContainer xFamily = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, xFamilies.getByName("PageStyles"));        
        XStyle xStyle = (XStyle)UnoRuntime.queryInterface(XStyle.class, xFamily.getByName("Default"));     
        XPropertySet xStyleProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xStyle);  
        Object propertyValue = xStyleProps.getPropertyValue(propertyName.toString());
        return propertyValue;       
	}
	
	/**
	 * set value for specific property of default page style.
	 * @param xComponent
	 * @param propertyName
	 * @param propertyValue
	 * @throws Exception
	 */
	public static void setDefaultPageStyleProperty(XComponent xComponent, String propertyName, Object propertyValue) throws Exception
	{
		XTextDocument textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, xComponent);		
        XStyleFamiliesSupplier xSupplier = (XStyleFamiliesSupplier)UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, textDocument);     
        XNameAccess xFamilies = (XNameAccess) UnoRuntime.queryInterface (XNameAccess.class, xSupplier.getStyleFamilies());        
        XNameContainer xFamily = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, xFamilies.getByName("PageStyles"));        
        XStyle xStyle = (XStyle)UnoRuntime.queryInterface(XStyle.class, xFamily.getByName("Default"));     
        XPropertySet xStyleProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xStyle);        
        xStyleProps.setPropertyValue (propertyName.toString(), propertyValue);
	}
	
	public static XTextDocument saveTo_Override_reload(XTextDocument xTextDocument,String filtervalue, String url,UnoApp app) throws Exception {
		XStorable xStorable_odt = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
		PropertyValue[] aStoreProperties = new PropertyValue[2];
		aStoreProperties[0] = new PropertyValue();
		aStoreProperties[1] = new PropertyValue();
		aStoreProperties[0].Name = "Override";
		aStoreProperties[0].Value = true;
		aStoreProperties[1].Name = "FilterName";
		aStoreProperties[1].Value = filtervalue;
		xStorable_odt.storeToURL(FileUtil.getUrl(url), aStoreProperties);
		//reopen the document
		return (XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(url));	
 	}
	/**
	 * create document from template
	 */
	public static XComponent newDocumentFromTemplate(String templatePath,UnoApp unoApp) throws Exception
	{
		XComponentLoader componentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, unoApp.getDesktop());
		PropertyValue[] pros = new PropertyValue[1];
		pros[0] = new PropertyValue();
		pros[0].Name = "AsTemplate";
		pros[0].Value = new Boolean(true);				
		XComponent component = componentLoader.loadComponentFromURL(FileUtil.getUrl(templatePath), "_blank", 0,pros);
		return component;
	}
}
