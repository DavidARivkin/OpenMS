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

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/SftpAction.h>

#include <QStringBuilder>

#include <stdexcept>

namespace OpenMS {

const QLatin1String SftpAction::action("SFTP");

SftpAction::SftpAction(QString user, QString code, int projectID) :
    BaseAction(user, code) {

    this->projectID = projectID;
}

QString SftpAction::buildQuery() const {
    QString builder = BaseAction::buildQuery()
    % QLatin1String("Action=") % action % QChar('&')
    % QLatin1String("ID=") % QString::number(projectID);

    return builder;
}

void SftpAction::preCheck() {
    if (!isReady(action)) {
        throw std::runtime_error("Response has not been set.");
    }
}

QString SftpAction::getHost() {
    preCheck();
    return getStringAttribute("Host");
}

int SftpAction::getPort() {
    preCheck();
    return (int) getLongAttribute("Port");
}

QString SftpAction::getDirectory() {
    preCheck();
    return getStringAttribute("Directory");
}

QString SftpAction::getSftpUsername() {
    preCheck();
    return getStringAttribute("Login");
}

QString SftpAction::getSftpPassword() {
    preCheck();
    return getStringAttribute("Password");
}

QString SftpAction::getErrorMessage() {
    preCheck();
    return BaseAction::getErrorMessage();
}

int SftpAction::getErrorCode() {
    preCheck();
    return BaseAction::getErrorCode();
}
}