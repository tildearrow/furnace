/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// Regex-driven line parser with a simple symbol table (std::regex + std::map).
// No DivEngine dep — pure text → arrays, so we can unit-test it standalone.

#include "scspdspasm.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace {

// ─── Numeric helpers ────────────────────────────────────────────────

static int twosComp13(int value) {
  if (value < -4096 || value > 4095)
    throw std::runtime_error("Coefficient out of range (-4096..4095)");
  if (value < 0) value = (1 << 13) + value;
  return value & 0x1FFF;
}

static std::string trim(const std::string& s) {
  size_t a = s.find_first_not_of(" \t\r\n");
  if (a == std::string::npos) return "";
  size_t b = s.find_last_not_of(" \t\r\n");
  return s.substr(a, b - a + 1);
}

static std::string toUpper(const std::string& s) {
  std::string r = s;
  for (char& c : r) c = (char)std::toupper((unsigned char)c);
  return r;
}

static bool startsWithI(const std::string& s, const std::string& pre) {
  if (s.size() < pre.size()) return false;
  for (size_t i = 0; i < pre.size(); i++) {
    if (std::toupper((unsigned char)s[i]) != std::toupper((unsigned char)pre[i])) return false;
  }
  return true;
}

static int parseCoefRhs(const std::string& sIn) {
  std::string s = trim(sIn);
  if (startsWithI(s, "&H"))
    return (int)std::strtol(s.c_str() + 2, NULL, 16) & 0x1FFF;
  if (!s.empty() && s[0] == '%') {
    double pct = std::atof(s.c_str() + 1);
    int v = (int)std::lround(4095.0 * (pct / 100.0));
    if (v < -4096) v = -4096;
    if (v >  4095) v =  4095;
    return twosComp13(v);
  }
  if (s.find('.') != std::string::npos) {
    double f = std::atof(s.c_str());
    return twosComp13((int)std::lround(4096.0 * f));
  }
  return twosComp13(std::atoi(s.c_str()));
}

static int parseAdrsRhs(const std::string& sIn) {
  std::string s = trim(sIn);
  if (startsWithI(s, "&H"))
    return (int)std::strtol(s.c_str() + 2, NULL, 16) & 0xFFFF;
  if (s.size() >= 2 && (s[0] == 'm' || s[0] == 'M') && (s[1] == 's' || s[1] == 'S')) {
    double ms = std::atof(s.c_str() + 2);
    return (int)std::lround(44100.0 * (ms / 1000.0)) & 0xFFFF;
  }
  return std::atoi(s.c_str()) & 0xFFFF;
}

// ─── Symbol table ───────────────────────────────────────────────────

struct Symbols {
  std::vector<std::string> coefOrder;
  std::map<std::string,int> coef;
  std::vector<std::string> adrsOrder;
  std::map<std::string,int> adrs;

  Symbols() {
    coefOrder.push_back("ZERO");
    coef["ZERO"] = 0;
  }

  void addCoef(const std::string& name, const std::string& rhs) {
    if (toUpper(name) == "ZERO") throw std::runtime_error("ZERO is reserved");
    coef[name] = parseCoefRhs(rhs);
    coefOrder.push_back(name);
  }

  void addAdrs(const std::string& name, const std::string& rhs) {
    adrs[name] = parseAdrsRhs(rhs);
    adrsOrder.push_back(name);
  }

  int coefIndex(const std::string& name) const {
    for (size_t i = 0; i < coefOrder.size(); i++)
      if (coefOrder[i] == name) return (int)i;
    throw std::runtime_error("Undefined coefficient symbol '" + name + "'");
  }

  int adrsIndex(const std::string& name) const {
    for (size_t i = 0; i < adrsOrder.size(); i++)
      if (adrsOrder[i] == name) return (int)i;
    throw std::runtime_error("Undefined address symbol '" + name + "'");
  }
};

// ─── MPRO instruction encoder ───────────────────────────────────────

