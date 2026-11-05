#include <klattsch/banks.hpp>

#include <cstring>

namespace klattsch {

const PhonemeBank* BankRegistry::cached(const std::string& name) const noexcept {
  for (const auto& r : resolved_) {
    if (r->name == name) return &r->bank;
  }
  return nullptr;
}

// Resolve a bank by flattening its extends chain: the parent's records first
// (recursively resolved), then this bank's records overlaid - same-name records
// override in place, new records append. Mirrors the JS reference resolver.
const PhonemeBank* BankRegistry::build(const std::string& name,
                                       std::vector<std::string>& visiting) {
  if (const auto* c = cached(name)) return c;

  const PhonemeBank* authored = nullptr;
  for (const auto& e : entries_) {
    if (e.name == name) { authored = e.bank; break; }
  }
  if (authored == nullptr) return nullptr;

  // cycle guard: extends chains must be acyclic.
  for (const auto& v : visiting) {
    if (v == name) return nullptr;
  }
  visiting.push_back(name);

  std::unique_ptr<Resolved> res(new Resolved());
  res->name = name;

  if (authored->extends != nullptr) {
    if (const auto* parent = build(authored->extends, visiting)) {
      res->records.assign(parent->records, parent->records + parent->count);
    }
  }

  for (std::size_t i = 0; i < authored->count; ++i) {
    const PhonemeRecord& rec = authored->records[i];
    bool overridden = false;
    for (auto& existing : res->records) {
      if (std::strcmp(existing.name, rec.name) == 0) {
        existing = rec;
        overridden = true;
        break;
      }
    }
    if (!overridden) res->records.push_back(rec);
  }

  visiting.pop_back();

  res->bank = PhonemeBank{res->name.c_str(), res->records.data(),
                          res->records.size(), nullptr};
  const PhonemeBank* out = &res->bank;
  resolved_.push_back(std::move(res));
  return out;
}

BankRegistry::BankRegistry() : default_name_("klatt1980-en") {
  entries_.push_back({"klatt1980-en", &banks::klatt1980_en});
  entries_.push_back({"ja-mokhtari-2000", &banks::ja_mokhtari_2000});
  entries_.push_back({"ja-hecko-2026", &banks::ja_hecko_2026});

  std::vector<std::string> visiting;
  for (const auto& e : entries_) build(e.name, visiting);
}

const std::string& BankRegistry::defaultName() const noexcept {
  return default_name_;
}

std::vector<std::string> BankRegistry::list() const {
  std::vector<std::string> out;
  out.reserve(entries_.size());
  for (const auto& e : entries_) out.push_back(e.name);
  return out;
}

const PhonemeBank* BankRegistry::get(const std::string& name) const noexcept {
  return cached(name);
}

const PhonemeBank& BankRegistry::resolve(const std::string& name) const noexcept {
  if (const auto* b = cached(name)) return *b;
  return *cached(default_name_);
}

const BankRegistry& builtInBanks() {
  static const BankRegistry instance;
  return instance;
}

}  // namespace klattsch
