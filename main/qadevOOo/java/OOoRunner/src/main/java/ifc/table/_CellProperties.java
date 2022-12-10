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



package ifc.table;

import lib.MultiPropertyTest;

import com.sun.star.container.XNameContainer;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.xml.AttributeData;

/**
* Testing <code>com.sun.star.table.CellProperties</code>
* service properties :
* <ul>
*  <li><code> CellStyle</code></li>
*  <li><code> CellBackColor</code></li>
*  <li><code> IsCellBackgroundTransparent</code></li>
*  <li><code> NumberFormat</code></li>
*  <li><code> ShadowFormat</code></li>
*  <li><code> HoriJustify</code></li>
*  <li><code> VertJustify</code></li>
*  <li><code> Orientation</code></li>
*  <li><code> CellProtection</code></li>
*  <li><code> TableBorder</code></li>
*  <li><code> IsTextWrapped</code></li>
*  <li><code> RotateAngle</code></li>
*  <li><code> RotateReference</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.table.CellProperties
*/
public class _CellProperties extends MultiPropertyTest {
    

    /**
     * This property is tested with custom property tester which
     * switches between 'Default' and 'Result' style names.
     */
    public void _CellStyle() {
        testProperty("CellStyle", new PropertyTester() {
            protected Object getNewValue(String name, Object old) {
                return "Default".equals(old) ? "Result" : "Default" ;
            }
        }) ;
    }
    
    public void _UserDefinedAttributes() {
        XNameContainer uda = null;
        boolean res = false;
        try {
            uda = (XNameContainer) AnyConverter.toObject(
                new Type(XNameContainer.class),
                    oObj.getPropertyValue("UserDefinedAttributes"));
            AttributeData attr = new AttributeData();
            attr.Namespace = "http://www.sun.com/staroffice/apitest/Cellprop";
            attr.Type="CDATA";
            attr.Value="true";
            uda.insertByName("Cellprop:has-first-alien-attribute",attr);            
            String[] els = uda.getElementNames();            
            oObj.setPropertyValue("UserDefinedAttributes",uda);
            uda = (XNameContainer) AnyConverter.toObject(
                new Type(XNameContainer.class),
                    oObj.getPropertyValue("UserDefinedAttributes"));
            els = uda.getElementNames();
            Object obj = uda.getByName("Cellprop:has-first-alien-attribute");
            res = true;
        } catch (com.sun.star.beans.UnknownPropertyException upe) {
            log.println("Don't know the Property 'UserDefinedAttributes'");
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            log.println("WrappedTargetException while getting Property 'UserDefinedAttributes'");
        } catch (com.sun.star.container.NoSuchElementException nee) {
            log.println("added Element isn't part of the NameContainer");
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("IllegalArgumentException while getting Property 'UserDefinedAttributes'");
        } catch (com.sun.star.beans.PropertyVetoException pve) {
            log.println("PropertyVetoException while getting Property 'UserDefinedAttributes'");
        } catch (com.sun.star.container.ElementExistException eee) {
            log.println("ElementExistException while getting Property 'UserDefinedAttributes'");
        } 
        tRes.tested("UserDefinedAttributes",res);
    }

} // finish class _CellProperties