struct InstrFields {
  int TRA=0, TWT=0, TWA=0, XSEL=0, YSEL=0, IRA=0, IWT=0, IWA=0;
  int TABLE=0, MWT=0, MRD=0, EWT=0, EWA=0, ADRL=0, FRCL=0, SHFT=0;
  int YRL=0, NEGB=0, ZERO=0, BSEL=0, CRA=0, NOFL=0, MASA=0, ADREB=0, NXADR=0;
};

struct MproWords { unsigned short w[4]; };

static MproWords packMpro(const InstrFields& f) {
  int TRA   = f.TRA   & 0x7F;
  int TWT   = f.TWT   & 1;
  int TWA   = f.TWA   & 0x7F;
  int XSEL  = f.XSEL  & 1;
  int YSEL  = f.YSEL  & 3;
  int IRA   = f.IRA   & 0x3F;
  int IWT   = f.IWT   & 1;
  int IWA   = f.IWA   & 0x1F;
  int TABLE = f.TABLE & 1;
  int MWT   = f.MWT   & 1;
  int MRD   = f.MRD   & 1;
  int EWT   = f.EWT   & 1;
  int EWA   = f.EWA   & 0xF;
  int ADRL  = f.ADRL  & 1;
  int FRCL  = f.FRCL  & 1;
  int SHFT  = f.SHFT  & 3;
  int YRL   = f.YRL   & 1;
  int NEGB  = f.NEGB  & 1;
  int ZRO   = f.ZERO  & 1;
  int BSEL  = f.BSEL  & 1;
  int CRA   = f.CRA   & 0x3F;
  int NOFL  = f.NOFL  & 1;
  int MASA  = f.MASA  & 0x1F;
  int ADREB = f.ADREB & 1;
  int NXADR = f.NXADR & 1;

  // word 3 (bits 63:48): TRA<<8 | TWT<<7 | TWA
  unsigned short w3 = (unsigned short)((TRA << 8) | (TWT << 7) | TWA);
  // word 2 (bits 47:32): XSEL<<15 | YSEL<<13 | IRA<<6 | IWT<<5 | IWA
  unsigned short w2 = (unsigned short)((XSEL << 15) | (YSEL << 13) | (IRA << 6) | (IWT << 5) | IWA);
  // word 1 (bits 31:16): TABLE<<15 | MWT<<14 | MRD<<13 | EWT<<12 | EWA<<8 |
  //                       ADRL<<7 | FRCL<<6 | SHFT<<4 | YRL<<3 | NEGB<<2 | ZERO<<1 | BSEL
  unsigned short w1 = (unsigned short)((TABLE << 15) | (MWT << 14) | (MRD << 13) | (EWT << 12) |
                            (EWA << 8) | (ADRL << 7) | (FRCL << 6) | (SHFT << 4) |
                            (YRL << 3) | (NEGB << 2) | (ZRO << 1) | BSEL);
  // word 0 (bits 15:0): CRA<<9 | NOFL<<8 | MASA<<2 | ADREB<<1 | NXADR
  unsigned short w0 = (unsigned short)((CRA << 9) | (NOFL << 8) | (MASA << 2) | (ADREB << 1) | NXADR);
  return MproWords{ {w3, w2, w1, w0} };
}

static bool instrAccessesMem(const MproWords& m) {
  // word index 2 in [w3,w2,w1,w0]: that's w1, bits 14 (MWT) and 13 (MRD)
  return (m.w[2] & 0x6000) != 0;
}

// ─── Source/coef/multiplicand helpers ───────────────────────────────

static int iraForSrc(const std::string& srcU) {
  // srcU is uppercase
  std::smatch m;
  static const std::regex re(R"(^(MEMS|MIXS|EXTS)(\d+)$)");
  if (!std::regex_match(srcU, m, re))
    throw std::runtime_error("Bad INPUT source '" + srcU + "'");
  std::string bank = m[1].str();
  int idx = std::atoi(m[2].str().c_str());
  if (bank == "MEMS") { if (idx < 0 || idx > 31) throw std::runtime_error("MEMS index 0..31"); return idx; }
  if (bank == "MIXS") { if (idx < 0 || idx > 15) throw std::runtime_error("MIXS index 0..15"); return 0x20 + idx; }
  /* EXTS */          { if (idx < 0 || idx >  1) throw std::runtime_error("EXTS index 0..1");  return 0x30 + idx; }
}

