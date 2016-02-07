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

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PrepAction.h>

#include <QStringBuilder>

#include <stdexcept>

namespace OpenMS {

const QLatin1String PrepAction::action("PREP");

PrepAction::PrepAction(QString versionOfApi, QString user, QString code, int projectID, QString filename) :
    BaseAction(versionOfApi, user, code)
{

    this->projectID = projectID;
    this->filename = filename;

}

QString PrepAction::buildQuery() {
    QString builder = BaseAction::buildQuery()
    % QLatin1String("Action=") % action + QChar('&')
    % QLatin1String("ID=") % QString::number(projectID) % QChar('&')
    % QLatin1String("File=") % filename;

    return builder;
}

void PrepAction::preCheck(){
    if (!isReady(action)) {
        throw std::runtime_error("Response has not been set.");
    }
}

QString PrepAction::getFilename() {
    preCheck();
    return getStringAttribute("File");
}

PrepAction::Status PrepAction::getStatus() {
    preCheck();
    QString status = getStringAttribute("Status");
    if(status == "Analyszing")
        return Analyzing;
    else if(status == "Ready")
        return Ready;
    else if(status == "Error")
        return Error;
    else
        throw std::runtime_error("Invalid status received.");
}

int PrepAction::getScanCount() {
    preCheck();
    return (int) getLongAttribute("ScanCount");
}

QString PrepAction::getPercentComplete() {
    preCheck();
    return getStringAttribute("PercentComplete");
};

QString PrepAction::getMStype() {
    preCheck();
    return getStringAttribute("MSType");
}

QString PrepAction::getErrorMessage() {
    preCheck();
    return BaseAction::getErrorMessage();
}

int PrepAction::getErrorCode() {
    preCheck();
    return BaseAction::getErrorCode();
}

}
