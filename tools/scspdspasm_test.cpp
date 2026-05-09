// Standalone unit tests for the SCSP DSP assembler (src/engine/scspdspasm.cpp).
// No test framework — minimal CHECK macros, prints PASS/FAIL counts, exits
// non-zero on any failure so CI can pick it up. Build target: scspdspasm_test.

#include "../src/engine/scspdspasm.h"

#include <cstdio>
#include <cstring>
#include <string>

static int g_passed = 0;
static int g_failed = 0;
static const char* g_currentTest = nullptr;

#define CHECK(cond) do { \
    if (!(cond)) { \
      std::fprintf(stderr, "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
      g_failed++; \
      return; \
    } \
  } while (0)

#define CHECK_EQ(actual, expected) do { \
    auto _a = (actual); \
    auto _e = (expected); \
    if (!(_a == _e)) { \
      std::fprintf(stderr, "  FAIL %s:%d: %s == %s (got %lld, expected %lld)\n", \
                   __FILE__, __LINE__, #actual, #expected, \
                   (long long)_a, (long long)_e); \
      g_failed++; \
      return; \
    } \
  } while (0)

static void runTest(const char* name, void (*fn)()) {
  g_currentTest = name;
  std::fprintf(stderr, "TEST %s\n", name);
  int before = g_failed;
  fn();
  if (g_failed == before) {
    g_passed++;
    std::fprintf(stderr, "  pass\n");
  }
}

// ── helpers ─────────────────────────────────────────────────────────

static SCSPDSPAssembly assembleOk(const char* src, int rbl = 0) {
  SCSPDSPAssembly a{};
  bool ok = scspdspAssemble(src, rbl, a);
  if (!ok) {
    for (auto& e : a.errors) std::fprintf(stderr, "  err: %s\n", e.c_str());
  }
  return a;
}

// ── test cases ──────────────────────────────────────────────────────

// 1. Empty source assembles successfully with zero steps.
static void test_empty() {
  auto a = assembleOk("");
  CHECK_EQ(a.errors.size(), 0u);
  CHECK_EQ(a.steps, 0);
  CHECK_EQ(a.coef[0], 0);  // ZERO is always present at index 0
}

// 2. NOP encodes as BSEL=1, YSEL=1 → w1 should have BSEL bit 0 set,
//    w2 should have YSEL bit set in YSEL[14:13]=01.
static void test_nop() {
  auto a = assembleOk("#PROG\nNOP\n");
  CHECK_EQ(a.errors.size(), 0u);
  CHECK_EQ(a.steps, 1);
  // word 1 is mpro[2]; BSEL is bit 0
  CHECK((a.mpro[2] & 0x0001) == 0x0001);
  // YSEL=1 in w2 (mpro[1]) at bits [14:13]
  CHECK((a.mpro[1] & 0x6000) == 0x2000);
  // Memory access bits clear
  CHECK((a.mpro[2] & 0x6000) == 0x0000);
}

// 3. COEF section: shift-left-by-3 packing, ZERO is always index 0.
static void test_coef_section() {
  auto a = assembleOk(
    "#COEF\n"
    "VOL = 1.0\n"      // 4096 → twosComp13 → 4095 (clamp would be 4096 OOR; actual: 4096 throws)
    "HALF = 0.5\n"     // 2048
    "QQ = &H100\n"     // 0x100 = 256
    "PCTHALF = %50\n"  // round(4095*0.5)=2048
    "#END\n"
  );
  // Note: 1.0 * 4096 = 4096 which is out of range. The assembler should error.
  // Adjust: use 0.999 to test pre-shift.
  CHECK(!a.errors.empty());

  auto b = assembleOk(
    "#COEF\n"
    "HALF = 0.5\n"      // raw 2048; <<3 = 16384 = 0x4000
    "QUARTER = 0.25\n"  // raw 1024; <<3 = 8192 = 0x2000
    "NEG = -0.5\n"      // raw -2048 → twosComp13 = 0x1800; <<3 = 0xC000 = -16384
    "HEX = &H100\n"     // raw 0x100; <<3 = 0x800
    "PCT = %25\n"       // round(4095*0.25) = 1024; <<3 = 0x2000
    "#END\n"
  );
  CHECK_EQ(b.errors.size(), 0u);
  CHECK_EQ(b.coef[0], 0);                // ZERO
  CHECK_EQ((int)b.coef[1], 0x4000);      // HALF
  CHECK_EQ((int)b.coef[2], 0x2000);      // QUARTER
  CHECK_EQ((int)b.coef[3], (int16_t)0xC000);  // NEG
  CHECK_EQ((int)b.coef[4], 0x0800);      // HEX
  CHECK_EQ((int)b.coef[5], 0x2000);      // PCT
}