static int shftForStore(const std::string& opt) {
  if (opt.empty()) return 0;
  std::string o = toUpper(opt);
  if (o == "S1") return 1;
  if (o == "S2") return 2;
  if (o == "S3") return 3;
  throw std::runtime_error("Invalid store option (use S1/S2/S3 or omit)");
}

// ─── Address-expression parser (MR[sym+DEC+ADREG+1/NF]) ─────────────

struct AddrFields { int masa=0, TABLE=1, ADREB=0, NXADR=0, NOFL=0; };

static AddrFields parseAddrExpr(const std::string& exprIn, const Symbols& syms) {
  // strip trailing comment after '
  std::string expr = exprIn;
  size_t q = expr.find('\'');
  if (q != std::string::npos) expr = expr.substr(0, q);
  expr = trim(expr);

  std::string body = expr;
  std::string flags;
  size_t slash = expr.find('/');
  if (slash != std::string::npos) {
    body = expr.substr(0, slash);
    flags = expr.substr(slash + 1);
  }
  // remove all spaces from body
  body.erase(std::remove_if(body.begin(), body.end(),
                            [](char c){ return c == ' ' || c == '\t'; }),
             body.end());

  std::vector<std::string> elems;
  {
    std::string cur;
    for (char c : body) {
      if (c == '+') { elems.push_back(cur); cur.clear(); }
      else cur.push_back(c);
    }
    elems.push_back(cur);
  }
  if (elems.empty() || elems[0].empty() || !std::isalpha((unsigned char)elems[0][0]))
    throw std::runtime_error("MR[...] must start with an address symbol");

  AddrFields a;
  a.masa = syms.adrsIndex(elems[0]);
  a.TABLE = 1;
  int DEC = 0;
  for (size_t i = 1; i < elems.size(); i++) {
    std::string eu = toUpper(elems[i]);
    if (eu == "DEC") DEC = 1;
    else if (eu == "ADREG" || eu == "ADRS") a.ADREB = 1;
    else if (elems[i] == "1") a.NXADR = 1;
    else if (elems[i].empty()) continue;
    else throw std::runtime_error("Unknown address element '+" + elems[i] + "'");
  }
  if (DEC) a.TABLE = 0;
  std::string flagsU = toUpper(flags);
  if (flagsU.find("NF") != std::string::npos) a.NOFL = 1;
  return a;
}

// ─── Coefficient / multiplicand helpers ─────────────────────────────

static void applyCoefRefToFields(const std::string& pcIn, const Symbols& syms, InstrFields& f) {
  std::string pc = trim(pcIn);
  std::string pcu = toUpper(pc);
  if (pcu == "YREGH") { f.YSEL = 2; f.CRA = 0; return; }
  if (pcu == "YREGL") { f.YSEL = 3; f.CRA = 0; return; }
  if (startsWithI(pc, "COEF[") && !pc.empty() && pc.back() == ']') {
    std::string name = pc.substr(5, pc.size() - 6);
    f.YSEL = 1; f.CRA = syms.coefIndex(name) & 0x3F; return;
  }
  static const std::regex idRe(R"(^[A-Za-z][A-Za-z0-9]{0,14}$)");
  if (std::regex_match(pc, idRe)) {
    f.YSEL = 1; f.CRA = syms.coefIndex(pc) & 0x3F; return;
  }
  throw std::runtime_error("Bad coefficient reference '" + pc + "'");
}

