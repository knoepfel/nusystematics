#pragma once

#include "nusystematics/interface/IGENIESystProvider_tool.hh"

#include "nusystematics/responsecalculators/FSILikeEAvailSmearing.hh"

#include "TFile.h"
#include "TTree.h"

#include <map>
#include <memory>
#include <string>

class FSILikeEAvailSmearing : public nusyst::IGENIESystProvider_tool {

  size_t ResponseParameterIdx;

public:
  enum class chan {
    kCCQE,
    kCCRes,
    kCCDIS,
    kCCMEC,
    kCCQE_bar,
    kCCRes_bar,
    kCCDIS_bar,
    kCCMEC_bar,
    kNC,
    kBadChan
  };

private:
  struct TemplateHelper {
    std::unique_ptr<nusyst::FSILikeEAvailSmearing_ReWeight> Template;
    bool ZeroIsValid;
  };

  std::map<chan, TemplateHelper> ChannelParameterMapping;
  std::pair<double, double> LimitWeights;

public:
  explicit FSILikeEAvailSmearing(fhiclsimple::ParameterSet const &);

  bool SetupResponseCalculator(fhiclsimple::ParameterSet const &);
  fhiclsimple::ParameterSet GetExtraToolOptions() { return tool_options; }

  systtools::SystMetaData BuildSystMetaData(fhiclsimple::ParameterSet const &,
                                            systtools::paramId_t);

  systtools::event_unit_response_t GetEventResponse(genie::EventRecord const &);

  std::string AsString();

  ~FSILikeEAvailSmearing();

private:
  fhiclsimple::ParameterSet tool_options;
};
