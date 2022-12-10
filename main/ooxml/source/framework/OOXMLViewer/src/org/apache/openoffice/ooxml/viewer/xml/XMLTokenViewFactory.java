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

package org.apache.openoffice.ooxml.viewer.xml;

import java.io.InputStream;

import org.apache.openoffice.ooxml.viewer.KeyListener;
import org.apache.openoffice.ooxml.viewer.tokenview.TokenView;

public class XMLTokenViewFactory
    extends Thread
{
    public XMLTokenViewFactory ()
    {
        maIn = null;
        start();
    }
    
    
    
    
    public TokenView<TokenType> Create (final InputStream aIn)
    {
        TokenView<TokenType> aView = null;
        synchronized(this)
        {
            maIn = aIn;
            if (maIn != null)
            {
                aView = new TokenView<TokenType>();
                aView.addKeyListener(new KeyListener());
                maCurrentView = aView;
                
                FillDocument();
                //notify();
            }
        }
        
        return aView;
    }
        
    
    
    
    @Override
    public void run ()
    {
        while(true)
        {
            FillDocument();
            
            synchronized(this)
            {
                if (maIn == null)
                {
                    try
                    {
                        wait();
                    }
                    catch (InterruptedException e)
                    {
                        e.printStackTrace();
                    }
                }
            }
        }
    }
    

    
    
    private void FillDocument ()
    {
        final InputStream aIn = maIn;
        maIn = null;
        if (aIn != null)
        {
            new DocumentTokenFormatter(new XMLScanner(aIn), maCurrentView).Parse();
        }
    }
    
    
    

    private InputStream maIn;
    private TokenView<TokenType> maCurrentView;
}
