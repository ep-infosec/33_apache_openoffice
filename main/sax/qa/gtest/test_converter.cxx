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



#include "preextstl.h"
#include "gtest/gtest.h"
#include "postextstl.h"

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Duration.hpp>

#include "sax/tools/converter.hxx"


using namespace ::com::sun::star;
using sax::Converter;


namespace {

class ConverterTest
    : public ::testing::Test
{
public:
    virtual void SetUp();
    virtual void TearDown();
};

void ConverterTest::SetUp()
{
}

void ConverterTest::TearDown()
{
}

static bool eqDuration(util::Duration a, util::Duration b) {
    return a.Years == b.Years && a.Months == b.Months && a.Days == b.Days
        && a.Hours == b.Hours && a.Minutes == b.Minutes
        && a.Seconds == b.Seconds
        && a.MilliSeconds == b.MilliSeconds
        && a.Negative == b.Negative;
}

static void doTest(util::Duration const & rid, char const*const pis,
        char const*const i_pos = 0)
{
    char const*const pos((i_pos) ? i_pos : pis);
    util::Duration od;
    ::rtl::OUString is(::rtl::OUString::createFromAscii(pis));
    bool bSuccess = Converter::convertDuration(od, is);
    OSL_TRACE("%d %dY %dM %dD %dH %dM %dS %dm",
        od.Negative, od.Years, od.Months, od.Days,
        od.Hours, od.Minutes, od.Seconds, od.MilliSeconds);
    ASSERT_TRUE(bSuccess);
    ASSERT_TRUE(eqDuration(rid, od));
    ::rtl::OUStringBuffer buf;
    Converter::convertDuration(buf, od);
    OSL_TRACE(
        ::rtl::OUStringToOString(buf.getStr(), RTL_TEXTENCODING_UTF8));
    ASSERT_TRUE(buf.makeStringAndClear().equalsAscii(pos));
}

static void doTestDurationF(char const*const pis)
{
    util::Duration od;
    bool bSuccess = Converter::convertDuration(od,
            ::rtl::OUString::createFromAscii(pis));
    OSL_TRACE("%d %dY %dM %dD %dH %dM %dS %dH",
        od.Negative, od.Years, od.Months, od.Days,
        od.Hours, od.Minutes, od.Seconds, od.MilliSeconds);
    ASSERT_TRUE(!bSuccess);
}

TEST_F(ConverterTest, testDuration)
{
    OSL_TRACE("\nSAX CONVERTER TEST BEGIN\n");
    doTest( util::Duration(false, 1, 0, 0, 0, 0, 0, 0), "P1Y" );
    doTest( util::Duration(false, 0, 42, 0, 0, 0, 0, 0), "P42M" );
    doTest( util::Duration(false, 0, 0, 111, 0, 0, 0, 0), "P111D" );
    doTest( util::Duration(false, 0, 0, 0, 52, 0, 0, 0), "PT52H" );
    doTest( util::Duration(false, 0, 0, 0, 0, 717, 0, 0), "PT717M" );
    doTest( util::Duration(false, 0, 0, 0, 0, 0, 121, 0), "PT121S" );
    doTest( util::Duration(false, 0, 0, 0, 0, 0, 0, 190), "PT0.19S" );
    doTest( util::Duration(false, 0, 0, 0, 0, 0, 0, 90), "PT0.09S" );
    doTest( util::Duration(false, 0, 0, 0, 0, 0, 0, 9), "PT0.009S" );
    doTest( util::Duration(false, 0, 0, 0, 0, 0, 9, 999),
            "PT9.999999999999999999999999999999S", "PT9.999S" );
    doTest( util::Duration(true , 0, 0, 9999, 0, 0, 0, 0), "-P9999D" );
    doTest( util::Duration(true , 7, 6, 5, 4, 3, 2, 10),
            "-P7Y6M5DT4H3M2.01S" );
    doTest( util::Duration(false, 0, 6, 0, 0, 3, 0, 0), "P6MT3M" );
    doTest( util::Duration(false, 0, 0, 0, 0, 0, 0, 0), "P0D" );
    doTestDurationF("1Y1M");        // invalid: no ^P
    doTestDurationF("P-1Y1M");      // invalid: - after P
    doTestDurationF("P1M1Y");       // invalid: Y after M
    doTestDurationF("PT1Y");        // invalid: Y after T
    doTestDurationF("P1Y1M1M");     // invalid: M twice, no T
    doTestDurationF("P1YT1MT1M");   // invalid: T twice
    doTestDurationF("P1YT");        // invalid: T but no H,M,S
    doTestDurationF("P99999999999Y");   // cannot parse so many Ys
    doTestDurationF("PT.1S");       // invalid: no 0 preceding .
    doTestDurationF("PT5M.134S");   // invalid: no 0 preceding .
    doTestDurationF("PT1.S");       // invalid: no digit following .
    OSL_TRACE("\nSAX CONVERTER TEST END\n");
}


static bool eqDateTime(util::DateTime a, util::DateTime b) {
    return a.Year == b.Year && a.Month == b.Month && a.Day == b.Day
        && a.Hours == b.Hours && a.Minutes == b.Minutes
        && a.Seconds == b.Seconds
        && a.HundredthSeconds == b.HundredthSeconds;
}

static void doTest(util::DateTime const & rdt, char const*const pis,
        char const*const i_pos = 0)
{
    char const*const pos((i_pos) ? i_pos : pis);
    ::rtl::OUString is(::rtl::OUString::createFromAscii(pis));
    util::DateTime odt;
    bool bSuccess( Converter::convertDateTime(odt, is) );
    OSL_TRACE("Y:%d M:%d D:%d  H:%d M:%d S:%d H:%d",
        odt.Year, odt.Month, odt.Day,
        odt.Hours, odt.Minutes, odt.Seconds, odt.HundredthSeconds);
    ASSERT_TRUE(bSuccess);
    ASSERT_TRUE(eqDateTime(rdt, odt));
    ::rtl::OUStringBuffer buf;
    Converter::convertDateTime(buf, odt, true);
    OSL_TRACE(
        ::rtl::OUStringToOString(buf.getStr(), RTL_TEXTENCODING_UTF8));
    ASSERT_TRUE(buf.makeStringAndClear().equalsAscii(pos));
}

static void doTestDateTimeF(char const*const pis)
{
    util::DateTime odt;
    bool bSuccess = Converter::convertDateTime(odt,
            ::rtl::OUString::createFromAscii(pis));
    OSL_TRACE("Y:%d M:%d D:%d  H:%dH M:%d S:%d H:%d",
        odt.Year, odt.Month, odt.Day,
        odt.Hours, odt.Minutes, odt.Seconds, odt.HundredthSeconds);
    ASSERT_TRUE(!bSuccess);
}

TEST_F(ConverterTest, testDateTime)
{
    OSL_TRACE("\nSAX CONVERTER TEST BEGIN\n");
    doTest( util::DateTime(0, 0, 0, 0, 1, 1, 1), "0001-01-01T00:00:00" );
    doTest( util::DateTime(0, 0, 0, 0, 1, 1, 1),
            "0001-01-01T00:00:00Z", "0001-01-01T00:00:00" );
//    doTest( util::DateTime(0, 0, 0, 0, 1, 1, -1), "-0001-01-01T00:00:00" );
//    doTest( util::DateTime(0, 0, 0, 0, 1, 1, -1), "-0001-01-01T00:00:00Z" );
    doTest( util::DateTime(0, 0, 0, 0, 1, 1, 1),
            "0001-01-01T00:00:00-00:00", "0001-01-01T00:00:00" );
    doTest( util::DateTime(0, 0, 0, 0, 1, 1, 1),
            "0001-01-01T00:00:00+00:00", "0001-01-01T00:00:00" );
    doTest( util::DateTime(0, 0, 0, 0, 2, 1, 1)/*(0, 0, 12, 0, 2, 1, 1)*/,
            "0001-01-02T00:00:00-12:00", "0001-01-02T00:00:00" );
//            "0001-02-01T12:00:00" );
    doTest( util::DateTime(0, 0, 0, 0, 2, 1, 1)/*(0, 0, 12, 0, 1, 1, 1)*/,
            "0001-01-02T00:00:00+12:00", "0001-01-02T00:00:00" );
//            "0001-01-01T12:00:00" );
    doTest( util::DateTime(99, 59, 59, 23, 31, 12, 9999),
            "9999-12-31T23:59:59.99" );
    doTest( util::DateTime(99, 59, 59, 23, 31, 12, 9999),
            "9999-12-31T23:59:59.99Z", "9999-12-31T23:59:59.99" );
    doTest( util::DateTime(99, 59, 59, 23, 31, 12, 9999),
            "9999-12-31T23:59:59.9999999999999999999999999999999999999",
            "9999-12-31T23:59:59.99" );
    doTest( util::DateTime(99, 59, 59, 23, 31, 12, 9999),
            "9999-12-31T23:59:59.9999999999999999999999999999999999999Z",
            "9999-12-31T23:59:59.99" );
    doTest( util::DateTime(0, 0, 0, 24, 1, 1, 333)
                /*(0, 0, 0, 0, 2, 1, 333)*/,
            "0333-01-01T24:00:00"/*, "0333-01-02T00:00:00"*/ );
    doTestDateTimeF( "+0001-01-01T00:00:00" ); // invalid: ^+
    doTestDateTimeF( "1-01-01T00:00:00" ); // invalid: < 4 Y
    doTestDateTimeF( "0001-1-01T00:00:00" ); // invalid: < 2 M
    doTestDateTimeF( "0001-01-1T00:00:00" ); // invalid: < 2 D
    doTestDateTimeF( "0001-01-01T0:00:00" ); // invalid: < 2 H
    doTestDateTimeF( "0001-01-01T00:0:00" ); // invalid: < 2 M
    doTestDateTimeF( "0001-01-01T00:00:0" ); // invalid: < 2 S
    doTestDateTimeF( "0001-01-01T00:00:00." ); // invalid: .$
    doTestDateTimeF( "0001-01-01T00:00:00+1:00" ); // invalid: < 2 TZ H
    doTestDateTimeF( "0001-01-01T00:00:00+00:1" ); // invalid: < 2 TZ M
    doTestDateTimeF( "0001-13-01T00:00:00" ); // invalid: M > 12
    doTestDateTimeF( "0001-01-32T00:00:00" ); // invalid: D > 31
    doTestDateTimeF( "0001-01-01T25:00:00" ); // invalid: H > 24
    doTestDateTimeF( "0001-01-01T00:60:00" ); // invalid: H > 59
    doTestDateTimeF( "0001-01-01T00:00:60" ); // invalid: S > 59
    doTestDateTimeF( "0001-01-01T24:01:00" ); // invalid: H=24, but M != 0
    doTestDateTimeF( "0001-01-01T24:00:01" ); // invalid: H=24, but S != 0
    doTestDateTimeF( "0001-01-01T24:00:00.1" ); // invalid: H=24, but H != 0
    doTestDateTimeF( "0001-01-02T00:00:00+15:00" ); // invalid: TZ > +14:00
    doTestDateTimeF( "0001-01-02T00:00:00+14:01" ); // invalid: TZ > +14:00
    doTestDateTimeF( "0001-01-02T00:00:00-15:00" ); // invalid: TZ < -14:00
    doTestDateTimeF( "0001-01-02T00:00:00-14:01" ); // invalid: TZ < -14:00
    OSL_TRACE("\nSAX CONVERTER TEST END\n");
}


}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
