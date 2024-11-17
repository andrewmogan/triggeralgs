/* @file: TriggerActivityFactory.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2023.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_TRIGGER_ACTIVITY_FACTORY_PDS_HPP_
#define TRIGGERALGS_TRIGGER_ACTIVITY_FACTORY_PDS_HPP_

#include "triggeralgs/TriggerActivityMakerPDS.hpp"
#include "triggeralgs/AbstractFactory.hpp"

#define REGISTER_TRIGGER_ACTIVITY_MAKER_PDS(tam_name, tam_class)                                                                                      \
  static struct tam_class##Registrar {                                                                                                            \
    tam_class##Registrar() {                                                                                                                      \
      TriggerActivityFactoryPDS::register_creator(tam_name, []() -> std::unique_ptr<TriggerActivityMakerPDS> {return std::make_unique<tam_class>();});   \
    }                                                                                                                                             \
  } tam_class##_registrar;

namespace triggeralgs {

class TriggerActivityFactoryPDS : public AbstractFactory<TriggerActivityMakerPDS> {};

} /* namespace triggeralgs */

#endif // TRIGGERALGS_TRIGGER_ACTIVITY_FACTORY_PDS_HPP_
