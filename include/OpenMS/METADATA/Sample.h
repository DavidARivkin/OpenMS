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
// $Id: Sample.h,v 1.2 2006/06/02 14:19:11 marc_sturm Exp $
// $Author: marc_sturm $
// $Maintainer: Marc Sturm $
// --------------------------------------------------------------------------

#ifndef OPENMS_METADATA_SAMPLE_H
#define OPENMS_METADATA_SAMPLE_H

#include <list>

#include <OpenMS/METADATA/SampleTreatment.h>

#include <OpenMS/DATASTRUCTURES/String.h>
#include <OpenMS/METADATA/MetaInfoInterface.h>
#include <OpenMS/CONCEPT/Exception.h>

namespace OpenMS 
{
	/**
		@brief Meta information about the sample
		
		It contains basic descriptions like name, number (i.e. order number), mass, 
		volume, concentration, state and a comment.<BR>
		Additionally sample treatments like Digestion, Modification or Tagging can be added.<BR>
		A Sample can be composed of other samples.
		
		@ingroup Metadata
	*/
  class Sample: public MetaInfoInterface
  {
    public:
    	///state of aggregation of the sample
    	enum SampleState {SAMPLENULL,SOLID,LIQUID,GAS,SOLUTION,EULSION,SUSPENSION,SIZE_OF_SAMPLESTATE};
			/// Names of sample states
			static const std::string NamesOfSampleState[SIZE_OF_SAMPLESTATE];
    	
    	///default constructor
      Sample();
      ///copy constructor
      Sample(const Sample& source);
      ///desctuctor
      ~Sample();

      ///assignment operator
      Sample& operator= (const Sample& source);

    	/// Equality operator
      bool operator== (const Sample& rhs) const;
      /// Equality operator
      bool operator!= (const Sample& rhs) const;
			
			///retuns the sample name
      const String& getName() const;
      ///sets the sample name
      void setName(const String& name);

			///retuns the sample name
      const String& getOrganism() const;
      ///sets the sample name
      void setOrganism(const String& organism);

			/// returns the sample number
      const String& getNumber() const;
      /// sets the sample number (e.g. sample ID)
      void setNumber(const String& number);

			/// returns the comment
      const String& getComment() const;
      /// sets the comment (may contain newline characters)
      void setComment(const String& comment);

			/// returns the state of aggregation
      SampleState getState() const;
      /// sets the state of aggregation
      void setState(SampleState state);

			/// returns the mass (in mg)
      float getMass() const;
      /// sets the mass (in mg)
      void setMass(float mass);

			/// returns the volume (in ml)
      float getVolume() const;
      /// sets the volume (in ml)
      void setVolume(float volume);

			/// returns the concentration (in mg/ml)
      float getConcentration() const;
      /// sets the concentration (in mg/ml)
      void setConcentration(float concentration);

			/// returns a vector of subsamples that were combined to create this sample
		  std::vector<Sample>& getSubsamples();
		  /// returns a vector of subsamples that were combined to create this sample
		  const std::vector<Sample>& getSubsamples() const;
		  /// sets the vector of subsamples that were combined to create this sample
		  void setSubsamples(const std::vector<Sample>& subsamples);
			
			///adds a sample treatment before the given postion (default is the end of the list). Sample treatments are ordered in the order of application to the sample. If before_position is smaller than 0, the sample treatment is appended to the list.
			void addTreatment(const SampleTreatment& treatment,SignedInt before_position=-1) throw (Exception::IndexOverflow);
			/// returns the sample treatment at the given position
			const SampleTreatment& getTreatment(UnsignedInt position) throw (Exception::IndexOverflow);
			/// removes the sample treatment at the given position
			void removeTreatment(UnsignedInt position) throw (Exception::IndexOverflow);
			/// returns the number of sample treatments
			SignedInt countTreatments();
			
    protected:
	    String name_;
	    String number_;
	    String comment_;
	    String organism_;
	    SampleState state_;
			float mass_;
			float volume_;
			float concentration_;
			std::vector<Sample> subsamples_;
			std::list<SampleTreatment*> treatments_; 

  };
} // namespace OpenMS

#endif // OPENMS_METADATA_SAMPLE_H
