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



#ifndef COMPTR_HXX
#define COMPTR_HXX

#include <sal/types.h>
#include <osl/diagnose.h>
#include <shobjidl.h>

template< class    T_INTERFACE          ,
          REFIID   P_IID   = IID_NULL   ,
          REFCLSID P_CLSID = CLSID_NULL >
class ComPtr
{
    public:
        
        //---------------------------------------------------------------------
        /** initialize com ptr with null.
         */
        ComPtr()
        {
            m_pInterface = NULL;
        }
    
        //---------------------------------------------------------------------
        /** initialize com ptr with given interface.
         */
        ComPtr(T_INTERFACE* pInterface)
        {
            m_pInterface = pInterface;
            if (m_pInterface)
                m_pInterface->AddRef();
        }
    
        //---------------------------------------------------------------------
        /** copy ctor.
         */
        ComPtr(const ComPtr< T_INTERFACE, P_IID, P_CLSID >& aCopy)
        {
            m_pInterface = aCopy.m_pInterface;
            if (m_pInterface)
                m_pInterface->AddRef();
        }
    
        //---------------------------------------------------------------------
        /** initialize object by quering external object for the right interface.
         */
        ComPtr(IUnknown* pIUnknown)
        {
            if (pIUnknown)
                pIUnknown->QueryInterface(P_IID, (void**)&m_pInterface);
        }
    
        //---------------------------------------------------------------------
        /** deinitialize com object right.
         */
        ~ComPtr()
        {
            release();
        }

    public:
        
        //---------------------------------------------------------------------
        HRESULT create()
        {
            return CoCreateInstance(P_CLSID, NULL, CLSCTX_ALL, P_IID, (void**)&m_pInterface);
        }
    
        //---------------------------------------------------------------------
        operator T_INTERFACE*() const
        {
            return m_pInterface;
        }
    
        //---------------------------------------------------------------------
        T_INTERFACE& operator*() const
        {
            return *m_pInterface;
        }
    
        //---------------------------------------------------------------------
        T_INTERFACE** operator&()
        {
            return &m_pInterface;
        }
    
        //---------------------------------------------------------------------
        T_INTERFACE* operator->() const
        {
            return m_pInterface;
        }
    
        //---------------------------------------------------------------------
        T_INTERFACE* operator=(T_INTERFACE* pInterface)
        {
            if ( equals(pInterface) )
                return m_pInterface;
            
            m_pInterface->Release();
            m_pInterface = pInterface;
            if (m_pInterface)
                m_pInterface->AddRef();
            
            return m_pInterface;
        }
    
        //---------------------------------------------------------------------
        T_INTERFACE* operator=(IUnknown* pIUnknown)
        {
            if (pIUnknown)
                pIUnknown->QueryInterface(P_IID, (void**)&m_pInterface);
            return m_pInterface;
        }
    
        //---------------------------------------------------------------------
        T_INTERFACE* operator=(const ComPtr< T_INTERFACE, P_IID, P_CLSID >& aCopy)
        {
            m_pInterface = aCopy.m_pInterface;
            if (m_pInterface)
                m_pInterface->AddRef();
            
            return m_pInterface;
        }
    
        //---------------------------------------------------------------------
        T_INTERFACE* get() const
        {
            return m_pInterface;
        }
    
        //---------------------------------------------------------------------
        void attach(T_INTERFACE* pInterface)
        {
            if (pInterface)
            {
                m_pInterface->Release();
                m_pInterface = pInterface;
            }
        }
    
        //---------------------------------------------------------------------
        T_INTERFACE* detach()
        {
            T_INTERFACE* pInterface = m_pInterface;
            m_pInterface = NULL;
            return pInterface;
        }
    
        //---------------------------------------------------------------------
        void release()
        {
            if (m_pInterface)
            {
                m_pInterface->Release();
                m_pInterface = NULL;
            }
        }
    
#ifndef __MINGW32__
        //---------------------------------------------------------------------
        template< class T_QUERYINTERFACE >
        HRESULT query(T_QUERYINTERFACE** pQuery)
        {
            return m_pInterface->QueryInterface(__uuidof(T_QUERYINTERFACE), (void**)pQuery);
        }
#endif
    
        //---------------------------------------------------------------------
        ::sal_Bool equals(IUnknown* pCheck)
        {
            if (
                ( ! m_pInterface ) &&
                ( ! pCheck       )
               )
                return sal_True;
        
            IUnknown* pCurrent = NULL;
            m_pInterface->QueryInterface(IID_IUnknown, (void**)&pCurrent);
        
            ::sal_Bool bEquals = (pCheck == pCurrent);
            pCurrent->Release();
        
            return bEquals;
        }
    
        //---------------------------------------------------------------------
        ::sal_Bool is()
        {
            return (m_pInterface != 0);
        }

    private:
        T_INTERFACE* m_pInterface;
};

#endif
