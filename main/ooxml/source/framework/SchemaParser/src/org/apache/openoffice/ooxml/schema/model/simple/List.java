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

package org.apache.openoffice.ooxml.schema.model.simple;

import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** Representation of the 'list' XML schema element.
 *  It is similar to an array of objects that are all of the same simple type.
 */
public class List
    extends Node
{
    public List (
        final Node aParent,
        final QualifiedName aItemType,
        final Location aLocation)
    {
        super(aParent, null, aLocation);
        maItemType = aItemType;
    }
    
    
    
    
    @Override
    public NodeType GetNodeType ()
    {
        return NodeType.List;
    }




    @Override
    public void AcceptVisitor (final INodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }
    
    
    
    
    public QualifiedName GetItemType ()
    {
        return maItemType;
    }
    
    
    
    
    @Override
    public String toString ()
    {
        return "list of "+maItemType.GetDisplayName()+" items";
    }
    
    
    
    
    private final QualifiedName maItemType;
}
