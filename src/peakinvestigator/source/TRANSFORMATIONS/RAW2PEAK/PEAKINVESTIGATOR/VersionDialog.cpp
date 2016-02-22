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

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/VersionDialog.h>

#include <QLabel>

namespace OpenMS {
VersionDialog::VersionDialog(QString title, QStringList versions, QString lastUsedVersion, QString currentVersion, int minMass, int maxMass, QWidget * parent, Qt::WindowFlags f) :
    QDialog(parent, f)
{
    setWindowTitle(title);
    mainLayout = new QVBoxLayout(this);
    verGB = new QGroupBox("Versions",this);
    verForm = new QFormLayout(verGB);
    verSelect = new QComboBox(this);
    verSelect->addItems(versions);
    int ci = versions.indexOf(lastUsedVersion.isEmpty() ? currentVersion : lastUsedVersion);
    verSelect->setCurrentIndex(ci);
    verForm->addRow(new QLabel("Version:", this), verSelect);
    mainLayout->addWidget(verGB);
    QGroupBox *massGB = new QGroupBox("Masses", this);
    QFormLayout *form = new QFormLayout(massGB);
    maxEdit = new QSpinBox(this);
    maxEdit->setRange(minMass+1, maxMass);
    maxEdit->setValue(maxMass);
    QObject::connect(maxEdit, SIGNAL(valueChanged(int)), this, SLOT(newMax(int)));
    minEdit = new QSpinBox(this);
    minEdit->setRange(minMass, maxMass-1);
    minEdit->setValue(minMass);
    QObject::connect(minEdit, SIGNAL(valueChanged(int)), this, SLOT(newMin(int)));
    form->addRow(new QLabel("Maximum Mass:", this), maxEdit);
    form->addRow(new QLabel("Minimum Mass:", this), minEdit);
    mainLayout->addWidget(massGB);
    QFrame *btnFrame = new QFrame(this);
    QPushButton *okBtn = new QPushButton("OK", this);
    QObject::connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
    QPushButton *rejectBtn = new QPushButton("Cancel", this);
    QObject::connect(rejectBtn, SIGNAL(clicked()), this, SLOT(reject()));
    QHBoxLayout *buttonLayout = new QHBoxLayout(btnFrame);
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(rejectBtn);
    mainLayout->addWidget(btnFrame);

}

void VersionDialog::newMax(int value) {
    if(minEdit->value() >= value)
        minEdit->setValue(value-1);
    minEdit->setMaximum(value-1);
}

void VersionDialog::newMin(int value) {
    if(maxEdit->value() <= value)
        maxEdit->setValue(value+1);
    maxEdit->setMinimum(value+1);
}

}
