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
// $Author: Adam Tenderholt $
// --------------------------------------------------------------------------
//

#include <fcntl.h> // used for SFTP transfers
#include <zlib.h> // used for g'zipping tar files

#include <OpenMS/FORMAT/PeakTypeEstimator.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PeakInvestigator.h>

#include <QtCore/QBuffer>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QEventLoop>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QProcess>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QtDebug>
#include <QtCore/QThread>
#include <QtCore/qglobal.h>

// JSON support
#include <qjson/parser.h>
#ifdef WITH_GUI
#include <QtGui/QDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QFormLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QFrame>
#endif

#define VI_API_SUFFIX "/api/"
#define VI_SSH_HASH String("Hash seed!")
#define reqVeritomyxCLIVersion String("2.12")
#define minutesCheckPrep 2
#define minutesTimeoutPrep 20

using namespace std;

namespace OpenMS
{
  PeakInvestigator::PeakInvestigator(QObject* parent) :
    QObject(parent),
    DefaultParamHandler("PeakInvestigator"),
    ProgressLogger()
  {
    // set default parameter values
    defaults_.setValue("server", "peakinvestigator.veritomyx.com", "Server address for PeakInvestigator (without https://)");
    defaults_.setValue("username", "USERNAME", "Username for account registered with Veritomyx");
    defaults_.setValue("password", "PASSWORD", "Password for account registered with Veritomyx");
    defaults_.setValue("account", "0", "Account number");

    defaults_.setValue("MinMass", "0", "Minimum mass to use");
    defaults_.setValue("MaxMass", QString::number(2^32), "Maximum mass to use");

#ifndef WITH_GUI
    defaults_.setValue("RTO", "RTO-24", "Response Time Objective to use");
    defaults_.setValue("PIVersion", "1.0.1", "Version of Peak Investigator to use");
#endif

    // write defaults into Param object param_
    defaultsToParam_();
    updateMembers_();

  }

  PeakInvestigator::~PeakInvestigator()
  {
  }

  void PeakInvestigator::run()
  {

    // filenames for the tar'd scans/results
    QString zipfilename;
    String localFilename;
    String remoteFilename;

    switch(mode_)
    {

    case SUBMIT:
      if (!initializeJob_())
      {
        break;
      }

      // Generate local and remote filenames of tar'd scans
      zipfilename = job_ + ".scans.tar";
      localFilename = QDir::tempPath() + "/" + zipfilename;
      remoteFilename = sftp_dir_ + "/" + zipfilename;
      tar.store(localFilename, experiment_);

      // Remove data values from scans in exp now that they have been bundled
      for (Size i = 0; i < experiment_.size(); i++)
      {
        experiment_[i].clear(false);
      }

      if(getSFTPCredentials_()) {

          // Set SFTP host paramters and upload file
          sftp.setHostname(server_);
          sftp.setUsername(sftp_username_);
          sftp.setPassword(sftp_password_);
          sftp.setExpectedServerHash(VI_SSH_HASH);

         if(sftp.uploadFile(localFilename, remoteFilename) && submitJob_())
         {
             // Do PREP
            long timeWait = minutesTimeoutPrep;
            while((getPrepFileMessage_() == PREP_ANALYZING) && timeWait > 0) {
                LOG_INFO << "Waiting for PREP analysis to complete, " << localFilename << ", on SaaS server...Please be patient.";
                QThread::currentThread()->wait(minutesCheckPrep * 60000);
                timeWait -= minutesCheckPrep;
            }
            // TODO:  If we timed out, report and error
         }
      }
      break;

    case CHECK:
      checkJob_();
      break;

    case DELETE:
      removeJob_();
      break;

    case FETCH:

      if(!getSFTPCredentials_())
      {
          break;
      }
      if(!checkJob_()) // Seems we need to check STATUS before file is moved to SFTP drop after completion
      {
        break;
      }

      // Set SFTP host paramters and upload file
      sftp.setHostname(sftp_host_);
      sftp.setPortnumber(sftp_port_);
      sftp.setUsername(sftp_username_);
      sftp.setPassword(sftp_password_);
      sftp.setExpectedServerHash(VI_SSH_HASH);

      // Generate local and remote filenames of tar'd scans
      sftp_file_ = zipfilename = results_file_;
      localFilename = QDir::tempPath() + "/" + zipfilename;
      remoteFilename = sftp_dir_ + "/" + account_number_.toQString() + "/" + zipfilename;

      if (!sftp.downloadFile(remoteFilename, localFilename))
      {
        break;
      }

      tar.load(localFilename, experiment_);

      // Set-up data processing meta data to add to each scan
      boost::shared_ptr<DataProcessing> dp(new DataProcessing());
      std::set<DataProcessing::ProcessingAction> actions;
      actions.insert(DataProcessing::PEAK_PICKING);
      dp->setProcessingActions(actions);
      dp->getSoftware().setName("PeakInvestigator");
      dp->setCompletionTime(DateTime::now());
      dp->setMetaValue("paramter: veritomyx:server", server_);
      dp->setMetaValue("paramter: veritomyx:username", username_);
      dp->setMetaValue("parameter: veritomyx:account", account_number_);
      dp->setMetaValue("veritomyx:job", job_);

#ifndef WITH_GUI
      dp->setMetaValue("veritomyx:RTO", RTO_);
      dp->setMetaValue("veritomyx:PIVersion", PIVersion_);
#endif

      // Now add meta data to the scans
      for (Size i = 0; i < experiment_.size(); i++)
      {
        experiment_[i].getDataProcessing().push_back(dp);
        experiment_[i].setType(SpectrumSettings::PEAKS);
      }

      removeJob_();
        break;

    } //end switch

    shutdown();

  }

