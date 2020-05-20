#pragma once

#include "catch.hpp"

#include <random>

// Based on the example from
// https://github.com/catchorg/Catch2/blob/master/examples/300-Gen-OwnGenerator.cpp

class RandomAddressGenerator : public Catch::Generators::IGenerator<uint64_t> {
  std::minstd_rand rand;
  std::uniform_int_distribution<> distribution;

  uint64_t current;

 public:
  RandomAddressGenerator();

  const uint64_t& get() const override;
  bool next() override;
};

Catch::Generators::GeneratorWrapper<uint64_t> random_addresses();
