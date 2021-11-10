/**
 * @file test_supernova.cxx
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

// NOLINTNEXTLINE(build/define_used)
#define BOOST_TEST_MODULE boost_test_macro_overview
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_macro_overview)
{
  int a = 1;
  int b = 2;
  BOOST_TEST(a == b - 1);
}
