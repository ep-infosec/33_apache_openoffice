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



package com.sun.star.lib.util;

/** jurt.jar internal string helper methods.
 */
public final class StringHelper
{
    private StringHelper() {} // do not instantiate

    public static String replace(String str, char from, String to) {
        StringBuffer b = new StringBuffer();
        for (int i = 0;;) {
            int j = str.indexOf(from, i);
            if (j == -1) {
                b.append(str.substring(i));
                break;
            } else {
                b.append(str.substring(i, j));
                b.append(to);
                i = j + 1;
            }
        }
        return b.toString();
    }
}