static void applyMultiplicandToFields(const std::string& pmIn, InstrFields& f) {
  std::string pm = trim(pmIn);
  std::string pmu = toUpper(pm);
  if (pmu == "INPUT") { f.XSEL = 1; return; }
  static const std::regex tempRe(R"(^TEMP(\d{1,2})$)");
  std::smatch tm;
  if (std::regex_match(pmu, tm, tempRe)) {
    f.XSEL = 0; f.TRA = std::atoi(tm[1].str().c_str()); return;
  }
  static const std::regex srcRe(R"(^(MEMS\d{1,2}|MIXS\d{1,2}|EXTS\d)$)");
  if (std::regex_match(pmu, srcRe)) {
    f.XSEL = 1; f.IRA = iraForSrc(pmu); return;
  }
  throw std::runtime_error("Bad multiplicand '" + pm + "'");
}

struct ProdPair { InstrFields pmf; InstrFields yf; };

static std::vector<ProdPair> expandProducts(const std::string& expr, const Symbols& syms) {
  // Match: <multiplicand> * <coef-ref>
  // Multiplicand: INPUT | TEMPxx | MEMSxx | MIXSxx | EXTSx
  // Coef-ref:     COEF[name] | YREGH | YREGL | <ident>
  static const std::regex prodRe(
    R"(\b(INPUT|TEMP\d{1,2}|MEMS\d{1,2}|MIXS\d{1,2}|EXTS\d)\b\s*\*\s*(COEF\[[^\]]+\]|YREGH|YREGL|[A-Za-z][A-Za-z0-9]{0,14}))",
    std::regex::icase);
  std::vector<ProdPair> out;
  std::sregex_iterator begin(expr.begin(), expr.end(), prodRe);
  std::sregex_iterator end;
  for (std::sregex_iterator it = begin; it != end; ++it) {
    ProdPair p;
    applyMultiplicandToFields((*it)[1].str(), p.pmf);
    applyCoefRefToFields((*it)[2].str(), syms, p.yf);
    out.push_back(p);
  }
  if (out.empty()) throw std::runtime_error("No product terms found in '@' expression");
  return out;
}

// ─── Assembler core ─────────────────────────────────────────────────

class Assembler {
public:
  Symbols syms;
  std::vector<MproWords> progWords;

  void assemble(const std::string& text);
  std::vector<std::string> alignMemoryOps();
  void getArrays(int rbl, SCSPDSPAssembly& out) const;

private:
  void emitInstr(const std::string& line);
  void emitAtChain(const std::string& line);
  void emitStoreSuffix(const std::string& text);
  void emitStoreLine(const std::string& opt, const std::string& destsStr);
};

static MproWords makeNop() {
  InstrFields f; f.BSEL = 1; f.YSEL = 1;
  return packMpro(f);
}

void Assembler::assemble(const std::string& text) {
  std::string section;
  std::istringstream is(text);
  std::string raw;
  while (std::getline(is, raw)) {
    // strip leading whitespace
    std::string stripped = raw;
    size_t a = stripped.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) continue;
    stripped = stripped.substr(a);
    if (stripped.empty()) continue;
    if (stripped[0] == '\'' || stripped[0] == ')') continue;

    // strip inline ' comment, then trailing whitespace
    std::string line = raw;
    size_t qp = line.find('\'');
    if (qp != std::string::npos) line = line.substr(0, qp);
    while (!line.empty() && (line.back() == ' ' || line.back() == '\t' ||
                             line.back() == '\r' || line.back() == '\n'))
      line.pop_back();
    if (trim(line).empty()) continue;

    std::string u = toUpper(trim(line));
    if (u == "#COEF") { section = "COEF"; continue; }
    if (u == "#ADRS") { section = "ADRS"; continue; }
    if (u == "#PROG") { section = "PROG"; continue; }
    if (u == "#END" || u == "=END") { section = "END"; continue; }

    static const std::regex coefRe(R"(^\s*([A-Za-z][A-Za-z0-9]{0,14})\s*=\s*(.+?)\s*$)");
    if (section == "COEF") {
      std::smatch m;
      if (!std::regex_match(line, m, coefRe))
        throw std::runtime_error("Bad coef line: " + line);
      syms.addCoef(m[1].str(), m[2].str());
      continue;
    }
    if (section == "ADRS") {
      std::smatch m;
      if (!std::regex_match(line, m, coefRe))
        throw std::runtime_error("Bad adrs line: " + line);
      syms.addAdrs(m[1].str(), m[2].str());
      continue;
    }
    if (section == "PROG") {
      emitInstr(line);
      continue;
    }
  }
}

