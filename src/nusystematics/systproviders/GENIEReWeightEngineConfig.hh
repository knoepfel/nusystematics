#pragma once

#include "systematicstools/interface/SystMetaData.hh"

#include "nusystematics/systproviders/GENIEResponseParameterAssociation.hh"

#include "fhiclcppsimple/ParameterSet.h"

// GENIE
#include "RwFramework/GReWeight.h"

#include <map>
#include <memory>

namespace nusyst {

std::vector<GENIEResponseParameter>
ConfigureQEWeightEngine(systtools::SystMetaData const &,
                        fhiclsimple::ParameterSet const &tool_options);

std::vector<GENIEResponseParameter>
ConfigureMECWeightEngine(systtools::SystMetaData const &,
                        fhiclsimple::ParameterSet const &tool_options);

std::vector<GENIEResponseParameter>
ConfigureNCELWeightEngine(systtools::SystMetaData const &,
                          fhiclsimple::ParameterSet const &tool_options);

std::vector<GENIEResponseParameter>
ConfigureRESWeightEngine(systtools::SystMetaData const &,
                         fhiclsimple::ParameterSet const &tool_options);

std::vector<GENIEResponseParameter>
ConfigureCOHWeightEngine(systtools::SystMetaData const &,
                         fhiclsimple::ParameterSet const &tool_options);

std::vector<GENIEResponseParameter>
ConfigureDISWeightEngine(systtools::SystMetaData const &,
                         fhiclsimple::ParameterSet const &tool_options);

std::vector<GENIEResponseParameter>
ConfigureFSIWeightEngine(systtools::SystMetaData const &,
                         fhiclsimple::ParameterSet const &tool_options);

std::vector<GENIEResponseParameter>
ConfigureOtherWeightEngine(systtools::SystMetaData const &,
                           fhiclsimple::ParameterSet const &tool_options);

} // namespace nusyst
