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

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/RtoDialog.h>

#include <QMessageBox>
#include <QLabel>

namespace OpenMS {

RtoDialog::RtoDialog(QString title, double funds, QMap<QString, ResponseTimeCosts> estCosts, QWidget * parent, Qt::WindowFlags f) : QDialog(parent, f) {
    QStringList rtos;

    setWindowTitle(title);

    mainLayout = new QVBoxLayout(this);
    losGB = new QGroupBox("Level of Service", this);
    losForm = new QGridLayout(losGB);
    losForm->addWidget(new QLabel("Response Time Objective (≤xx hrs):", this), 0, 0);
    losForm->addWidget(new QLabel("Price Quotation:", this), 1, 0);
    int row = 2, col = 1;
    foreach(QString instName, estCosts.keys()) {
        losForm->addWidget(new QLabel(instName), row, 0);
        ResponseTimeCosts rtcs = estCosts.value(instName);
        foreach(QString rto, rtcs.getRTOs()) {
            if(row == 2) {
                rtos << rto;
                losForm->addWidget(new QLabel(rto), 0, col);
            }
            losForm->addWidget(new QLabel(QString("$") + QString::number(rtcs.getCost(rto), 'g', 2)));
            col++;
        }
        row++;
    }
    mainLayout->addWidget(losGB);

    fundsGB = new QGroupBox("Customer Account", this);
    form = new QFormLayout(fundsGB);
    form->addRow(new QLabel("Available Balance:", this),
                  new QLabel(QString("$") + QString::number(funds, 'g', 2), this));
    mainLayout->addWidget(fundsGB);

    rtoFrame = new QFrame(this);
    rtoForm = new QFormLayout(rtoFrame);
    rtoCombo = new QComboBox(this);
    rtoCombo->addItems(rtos);
    rtoForm->addRow(new QLabel("Select Level of Service:", this), rtoCombo);
    mainLayout->addWidget(rtoFrame);

    btnFrame = new QFrame(this);
    moreBtn = new QPushButton("Price Quote Details...", this);
    QObject::connect(moreBtn, SIGNAL(clicked()), this, SIGNAL(moreInfo()));
    okBtn = new QPushButton("Purchase", this);
    QObject::connect(okBtn, SIGNAL(clicked()), this, SIGNAL(accept()));
    rejectBtn = new QPushButton("Cancel", this);
    QObject::connect(rejectBtn, SIGNAL(clicked()), this, SIGNAL(reject()));
    buttonLayout = new QHBoxLayout(btnFrame);
    buttonLayout->addWidget(moreBtn);
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(rejectBtn);
    mainLayout->addWidget(btnFrame);
}

#define PRICE_INFO \
"Job Pricing:\n \
============\n \
\n \
Before Job is Started\n \
---------------------\n \
\n \
1) Maximum job price estimate is quoted for scans submitted (may vary by MS type).\n \
\n \
\n \
When Job is Started\n \
-------------------\n \
\n \
2) Highest price estimate is deducted from account balance when Purchase is selected. (MS type is unverified.)\n \
\n \
3) If actual job parameters disagree with initial estimates from quotation phase, job execution may terminate (at no charge).\n \
\n \
4) MS type for scans submitted is determined, and job execution proceeds.\n \
\n \
After Job Completion\n \
--------------------\n \
\n \
5) If job completes more efficiently than planned, a partial refund is credited to customer account."

// More information on the Price Quote
void RtoDialog::moreInfo(void) {
    QMessageBox::information (this, "Price Quote Details...", PRICE_INFO, QMessageBox::Ok);
}

}
