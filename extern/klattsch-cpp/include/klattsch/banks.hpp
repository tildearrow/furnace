#ifndef KLATTSCH_BANKS_HPP
#define KLATTSCH_BANKS_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <klattsch/klattsch.hpp>

namespace klattsch {

class BankRegistry {
 public:
  BankRegistry();

  const std::string& defaultName() const noexcept;
  std::vector<std::string> list() const;

  // Both accessors return the fully resolved bank (extends chain flattened in:
  // inherited phonemes first, then this bank's overrides/additions). get()
  // returns nullptr for an unknown name; resolve() falls back to the default.
  const PhonemeBank* get(const std::string& name) const noexcept;
  const PhonemeBank& resolve(const std::string& name) const noexcept;

 private:
  struct Entry {
    std::string name;
    const PhonemeBank* bank;  // authored (pre-resolution) bank
  };
  // A resolved bank owns its flattened record list; bank.records points into it.
  // Held via unique_ptr so the pointer stays stable as more banks are added.
  struct Resolved {
    std::string name;
    std::vector<PhonemeRecord> records;
    PhonemeBank bank;
  };
  std::vector<Entry> entries_;
  std::vector<std::unique_ptr<Resolved>> resolved_;
  std::string default_name_;

  const PhonemeBank* cached(const std::string& name) const noexcept;
  const PhonemeBank* build(const std::string& name,
                           std::vector<std::string>& visiting);
};

// Process-wide registry, populated with all built-in banks at static init.
const BankRegistry& builtInBanks();

}  // namespace klattsch

#endif