  bool PeakInvestigator::setExperiment(MSExperiment<Peak1D>& experiment)
  {
    if (experiment.empty())
    {
      LOG_ERROR << "The given file appears to not contain any m/z-intensity data points.";
      return false;
    }

    //check for peak type (profile data required)
    if (PeakTypeEstimator().estimateType(experiment[0].begin(), experiment[0].end()) == SpectrumSettings::PEAKS)
    {
      LOG_ERROR << "OpenMS peak type estimation indicates that this is not profile data!";
      return false;
    }

    experiment_ = experiment;
    return true;
  }

  bool PeakInvestigator::initializeJob_()
  {
    LOG_DEBUG << "Requsting credentials for " + username_ + "..." << endl;

    url_.setUrl("https://" + server_.toQString() + VI_API_SUFFIX);

    Size minMass = 0, maxMass = 0;
    for (Size i = 0; i < experiment_.size(); i++)
    {
      maxMass = qMax(maxMass, experiment_[i].size());
    }
#ifdef WITH_GUI
// Ask the user for a min and max value
    QDialog *massDlg = new QDialog();
    massDlg->setWindowTitle("Peak Investigator Job");
    QVBoxLayout *mainLayout = new QVBoxLayout(massDlg);
    QFrame *formFrame = new QFrame(massDlg);
    QFormLayout *form = new QFormLayout(formFrame);
    QLabel *maxLabel = new QLabel("Maximum Mass:", massDlg);
    QLabel *minLabel = new QLabel("Minimum Mass:", massDlg);
    QLineEdit *maxEdit = new QLineEdit(QString::number(maxMass), massDlg);
    QLineEdit *minEdit = new QLineEdit(QString::number(minMass), massDlg);
    form->addRow(maxLabel, maxEdit);
    form->addRow(minLabel, minEdit);
    mainLayout->addWidget(formFrame);
    QFrame *btnFrame = new QFrame(massDlg);
    QPushButton *okBtn = new QPushButton("Accept", massDlg);
    QObject::connect(okBtn, SIGNAL(clicked()), massDlg, SIGNAL(accept()));
    QPushButton *rejectBtn = new QPushButton("Reject", massDlg);
    QObject::connect(rejectBtn, SIGNAL(clicked()), massDlg, SIGNAL(reject()));
    QHBoxLayout *buttonLayout = new QHBoxLayout(btnFrame);
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(rejectBtn);
    mainLayout->addWidget(btnFrame);
    if(massDlg->exec() == QDialog::Accepted) {
        uint xmass = maxEdit->text().toUInt();
        if(xmass > maxMass) {
            LOG_ERROR << "The Maximum Mass must be less than " <<  maxMass;
            return false;
        } else {
            maxMass = xmass;
        }
        xmass = minEdit->text().toUInt();
        if(xmass > maxMass) {
            LOG_ERROR << "The Minimum Mass must be less than the Maximum Mass";
            return false;
        } else {
            minMass = xmass;
        }
    } else {
        return false;
    }
#else
    dp.getMetaValue("veritomyx:MinMass", minMass);
    dp.getMetaValue("veritomyx:MaxMass", maxMass);
#endif

    QString params = QString("Version=" + reqVeritomyxCLIVersion.toQString()); // online CLI version that matches this interface
    params += "&User="	+ username_.toQString() +
            "&Code="    + password_.toQString() +
            "&Action="  + "INIT" +
            "&ID=" + account_number_.toQString() +
            "&ScanCount=" + experiment_.size() +
//		",\"CalibrationCount\": " + calibrationCount + "\"" +
            "&MinMass=" + minMass +
            "&MaxMass=" + maxMass;

    QNetworkRequest request(url_);
    reply_ = manager_.put(request, params.toUtf8());

    QEventLoop loop;
    QObject::connect(reply_, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply_->error() != QNetworkReply::NoError)
    {
      LOG_ERROR << "There was an error making a network request:\n";
      LOG_ERROR << reply_->errorString().toAscii().data() << endl;
      reply_->deleteLater();
      return false;
    }

    QString contents(reply_->readAll());
    reply_->deleteLater();

    QJson::Parser parser;
    bool ok;

    QVariantMap jMap = parser.parse(contents.toAscii(), &ok).toMap();
    if(!ok) {
        LOG_ERROR << "Error parsing JSON return from INIT occurred:" << contents.toAscii().data() << endl;
        return false;
    }

    if (jMap.contains("Error"))
    {
      LOG_ERROR << "Error occurred:" << jMap["Error"].toByteArray().data() << endl;
      return false;
    }
    else if (contents.startsWith("<html><head>"))
    {
      LOG_ERROR << "There is a problem with the specified server address." << endl;
      return false;
    }

    job_ = jMap["Job"].toString();
    funds_ = jMap["Funds"].toString();
 #ifdef WITH_GUI
    PI_versions_.clear();
    foreach(QVariant pi, jMap["PI_Versions"].toList()) {
        PI_versions_ << pi.toString();
    }
    PIVersion_ = PI_versions_[0];
    RTOs_.clear();
    foreach(QVariant rto, jMap["RTOs"].toList()) {
        RTOs_ << rto.toMap();
    }
    RTO_ = RTOs_[0]["RTO"].toString() ;

    // Ask the user what RTO and Version they want to use.

    // First build the string list for the RTOs.
    QStringList l;
    foreach(QVariantMap i, RTOs_) {
        l << i["RTO"].toString() + ", Estimated Cost: " + i["EstCost"].toString();
    }

    PIVersion_ = QInputDialog::getItem(NULL, "Peak Investigator", "Please select which version you wish to use.", PI_versions_);

    QString ret = QInputDialog::getItem(NULL, "Peak Investigator", "Please select which RTO you wish to use.\nYou have available funds of " + funds_, l);
    RTO_ = ret.split(",")[0];

#else
    RTO_ = experiment_.getMetaValue("veritomyx:RTO").toQString();
    PIVersion_ = experiment_.getMetaValue("veritomyx:PIVersion").toQString();
#endif

    return true;
  }

