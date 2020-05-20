#include "RandomAddressGenerator.hh"

RandomAddressGenerator::RandomAddressGenerator()
    : rand(std::random_device {}()), distribution() {
  static_cast<void>(next());
}

const uint64_t& RandomAddressGenerator::get() const { return current; }

bool RandomAddressGenerator::next() {
  current = distribution(rand);
  return true;
}


Catch::Generators::GeneratorWrapper<uint64_t> random_addresses() {
  return Catch::Generators::GeneratorWrapper<uint64_t>(
      std::unique_ptr<Catch::Generators::IGenerator<uint64_t>>(
          new RandomAddressGenerator()));
}
