// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// --------------------------------------------------------------------------
//                   OpenMS Mass Spectrometry Framework
// --------------------------------------------------------------------------
//  Copyright (C) 2003-2006 -- Oliver Kohlbacher, Knut Reinert
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// --------------------------------------------------------------------------
// $Id: PreferencesDialog.h,v 1.5 2006/03/28 08:03:27 marc_sturm Exp $
// $Author: marc_sturm $
// $Maintainer: Marc Sturm $
// --------------------------------------------------------------------------


#ifndef OPENMS_VISUAL_DIALOGS_PREFERENCESDIALOG_H
#define OPENMS_VISUAL_DIALOGS_PREFERENCESDIALOG_H

// QT
#include <qdialog.h>

// STL
#include <string>

// OpenMS
#include <OpenMS/CONCEPT/Types.h>
#include <OpenMS/VISUAL/ListStack.h>
#include <OpenMS/VISUAL/DIALOGS/PreferencesDialogPage.h>
#include <OpenMS/VISUAL/PreferencesManager.h>

namespace OpenMS
{
	/**
		@brief Main dialog for the PreferencesManager classes.
		
		
		
		@ingroup Dialogs
	*/
	class PreferencesDialog: public QDialog
	{
		Q_OBJECT

		public:
			///constructor
			PreferencesDialog();
			///destructor
			virtual ~PreferencesDialog();


			/// add a new PreferencesDailogPage
			void addPage(std::string name, PreferencesDialogPage* page, PreferencesManager* creator, PreferencesManager* parent=0);

		protected slots:
			/// ok button pressed
			void ok_();
			/// cancel button pressed
			void cancel_();
			/// apply button pressed
			void apply_();
			/// help button pressed
			void help_();
		
		protected:
			/// the Liststack in which the PreferencesDialogPages are displayed
			ListStack* stack_;
			/// a vector with pointers to all pages
			std::vector<PreferencesDialogPage*> pages_;
	};
} // namespace OpenMS


#endif

