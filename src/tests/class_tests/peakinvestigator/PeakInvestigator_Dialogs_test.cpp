// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry               
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2015.
// 
// This software is released under a three-clause BSD license:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of any author or any participating institution 
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.
// For a full list of authors, refer to the file AUTHORS. 
// --------------------------------------------------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING 
// INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// --------------------------------------------------------------------------
// $Maintainer: David Rivkin $
// $Authors: David Rivkin $
// --------------------------------------------------------------------------

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/RtoDialog.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/VersionDialog.h>
#include <QApplication>

#ifdef USE_BOOST_UNIT_TEST

// include boost unit test framework
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>
// macros for boost
#define EPS_05 boost::test_tools::fraction_tolerance(1.e-5)
#define TEST_REAL_SIMILAR(val1, val2) \
    BOOST_CHECK ( boost::test_tools::check_is_close(val1, val2, EPS_05 ));
#define TEST_EQUAL(val1, val2) BOOST_CHECK_EQUAL(val1, val2);
#define END_SECTION
#define START_TEST(var1, var2)
#define END_TEST

#else

#include <OpenMS/CONCEPT/ClassTest.h>
#define BOOST_AUTO_TEST_CASE START_SECTION
using namespace OpenMS;

#endif

using namespace std;

///////////////////////////

#define USERNAME "david.rivkin@gcpintl.com"
#define PASSWORD ""

///////////////////////////
START_TEST(Parsing, "$Id$")

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(VersionsDialog_display)
{

    QApplication app(argc, argv);

    QString title("Peak Investigator");
    QStringList versions;
    versions << "1.1" << "1.1.1" << "1.2";
    VersionDialog dlg(title, versions, "1.2", "1.2", 0, 200);
    bool ok  = dlg.exec();

    TEST_EQUAL(ok, true)
    TEST_EQUAL(dlg.version().compare("1.2"), 0)
    TEST_EQUAL(dlg.minMass(), 0)
    TEST_EQUAL(dlg.maxMass(), 1000)
    cout << endl << "Selected Version:  " << dlg.version().toUtf8().constData()
         << "  MinMass:  " << QString::number(dlg.minMass()).toUtf8().constData()
         << "  MaxMass:  " << QString::number(dlg.maxMass()).toUtf8().constData()
         << endl;

}
END_SECTION

BOOST_AUTO_TEST_CASE(RtoDialog_display)
{
    QApplication app(argc, argv);

    QString title("Peak Investigator 1.2");
    QMap<QString, ResponseTimeCosts>  estCosts;
    ResponseTimeCosts rto;
    rto.insert("RTO-0", 1.23);
    rto.insert("RTO-24", 45.67);
    estCosts.insert("TOF", rto);
    rto.insert("RTO-0", 8.90);
    rto.insert("RTO-24", 123.45);
    estCosts.insert("Ion Trap", rto);
    rto.insert("RTO-0", 6.78);
    rto.insert("RTO-24", 901.23);
    estCosts.insert("Orbitrap", rto);

    RtoDialog dlg(title, 1234.56, estCosts);

    bool ok = dlg.exec();
    TEST_EQUAL(ok, true)

    TEST_EQUAL(dlg.getRto().compare("RTO-24"), 0)
    cout << endl << "RTO selected:  " << dlg.getRto().toUtf8().constData()
         << endl;
}
END_SECTION

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
END_TEST
