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

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PiVersionsAction.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/InitAction.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/SftpAction.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PrepAction.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/RunAction.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/StatusAction.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DeleteAction.h>

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

START_TEST(Parsing, "$Id$")

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(PiVersionsAction_build_query)
{

    QString user("username"),
            code("password");

    PiVersionsAction *action = new PiVersionsAction(user, code);
    cout << "Built Query:  " << action->buildQuery().toUtf8().constData() << endl;
    delete action;
}
END_SECTION

BOOST_AUTO_TEST_CASE(PiVersionsAction_response)
{
    QString user("username"),
            code("password");

    QString response;
    PiVersionsAction *action = new PiVersionsAction(user, code);
    action->processResponse(response);
    TEST_POSTCONDITION_VIOLATED (!action->hasError())
    cout << "Current Version:  " << action->getCurrentVersion().toUtf8().constData()
         << "Last Used Version:  " << action->getLastUsedVersion().toUtf8().constData()
         << endl << "Available Versions:  ";

    foreach(QString ver, action->getVersions()) {
        cout << ver.toUtf8().constData() << ", ";
    }
    cout << endl;
    delete action;
}
END_SECTION

BOOST_AUTO_TEST_CASE(InitAction_build_query)
{

    QString user("username"),
            code("password"),
            versionOfPi("1.2");
    int ID(1234), scanCount(100), maxPoints(1000), minMass(10), maxMass(750), calibrationCount(100);

    InitAction *action = new InitAction(user, code,
                                        ID, versionOfPi, scanCount,
                                        maxPoints, minMass, maxMass, calibrationCount);
    cout << "Built Query:  " << action->buildQuery().toUtf8().constData() << endl;
}
END_SECTION

BOOST_AUTO_TEST_CASE(InitAction_response)
{

    QString user("username"),
            code("password"),
            versionOfPi("1.2");
    int ID(1234), scanCount(100), maxPoints(1000), minMass(10), maxMass(750), calibrationCount(100);

    QString response;
    InitAction *action = new InitAction(user, code,
                                        ID, versionOfPi, scanCount,
                                        maxPoints, minMass, maxMass, calibrationCount);
    action->processResponse(response);
    TEST_POSTCONDITION_VIOLATED (!action->hasError())
    cout << "Job:  " << action->getJob().toUtf8().constData()
         << "Project ID:  " << QString::number(action->getProjectId()).toUtf8().constData();
    QMap<QString, ResponseTimeCosts> RTOMap = action->getEstimatedCosts();
    foreach(QString inst, RTOMap.keys()) {
        cout << "Instrument:  " << inst.toUtf8().constData() << endl;
        foreach(ResponseTimeCosts costs, RTOMap) {
            foreach(QString rto, costs.getRTOs()) {
                cout << " RTO:  " << rto.toUtf8().constData() << " Cost:  " << QString::number(costs.getCost(rto)).toUtf8().constData() << endl;
            }
        }
        cout << endl;
    }
    delete action;
}
END_SECTION

BOOST_AUTO_TEST_CASE(SftpAction_build_query)
{

    QString user("username"),
            code("password");
    int projectID(1234);

    SftpAction *action = new SftpAction(user, code,
                                        projectID);
    cout << "Built Query:  " << action->buildQuery().toUtf8().constData() << endl;
}
END_SECTION

BOOST_AUTO_TEST_CASE(SftpAction_response)
{

    QString user("username"),
            code("password");
    int projectID(1234);
    QString response;
    SftpAction *action = new SftpAction(user, code,
                                        projectID);
    action->processResponse(response);
    TEST_POSTCONDITION_VIOLATED (!action->hasError())
    cout << "Host:  " << action->getHost().toUtf8().constData()
         << "Port:  " << QString::number(action->getPort()).toUtf8().constData()
         << "Director:  " << action->getDirectory().toUtf8().constData()
         << "Username:  " << action->getSftpUsername().toUtf8().constData()
         << "Password:  " << action->getSftpPassword().toUtf8().constData();

    delete action;
}
END_SECTION

BOOST_AUTO_TEST_CASE(PrepAction_build_query)
{

    QString user("username"),
            code("password"),
            filename("test.tar");
    int projectID(1234);

    PrepAction *action = new PrepAction(user, code,
                                        projectID, filename);
    cout << "Built Query:  " << action->buildQuery().toUtf8().constData() << endl;
}
END_SECTION

