#include "triggeralgs/Michel/TriggerCandidateMakerMichel.hpp"
#include <algorithm>
#include <chrono>
#include <limits>
#include <vector>
#include <iostream>
using namespace std;
using namespace triggeralgs;

void
TriggerCandidateMakerMichel::operator()(const TriggerActivity& activity, std::vector<TriggerCandidate>& cand)
{
  std::cout << "candidate maker check" << std::endl;  
}



