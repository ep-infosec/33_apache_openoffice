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



package org.openoffice.idesupport.filter;

public class ExceptParcelFilter implements FileFilter {
    private static final String DESCRIPTION = "Remove specified Parcel";
    private static final ExceptParcelFilter filter = new ExceptParcelFilter();
    private static String parcelName = null;
    
    private ExceptParcelFilter() {
    }
    
    public void setParcelToRemove(String parcelName)
    {
        this.parcelName = parcelName;
    }
    
    public static ExceptParcelFilter getInstance() {
        return filter;
    }
    public boolean validate(String name) {
        if (name.startsWith(this.parcelName))
            return true;
        return false;
    }

    public String toString() {
        StringBuffer buf = new StringBuffer(DESCRIPTION + ": ");

        buf.append("<" + this.parcelName + ">");

        return buf.toString();
    }
}
