/**
 *  * @file TriggerDecisionMakerMichel.cpp
 *   *
 *    * This is part of the DUNE DAQ Application Framework, copyright 2020.
 *     * Licensing/copyright details are in the COPYING file that you should have
 *      * received with this code.
 *       */

#include "triggeralgs/Michel/TriggerDecisionMakerMichel.hpp"

#include <algorithm>
#include <chrono>
#include <limits>
#include <vector>
#include <iostream>
using namespace std;
using namespace triggeralgs;

void
TriggerDecisionMakerMichel::operator()(const TriggerCandidate& cand, std::vector<TriggerDecision>& decisions)
{
  std::cout << "Decision working" << std::endl;
  return;
}
