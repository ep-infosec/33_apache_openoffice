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



import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.Type;

/** This class capsulates the class, that implements the minimal component, a
 * factory for creating the service (<CODE>__getComponentFactory</CODE>) and a
 * method, that writes the information into the given registry key
 * (<CODE>__writeRegistryServiceInfo</CODE>).
 */
public class MinimalComponent {
    /** This class implements the component. At least the interfaces XServiceInfo,
     * XTypeProvider, and XInitialization should be provided by the service.
     */
    public static class _MinimalComponent extends WeakBase
        implements XInitialization, XServiceInfo {
        /** The service name, that must be used to get an instance of this service.
         */
        static private final String __serviceName =
        "org.openoffice.MinimalComponent";

        /** The initial component contextr, that gives access to
         * the service manager, supported singletons, ...
         * It's often later used
         */
        private XComponentContext m_cmpCtx;

        /** The service manager, that gives access to all registered services.
         * It's often later used
         */
        private XMultiComponentFactory m_xMCF;

        /** The constructor of the inner class has a XMultiServiceFactory parameter.
         * @param xmultiservicefactoryInitialization A special service factory
         * could be introduced while initializing.
         */
        public _MinimalComponent(XComponentContext xCompContext) {
            try {
                m_cmpCtx = xCompContext;
                m_xMCF = m_cmpCtx.getServiceManager();
            }
            catch( Exception e ) {
                e.printStackTrace();
            }
        }

        /** This method is a member of the interface for initializing an object
         * directly after its creation.
         * @param object This array of arbitrary objects will be passed to the
         * component after its creation.
         * @throws Exception Every exception will not be handled, but will be
         * passed to the caller.
         */
        public void initialize( Object[] object )
            throws com.sun.star.uno.Exception {
            /* The component describes what arguments its expected and in which
             * order!At this point you can read the objects and can initialize
             * your component using these objects.
             */
        }

        /** This method returns an array of all supported service names.
         * @return Array of supported service names.
         */
        public String[] getSupportedServiceNames() {
            return getServiceNames();
        }

        /** This method is a simple helper function to used in the
         * static component initialization functions as well as in
         * getSupportedServiceNames.
         */
        public static String[] getServiceNames() {
            String[] sSupportedServiceNames = { __serviceName };
            return sSupportedServiceNames;
        }

        /** This method returns true, if the given service will be
         * supported by the component.
         * @param sServiceName Service name.
         * @return True, if the given service name will be supported.
         */
        public boolean supportsService( String sServiceName ) {
            return sServiceName.equals( __serviceName );
        }

        /** Return the class name of the component.
         * @return Class name of the component.
         */
        public String getImplementationName() {
            return _MinimalComponent.class.getName();
        }
    }


    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleComponentFactory</code> for creating
     *          the component
     * @param   sImplName the name of the implementation for which a
     *          service is desired
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleComponentFactory __getComponentFactory(String sImplName)
    {
        XSingleComponentFactory xFactory = null;

        if ( sImplName.equals( _MinimalComponent.class.getName() ) )
            xFactory = Factory.createComponentFactory(_MinimalComponent.class,
                                             _MinimalComponent.getServiceNames());

        return xFactory;
    }

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey the registryKey
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    // This method not longer necessary since OOo 3.4 where the component registration
    // was changed to passive component registration. For more details see
    // https://wiki.openoffice.org/wiki/Passive_Component_Registration

//     public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
//         return Factory.writeRegistryServiceInfo(_MinimalComponent.class.getName(),
//                                                 _MinimalComponent.getServiceNames(),
//                                                 regKey);
//     }
}
