/**
 * @file test_trivial.cxx
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

// NOLINTNEXTLINE(build/define_used)
#define BOOST_TEST_MODULE boost_test_macro_overview

#include "TriggerPrimitive.hpp"

#include <boost/test/included/unit_test.hpp>

#include <chrono>
#include <random>
#include <thread>

using namespace DuneTriggers;

BOOST_AUTO_TEST_CASE(test_macro_overview)
{
  std::default_random_engine generator;
  // All this in milliseconds
  std::exponential_distribution<int> tp_coming(1000);
  std::normal_distribution<int> tp_time_over_threshold(20, 2);
  std::exponential_distribution<int> electronic_delay(500);

  auto start = std::chrono::steady_clock::now();
  while (true) {
    auto now = std::chrono::steady_clock::now();

    TriggerPrimitive tp;

    std::chrono::milliseconds millisec_wait(tp_coming(generator));
    std::this_thread::sleep_for(millisec_wait);
    if (start - now > std::chrono::seconds(10)) {
      break;
    }
  }
  int a = 1;
  int b = 2;
  BOOST_TEST(a == b - 1);
}
