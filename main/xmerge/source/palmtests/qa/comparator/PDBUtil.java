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



/**
 *  Contains common static methods and contants for use within the package.
 *
 *  @author    Herbie Ong
 */

public final class PDBUtil {

    /** difference in seconds from Jan 01, 1904 to Jan 01, 1970 */
    final static long TIME_DIFF = 2082844800;

    /** encoding scheme used */
    final static String ENCODING = "8859_1";

    /** size of a pdb header in bytes */
    final static int HEADER_SIZE = 78;

    /**
     *  This method converts a 4 letter string into the Palm ID integer.
     *
     *  It is normally used to convert the Palm creator ID string into
     *  the integer version of it.  Also use for data types, etc.
     *
     *  @param   s    4 character string.
     *  @return   int    Palm ID representing the string.
     *  @throws   ArrayIndexOutOfBoundsException    if string parameter
     *                contains less than 4 characters.
     */

    public static int intID(String s) {

        int id = -1;
        int temp = 0;

        // grab the first char and put it in the high bits
        // note that we only want 8 lower bits of it.
        temp = (int) s.charAt(0);
        id = temp << 24;

        // grab the second char and add it in.
        temp = ((int) s.charAt(1)) & 0x00ff;
        id += temp << 16;

        // grab the second char and add it in.
        temp = ((int) s.charAt(2)) & 0x00ff;
        id += temp << 8;

        // grab the last char and add it in
        id += ((int) s.charAt(3)) & 0x00ff;

        return id;
    }

    /**
     *  This method converts an integer into a String given
     *  the Palm ID format.
     *
     *  @param  i   Palm id.
     *  @return  String   string representation.
     */

    public static String stringID(int i) {

        char ch[] = new char[4];
        ch[0] = (char) (i >>> 24);
        ch[1] = (char) ((i >> 16) & 0x00ff);
        ch[2] = (char) ((i >> 8) & 0x00ff);
        ch[3] = (char) (i & 0x00ff);

        return new String(ch);
    }
}