// 4. ADRS section: literal int, hex, ms-time conversion.
static void test_adrs_section() {
  auto a = assembleOk(
    "#ADRS\n"
    "BUF1 = 1024\n"
    "BUF2 = &H400\n"
    "DLY10 = ms10\n"   // round(44100 * 0.010) = 441
    "#END\n"
  );
  CHECK_EQ(a.errors.size(), 0u);
  CHECK_EQ((int)a.madrs[0], 1024);
  CHECK_EQ((int)a.madrs[1], 0x400);
  CHECK_EQ((int)a.madrs[2], 441);
}

// 5. Single-product '@' chain → first step has ZERO=1, accumulators after.
static void test_at_chain_single() {
  auto a = assembleOk(
    "#COEF\n"
    "VOL = 0.5\n"
    "#PROG\n"
    "@ INPUT * VOL\n"
  );
  CHECK_EQ(a.errors.size(), 0u);
  CHECK_EQ(a.steps, 1);
  // ZERO bit is w1 bit 1
  CHECK((a.mpro[2] & 0x0002) == 0x0002);
  // XSEL=1 (INPUT) is w2 bit 15
  CHECK((a.mpro[1] & 0x8000) == 0x8000);
  // YSEL=1 (COEF) at bits [14:13]
  CHECK((a.mpro[1] & 0x6000) == 0x2000);
  // CRA=1 (VOL is the second symbol after ZERO) at w0 bits [14:9]
  CHECK_EQ((int)((a.mpro[3] >> 9) & 0x3F), 1);
}

// 6. Multi-product '@' chain accumulates (BSEL=1 on subsequent steps).
static void test_at_chain_multi() {
  auto a = assembleOk(
    "#COEF\n"
    "A = 0.5\n"
    "B = 0.25\n"
    "#PROG\n"
    "@ INPUT*A + INPUT*B\n"
  );
  CHECK_EQ(a.errors.size(), 0u);
  CHECK_EQ(a.steps, 2);
  // step 0: ZERO=1, BSEL=0
  CHECK((a.mpro[0*4 + 2] & 0x0002) == 0x0002);
  CHECK((a.mpro[0*4 + 2] & 0x0001) == 0x0000);
  // step 1: ZERO=0, BSEL=1
  CHECK((a.mpro[1*4 + 2] & 0x0002) == 0x0000);
  CHECK((a.mpro[1*4 + 2] & 0x0001) == 0x0001);
}

// 7. Inline @ ... > ... emits two micro-instructions (multiply, store).
static void test_at_with_inline_store() {
  auto a = assembleOk(
    "#COEF\n"
    "VOL = 0.5\n"
    "#PROG\n"
    "@ INPUT * VOL > EFREG0\n"
  );
  CHECK_EQ(a.errors.size(), 0u);
  CHECK_EQ(a.steps, 2);
  // step 1 should be a store: EWT=1 in w1 bit 12
  CHECK((a.mpro[1*4 + 2] & 0x1000) == 0x1000);
  // EWA=0 at w1 bits [11:8] → low 4 bits of byte
  CHECK_EQ((int)((a.mpro[1*4 + 2] >> 8) & 0xF), 0);
}

// 8. Memory access alignment: a single MR[...] read on an even step
//    should get a NOP inserted before it so it lands on an odd step.
static void test_mem_alignment() {
  auto a = assembleOk(
    "#ADRS\n"
    "BUF = 1024\n"
    "#PROG\n"
    "MR MR[BUF]\n"  // Would land on step 0 (even); expect NOP first
  );
  CHECK_EQ(a.errors.size(), 0u);
  CHECK_EQ(a.steps, 2);
  // step 0 should be the inserted NOP — no MWT/MRD bits
  CHECK((a.mpro[0*4 + 2] & 0x6000) == 0x0000);
  // step 1 should have MRD set
  CHECK((a.mpro[1*4 + 2] & 0x2000) == 0x2000);
  // Should have produced an alignment warning
  CHECK(!a.warnings.empty());
}

// 9. Mem access already on odd step: no NOP inserted.
static void test_mem_no_realignment() {
  auto a = assembleOk(
    "#ADRS\n"
    "BUF = 0\n"
    "#PROG\n"
    "NOP\n"
    "MR MR[BUF]\n"  // step 1 (odd), no insertion needed
  );
  CHECK_EQ(a.errors.size(), 0u);
  CHECK_EQ(a.steps, 2);
  CHECK(a.warnings.empty());
  CHECK((a.mpro[1*4 + 2] & 0x2000) == 0x2000);
}