void Assembler::emitInstr(const std::string& lineIn) {
  std::string trimmed = trim(lineIn);
  std::string upper = toUpper(trimmed);

  if (upper == "NOP") { progWords.push_back(makeNop()); return; }

  // @ with inline > store
  if (!trimmed.empty() && trimmed[0] == '@' && trimmed.find('>') != std::string::npos) {
    size_t gt = trimmed.find('>');
    std::string atPart = trimmed.substr(0, gt);
    std::string storePart = trim(trimmed.substr(gt + 1));
    emitAtChain(atPart);
    emitStoreSuffix(storePart);
    return;
  }
  // @ chain
  if (!trimmed.empty() && trimmed[0] == '@') {
    emitAtChain(trimmed);
    return;
  }

  // LDI MEMSxx, MR[...]
  static const std::regex ldiRe(R"(^\s*LDI\s+(MEMS(\d{1,2})),\s*MR\[(.+?)\]\s*$)", std::regex::icase);
  std::smatch m;
  if (std::regex_match(lineIn, m, ldiRe)) {
    int memIdx = std::atoi(m[2].str().c_str());
    AddrFields a = parseAddrExpr(m[3].str(), syms);
    InstrFields f;
    f.MRD = 1; f.IWT = 1; f.IWA = memIdx & 0x1F;
    f.MASA = a.masa & 0x1F; f.TABLE = a.TABLE;
    f.ADREB = a.ADREB; f.NXADR = a.NXADR; f.NOFL = a.NOFL;
    progWords.push_back(packMpro(f));
    return;
  }
  // MR MR[...]
  static const std::regex mrRe(R"(^\s*MR\s+MR\[(.+?)\]\s*$)", std::regex::icase);
  if (std::regex_match(lineIn, m, mrRe)) {
    AddrFields a = parseAddrExpr(m[1].str(), syms);
    InstrFields f;
    f.MRD = 1; f.MASA = a.masa & 0x1F; f.TABLE = a.TABLE;
    f.ADREB = a.ADREB; f.NXADR = a.NXADR; f.NOFL = a.NOFL;
    progWords.push_back(packMpro(f));
    return;
  }
  // IW MEMSxx
  static const std::regex iwRe(R"(^\s*IW\s+MEMS(\d{1,2})\s*$)", std::regex::icase);
  if (std::regex_match(lineIn, m, iwRe)) {
    InstrFields f;
    f.IWT = 1; f.IWA = std::atoi(m[1].str().c_str()) & 0x1F;
    progWords.push_back(packMpro(f));
    return;
  }
  // MW MR[...]
  static const std::regex mwRe(R"(^\s*MW\s+MR\[(.+?)\]\s*$)", std::regex::icase);
  if (std::regex_match(lineIn, m, mwRe)) {
    AddrFields a = parseAddrExpr(m[1].str(), syms);
    InstrFields f;
    f.MWT = 1; f.BSEL = 1; f.YSEL = 1;
    f.MASA = a.masa & 0x1F; f.TABLE = a.TABLE;
    f.ADREB = a.ADREB; f.NXADR = a.NXADR; f.NOFL = a.NOFL;
    progWords.push_back(packMpro(f));
    return;
  }
  // LDY <src>
  static const std::regex ldyRe(R"(^\s*LDY\s+(MEMS\d{1,2}|MIXS\d{1,2}|EXTS\d)\s*$)", std::regex::icase);
  if (std::regex_match(lineIn, m, ldyRe)) {
    InstrFields f;
    f.IRA = iraForSrc(toUpper(m[1].str())); f.YRL = 1;
    progWords.push_back(packMpro(f));
    return;
  }
  // LDA <src>
  static const std::regex ldaRe(R"(^\s*LDA\s+(MEMS\d{1,2}|MIXS\d{1,2}|EXTS\d)\s*$)", std::regex::icase);
  if (std::regex_match(lineIn, m, ldaRe)) {
    InstrFields f;
    f.IRA = iraForSrc(toUpper(m[1].str())); f.ADRL = 1;
    progWords.push_back(packMpro(f));
    return;
  }
  // > store (standalone, may include MW[...])
  static const std::regex stRe(R"(^\s*>\s*([Ss][123])?\s*(.+?)\s*$)");
  if (std::regex_match(trimmed, m, stRe)) {
    std::string opt = m[1].str();
    std::string rest = trim(m[2].str());
    if (startsWithI(rest, "MW[")) {
      emitStoreSuffix((opt.empty() ? std::string() : opt) + " " + rest);
    } else {
      emitStoreLine(opt, m[2].str());
    }
    return;
  }
  throw std::runtime_error("Unknown PROG line: " + lineIn);
}

