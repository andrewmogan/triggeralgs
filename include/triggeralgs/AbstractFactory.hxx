/* @file: AbstractFactory.hxx
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2023.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_ABSTRACT_FACTORY_HXX_
#define TRIGGERALGS_ABSTRACT_FACTORY_HXX_

namespace triggeralgs {

template <typename T>
std::shared_ptr<AbstractFactory<T>> AbstractFactory<T>::s_single_factory = nullptr;

template <typename T>
typename AbstractFactory<T>::creation_map& AbstractFactory<T>::get_makers(){
  static creation_map s_makers;
  return s_makers;
}

template <typename T>
void AbstractFactory<T>::register_creator(const std::string alg_name, maker_creator creator)
{
  creation_map& makers = get_makers();
  auto it = makers.find(alg_name);

  if (it == makers.end()) {
    makers[alg_name] = creator;
    return;
  }
  TLOG(0) << "Attempted to overwrite a creator in factory with " << alg_name << ".";
  throw; // creators should not be overwritten.
  return;
}

template <typename T>
std::shared_ptr<T> AbstractFactory<T>::build_maker(const std::string& alg_name)
{
  creation_map& makers = get_makers();
  auto it = makers.find(alg_name);

  if (it != makers.end()) {
    TLOG(0) << "Factory building " << alg_name << ".";
    return it->second();
  }

  TLOG(0) << "Factory couldn't find " << alg_name << ".";
  return nullptr;
}

template <typename T>
std::shared_ptr<AbstractFactory<T>> AbstractFactory<T>::get_instance()
{
  if (s_single_factory == nullptr) {
    s_single_factory = std::make_shared<AbstractFactory<T>>();
  }
  return s_single_factory;
}

} // namespace triggeralgs

#endif // TRIGGERALGS_ABSTRACT_FACTORY_HXX_
