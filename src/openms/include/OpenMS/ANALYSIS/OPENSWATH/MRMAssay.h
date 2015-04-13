// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2015.
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
// $Maintainer: George Rosenberger $
// $Authors: George Rosenberger $
// --------------------------------------------------------------------------

#ifndef OPENMS_ANALYSIS_OPENSWATH_MRMASSAY_H
#define OPENMS_ANALYSIS_OPENSWATH_MRMASSAY_H

#include <OpenMS/ANALYSIS/TARGETED/TargetedExperiment.h>
#include <OpenMS/CONCEPT/ProgressLogger.h>
#include <OpenMS/CONCEPT/LogStream.h>
#include <OpenMS/ANALYSIS/OPENSWATH/MRMIonSeries.h>
#include <OpenMS/ANALYSIS/OPENSWATH/MRMDecoy.h>
#include <OpenMS/CHEMISTRY/ModificationsDB.h>

#include <boost/bind.hpp>

// #define DEBUG_MRMASSAY

namespace OpenMS
{
  /**
    @brief Generate assays from a TargetedExperiment

    Will generate assays from a raw, unfiltered TargetedExperiment, as can be
    generated by ConvertTSVToTraML.

    Transitions can be selected according to a set of rules, as described in
    Schubert et al., 2015 (PMID: 25675208).

    In addition, unique ion signature (UIS) (Sherman et al., 2009 (PMID: 19556279))
    or site-specific transitions can be generated based on empirically observed
    or in silico generated ion series.

  */
  class OPENMS_DLLAPI MRMAssay :
    public ProgressLogger
  {

public:
    //@{
    /// Constructor
    MRMAssay(); // empty, no members

    /// Destructor
    ~MRMAssay();
    //@}

    typedef std::vector<OpenMS::TargetedExperiment::Protein> ProteinVectorType;
    typedef std::vector<OpenMS::TargetedExperiment::Peptide> PeptideVectorType;
    typedef std::vector<OpenMS::ReactionMonitoringTransition> TransitionVectorType;

    /**
      @brief Annotates and filters transitions in a TargetedExperiment

      @param exp the input, unfiltered transitions
      @param mz_threshold the product m/z threshold in Th for annotation
      @param fragment_types the fragment types to consider for annotation
      @param fragment_charges the fragment charges to consider for annotation
      @param enable_alternative_localizations whether to trust the site-localizations
      of the transitions or whether alternative options should be generated
      @param enable_reannotation whether the original annotation should be taken
      or reannotation should be conducted
      @param enable_losses whether neutral losses for the fragment ion (according to SpectraST)
      should be enabled
      @param round_decPow round product m/z values to decimal power (default: -4)

    */
    void reannotateTransitions(OpenMS::TargetedExperiment& exp, double mz_threshold, std::vector<String> fragment_types, std::vector<size_t> fragment_charges, bool enable_alternative_localizations, bool enable_reannotation, bool enable_losses, int round_decPow = -4);

    /**
      @brief Restrict and filter transitions in a TargetedExperiment

      @param exp the input, unfiltered transitions
      @param lower_mz_limit the lower product m/z limit in Th
      @param upper_mz_limit the upper product m/z limit in Th
      @param swathes the swath window settings (to exclude fragment ions falling
      into the precursor isolation window)

    */
    void restrictTransitions(OpenMS::TargetedExperiment& exp, double lower_mz_limit, double upper_mz_limit, std::vector<std::pair<double, double> > swathes);

    /**
      @brief Select detecting fragment ions

      @param exp the input, unfiltered transitions
      @param min_transitions the minimum number of transitions required per assay
      @param max_transitions the maximum number of transitions required per assay

    */
    void detectingTransitions(OpenMS::TargetedExperiment& exp, int min_transitions, int max_transitions);

    /**
      @brief Annotate UIS / site-specific transitions

      @param exp the input, unfiltered transitions
      @param mz_threshold the product m/z threshold in Th for annotation
      @param fragment_types the fragment types to consider for annotation
      @param fragment_charges the fragment charges to consider for annotation
      @param enable_losses whether neutral losses for the fragment ion (according to SpectraST)
      should be enabled
      @param enable_uis_scoring whether UIS transitions should be annotated
      @param enable_site_scoring whether site-specific transitions should be annotated
      @param mz_threshold the product m/z threshold in Th for annotation
      @param swathes the swath window settings (to exclude fragment ions falling

    */
    void uisTransitions(OpenMS::TargetedExperiment& exp, std::vector<String> fragment_types, std::vector<size_t> fragment_charges, bool enable_losses, bool enable_uis_scoring, bool enable_site_scoring, double mz_threshold, std::vector<std::pair<double, double> > swathes);

    /**
      @brief Generates in silico transitions

      @param exp the input, unfiltered transitions
      @param fragment_types the fragment types to consider for annotation
      @param fragment_charges the fragment charges to consider for annotation
      @param enable_losses whether neutral losses for the fragment ion (according to SpectraST)
      should be enabled
      @param round_decPow round product m/z values to decimal power (default: -4)

    */
    void insilicoTransitions(OpenMS::TargetedExperiment& exp, std::vector<String> fragment_types, std::vector<size_t> fragment_charges, bool enable_losses, int round_decPow = -4);

protected:
    /**
      @brief Check whether fragment ion are unique ion signatures in vector within threshold

      @param fragment_ion the queried fragment ion
      @param ions a vector of fragment ions which could interfere with fragment_ion
      @param mz_threshold the threshold within which to search for interferences
    */
    bool isUIS_(const double fragment_ion, std::vector<double> ions, const double mz_threshold);

    /**
      @brief Annotate fragment ions with site-specific attributes

      @param sequence the corresponding AASequence including modifications
      @param tr a reference to the query transition
    */
    void isSiteUIS_(const AASequence sequence, ReactionMonitoringTransition& tr);

    /**
      @brief Get swath index (precursor isolation window ordinal) for a particular precursor

      @param swathes the swath window settings
      @param precursor_mz the query precursor m/z
    */
    int getSwath_(const std::vector<std::pair<double, double> > swathes, const double precursor_mz);

    /**
      @brief Check whether the product m/z of a transition falls into the precursor isolation window

      @param swathes the swath window settings
      @param precursor_mz the query precursor m/z
      @param precursor_mz the query product m/z
    */
    bool isInSwath_(const std::vector<std::pair<double, double> > swathes, const double precursor_mz, const double product_mz);

    /**
      @brief Computes all N choose K combinations

      @param n vector of N indices
      @param k number of K

      @value a vector of all N index combinations
    */
    std::vector<std::vector<size_t> > nchoosekcombinations_(std::vector<size_t> n, size_t k);

    /**
      @brief Generate alternative modified peptide forms according to ModificationsDB

      @param sequence template AASequence

      @value a vector of all alternative modified peptides.
    */
    std::vector<OpenMS::AASequence> combineModifications_(OpenMS::AASequence sequence);

    /**
      @brief Generate alternative modified peptide forms according to ModificationsDB

      @param sequence template AASequence

      @value a vector of all alternative modified peptides.
    */
    std::vector<OpenMS::AASequence> addModificationsSequences_(std::vector<OpenMS::AASequence> sequences, std::vector<std::vector<size_t> > mods_combs, OpenMS::String modification);

    /**
      @brief Adds ResidueModification to vector of modifications 

      @param mods a vector of targeted experiment modifications
      @param location the residue location of the modification
      @param rmod the residue modification
      @param name the name of the residue modification

    */
    void addModification_(std::vector<TargetedExperiment::Peptide::Modification>& mods, int location, ResidueModification& rmod, const String& name);

  };
}

#endif