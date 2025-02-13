/* @file: TriggerCandidateFactoryPDS.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2023.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_TRIGGER_CANDIDATE_FACTORY_PDS_HPP_
#define TRIGGERALGS_TRIGGER_CANDIDATE_FACTORY_PDS_HPP_

#include "triggeralgs/TriggerCandidateMakerPDS.hpp"
#include "triggeralgs/AbstractFactory.hpp"

#define REGISTER_TRIGGER_CANDIDATE_MAKER_PDS(tcm_name, tcm_class)                                                                                      \
  static struct tcm_class##Registrar {                                                                                                            \
    tcm_class##Registrar() {                                                                                                                      \
      TriggerCandidateFactoryPDS::register_creator(tcm_name, []() -> std::unique_ptr<TriggerCandidateMakerPDS> {return std::make_unique<tcm_class>();});   \
    }                                                                                                                                             \
  } tcm_class##_registrar;

namespace triggeralgs {

class TriggerCandidateFactoryPDS : public AbstractFactory<TriggerCandidateMakerPDS> {};

} /* namespace triggeralgs */

#endif // TRIGGERALGS_TRIGGER_CANDIDATE_FACTORY_PDS_HPP_