BOOST_AUTO_TEST_CASE(PrepAction_response)
{
    QString user("username"),
            code("password"),
            filename("test.tar");
    int projectID(1234);

    QString response;
    PrepAction *action = new PrepAction(user, code,
                                        projectID, filename);
    action->processResponse(response);
    TEST_POSTCONDITION_VIOLATED (!action->hasError())
    cout << "Filename:  " << action->getFilename().toUtf8().constData();
    switch(action->getStatus()) {
    case PrepAction::Analyzing :
        cout << "Analyzing! % Complete:" << action->getPercentComplete().toUtf8().constData() << endl;
        break;
    case PrepAction::Ready :
        cout << "Ready!  Scan Count:  " << QString::number(action->getScanCount()).toUtf8().constData()
             << "MS Type:  " << action->getMStype().toUtf8().constData() << endl;
        break;
    case PrepAction::Error :
        cout << "Error!  " << action->getErrorMessage().toUtf8().constData() << endl;
        break;
    }

    delete action;
}
END_SECTION

BOOST_AUTO_TEST_CASE(RunAction_build_query)
{

    QString user("username"),
            code("password"),
            job("P1234-1234"),
            RTO("RTO-24"),
            inputFilename("P1234-1234.tar"),
            calibrationFilename("P1234-1234_calib.tar");

    RunAction *action = new RunAction(user, code,
                                      job, RTO, inputFilename, calibrationFilename);
    cout << "Built Query:  " << action->buildQuery().toUtf8().constData() << endl;
}
END_SECTION

BOOST_AUTO_TEST_CASE(RunAction_response)
{
    QString user("username"),
            code("password"),
            job("P1234-1234"),
            RTO("RTO-24"),
            inputFilename("P1234-1234.tar"),
            calibrationFilename("P1234-1234_calib.tar");

    QString response;
    RunAction *action = new RunAction(user, code,
                                      job, RTO, inputFilename, calibrationFilename);
    action->processResponse(response);
    TEST_POSTCONDITION_VIOLATED (!action->hasError())
    cout << "Job:  " << action->getJob().toUtf8().constData();

    delete action;
}
END_SECTION

BOOST_AUTO_TEST_CASE(StatusAction_build_query)
{

    QString user("username"),
            code("password"),
            jobID("P1234-1234");

    StatusAction *action = new StatusAction(user, code,
                                            jobID);
    cout << "Built Query:  " << action->buildQuery().toUtf8().constData() << endl;
}
END_SECTION

BOOST_AUTO_TEST_CASE(StatusAction_response)
{
    QString user("username"),
            code("password"),
            jobID("P1234-1234");

    QString response;
    StatusAction *action = new StatusAction( user, code,
                                            jobID);
    action->processResponse(response);
    TEST_POSTCONDITION_VIOLATED (!action->hasError())
    switch(action->getStatus()) {
    case StatusAction::Preparing :
        cout << "Status:  Preparing." << endl;
        break;
    case StatusAction::Running :
        cout << "Status:  Running!" << endl;
        break;
    case StatusAction::Deleted:
        cout << "Status:  Deleted!" << endl;
        break;
    case StatusAction::Done:
        cout << "Status:  Done!" << endl;
        cout << "Input Scans:  " << QString::number(action->getNumberOfInputScans()).toUtf8().constData()
             << "Complete Scans:  " << QString::number(action->getNumberOfCompleteScans()).toUtf8().constData()
             << "Actual Cost:  " << QString::number(action->getActualCost()).toUtf8().constData()
             << "Log filename:  " << action->getLogFilename().toUtf8().constData()
             << "Results filename:  " << action->getResultsFilename().toUtf8().constData();
    }
    delete action;
}
END_SECTION

BOOST_AUTO_TEST_CASE(DeleteAction_build_query)
{

    QString user("username"),
            code("password"),
            jobID("P1234-1234");

    DeleteAction *action = new DeleteAction(user, code,
                                            jobID);
    cout << "Built Query:  " << action->buildQuery().toUtf8().constData() << endl;
}
END_SECTION

BOOST_AUTO_TEST_CASE(DeleteAction_response)
{

    QString response;
    QString user("username"),
            code("password"),
            jobID("P1234-1234");
    DeleteAction *action = new DeleteAction(user, code,
                                            jobID);
    action->processResponse(response);
    TEST_POSTCONDITION_VIOLATED (!action->hasError())

    cout << "Job:  " << action->getJob().toUtf8().constData()
         << "Date:  " << action->getDate().toString().toUtf8().constData();
    delete action;
}
END_SECTION

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
END_TEST
