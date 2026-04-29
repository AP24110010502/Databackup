#ifndef RECOVERY_SYSTEM_H
#define RECOVERY_SYSTEM_H

#include "File.h"
#include <vector>
#include <string>

/**
 * @class RecoverySystem
 * @brief Simulates data loss and uses Dynamic Programming to maximise recovery value.
 *
 * WHY DYNAMIC PROGRAMMING?
 * -------------------------
 * After a loss event the administrator can recover files from backup, but
 * recovery itself has a bandwidth / time constraint (modelled as a second
 * capacity limit, e.g. network bandwidth in MB).
 *
 * This is a classic 0/1 Knapsack problem:
 *   - Items   : backed-up files that were also lost.
 *   - Weight  : file size (recovery bandwidth consumed).
 *   - Value   : integer value  =  importance × risk × 100  (rounded).
 *   - Capacity: recovery bandwidth limit.
 *
 * DP guarantees the OPTIMAL solution in O(n × W) time, which is essential
 * here because we want to maximise the value of what we get back, not just
 * approximate it.  Unlike the greedy phase (where speed dominates), the
 * recovery phase is a one-time decision where optimality matters.
 *
 * DP Table:
 *   dp[i][w] = max value using first i files with w MB of bandwidth remaining.
 *   Transition:
 *     dp[i][w] = max( dp[i-1][w],                         // skip file i
 *                     dp[i-1][w - size_i] + value_i )     // recover file i
 *
 * Back-tracking through the table identifies exactly which files to recover.
 */
class RecoverySystem {
public:
    // ── Construction ─────────────────────────────────────────────────────────
    RecoverySystem(const std::vector<File>& backedUpFiles,
                   double recoveryBandwidthMB);

    // ── Loss simulation ──────────────────────────────────────────────────────
    /**
     * @brief Randomly simulates data loss based on each file's risk value.
     *        A file is "lost" if rand()/RAND_MAX < risk.
     */
    void simulateLossRandom(unsigned int seed = 42);

    /**
     * @brief Predefined loss – caller specifies which file IDs are lost.
     */
    void simulateLossPredefined(const std::vector<int>& lostFileIds);

    // ── Core algorithm ───────────────────────────────────────────────────────
    /**
     * @brief Phase 2 – 0/1 Knapsack DP to maximise recovery value.
     * @return Maximum achievable recovery value.
     */
    int runDPRecovery();

    // ── Accessors ────────────────────────────────────────────────────────────
    const std::vector<File>& getLostFiles()      const;
    const std::vector<File>& getRecoveredFiles() const;
    int                      getTotalRecoveryValue() const;

    // ── Display ──────────────────────────────────────────────────────────────
    void displayLostFiles()      const;
    void displayRecoveredFiles() const;
    void displayRecoverySummary() const;

private:
    std::vector<File> backedUpFiles_;
    std::vector<File> lostFiles_;       // subset of backedUpFiles_ marked lost
    std::vector<File> recoveredFiles_;  // optimal subset chosen by DP
    double            bandwidthMB_;     // recovery capacity
    int               totalValue_;

    // DP implementation (integer weights via unit scaling)
    int dpKnapsack(const std::vector<File>& items, int capacityUnits);
};

#endif // RECOVERY_SYSTEM_H
