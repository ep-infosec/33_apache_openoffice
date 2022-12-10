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


package com.sun.star.report.pentaho;

import com.sun.star.report.DataSourceException;
import com.sun.star.report.DataSourceFactory;

import java.util.HashMap;

import org.jfree.report.DataSet;
import org.jfree.report.ReportData;
import org.jfree.report.ReportDataFactory;
import org.jfree.report.ReportDataFactoryException;


public class StarReportDataFactory implements ReportDataFactory, Cloneable
{

    private final DataSourceFactory backend;

    public StarReportDataFactory(DataSourceFactory backend)
    {
        this.backend = backend;
    }

    /**
     * Queries a datasource. The string 'query' defines the name of the query. The
     * Parameterset given here may contain more data than actually needed.
     * <p/>
     * The dataset may change between two calls, do not assume anything!
     *
     * @param query
     * @param parameters
     * @return
     */
    public ReportData queryData(final String query, final DataSet parameters)
            throws ReportDataFactoryException
    {
        try
        {
            final HashMap map = new HashMap();
            final int count = parameters.getColumnCount();
            for (int i = 0; i < count; i++)
            {
                final Object o = parameters.get(i);
                map.put(parameters.getColumnName(i), o);
            }
            return new StarReportData(backend.queryData(query, map));
        }
        catch (DataSourceException dse)
        {
            String message = dse.getMessage();
            if (message.length() == 0)
            {
                message = "Failed to create report data wrapper";
            }
            throw new ReportDataFactoryException(message, dse);
        }
        catch (org.jfree.report.DataSourceException e)
        {
            String message = e.getMessage();
            if (message.length() == 0)
            {
                message = "Failed to query data";
            }
            throw new ReportDataFactoryException(message);
        }
    }

    public void open()
    {
    }

    public void close()
    {
    }

    /**
     * Derives a freshly initialized report data factory, which is independend of
     * the original data factory. Opening or Closing one data factory must not
     * affect the other factories.
     *
     * @return
     */
    public ReportDataFactory derive()
    {
        try
        {
            return (ReportDataFactory) clone();
        }
        catch (CloneNotSupportedException e)
        {
            throw new IllegalStateException("Clone failed?");
        }
    }

    public Object clone() throws CloneNotSupportedException
    {
        return super.clone();
    }
}
