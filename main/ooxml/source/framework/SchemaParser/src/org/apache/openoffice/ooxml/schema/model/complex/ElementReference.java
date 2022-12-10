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

package org.apache.openoffice.ooxml.schema.model.complex;

import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.INodeReference;
import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;

public class ElementReference
    extends Element
    implements INodeReference
{
    public ElementReference (
        final Node aParent,
        final QualifiedName aReferencedElementName,
        final Location aLocation)
    {
        super(aParent, null, null, aLocation);
        
        maReferencedElementName = aReferencedElementName;
    }
    
    
    
    
    public QualifiedName GetReferencedElementName ()
    {
        return maReferencedElementName;
    }
    
    


    public Element GetReferencedElement (final SchemaBase aSchema)
    {
        final Element aElement = aSchema.TopLevelElements.Get(maReferencedElementName);
        if (aElement == null)
            throw new RuntimeException("can not find element "+maReferencedElementName.GetDisplayName());
        return aElement;
    }

    
    
    
    @Override
    public INode GetReferencedNode (final SchemaBase aSchema)
    {
        return GetReferencedElement(aSchema);
    }

        
    
    
    @Override
    public NodeType GetNodeType ()
    {
        return NodeType.ElementReference;
    }

    
    
    
    @Override
    public void AcceptVisitor (final INodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }

    
    
    
    @Override
    public String toString ()
    {
        return "element reference to "+maReferencedElementName.GetDisplayName();
    }
    
    
    
    
    private final QualifiedName maReferencedElementName;
}
