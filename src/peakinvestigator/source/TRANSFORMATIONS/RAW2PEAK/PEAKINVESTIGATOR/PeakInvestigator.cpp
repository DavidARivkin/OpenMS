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
// $Maintainer:  David Rivkin $
// $Author: Adam Tenderholt $
// --------------------------------------------------------------------------
//

#include <fcntl.h> // used for SFTP transfers
#include <zlib.h> // used for g'zipping tar files

#include <OpenMS/FORMAT/PeakTypeEstimator.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PeakInvestigator.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PiVersionsAction.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/InitAction.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/SftpAction.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PrepAction.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/RunAction.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/StatusAction.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DeleteAction.h>

#ifdef WITH_GUI
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/RtoDialog.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/VersionDialog.h>
#endif

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

#define VI_API_SUFFIX "/api/"
#define VI_SSH_HASH String("D2:BE:B8:2E:3C:BE:84:E4:A3:0A:C8:42:5C:6B:39:4E")
#define minutesCheckPrep 2
#define minutesTimeoutPrep 20

using namespace std;

namespace OpenMS
{
  PeakInvestigator::PeakInvestigator(QObject* parent) :
    QObject(parent),
    DefaultParamHandler("PeakInvestigator"),
    ProgressLogger(),
    manager_(this)
  {
    // set default parameter values
    defaults_.setValue("server", "peakinvestigator.veritomyx.com", "Server address for PeakInvestigator (without https://)");
    defaults_.setValue("username", "USERNAME", "Username for account registered with Veritomyx");
    defaults_.setValue("password", "PASSWORD", "Password for account registered with Veritomyx");
    defaults_.setValue("account", 12345, "Account number");

    defaults_.setValue("m/z", "[min]:[max]", "m/z range to extract (applies to ALL ms levels!");

    defaults_.setValue("RTO", "RTO-24", "Response Time Objective to use");
    defaults_.setValue("Version", "1.2", "Version of Peak Investigator to use");

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
      if (!PIVersionsJob_())
      {
        break;
      }
#ifdef WITH_GUI
      if(!getVersionDlg())
      {
          break;
      }
#endif
      if (!initializeJob_())
      {
        break;
      }
#ifdef WITH_GUI
      if(!getRTODlg())
      {
          break;
      }
#endif

      if(getSFTPCredentials_()) {

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

         // Set SFTP host paramters and upload file
          sftp.setHostname(server_);
          String portnumber(sftp_port_);
          sftp.setPortnumber(portnumber);
          sftp.setUsername(sftp_username_);
          sftp.setPassword(sftp_password_);
          sftp.setExpectedServerHash(VI_SSH_HASH);

         if(sftp.uploadFile(localFilename, remoteFilename))
         {
             // Do PREP
            long timeWait = minutesTimeoutPrep;
            PeakInvestigator::PIStatus prep_stat = getPrepFileMessage_(zipfilename);
            while((prep_stat == PREP_ANALYZING) && timeWait > 0) {
                // LOG_INFO << "Waiting for PREP analysis to complete, " << zipfilename.toAscii().constData() << ", on SaaS server...Please be patient.";
                sleep(minutesCheckPrep * 60);
                timeWait -= minutesCheckPrep;
                prep_stat = getPrepFileMessage_(zipfilename);
            }
            // TODO:  If we timed out, report and error
            if(prep_stat == PREP_READY) {
                sftp_file_ = zipfilename;
                submitJob_();
            } else {
                cout << endl << "Error trying to PREP the file on the Saas server!" << endl;
            }
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

      if(!getSFTPCredentials_() || !checkJob_()) // Seems we need to check STATUS before file is moved to SFTP drop after completion
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
      sftp_file_ = zipfilename = QFileInfo(results_file_).fileName();
      localFilename = QDir::tempPath() + "/" + zipfilename;
      remoteFilename = results_file_;

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
      dp->setMetaValue("parameter: peakinvestigator:server", server_);
      dp->setMetaValue("peakinvestigator:job", job_);

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

  bool PeakInvestigator::PIVersionsJob_()
  {
    LOG_DEBUG << "Requsting credentials for " + username_ + "..." << endl;

    PiVersionsAction action(username_.toQString(), password_.toQString());
    QString params = action.buildQuery();

    bool ok, ret;
    QString contents = Post_(params, ok);
    action.processResponse(contents);

    ret = (ok && !action.hasError());
    if(ret) {
        CurrentVersion_ = action.getCurrentVersion();
        LastUsedVersion_ = action.getLastUsedVersion();
        PI_versions_ = action.getVersions();
        PIVersion_ = (LastUsedVersion_.isEmpty() ? CurrentVersion_ : LastUsedVersion_);
    }
    min_mass_ = 0;
    max_mass_ = INT_MAX;
    int minMass = INT_MAX,
         maxMass = 0;
    points_count_ = 0;
    // Loop over the scans
    for(Size s = 0; s < experiment_.size(); s++) {
        // Loop over the datapoints
        experiment_[s].sortByPosition();
        // Get the Position_/Coordinate_
        minMass = qMin(minMass, (int)floor(experiment_[s][0].getPosition()[0]));
        Size sEnd = experiment_[s].size()-1;
        maxMass = qMax(maxMass, (int)ceil(experiment_[s][sEnd].getPosition()[0]));
        // Get the number of datapoints in the scan
        points_count_ = qMax(points_count_, (int)sEnd);
    }
    min_mass_ = qMax(minMass, min_mass_);
    max_mass_ = qMin(maxMass, max_mass_);

    return ret;
  }

 bool PeakInvestigator::initializeJob_()
  {
    LOG_DEBUG << "Requsting credentials for " + username_ + "..." << endl;

    // Ensure the selected PIVersion_ is available
    if(!PI_versions_.contains(PIVersion_)) {
        LOG_ERROR << "There was an error with the seleted version: " << PIVersion_.constData() << endl;
        return false;
    }

    InitAction action(username_.toQString(), password_.toQString(),
               account_number_, PIVersion_,
               (int)experiment_.size(), points_count_+1, min_mass_, max_mass_);
    // TODO:  Add calibrationCount to the InitAction

    bool ok, ret;
    QString contents = Post_(action.buildQuery(), ok);
    action.processResponse(contents);

    ret = (ok && !action.hasError());
    if(ret) {
        job_ = action.getJob();
        funds_ = action.getFunds();
        projectId_ = action.getProjectId();
        estimatedCosts_ = action.getEstimatedCosts();
    }
    return ret;
  }

  bool PeakInvestigator::submitJob_()
  {
    RunAction action(username_.toQString(), password_.toQString(),
                job_, RTO_, sftp_file_);
    // TODO:  CalibrationFile should be added calib_file_;

    bool ok;
    QString contents = Post_(action.buildQuery(), ok);
    action.processResponse(contents);

    if(ok && !action.hasError()) {

        cout << endl << "Job " << action.getJob().toAscii().constData() << " started." << endl;
    //    LOG_INFO << contents.toAscii().constData() << endl;

        experiment_.setMetaValue("peakinvestigator:job", job_);
        return ok;
    } else {
        cout << action.getErrorMessage().toAscii().constData() << endl;
        return false;
    }

  }

  bool PeakInvestigator::checkJob_()
  {
    bool retval = false;

    if(job_.isEmpty()) {
        job_ = experiment_.getMetaValue("peakinvestigator:job").toQString();

        if (job_.isEmpty())
        {
          LOG_WARN << "Problem getting job ID from meta data.\n";
          return retval;
        }
    }

    StatusAction action(username_.toQString(), password_.toQString(),
                        job_);

    bool ok;
    QString contents = Post_(action.buildQuery(), ok);
    action.processResponse(contents);

    if(ok && !action.hasError()) {
        switch(action.getStatus())
        {
        case StatusAction::Preparing :
            LOG_INFO << job_.toAscii().constData() << " is still preparing.\n";
            date_updated_ = action.getDate();
            retval = false;
            break;
        case StatusAction::Running :
            LOG_INFO << job_.toAscii().constData() << " is still runing.\n";
            date_updated_ = action.getDate();
            retval = false;
            break;
        case StatusAction::Deleted :
            LOG_INFO << job_.toAscii().constData() << " was deleted.\n";
            date_updated_ = action.getDate();
            retval = false;
            break;
        case StatusAction::Done :
            LOG_INFO << job_.toAscii().constData() << " has finished.\n";
            results_file_ = action.getResultsFilename();
            log_file_ = action.getLogFilename();
            actual_cost_ = action.getActualCost();
            date_updated_ = action.getDate();
//            if( action.getNumberOfInputScans() != action.getNumberOfCompleteScans()) {
//                LOG_ERROR << "The number of input scans does not match the number of scans completed!";
//                retval = false;
//            } else {
                retval = true;
 //           }
        }
        return retval;
    } else {
        cout << action.getErrorMessage().toAscii().constData() << endl;
        return false;
    }
  }

  bool PeakInvestigator::removeJob_()
  {

    DeleteAction action(username_.toQString(), password_.toQString(),
                        job_);

    bool ok;
    QString toSaas = action.buildQuery();
    QString contents = Post_(action.buildQuery(), ok);
    action.processResponse(contents);
//    LOG_INFO << contents.toAscii().constData() << endl;
    return ok;

  }

  bool PeakInvestigator::getSFTPCredentials_()
  {
      SftpAction action(username_.toQString(), password_.toQString(),
                        account_number_) ;

      bool ok;
      QString contents = Post_(action.buildQuery(), ok);
      action.processResponse(contents);

      if(ok) {
          sftp_host_ = action.getHost();
          sftp_port_ = action.getPort();
          sftp_dir_  = action.getDirectory();
          sftp_username_ = action.getSftpUsername();
          sftp_password_ = action.getSftpPassword();
      }
//      LOG_INFO << contents.toAscii().constData() << endl;

      return ok;
  }

  PeakInvestigator::PIStatus PeakInvestigator::getPrepFileMessage_(QString filename)
  {
      PrepAction action(username_.toQString(), password_.toQString(),
                        account_number_,
                        filename);

      bool ok;
      QString contents = Post_(action.buildQuery(), ok);
      action.processResponse(contents);

      if(ok && !action.hasError()) {
          switch(action.getStatus())
          {
          case PrepAction::Ready :
              LOG_INFO << "Preparation of the job file completed, ";
              prep_count_ = action.getScanCount();
              LOG_INFO << "found " << prep_count_ << " scans, and mass spectrometer type ";
              prep_ms_type_ = action.getMStype();
              LOG_INFO <<  prep_ms_type_.toAscii().constData() << endl;
 //             cout << contents.toAscii().constData() << endl;
              return PREP_READY;
          case PrepAction::Analyzing :
              prep_percent_complete_ = action.getPercentComplete().left(action.getPercentComplete().indexOf("%")).toDouble();
              LOG_INFO << "Preparation of the job file is still analyzing.  % Complete: " << action.getPercentComplete().toAscii().constData() << endl;
              return PREP_ANALYZING;
          case PrepAction::Error :
          default:
              LOG_INFO << "Preparation of the job file returned an error occurred" << endl;
              return PREP_ERROR;
          }
      } else {
          cout << endl << action.getErrorMessage().toAscii().constData() << endl;
          return PREP_ERROR;
      }
  }

  void PeakInvestigator::updateMembers_()
  {
    server_ = param_.getValue("server");
    username_ = param_.getValue("username");
    password_ = param_.getValue("password");
    account_number_ = param_.getValue("account");
    String  minMaxString = param_.getValue("m/z");
    std::vector<String> minMaxSplit;
    if((minMaxString != "[min]:[max]") && minMaxString.split(':', minMaxSplit)) {
        min_mass_ = minMaxSplit[0].toInt();
        max_mass_ = minMaxSplit[1].toInt();
    }
    RTO_ = param_.getValue("RTO").toQString();
    PIVersion_ = param_.getValue("Version").toQString();
    DefaultParamHandler::updateMembers_();
  }

  QString PeakInvestigator::Post_(QString params, bool &success)
  {
      if(params.isEmpty()) {
          success = false;
          return "";
      }

      url_.setUrl("https://" + server_.toQString() + VI_API_SUFFIX);

      QNetworkRequest request(url_);
      request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
      request.setHeader(QNetworkRequest::ContentLengthHeader, QString::number(params.size()));

      reply_ = manager_.post(request, params.toUtf8());

      QEventLoop loop;
      QObject::connect(reply_, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      if (reply_->error() != QNetworkReply::NoError)
      {
        LOG_ERROR << "There was an error making a network request:\n";
        LOG_ERROR << reply_->errorString().toAscii().constData() << endl;
        reply_->deleteLater();
        success = false;
        return "";
      }

      QString contents(reply_->readAll());

      reply_->deleteLater();
      success = true;
      return contents;
  }

#ifdef WITH_GUI
  bool PeakInvestigator::getVersionDlg(void) {
      // Ask the user for a Version of PI to use and the min and max mass values
      VersionDialog verDlg("Peak Investigator", PI_versions_, LastUsedVersion_, CurrentVersion_, min_mass_, max_mass_);
      if(verDlg.exec() == QDialog::Accepted) {
          PIVersion_ = verDlg.version();
          int xmass = verDlg.maxMass();
          if(xmass > max_mass_) {
              LOG_ERROR << "The Maximum Mass must be less than " <<  max_mass_;
              return false;
          } else {
              max_mass_ = xmass;
          }
          xmass = verDlg.minMass();
          if(xmass > max_mass_) {
              LOG_ERROR << "The Minimum Mass must be less than the Maximum Mass";
              return false;
          } else {
              min_mass_ = xmass;
          }
          return true;
      } else {
          return false;
      }
  }

  bool PeakInvestigator::getRTODlg(void) {

      // Ask the user for a RTO to use
      RtoDialog rtoDlg("Peak Investigator " + PIVersion_, funds_, estimatedCosts_);
      if(rtoDlg.exec() == QDialog::Accepted) {
          RTO_ = rtoDlg.getRto();
          return true;
      } else {
          return false;
      }
  }
#endif

}

