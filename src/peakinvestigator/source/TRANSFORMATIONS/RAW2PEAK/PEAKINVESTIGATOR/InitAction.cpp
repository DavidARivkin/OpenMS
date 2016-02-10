// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2013.
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
// $Maintainer:$
// $Author: David Rivkin $
// --------------------------------------------------------------------------
//

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/InitAction.h>

#include <QStringBuilder>
#include <QMap>
#include <QVariant>

#include <stdexcept>

namespace OpenMS {

const long ResponseTimeCosts::serialVersionUID = 1L;

const QLatin1String InitAction::action("INIT");

InitAction::InitAction(QString versionOfApi, QString user, QString code, int ID, QString versionOfPi, int scanCount,
                       int maxPoints, int minMass, int maxMass, int calibrationCount) :
    BaseAction(versionOfApi, user, code)
{

     this->ID = ID;
     this->versionOfPi = versionOfPi;
     this->scanCount = scanCount;
     this->maxPoints = maxPoints;
     this->minMass = minMass;
     this->maxMass = maxMass;
     this->calibrationCount = calibrationCount;
}

QString InitAction::buildQuery() {
    QString builder = BaseAction::buildQuery()
    % QLatin1String("Action=") % action % QChar('&')
    % QLatin1String("ID=") % QString::number(ID) % QChar('&')
    % QLatin1String("PI_Version=") % versionOfPi % QChar('&')
    % QLatin1String("ScanCount=") % QString::number(scanCount) % QChar('&')
    % QLatin1String("MaxPoints=") % QString::number(maxPoints) % QChar('&')
    % QLatin1String("MinMass=") % QString::number(minMass) % QChar('&')
    % QLatin1String("MaxMass=") % QString::number(maxMass) % QChar('&')
    % QLatin1String("CalibrationCount=") % QString::number(calibrationCount);

    return builder;
}

void InitAction::preCheck() {
    if (!isReady(action)) {
        throw std::runtime_error("Failed Pre-Check");
    }
}

QString InitAction::getJob() {
    preCheck();
    return getStringAttribute("Job");
}

long InitAction::getProjectId() {
    preCheck();
    return getLongAttribute("ProjectID");
}

double InitAction::getFunds() {
    preCheck();
    return getDoubleAttribute("Funds");
}

QMap<QString, ResponseTimeCosts> InitAction::getEstimatedCosts() {
    preCheck();

    QMap<QString, ResponseTimeCosts> estimatedCosts;
    QVariantMap Insts = responseObject.value("EstimatedCost").toMap();
    foreach (QString stringInst, Insts.keys()) {
        ResponseTimeCosts costs;
        QVariantMap RTOs = Insts.value(stringInst).toMap();
        foreach (QString stringRTO, RTOs.keys()) {
            costs.insert(stringRTO, RTOs.value(stringRTO).toDouble());
        }
        estimatedCosts.insert(stringInst, costs);
    }

    return estimatedCosts;
}

QString InitAction::getErrorMessage() {
    preCheck();
    return BaseAction::getErrorMessage();
}

int InitAction::getErrorCode() {
    preCheck();
    return BaseAction::getErrorCode();
}

}

