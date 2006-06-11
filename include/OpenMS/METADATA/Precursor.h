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
// $Id: Precursor.h,v 1.2 2006/06/02 14:19:11 marc_sturm Exp $
// $Author: marc_sturm $
// $Maintainer: Marc Sturm $
// --------------------------------------------------------------------------

#ifndef OPENMS_METADATA_PRECURSOR_H
#define OPENMS_METADATA_PRECURSOR_H

#include <OpenMS/CONCEPT/Types.h>
#include <OpenMS/METADATA/MetaInfoInterface.h>
namespace OpenMS 
{
	/**
		@brief Precursor meta information.
		
		This class covers precursor meta information, that is not already covered by DSpectrum.
		
		@ingroup Metadata
	*/  
  class Precursor: public MetaInfoInterface
  {
    public:
    	/// Method of activation
      enum ActivationMethod{ACTMETHNULL,CID,PSD,PD,SID,SIZE_OF_ACTIVATIONMETHOD};
			/// Names of activation methods
			static const std::string NamesOfActivationMethod[SIZE_OF_ACTIVATIONMETHOD];

      /// Energy unit
      enum EnergyUnits{UNITSNULL,EV,PERCENT,SIZE_OF_ENERGYUNITS};
			/// Names of energy units
			static const std::string NamesOfEnergyUnits[SIZE_OF_ENERGYUNITS];
      
      /// Constructor
      Precursor();
      /// Copy constructor
      Precursor(const Precursor& source);
      /// Destructor
      ~Precursor();
      
      /// Assignment operator
      Precursor& operator= (const Precursor& source);

      /// Equality operator
      bool operator== (const Precursor& rhs) const;
      /// Equality operator
      bool operator!= (const Precursor& rhs) const;
			
			/// returns the activation method
      ActivationMethod getActivationMethod() const;
      /// sets the activation method
      void setActivationMethod(ActivationMethod activation_method);
			
			/// returns the activation energy
      float getActivationEnergy() const;
      /// sets the activation energy
      void setActivationEnergy(float activation_energy);
			
			/// return the actication energy unit
      EnergyUnits getActivationEnergyUnit() const;
      /// sets the activation energy unit
      void setActivationEnergyUnit(EnergyUnits activation_energy_unit);
 
    protected:
      ActivationMethod activation_method_;
      float activation_energy_;
      EnergyUnits activation_energy_unit_;
  };
} // namespace OpenMS

#endif // OPENMS_METADATA_PRECURSOR_H
