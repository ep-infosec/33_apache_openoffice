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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "treevisitorfactory.hxx"
#include "writertreevisiting.hxx"
#include "drawtreevisiting.hxx"

namespace pdfi
{
    struct WriterTreeVisitorFactory : public TreeVisitorFactory
    {
        WriterTreeVisitorFactory() {}

        virtual boost::shared_ptr<ElementTreeVisitor> createOptimizingVisitor(PDFIProcessor& rProc) const
        {
            return boost::shared_ptr<ElementTreeVisitor>(new WriterXmlOptimizer(rProc));
        }

        virtual boost::shared_ptr<ElementTreeVisitor> createStyleCollectingVisitor(
            StyleContainer& rStyles, 
            PDFIProcessor&  rProc ) const
        {
            return boost::shared_ptr<ElementTreeVisitor>(new WriterXmlFinalizer(rStyles,rProc));
        }

        virtual boost::shared_ptr<ElementTreeVisitor> createEmittingVisitor(EmitContext& rEmitContext, PDFIProcessor&) const
        {
            return boost::shared_ptr<ElementTreeVisitor>(new WriterXmlEmitter(rEmitContext));
        }
    };

    struct ImpressTreeVisitorFactory : public TreeVisitorFactory
    {
        ImpressTreeVisitorFactory() {}

        virtual boost::shared_ptr<ElementTreeVisitor> createOptimizingVisitor(PDFIProcessor& rProc) const
        {
            return boost::shared_ptr<ElementTreeVisitor>(new DrawXmlOptimizer(rProc));
        }

        virtual boost::shared_ptr<ElementTreeVisitor> createStyleCollectingVisitor(
            StyleContainer& rStyles, 
            PDFIProcessor&  rProc ) const
        {
            return boost::shared_ptr<ElementTreeVisitor>(new DrawXmlFinalizer(rStyles,rProc));
        }

        virtual boost::shared_ptr<ElementTreeVisitor> createEmittingVisitor(EmitContext& rEmitContext, PDFIProcessor& rProc) const
        {
            return boost::shared_ptr<ElementTreeVisitor>(new DrawXmlEmitter(rEmitContext,
                                                                            DrawXmlEmitter::IMPRESS_DOC,
                                                                            rProc
                                                                            ));
        }
    };

    struct DrawTreeVisitorFactory : public TreeVisitorFactory
    {
        DrawTreeVisitorFactory() {}

        virtual boost::shared_ptr<ElementTreeVisitor> createOptimizingVisitor(PDFIProcessor& rProc) const
        {
            return boost::shared_ptr<ElementTreeVisitor>(new DrawXmlOptimizer(rProc));
        }

        virtual boost::shared_ptr<ElementTreeVisitor> createStyleCollectingVisitor(
            StyleContainer& rStyles, 
            PDFIProcessor&  rProc ) const
        {
            return boost::shared_ptr<ElementTreeVisitor>(new DrawXmlFinalizer(rStyles,rProc));
        }

        virtual boost::shared_ptr<ElementTreeVisitor> createEmittingVisitor(EmitContext& rEmitContext, PDFIProcessor& rProc) const
        {
            return boost::shared_ptr<ElementTreeVisitor>(new DrawXmlEmitter(rEmitContext,
                                                                            DrawXmlEmitter::DRAW_DOC,
                                                                            rProc
                                                                            ));
        }
    };

    TreeVisitorFactorySharedPtr createWriterTreeVisitorFactory() 
    { 
        return TreeVisitorFactorySharedPtr(new WriterTreeVisitorFactory());
    }
    TreeVisitorFactorySharedPtr createImpressTreeVisitorFactory()
    { 
        return TreeVisitorFactorySharedPtr(new ImpressTreeVisitorFactory());
    }
    TreeVisitorFactorySharedPtr createDrawTreeVisitorFactory()
    { 
        return TreeVisitorFactorySharedPtr(new DrawTreeVisitorFactory());
    }
}

