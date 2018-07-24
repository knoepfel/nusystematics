#include "nusystematics/systproviders/GENIEReWeightEngineConfig.hh"

// GENIE
#include "ReWeight/GReWeightAGKY.h"
#include "ReWeight/GReWeightFZone.h"
#include "ReWeight/GReWeightNonResonanceBkg.h"
#include "ReWeight/GReWeightNuXSecCCQE.h"
#include "ReWeight/GReWeightNuXSecCCQEaxial.h"
#include "ReWeight/GReWeightNuXSecCCQEvec.h"
#include "ReWeight/GReWeightNuXSecCCRES.h"
#include "ReWeight/GReWeightNuXSecCOH.h"
#include "ReWeight/GReWeightNuXSecDIS.h"
#include "ReWeight/GReWeightNuXSecNCEL.h"
#include "ReWeight/GReWeightNuXSecNCRES.h"
#include "ReWeight/GReWeightResonanceDecay.h"

#include <functional>

using namespace systtools;
using namespace genie::rew;

namespace nusyst {

void AddResponseAndDependentDials(
    SystMetaData const &md, std::string const &ResponseDialName,
    std::vector<GSyst_t> const &DependentDials, std::string const &engine_name,
    std::function<GReWeightI *()> EngineInstantiator, bool UseFullHERG,
    std::vector<GENIEResponseParameter> &param_map) {

  std::vector<std::string> DependentDialNames;
  std::transform(DependentDials.begin(), DependentDials.end(),
                 std::back_inserter(DependentDialNames),
                 [](GSyst_t const &gdial) { return GSyst::AsString(gdial); });

  // If not ignoring dependence then have to hook up multiple parameter
  // variations to the response parameter.
  if (HasParam(md, ResponseDialName) && HasAnyParams(md, DependentDialNames)) {
    size_t pidx = GetParamIndex(md, ResponseDialName);
    GENIEResponseParameter ResponsePar;
    ResponsePar.pidx = pidx;

    for (GSyst_t const &depdial : DependentDials) {
      std::string const &pname = GSyst::AsString(depdial);
      if (!HasParam(md, pname)) {
        continue;
      }
      ResponsePar.dependents.push_back({depdial, GetParamIndex(md, pname)});
    }

    for (size_t i = 0; i < md[pidx].paramVariations.size(); ++i) {
      std::unique_ptr<GReWeight> grw = std::make_unique<GReWeight>();

      grw->AdoptWghtCalc(engine_name, EngineInstantiator());

      for (auto const &dep : ResponsePar.dependents) {
        grw->Systematics().Init(dep.gdial, md[dep.pidx].paramVariations[i]);
      }

      grw->Reconfigure();
      ResponsePar.Herg.push_back(std::move(grw));
      if (!UseFullHERG) {
        break;
      }
    }
    if (md[pidx].isCorrection) {
      std::unique_ptr<GReWeight> grw = std::make_unique<GReWeight>();

      grw->AdoptWghtCalc(engine_name, EngineInstantiator());

      for (auto const &dep : ResponsePar.dependents) {
        grw->Systematics().Init(dep.gdial, md[dep.pidx].centralParamValue);
      }

      grw->Reconfigure();
      ResponsePar.Herg.push_back(std::move(grw));
    }
    param_map.push_back(std::move(ResponsePar));
    // We are ignoring the dependence of the zexpansion parameters.
  } else if (HasAnyParams(md, DependentDialNames)) {
    for (GSyst_t const &depdial : DependentDials) {
      std::string const &pname = GSyst::AsString(depdial);
      if (!HasParam(md, pname)) {
        continue;
      }
      size_t pidx = GetParamIndex(md, pname);
      GENIEResponseParameter DialPar;
      DialPar.pidx = pidx;
      DialPar.dependents.push_back({depdial, pidx});
      for (double var : md[pidx].paramVariations) {
        std::unique_ptr<GReWeight> grw = std::make_unique<GReWeight>();

        grw->AdoptWghtCalc(engine_name, EngineInstantiator());
        grw->Systematics().Init(depdial, var);

        grw->Reconfigure();
        DialPar.Herg.push_back(std::move(grw));
        if (!UseFullHERG) {
          break;
        }
      }
      if (md[pidx].isCorrection) {
        std::unique_ptr<GReWeight> grw = std::make_unique<GReWeight>();

        grw->AdoptWghtCalc(engine_name, EngineInstantiator());
        grw->Systematics().Init(depdial, md[pidx].centralParamValue);

        grw->Reconfigure();
        DialPar.Herg.push_back(std::move(grw));
      }
      param_map.push_back(std::move(DialPar));
    }
  }
}

void AddIndependentParameters(SystMetaData const &md,
                              std::vector<GSyst_t> const &Dials,
                              std::string const &engine_name,
                              std::function<GReWeightI *()> EngineInstantiator,
                              bool UseFullHERG,
                              std::vector<GENIEResponseParameter> &param_map) {

  for (GSyst_t const &dial : Dials) {
    if (!HasParam(md, GSyst::AsString(dial))) {
      continue;
    }
    size_t pidx = GetParamIndex(md, GSyst::AsString(dial));
    GENIEResponseParameter dialPar;
    dialPar.pidx = pidx;
    dialPar.dependents.push_back({dial, pidx});

    for (double var : md[pidx].paramVariations) {
      std::unique_ptr<GReWeight> grw = std::make_unique<GReWeight>();

      grw->AdoptWghtCalc(engine_name, EngineInstantiator());
      grw->Systematics().Init(dial, var);

      grw->Reconfigure();
      dialPar.Herg.push_back(std::move(grw));
      if (!UseFullHERG) {
        break;
      }
    }
    if (md[pidx].isCorrection) {
      std::unique_ptr<GReWeight> grw = std::make_unique<GReWeight>();

      grw->AdoptWghtCalc(engine_name, EngineInstantiator());
      grw->Systematics().Init(dial, md[pidx].centralParamValue);

      grw->Reconfigure();
      dialPar.Herg.push_back(std::move(grw));
    }

    param_map.push_back(std::move(dialPar));
  }
}

std::vector<GENIEResponseParameter>
ConfigureQEWeightEngine(SystMetaData const &QEmd,
                        fhicl::ParameterSet const &tool_options) {

  std::vector<GENIEResponseParameter> param_map;

  bool UseFullHERG = tool_options.get<bool>("UseFullHERG", false);

  // Add NormCCQE
  AddIndependentParameters(
      QEmd, {{kXSecTwkDial_NormCCQE}}, "xsec_ccqe_axFF",
      []() {
        GReWeightNuXSecCCQE *rwccqe = new GReWeightNuXSecCCQE();

        rwccqe->SetMode(GReWeightNuXSecCCQE::kModeNormAndMaShape);
        return rwccqe;
      },
      UseFullHERG, param_map);

  // Add MACCQE
  bool MAQEIsShapeOnly = tool_options.get<bool>("MAQEIsShapeOnly", false);
  AddIndependentParameters(
      QEmd,
      {{MAQEIsShapeOnly ? kXSecTwkDial_MaCCQEshape : kXSecTwkDial_MaCCQE}},
      "xsec_ccqe_axFF",
      [=]() {
        GReWeightNuXSecCCQE *rwccqe = new GReWeightNuXSecCCQE();

        rwccqe->SetMode(MAQEIsShapeOnly
                            ? GReWeightNuXSecCCQE::kModeNormAndMaShape
                            : GReWeightNuXSecCCQE::kModeMa);
        return rwccqe;
      },
      UseFullHERG, param_map);

  // Add AxFFCCQEShape
  AddIndependentParameters(
      QEmd, {{kXSecTwkDial_AxFFCCQEshape}}, "xsec_ccqe_axFF",
      []() { return new GReWeightNuXSecCCQEaxial(); }, UseFullHERG, param_map);

  // Add ZNormCCQE
  AddIndependentParameters(QEmd, {{kXSecTwkDial_ZNormCCQE}}, "xsec_ccqe_axFF",
                           []() {
                             GReWeightNuXSecCCQE *rwccqe =
                                 new GReWeightNuXSecCCQE();

                             rwccqe->SetMode(GReWeightNuXSecCCQE::kModeZExp);
                             return rwccqe;
                           },
                           UseFullHERG, param_map);

  // Add ZExpansion dials
  AddResponseAndDependentDials(
      QEmd, "ZExpAVariationResponse",
      {kXSecTwkDial_ZExpA1CCQE, kXSecTwkDial_ZExpA2CCQE,
       kXSecTwkDial_ZExpA3CCQE, kXSecTwkDial_ZExpA4CCQE},
      "xsec_ccqe_axFF",
      []() {
        GReWeightNuXSecCCQE *rwccqe = new GReWeightNuXSecCCQE();

        rwccqe->SetMode(GReWeightNuXSecCCQE::kModeZExp);
        return rwccqe;
      },
      UseFullHERG, param_map);

  AddIndependentParameters(
      QEmd, {{kXSecTwkDial_VecFFCCQEshape}}, "xsec_ccqe_vecFF",
      []() { return new GReWeightNuXSecCCQEvec; }, UseFullHERG, param_map);

  return param_map;
}

std::vector<GENIEResponseParameter>
ConfigureNCELWeightEngine(SystMetaData const &NCELmd,
                          fhicl::ParameterSet const &tool_options) {

  std::vector<GENIEResponseParameter> param_map;

  bool UseFullHERG = tool_options.get<bool>("UseFullHERG", false);

  AddResponseAndDependentDials(
      NCELmd, "NCELVariationResponse",
      {kXSecTwkDial_MaNCEL, kXSecTwkDial_EtaNCEL}, "xsec_NCEl_FF",
      []() { return new GReWeightNuXSecNCEL; }, UseFullHERG, param_map);

  return param_map;
}

std::vector<GENIEResponseParameter>
ConfigureRESWeightEngine(SystMetaData const &RESmd,
                         fhicl::ParameterSet const &tool_options) {

  std::vector<GENIEResponseParameter> param_map;

  bool UseFullHERG = tool_options.get<bool>("UseFullHERG", false);

  // Add any CCRES parameters
  AddIndependentParameters(
      RESmd, {{kXSecTwkDial_NormCCRES}}, "xsec_ccres_FF",
      []() {
        GReWeightNuXSecCCRES *rwccres = new GReWeightNuXSecCCRES();

        rwccres->SetMode(GReWeightNuXSecCCRES::kModeNormAndMaMvShape);
        return rwccres;
      },
      UseFullHERG, param_map);

  bool CCRESIsShapeOnly = tool_options.get<bool>("CCRESIsShapeOnly", false);
  AddResponseAndDependentDials(
      RESmd, "CCRESVariationResponse",
      {CCRESIsShapeOnly ? kXSecTwkDial_MaCCRESshape : kXSecTwkDial_MaCCRES,
       CCRESIsShapeOnly ? kXSecTwkDial_MvCCRESshape : kXSecTwkDial_MvCCRES},
      "xsec_ccres_FF",
      [=]() {
        GReWeightNuXSecCCRES *rwccres = new GReWeightNuXSecCCRES();
        rwccres->SetMode(CCRESIsShapeOnly
                             ? GReWeightNuXSecCCRES::kModeNormAndMaMvShape
                             : GReWeightNuXSecCCRES::kModeMaMv);
        return rwccres;
      },
      UseFullHERG, param_map);

  AddIndependentParameters(
      RESmd, {{kXSecTwkDial_NormNCRES}}, "xsec_ncres_FF",
      []() {
        GReWeightNuXSecNCRES *rwncres = new GReWeightNuXSecNCRES();

        rwncres->SetMode(GReWeightNuXSecNCRES::kModeNormAndMaMvShape);
        return rwncres;
      },
      UseFullHERG, param_map);

  bool NCRESIsShapeOnly = tool_options.get<bool>("NCRESIsShapeOnly", false);
  AddResponseAndDependentDials(
      RESmd, "NCRESVariationResponse",
      {NCRESIsShapeOnly ? kXSecTwkDial_MaNCRESshape : kXSecTwkDial_MaNCRES,
       NCRESIsShapeOnly ? kXSecTwkDial_MvNCRESshape : kXSecTwkDial_MvNCRES},
      "xsec_ncres_FF",
      [=]() {
        GReWeightNuXSecNCRES *rwncres = new GReWeightNuXSecNCRES();
        rwncres->SetMode(NCRESIsShapeOnly
                             ? GReWeightNuXSecNCRES::kModeNormAndMaMvShape
                             : GReWeightNuXSecNCRES::kModeMaMv);
        return rwncres;
      },
      UseFullHERG, param_map);

  AddIndependentParameters(
      RESmd,
      {{kXSecTwkDial_RvpCC1pi, kXSecTwkDial_RvpCC2pi, kXSecTwkDial_RvpNC1pi,
        kXSecTwkDial_RvpNC2pi, kXSecTwkDial_RvnCC1pi, kXSecTwkDial_RvnCC2pi,
        kXSecTwkDial_RvnNC1pi, kXSecTwkDial_RvnNC2pi, kXSecTwkDial_RvbarpCC1pi,
        kXSecTwkDial_RvbarpCC2pi, kXSecTwkDial_RvbarpNC1pi,
        kXSecTwkDial_RvbarpNC2pi, kXSecTwkDial_RvbarnCC1pi,
        kXSecTwkDial_RvbarnCC2pi, kXSecTwkDial_RvbarnNC1pi,
        kXSecTwkDial_RvbarnNC2pi}},
      "xsec_NonResBkg", []() { return new GReWeightNonResonanceBkg(); },
      UseFullHERG, param_map);

  AddIndependentParameters(RESmd,
                           {{kRDcyTwkDial_BR1gamma, kRDcyTwkDial_BR1eta,
                             kRDcyTwkDial_Theta_Delta2Npi}},
                           "xsec_ResDecay",
                           []() { return new GReWeightResonanceDecay(); },
                           UseFullHERG, param_map);

  return param_map;
}

std::vector<GENIEResponseParameter>
ConfigureCOHWeightEngine(SystMetaData const &COHmd,
                         fhicl::ParameterSet const &tool_options) {

  std::vector<GENIEResponseParameter> param_map;

  bool UseFullHERG = tool_options.get<bool>("UseFullHERG", false);

  AddResponseAndDependentDials(
      COHmd, "COHVariationResponse",
      {kXSecTwkDial_MaCOHpi, kXSecTwkDial_R0COHpi}, "xsec_COH",
      []() { return new GReWeightNuXSecCOH; }, UseFullHERG, param_map);

  return param_map;
}

std::vector<GENIEResponseParameter>
ConfigureDISWeightEngine(SystMetaData const &DISmd,
                         fhicl::ParameterSet const &tool_options) {

  std::vector<GENIEResponseParameter> param_map;

  bool UseFullHERG = tool_options.get<bool>("UseFullHERG", false);
  bool DISBYIsShapeOnly = tool_options.get<bool>("DISBYIsShapeOnly", false);
  AddResponseAndDependentDials(
      DISmd, "DISBYVariationResponse",
      {DISBYIsShapeOnly ? kXSecTwkDial_AhtBYshape : kXSecTwkDial_AhtBY,
       DISBYIsShapeOnly ? kXSecTwkDial_BhtBYshape : kXSecTwkDial_BhtBY,
       DISBYIsShapeOnly ? kXSecTwkDial_CV1uBYshape : kXSecTwkDial_CV1uBY,
       DISBYIsShapeOnly ? kXSecTwkDial_CV2uBYshape : kXSecTwkDial_CV2uBY},
      "xsec_dis_FF",
      [=]() {
        GReWeightNuXSecDIS *rwdis = new GReWeightNuXSecDIS();
        rwdis->SetMode(DISBYIsShapeOnly ? GReWeightNuXSecDIS::kModeABCV12uShape
                                        : GReWeightNuXSecDIS::kModeABCV12u);
        return rwdis;
      },
      UseFullHERG, param_map);

  AddResponseAndDependentDials(
      DISmd, "AGKYVariationResponse",
      {kHadrAGKYTwkDial_xF1pi, kHadrAGKYTwkDial_pT1pi}, "hadronization",
      []() { return new GReWeightAGKY; }, UseFullHERG, param_map);

  AddIndependentParameters(DISmd, {{kHadrNuclTwkDial_FormZone}}, "form_zone",
                           []() { return new GReWeightFZone; }, UseFullHERG,
                           param_map);

  return param_map;
}

#ifndef GRWTEST

std::vector<GENIEResponseParameter>
ConfigureFSIWeightEngine(SystMetaData const &FSImd,
                         std::unique_ptr<GReWeight> &GReWeightEngine) {
  return {};
}

std::vector<GENIEResponseParameter>
ConfigureOtherWeightEngine(SystMetaData const &Othermd,
                           std::unique_ptr<GReWeight> &GReWeightEngine) {
  return {};
}

#endif

} // namespace nusyst
