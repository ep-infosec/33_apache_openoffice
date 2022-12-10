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


package com.sun.star.report.pentaho.parser.rpt;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.FormattedTextElement;
import com.sun.star.report.pentaho.parser.ElementReadHandler;

import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.xmlns.parser.IgnoreAnyChildReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Creation-Date: 01.10.2006, 19:06:45
 *
 * @author Thomas Morgner
 */
public class FormattedTextReadHandler extends ElementReadHandler
{

    private final FormattedTextElement element;

    public FormattedTextReadHandler()
    {
        element = new FormattedTextElement();
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs) throws SAXException
    {
        super.startParsing(attrs);

        final String formula = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "formula");
        if (formula != null)
        {
            final FormulaExpression valueExpression = new FormulaExpression();
            valueExpression.setFormula(formula);
            element.setValueExpression(valueExpression);
        }

        // * Print-Repeated-Values
        // * Print-In-First-New-Section
        // * Print-When-Group-Changes

        // * Print-When-Section-Overflows
        // That property cannot be evaluated yet, as this would require us to
        // have a clue about pagebreaking. We dont have that - not yet and never
        // in the future, as pagebreaks are computed by OpenOffice instead
    }

    /**
     * Returns the handler for a child element.
     *
     * @param tagName the tag name.
     * @param atts    the attributes.
     * @return the handler or null, if the tagname is invalid.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected XmlReadHandler getHandlerForChild(final String uri,
            final String tagName,
            final Attributes atts)
            throws SAXException
    {
        if (OfficeNamespaces.OOREPORT_NS.equals(uri))
        {
            // expect a report control. The control will modifiy the current
            // element (as we do not separate the elements that strictly ..)
            if ("report-control".equals(tagName))
            {
                return new IgnoreAnyChildReadHandler();
            }
            if ("report-element".equals(tagName))
            {
                return new ReportElementReadHandler(element);
            }
        }
        return null;
    }

    public Element getElement()
    {
        return element;
    }
}