void Assembler::emitAtChain(const std::string& lineIn) {
  static const std::regex atRe(R"(^\s*@\s*(.+?)\s*$)");
  std::smatch m;
  if (!std::regex_match(lineIn, m, atRe))
    throw std::runtime_error("Bad '@' line: " + lineIn);
  std::vector<ProdPair> prods = expandProducts(m[1].str(), syms);

  // First product: ZERO=1 (no augend)
  {
    InstrFields f;
    f.ZERO = 1;
    f.TRA = prods[0].pmf.TRA; f.XSEL = prods[0].pmf.XSEL; f.IRA = prods[0].pmf.IRA;
    f.YSEL = prods[0].yf.YSEL; f.CRA = prods[0].yf.CRA;
    progWords.push_back(packMpro(f));
  }
  // Subsequent: BSEL=1 (REG/ACC accumulate)
  for (size_t i = 1; i < prods.size(); i++) {
    InstrFields f;
    f.BSEL = 1;
    f.TRA = prods[i].pmf.TRA; f.XSEL = prods[i].pmf.XSEL; f.IRA = prods[i].pmf.IRA;
    f.YSEL = prods[i].yf.YSEL; f.CRA = prods[i].yf.CRA;
    progWords.push_back(packMpro(f));
  }
}

void Assembler::emitStoreSuffix(const std::string& textIn) {
  static const std::regex sufRe(R"(^([Ss][123])?\s*(.*)$)");
  std::smatch m;
  if (!std::regex_match(textIn, m, sufRe))
    throw std::runtime_error("Bad store suffix '> " + textIn + "'");
  std::string opt = m[1].str();
  std::string rest = trim(m[2].str());
  if (startsWithI(rest, "MW[") && !rest.empty() && rest.back() == ']') {
    std::string inner = rest.substr(3, rest.size() - 4);
    AddrFields a = parseAddrExpr(inner, syms);
    InstrFields f;
    f.MWT = 1; f.BSEL = 1; f.YSEL = 1;
    f.MASA = a.masa & 0x1F; f.TABLE = a.TABLE;
    f.ADREB = a.ADREB; f.NXADR = a.NXADR; f.NOFL = a.NOFL;
    progWords.push_back(packMpro(f));
    return;
  }
  emitStoreLine(opt, rest);
}

