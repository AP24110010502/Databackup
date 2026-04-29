/**
 * @file test_main.cpp
 * @brief Lightweight unit tests (no external framework required).
 *
 * Build:  make test   OR   g++ -std=c++17 -Iinclude tests/test_main.cpp
 *         src/File.cpp src/BackupManager.cpp src/RecoverySystem.cpp -o build/RunTests
 * Run:    ./build/RunTests
 */

#include "File.h"
#include "BackupManager.h"
#include "RecoverySystem.h"

#include <iostream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

// ── Tiny test harness ─────────────────────────────────────────────────────────

static int passed = 0, failed = 0;

#define TEST(name, expr)                                              \
    do {                                                              \
        if (expr) {                                                   \
            std::cout << "  [PASS] " << (name) << "\n";              \
            ++passed;                                                 \
        } else {                                                      \
            std::cout << "  [FAIL] " << (name) << "\n";              \
            ++failed;                                                 \
        }                                                             \
    } while (false)

// ── Test suites ───────────────────────────────────────────────────────────────

void testFileClass() {
    std::cout << "\n  === File class tests ===\n";

    File f(1, "test.txt", 50.0, 8.0, 0.5);
    TEST("File id",         f.getId()         == 1);
    TEST("File name",       f.getName()       == "test.txt");
    TEST("File size",       f.getSize()       == 50.0);
    TEST("File importance", f.getImportance() == 8.0);
    TEST("File risk",       f.getRisk()       == 0.5);

    // priority = 8.0 * 0.5 / 50.0 = 0.08
    TEST("File priority",   std::fabs(f.getPriority() - 0.08) < 1e-9);

    // value = round(8.0 * 0.5 * 100) = 400
    TEST("File value",      f.getValue() == 400);

    // Invalid constructions
    bool threw = false;
    try { File bad(2, "x", -1.0, 5.0, 0.5); }
    catch (...) { threw = true; }
    TEST("Negative size throws", threw);

    threw = false;
    try { File bad(3, "x", 1.0, 11.0, 0.5); }
    catch (...) { threw = true; }
    TEST("Importance > 10 throws", threw);

    threw = false;
    try { File bad(4, "x", 1.0, 5.0, 1.5); }
    catch (...) { threw = true; }
    TEST("Risk > 1 throws", threw);
}

void testGreedySelection() {
    std::cout << "\n  === BackupManager (Greedy) tests ===\n";

    BackupManager bm(200.0); // 200 MB capacity

    // File A: priority = 9*0.9/50  = 0.162  (should be picked first)
    // File B: priority = 7*0.5/100 = 0.035
    // File C: priority = 5*0.8/20  = 0.200  (highest priority, tiny)
    bm.addFile(File(1, "A.bin", 50.0,  9.0, 0.90));
    bm.addFile(File(2, "B.bin", 100.0, 7.0, 0.50));
    bm.addFile(File(3, "C.bin", 20.0,  5.0, 0.80));

    bm.runGreedyBackup();
    const auto& backed = bm.getBackedUpFiles();

    // All three fit (50+100+20 = 170 ≤ 200)
    TEST("All three files fit", backed.size() == 3);
    TEST("Used storage correct", std::fabs(bm.getUsedStorage() - 170.0) < 1e-9);

    // Now test with tight capacity (only 60 MB)
    BackupManager bm2(60.0);
    bm2.addFile(File(1, "A.bin", 50.0, 9.0, 0.90)); // priority 0.162
    bm2.addFile(File(2, "B.bin", 100.0, 7.0, 0.50)); // priority 0.035
    bm2.addFile(File(3, "C.bin", 20.0,  5.0, 0.80)); // priority 0.200
    bm2.runGreedyBackup();
    const auto& backed2 = bm2.getBackedUpFiles();

    // C (20MB, highest priority) and A (50MB) = 70 MB → doesn't fit together
    // C fits (20MB), then A (50MB) exceeds 60, B (100MB) also too big → only C
    TEST("Tight capacity: only highest-priority small file selected",
         backed2.size() == 1 && backed2[0].getId() == 3);
}