  bool PeakInvestigator::submitJob_()
  {
    url_.setUrl("https://" + server_.toQString() + VI_API_SUFFIX);
    QString params = QString("Version=") + reqVeritomyxCLIVersion.toQString(); // online CLI version that matches this interface
    params += "&User="	+ username_.toQString() +
            "&Code="    + password_.toQString() +
            "&Action="  + "RUN" +
            "&Job" + job_ +
            "&InputFile=" + sftp_file_ +
            "&RTO=" + RTO_ +
            "&PIVersion=" + PIVersion_;

    QNetworkRequest request(url_);
    reply_ = manager_.put(request, params.toUtf8());

    QEventLoop loop;
    QObject::connect(reply_, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QString contents(reply_->readAll());
    reply_->deleteLater();

    QJson::Parser parser;
    bool ok;

    QVariantMap jMap = parser.parse(contents.toAscii(), &ok).toMap();

    if(!ok) {
        LOG_ERROR << "Error parsing JSON return from RUN occurred:" << contents.toAscii().data() << endl;
        return false;
    }

    if (jMap.contains("Error"))
    {
      LOG_ERROR << "Error occurred:" << jMap["Error"].toByteArray().data() << endl;
      return false;
    }
    else if (contents.startsWith("<html><head>"))
    {
      LOG_ERROR << "There is a problem with the specified server address." << endl;
      return false;
    }

    cout << contents.toAscii().data() << endl;
    return true;

  }

  bool PeakInvestigator::checkJob_()
  {
    bool retval = false;

    server_ = experiment_.getMetaValue("veritomyx:server");
    job_ = experiment_.getMetaValue("veritomyx:job").toQString();

    if (job_.isEmpty())
    {
      LOG_WARN << "Problem getting job ID from meta data.\n";
      return retval;
    }

    url_.setUrl("https://" + server_.toQString() + VI_API_SUFFIX);
    QString params = QString("Version=") + reqVeritomyxCLIVersion.toQString(); // online CLI version that matches this interface
    params += "&User="	+ username_.toQString() +
              "&Code="    + password_.toQString() +
              "&Action="  + "STATUS" +
              "&Job" + job_ ;

    QNetworkRequest request(url_);
    reply_ = manager_.put(request, params.toUtf8());

    QEventLoop loop;
    QObject::connect(reply_, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply_->error() != QNetworkReply::NoError)
    {
      LOG_ERROR << "There was an error making a network request:\n";
      LOG_ERROR << reply_->errorString().toAscii().data() << endl;
      reply_->deleteLater();
      return false;
    }

    QString contents(reply_->readAll());
    reply_->deleteLater();

    QJson::Parser parser;
    bool ok;

    QVariantMap jMap = parser.parse(contents.toAscii(), &ok).toMap();

    if(!ok) {
        LOG_ERROR << "Error parsing JSON return from INIT occurred:" << contents.toAscii().data() << endl;
        return false;
    }

    if (jMap.contains("Error"))
    {
      LOG_ERROR << "Error occurred:" << jMap["Error"].toByteArray().data() << endl;
      return false;
    }
    else if (contents.startsWith("<html><head>"))
    {
      LOG_ERROR << "There is a problem with the specified server address." << endl;
      return false;
    }
    else if (jMap["Status"] == "Running")
    {
      LOG_INFO << job_.toAscii().data() << " is still running.\n";
      date_updated_ = jMap["Datetime"].toDate();
      retval = false;
    }
    else if (jMap["Status"] == "Done")
    {
      LOG_INFO << job_.toAscii().data() << " has finished.\n";
      results_file_ = jMap["ResultsFile"].toString();
      log_file_ = jMap["JobLogFile"].toString();
      actual_cost_ = jMap["ActualCost"].toString();
      date_updated_ = jMap["Datetime"].toDate();
      retval = true;
    }

    return retval;
  }

  bool PeakInvestigator::removeJob_()
  {
    url_.setUrl("https://" + server_.toQString() + VI_API_SUFFIX);
    QString params = QString("Version=") + reqVeritomyxCLIVersion.toQString(); // online CLI version that matches this interface
    params += "&User="	+ username_.toQString() +
              "&Code="    + password_.toQString() +
              "&Action="  + "DELETE" +
              "&Job" + job_ ;

    QNetworkRequest request(url_);
    reply_ = manager_.put(request, params.toUtf8());

    QEventLoop loop;
    QObject::connect(reply_, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply_->error() != QNetworkReply::NoError)
    {
      LOG_ERROR << "There was an error making a network request:\n";
      LOG_ERROR << reply_->errorString().toAscii().data() << endl;
      reply_->deleteLater();
      return false;
    }

    QString contents(reply_->readAll());
    reply_->deleteLater();

    QJson::Parser parser;
    bool ok;

    QVariantMap jMap = parser.parse(contents.toAscii(), &ok).toMap();

    if(!ok) {
        LOG_ERROR << "Error parsing JSON return from INIT occurred:" << contents.toAscii().data() << endl;
        return false;
    }

    if (jMap.contains("Error"))
    {
      LOG_ERROR << "Error occurred:" << jMap["Error"].toByteArray().data() << endl;
      return false;
    }
    else if (contents.startsWith("<html><head>"))
    {
      LOG_ERROR << "There is a problem with the specified server address." << endl;
      return false;
    }

    cout << contents.toAscii().data() << endl;
    return true;

  }

  bool PeakInvestigator::getSFTPCredentials_()
  {
      url_.setUrl("https://" + server_.toQString() + VI_API_SUFFIX);
      QString params = QString("Version=") + reqVeritomyxCLIVersion.toQString(); // online CLI version that matches this interface
      params += "&User="	+ username_.toQString() +
                "&Code="    + password_.toQString() +
                "&Action="  + "SFTP" +
                "&ID" + account_number_.toQString() ;

      QNetworkRequest request(url_);
      reply_ = manager_.put(request, params.toUtf8());

      QEventLoop loop;
      QObject::connect(reply_, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      if (reply_->error() != QNetworkReply::NoError)
      {
        LOG_ERROR << "There was an error making a network request:\n";
        LOG_ERROR << reply_->errorString().toAscii().data() << endl;
        reply_->deleteLater();
        return false;
      }

      QString contents(reply_->readAll());
      reply_->deleteLater();

      QJson::Parser parser;
      bool ok;

      QVariantMap jMap = parser.parse(contents.toAscii(), &ok).toMap();

      if(!ok) {
          LOG_ERROR << "Error parsing JSON return from INIT occurred:" << contents.toAscii().data() << endl;
          return false;
      }

      if (jMap.contains("Error"))
      {
        LOG_ERROR << "Error occurred:" << jMap["Error"].toByteArray().data() << endl;
        return false;
      }
      else if (contents.startsWith("<html><head>"))
      {
        LOG_ERROR << "There is a problem with the specified server address." << endl;
        return false;
      }

      sftp_host_ = jMap["Host"].toString();
      sftp_port_ = jMap["Port"].toString().toInt();
      sftp_dir_  = jMap["Directory"].toString();
      sftp_username_ = jMap["Login"].toString();
      sftp_password_ = jMap["Password"].toString();

      cout << contents.toAscii().data() << endl;
      return true;
  }

  PeakInvestigator::PIStatus PeakInvestigator::getPrepFileMessage_()
  {
      url_.setUrl("https://" + server_.toQString() + VI_API_SUFFIX);
      QString params = QString("Version=") + reqVeritomyxCLIVersion.toQString(); // online CLI version that matches this interface
      params += "&User="	+ username_.toQString() +
                "&Code="    + password_.toQString() +
                "&Action="  + "PREP" +
                "&ID" + account_number_.toQString() +
                "&File" + sftp_file_;

      QNetworkRequest request(url_);
      reply_ = manager_.put(request, params.toUtf8());

      QEventLoop loop;
      QObject::connect(reply_, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      if (reply_->error() != QNetworkReply::NoError)
      {
        LOG_ERROR << "There was an error making a network request:\n";
        LOG_ERROR << reply_->errorString().toAscii().data() << endl;
        reply_->deleteLater();
        return PREP_ERROR;
      }

      QString contents(reply_->readAll());
      reply_->deleteLater();

      QJson::Parser parser;
      bool ok;

      QVariantMap jMap = parser.parse(contents.toAscii(), &ok).toMap();

      if(!ok) {
          LOG_ERROR << "Error parsing JSON return from INIT occurred:" << contents.toAscii().data() << endl;
          return PREP_ERROR;
      }

      if (jMap.contains("Error"))
      {
        LOG_ERROR << "Error occurred:" << jMap["Error"].toByteArray().data() << endl;
        return PREP_ERROR;
      }
      else if (contents.startsWith("<html><head>"))
      {
        LOG_ERROR << "There is a problem with the specified server address." << endl;
        return PREP_ERROR;
      }

      QString status = jMap["Status"].toString();

      if(status == "Ready")
      {
          prep_count_ = jMap["ScanCount"].toString().toInt();
          // TODO check ScanCount vs count saved, report error if not equal.
          prep_ms_type = jMap["MSType"].toString();
      }
      else if(status == "Analyzing")
      {
          return PREP_ANALYZING;
      }
      else
      {
          return PREP_ERROR;
      }

      cout << contents.toAscii().data() << endl;
      return PREP_READY;
  }

  void PeakInvestigator::updateMembers_()
  {
    server_ = param_.getValue("server");
    username_ = param_.getValue("username");
    password_ = param_.getValue("password");
    account_number_ = param_.getValue("account");
  }

}