void Assembler::emitStoreLine(const std::string& opt, const std::string& destsStr) {
  // split on comma, uppercase, trim each
  std::vector<std::string> dests;
  std::string cur;
  for (char c : destsStr) {
    if (c == ',') { dests.push_back(toUpper(trim(cur))); cur.clear(); }
    else cur.push_back(c);
  }
  dests.push_back(toUpper(trim(cur)));

  int SHFT = shftForStore(opt);
  int TWT=0, TWA=0, EWT=0, EWA=0, FRCL=0, ADRL=0;
  for (const std::string& d : dests) {
    if (d.rfind("TEMP", 0) == 0)       { TWT = 1; TWA = std::atoi(d.c_str() + 4); }
    else if (d.rfind("EFREG", 0) == 0) { EWT = 1; EWA = std::atoi(d.c_str() + 5); }
    else if (d == "FREG")              { FRCL = 1; }
    else if (d == "ADREG")             { ADRL = 1; }
    else if (d.empty())                { continue; }
    else throw std::runtime_error("Bad store dest '" + d + "' (use TEMPxx, EFREGxx, FREG, ADREG)");
  }
  InstrFields f;
  f.SHFT = SHFT; f.TWT = TWT; f.TWA = TWA; f.EWT = EWT; f.EWA = EWA;
  f.FRCL = FRCL; f.ADRL = ADRL; f.BSEL = 1; f.YSEL = 1;
  progWords.push_back(packMpro(f));
}

std::vector<std::string> Assembler::alignMemoryOps() {
  std::vector<MproWords> aligned;
  std::vector<std::string> warnings;
  for (size_t i = 0; i < progWords.size(); i++) {
    const MproWords& w = progWords[i];
    if (instrAccessesMem(w) && (aligned.size() & 1) == 0) {
      std::ostringstream o;
      o << "Inserted NOP at step " << aligned.size()
        << " to align memory access (originally instruction " << (i + 1)
        << ") to odd step " << (aligned.size() + 1);
      warnings.push_back(o.str());
      aligned.push_back(makeNop());
    }
    aligned.push_back(w);
  }
  if (aligned.size() > 128) {
    std::ostringstream o;
    o << "Program is " << aligned.size() << " steps after alignment (max 128) — truncated";
    warnings.push_back(o.str());
  }
  progWords = aligned;
  return warnings;
}

void Assembler::getArrays(int rbl, SCSPDSPAssembly& out) const {
  std::memset(out.mpro, 0, sizeof(out.mpro));
  std::memset(out.coef, 0, sizeof(out.coef));
  std::memset(out.madrs, 0, sizeof(out.madrs));

  // COEF[0]=ZERO=0; rest are pre-shifted left by 3 (SCSP reads bits [15:3]).
  for (size_t i = 0; i < syms.coefOrder.size() && i < 64; i++) {
    int raw = (i == 0) ? 0 : syms.coef.at(syms.coefOrder[i]);
    int shifted = (raw << 3) & 0xFFFF;
    if (shifted & 0x8000) shifted |= ~0xFFFF;  // sign-extend to int
    out.coef[i] = (short)shifted;
  }
  for (size_t i = 0; i < syms.adrsOrder.size() && i < 32; i++)
    out.madrs[i] = (unsigned short)syms.adrs.at(syms.adrsOrder[i]);

  size_t n = progWords.size(); if (n > 128) n = 128;
  for (size_t i = 0; i < n; i++) {
    out.mpro[i*4 + 0] = progWords[i].w[0];
    out.mpro[i*4 + 1] = progWords[i].w[1];
    out.mpro[i*4 + 2] = progWords[i].w[2];
    out.mpro[i*4 + 3] = progWords[i].w[3];
  }
  out.rbl = rbl & 3;
  out.steps = (int)n;
}

}  // namespace

bool scspdspAssemble(const std::string& src, int rbl, SCSPDSPAssembly& out) {
  out.errors.clear();
  out.warnings.clear();
  Assembler asmblr;
  try {
    asmblr.assemble(src);
  } catch (const std::exception& e) {
    out.errors.push_back(e.what());
  }
  if (out.errors.empty()) {
    std::vector<std::string> w = asmblr.alignMemoryOps();
    out.warnings.insert(out.warnings.end(), w.begin(), w.end());
  }
  asmblr.getArrays(rbl, out);
  return out.errors.empty();
}
