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


package com.sun.star.wizards.reportbuilder.layout;

import com.sun.star.awt.Rectangle;
import com.sun.star.report.XGroup;
import com.sun.star.report.XSection;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.report.IReportDefinitionReadAccess;
import com.sun.star.wizards.ui.*;

/**
 *
 * @author ll93751
 */
public class InBlocksLabelsLeft extends ColumnarTwoColumns
{

    public InBlocksLabelsLeft(IReportDefinitionReadAccess _xDefinitionAccess, Resource _aResource)
    {
        super(_xDefinitionAccess, _aResource);
    }

    public String getName()
    {
        return "InBlocksLabelsLeftLayoutOfData";
    }

    public String getLocalizedName()
    {
        return getResource().getResText(UIConsts.RID_REPORT + 84);
    }

    protected void insertDetailFields()
    {
        copyDetailProperties();

        final String[] aFieldTitleNames = getFieldTitleNames();
        if (aFieldTitleNames == null)
        {
            return;
        }
        final String[] aFieldNames = getFieldNames();
        if (aFieldNames == null)
        {
            return;
        }
//        int nGroups = getReportDefinition().getGroups().getCount();

        final XSection xSection = getReportDefinition().getDetail();

        Rectangle aRectLabelFields = new Rectangle();

        final int nUsablePageWidth = getPageWidth() - getLeftPageIndent() - getRightPageIndent() - getLeftGroupIndent(getCountOfGroups());

        final int nLabelWidth = getMaxLabelWidth(); // 3000;
        int nFieldWidth = 3000;

        final int nFactor = nUsablePageWidth / (nLabelWidth + nFieldWidth);
        int nDeltaTotal = 0;
        int nDelta = 0;
        if (nFactor > 0)
        {
            nDeltaTotal = nUsablePageWidth - (nFactor * (nLabelWidth + nFieldWidth));
            nDelta = nDeltaTotal / nFactor;
        }

        int i = 0;
        int nCount = aFieldTitleNames.length;
        // int x = 0;
        aRectLabelFields.Y = 0;
        aRectLabelFields.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());

        SectionObject aSOLabel = getDesignTemplate().getDetailLabel();
        aSOLabel.setFontToBold();
        SectionObject aSOTextField = getDesignTemplate().getDetailTextField();
        int nLastHeight = 0;
        while (nCount > 0)
        {
            final String sLabel = aFieldTitleNames[i];
//            nLabelWidth = 3000;
            aRectLabelFields = insertLabel(xSection, sLabel, aRectLabelFields, nLabelWidth, aSOLabel);
            final String sFieldName = convertToFieldName(aFieldNames[i]);
            nFieldWidth = 3000 + nDelta;
            aRectLabelFields = insertFormattedField(xSection, sFieldName, aRectLabelFields, nFieldWidth, aSOTextField);
            nLastHeight = Math.max(aRectLabelFields.Height, nLastHeight);
            final int nNextX = aRectLabelFields.X + nLabelWidth + nFieldWidth;
            if (nNextX > (getPageWidth() - getRightPageIndent()))
            {
                // TODO: label height is fix
                aRectLabelFields.Y += Math.max(aSOTextField.getHeight(LayoutConstants.FormattedFieldHeight), nLastHeight);
                nLastHeight = 0;
                aRectLabelFields.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());
            }
            ++i;
            --nCount;
        }
        aRectLabelFields.Y += Math.max(aSOLabel.getHeight(LayoutConstants.EmptyLineHeight), nLastHeight); // one empty line
        xSection.setHeight(aRectLabelFields.Y);
        doNotBreakInTable(xSection);
    }

    protected void insertDetailFieldTitles(int lastGroupPostion)
    {
        // we won't extra field titles
    }
}