// 10. Address-expression flags: DEC clears TABLE, ADREG sets ADREB,
//     +1 sets NXADR, /NF sets NOFL.
static void test_addr_flags() {
  auto a = assembleOk(
    "#ADRS\n"
    "DLY = 1024\n"
    "#PROG\n"
    "NOP\n"
    "MR MR[DLY+DEC+ADREG+1/NF]\n"
  );
  CHECK_EQ(a.errors.size(), 0u);
  CHECK_EQ(a.steps, 2);
  uint16_t w1 = a.mpro[1*4 + 2];
  uint16_t w0 = a.mpro[1*4 + 3];
  // TABLE=0 (cleared by DEC) at w1 bit 15
  CHECK((w1 & 0x8000) == 0x0000);
  // ADREB at w0 bit 1
  CHECK((w0 & 0x0002) == 0x0002);
  // NXADR at w0 bit 0
  CHECK((w0 & 0x0001) == 0x0001);
  // NOFL at w0 bit 8
  CHECK((w0 & 0x0100) == 0x0100);
}

// 11. Undefined coefficient symbol → error.
static void test_undefined_coef() {
  auto a = assembleOk(
    "#PROG\n"
    "@ INPUT * NOSUCH\n"
  );
  CHECK(!a.errors.empty());
  CHECK_EQ(a.steps, 0);
}

// 12. Bad PROG line → error.
static void test_bad_prog_line() {
  auto a = assembleOk(
    "#PROG\n"
    "GARBAGE FOO\n"
  );
  CHECK(!a.errors.empty());
}

// 13. Comments (lines starting with ' or trailing ') are ignored.
static void test_comments() {
  auto a = assembleOk(
    "' top-level comment\n"
    "#COEF\n"
    "VOL = 0.5  ' inline comment\n"
    "#PROG\n"
    "NOP  ' another comment\n"
  );
  CHECK_EQ(a.errors.size(), 0u);
  CHECK_EQ(a.steps, 1);
  CHECK_EQ((int)a.coef[1], 0x4000);  // VOL=0.5 still parsed
}

// 14. RBL is clamped to 0..3.
static void test_rbl_clamp() {
  auto a = assembleOk("", 7);
  CHECK_EQ(a.rbl, 3);
  auto b = assembleOk("", 2);
  CHECK_EQ(b.rbl, 2);
}

// 15. LDI MEMSx, MR[...] sets both MRD and IWT.
static void test_ldi() {
  auto a = assembleOk(
    "#ADRS\n"
    "BUF = 0\n"
    "#PROG\n"
    "LDI MEMS5, MR[BUF]\n"
  );
  CHECK_EQ(a.errors.size(), 0u);
  // Single instruction may have NOP prepended for memory alignment
  CHECK(a.steps >= 1);
  // The LDI step has IWT and MRD set
  int idx = a.steps - 1;
  uint16_t w1 = a.mpro[idx*4 + 2];
  uint16_t w2 = a.mpro[idx*4 + 1];
  CHECK((w1 & 0x2000) == 0x2000);  // MRD
  CHECK((w2 & 0x0020) == 0x0020);  // IWT bit 5 of w2
  // IWA=5 in low 5 bits of w2
  CHECK_EQ((int)(w2 & 0x1F), 5);
}

// ── main ────────────────────────────────────────────────────────────

int main() {
  runTest("empty",                    test_empty);
  runTest("nop",                      test_nop);
  runTest("coef_section",             test_coef_section);
  runTest("adrs_section",             test_adrs_section);
  runTest("at_chain_single",          test_at_chain_single);
  runTest("at_chain_multi",           test_at_chain_multi);
  runTest("at_with_inline_store",     test_at_with_inline_store);
  runTest("mem_alignment",            test_mem_alignment);
  runTest("mem_no_realignment",       test_mem_no_realignment);
  runTest("addr_flags",               test_addr_flags);
  runTest("undefined_coef",           test_undefined_coef);
  runTest("bad_prog_line",            test_bad_prog_line);
  runTest("comments",                 test_comments);
  runTest("rbl_clamp",                test_rbl_clamp);
  runTest("ldi",                      test_ldi);

  std::fprintf(stderr, "\n%d passed, %d failed\n", g_passed, g_failed);
  return g_failed == 0 ? 0 : 1;
}
