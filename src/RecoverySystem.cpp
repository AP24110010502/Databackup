#include "RecoverySystem.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <vector>

// ── Construction ──────────────────────────────────────────────────────────────

RecoverySystem::RecoverySystem(const std::vector<File>& backedUpFiles,
                               double recoveryBandwidthMB)
    : backedUpFiles_(backedUpFiles),
      bandwidthMB_(recoveryBandwidthMB),
      totalValue_(0) {}

// ── Loss simulation ───────────────────────────────────────────────────────────

void RecoverySystem::simulateLossRandom(unsigned int seed) {
    lostFiles_.clear();
    std::srand(seed);
    for (const auto& f : backedUpFiles_) {
        double roll = static_cast<double>(std::rand()) / RAND_MAX;
        if (roll < f.getRisk()) {
            lostFiles_.push_back(f);
        }
    }
}

void RecoverySystem::simulateLossPredefined(const std::vector<int>& lostFileIds) {
    lostFiles_.clear();
    for (const auto& f : backedUpFiles_) {
        for (int id : lostFileIds) {
            if (f.getId() == id) {
                lostFiles_.push_back(f);
                break;
            }
        }
    }
}

// ── DP Knapsack (private helper) ──────────────────────────────────────────────

/**
 * Classic 0/1 Knapsack via bottom-up DP.
 *
 * Sizes are scaled to integers (×10, rounded) so we work with integer weights.
 * The DP table is (n+1) × (capacity+1).
 *
 * After filling the table we back-track to find which items were selected.
 */
int RecoverySystem::dpKnapsack(const std::vector<File>& items, int capacityUnits) {
    int n = static_cast<int>(items.size());
    if (n == 0 || capacityUnits <= 0) return 0;

    // Build weight & value arrays (integer-scaled)
    std::vector<int> w(n), v(n);
    for (int i = 0; i < n; ++i) {
        w[i] = static_cast<int>(std::round(items[i].getSize() * 10.0)); // ×10 MB → 0.1 MB units
        v[i] = items[i].getValue();
    }

    // 2-D DP table  dp[i][c] = best value using first i items with capacity c
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(capacityUnits + 1, 0));

    for (int i = 1; i <= n; ++i) {
        for (int c = 0; c <= capacityUnits; ++c) {
            dp[i][c] = dp[i - 1][c]; // skip item i
            if (w[i - 1] <= c) {
                int take = dp[i - 1][c - w[i - 1]] + v[i - 1];
                if (take > dp[i][c]) dp[i][c] = take;
            }
        }
    }

    // Back-track to find selected items
    recoveredFiles_.clear();
    int c = capacityUnits;
    for (int i = n; i >= 1; --i) {
        if (dp[i][c] != dp[i - 1][c]) {
            recoveredFiles_.push_back(items[i - 1]);
            c -= w[i - 1];
        }
    }
    // Reverse so output is in original order
    std::reverse(recoveredFiles_.begin(), recoveredFiles_.end());

    return dp[n][capacityUnits];
}

// ── Core algorithm ────────────────────────────────────────────────────────────

int RecoverySystem::runDPRecovery() {
    recoveredFiles_.clear();
    totalValue_ = 0;

    if (lostFiles_.empty()) {
        std::cout << "  No lost files to recover.\n";
        return 0;
    }

    // Scale bandwidth to integer units (×10 → 0.1 MB resolution)
    int capacityUnits = static_cast<int>(std::round(bandwidthMB_ * 10.0));
    totalValue_ = dpKnapsack(lostFiles_, capacityUnits);
    return totalValue_;
}

// ── Accessors ─────────────────────────────────────────────────────────────────

const std::vector<File>& RecoverySystem::getLostFiles()       const { return lostFiles_; }
const std::vector<File>& RecoverySystem::getRecoveredFiles()  const { return recoveredFiles_; }
int                      RecoverySystem::getTotalRecoveryValue() const { return totalValue_; }

// ── Display ───────────────────────────────────────────────────────────────────

void RecoverySystem::displayLostFiles() const {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║                             SIMULATED DATA LOSS                                          ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════════════════════════════════════╝\n";
    if (lostFiles_.empty()) {
        std::cout << "  No backed-up files were lost.\n";
        return;
    }
    std::cout << "  " << std::string(92, '-') << "\n";
    for (const auto& f : lostFiles_) f.display();
    std::cout << "  " << std::string(92, '-') << "\n";
    std::cout << "  Total lost : " << lostFiles_.size() << " files\n";
}

void RecoverySystem::displayRecoveredFiles() const {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║                        FILES RECOVERED VIA DP OPTIMISATION                               ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════════════════════════════════════╝\n";
    if (recoveredFiles_.empty()) {
        std::cout << "  No files could be recovered within bandwidth constraints.\n";
        return;
    }
    std::cout << "  " << std::string(92, '-') << "\n";
    for (const auto& f : recoveredFiles_) f.display();
    std::cout << "  " << std::string(92, '-') << "\n";
}

void RecoverySystem::displayRecoverySummary() const {
    // Compute totals
    double totalLostSize = 0, totalRecoveredSize = 0;
    int    totalLostValue = 0;
    for (const auto& f : lostFiles_)      { totalLostSize  += f.getSize(); totalLostValue += f.getValue(); }
    for (const auto& f : recoveredFiles_) { totalRecoveredSize += f.getSize(); }

    double pct = (totalLostValue > 0)
                 ? (static_cast<double>(totalValue_) / totalLostValue) * 100.0
                 : 0.0;

    std::cout << "\n  ╔══════════════════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║                              RECOVERY SUMMARY                                            ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════════════════════════════════════╝\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "  Recovery bandwidth limit : " << bandwidthMB_ << " MB\n";
    std::cout << "  Files lost               : " << lostFiles_.size()      << "  (" << totalLostSize      << " MB)\n";
    std::cout << "  Files recovered (DP opt) : " << recoveredFiles_.size() << "  (" << totalRecoveredSize << " MB)\n";
    std::cout << "  Value of lost files      : " << totalLostValue  << "\n";
    std::cout << "  Value recovered          : " << totalValue_     << "  (" << pct << "% of lost value)\n";
    std::cout << "  " << std::string(50, '=') << "\n";
}