void testRandomLoss() {
    std::cout << "\n  === RecoverySystem (Random loss) tests ===\n";

    // File with risk 1.0 must always be lost
    // File with risk 0.0 must never be lost
    std::vector<File> files = {
        File(1, "certain_loss.dat", 10.0, 9.0, 1.0),
        File(2, "safe.dat",         10.0, 5.0, 0.0),
    };
    RecoverySystem rs(files, 500.0);
    rs.simulateLossRandom(42);

    bool foundLost  = false, foundSafe = false;
    for (const auto& f : rs.getLostFiles()) {
        if (f.getId() == 1) foundLost = true;
        if (f.getId() == 2) foundSafe = true;
    }
    TEST("Risk=1.0 file is always lost",  foundLost);
    TEST("Risk=0.0 file is never lost",  !foundSafe);
}

void testDPRecovery() {
    std::cout << "\n  === RecoverySystem (DP Recovery) tests ===\n";

    // Two files lost; bandwidth can only recover one.
    // File A: value=900, size=100 MB → value density 9 /MB
    // File B: value=300, size=50  MB → value density 6 /MB
    // With 100 MB bandwidth, greedy would pick A; DP should also pick A.
    std::vector<File> files = {
        File(1, "A.bin", 100.0, 9.0, 1.0),  // value = round(9*1*100) = 900
        File(2, "B.bin",  50.0, 3.0, 1.0),  // value = round(3*1*100) = 300
    };
    RecoverySystem rs(files, 100.0);
    rs.simulateLossPredefined({1, 2});
    int val = rs.runDPRecovery();

    TEST("DP picks higher-value file",       val == 900);
    TEST("Recovered list has 1 file",        rs.getRecoveredFiles().size() == 1);
    TEST("Recovered file is A",              rs.getRecoveredFiles()[0].getId() == 1);

    // Both fit when bandwidth is 200 MB
    RecoverySystem rs2(files, 200.0);
    rs2.simulateLossPredefined({1, 2});
    int val2 = rs2.runDPRecovery();
    TEST("DP recovers both when bandwidth allows", val2 == 1200);
    TEST("Recovered list has 2 files",             rs2.getRecoveredFiles().size() == 2);
}

void testPredefinedLoss() {
    std::cout << "\n  === Predefined loss tests ===\n";

    std::vector<File> files = {
        File(10, "alpha.txt", 30.0, 7.0, 0.5),
        File(20, "beta.txt",  40.0, 6.0, 0.4),
        File(30, "gamma.txt", 20.0, 8.0, 0.6),
    };
    RecoverySystem rs(files, 1000.0);
    rs.simulateLossPredefined({10, 30});

    const auto& lost = rs.getLostFiles();
    TEST("Predefined loss count",      lost.size() == 2);
    TEST("Predefined loss contains 10",
         std::any_of(lost.begin(), lost.end(), [](const File& f){ return f.getId() == 10; }));
    TEST("Predefined loss contains 30",
         std::any_of(lost.begin(), lost.end(), [](const File& f){ return f.getId() == 30; }));
    TEST("File 20 not in lost list",
         std::none_of(lost.begin(), lost.end(), [](const File& f){ return f.getId() == 20; }));
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════╗\n";
    std::cout <<   "  ║   DATA BACKUP PLANNER  — UNIT TESTS     ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════╝\n";

    testFileClass();
    testGreedySelection();
    testRandomLoss();
    testDPRecovery();
    testPredefinedLoss();

    std::cout << "\n  " << std::string(44, '=') << "\n";
    std::cout << "  Results : " << passed << " passed, " << failed << " failed.\n";
    std::cout << "  " << std::string(44, '=') << "\n\n";

    return (failed == 0) ? 0 : 1;
}
