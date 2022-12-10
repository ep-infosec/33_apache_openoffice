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

package org.apache.openoffice.ooxml.schema.automaton;

import java.io.File;
import java.util.HashSet;
import java.util.Set;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.iterator.AttributeIterator;
import org.apache.openoffice.ooxml.schema.iterator.DereferencingNodeIterator;
import org.apache.openoffice.ooxml.schema.misc.Log;
import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;

/** Base class of the creator classes for DFAs and NFAs.
 */
public class CreatorBase
{
    CreatorBase (
        final SchemaBase aSchemaBase,
        final File aLogFile)
    {
        maSchemaBase = aSchemaBase;
        maStateContainer = new StateContainer();
        maLog = new Log(aLogFile);
        msLogIndentation = "";
        maElementSimpleTypes = new HashSet<>();
    }
    
    
    
    
    /** Create a very simple automaton:
     *  a) Its start state is also the accepting state.
     *  b) It does not allow any transitions.
     *  c) Its text items have the given simple type.
     */
    protected FiniteAutomaton CreateForSimpleType (final INode aSimpleType)
    {
        final StateContext aStateContext = new StateContext(
            maStateContainer,
            aSimpleType.GetName().GetStateName());
        aStateContext.GetStartState().SetIsAccepting();
        aStateContext.GetStartState().SetTextType(aSimpleType);
        return new FiniteAutomaton(
            aStateContext, 
            new Vector<Attribute>(),
            aSimpleType.GetLocation());
    }
    
    
    
    
    protected Vector<Attribute> CollectAttributes (final INode aRoot)
    {
        final Vector<Attribute> aAttributes = new Vector<>();
        for (final INode aNode : new DereferencingNodeIterator(aRoot, maSchemaBase, true))
            for (final Attribute aAttribute : new AttributeIterator(aNode, maSchemaBase))
                aAttributes.add(aAttribute);
        return aAttributes;
    }

    
    
    
    protected void AddSkipTransition (
        final State aState,
        final SkipData aSkipData)
    {
        aState.AddSkipData(aSkipData);
        
        if (maLog != null)
        {
            maLog.printf("%sskip state %s\n",
                msLogIndentation,
                aState.GetFullname());
        }
    }
    
    
    
    
    protected void ProcessAttributes (final INode aNode)
    {
        for (final Attribute aAttribute : new AttributeIterator(aNode, maSchemaBase))
        {
            maLog.printf("%sattribute %s\n",
                msLogIndentation,
                aAttribute.GetName().GetDisplayName());
            maAttributes.add(aAttribute);
        }
    }
    
        

    
    protected final SchemaBase maSchemaBase;
    protected final StateContainer maStateContainer;
    protected final Log maLog;
    protected String msLogIndentation;
    protected Vector<Attribute> maAttributes;
    protected final Set<INode> maElementSimpleTypes;
}
