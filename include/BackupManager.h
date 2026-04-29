#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

#include "File.h"
#include <vector>
#include <string>

/**
 * @class BackupManager
 * @brief Manages the file dataset and applies a Greedy Algorithm for backup selection.
 *
 * WHY GREEDY?
 * -----------
 * Selecting which files to back up under a storage limit is a variant of the
 * 0/1 Knapsack problem.  The exact DP solution for knapsack is O(n × W) and
 * becomes impractical when n is large or W is continuous.
 *
 * A greedy approach — sort by priority (importance × risk / size) and pick
 * greedily — runs in O(n log n) and yields near-optimal results for this
 * domain because:
 *   1. Files with the highest "bang per byte" are selected first.
 *   2. The priority ratio naturally balances value against cost.
 *   3. In real backup systems speed matters; greedy is fast enough for
 *      thousands of files.
 *
 * Trade-off: Greedy is not always globally optimal for 0/1 knapsack, but it
 * is a well-accepted heuristic when items cannot be split.
 */
class BackupManager {
public:
    // ── Construction ─────────────────────────────────────────────────────────
    explicit BackupManager(double storageCapacityMB);

    // ── Dataset management ───────────────────────────────────────────────────
    void addFile(const File& file);
    void loadDefaultDataset();
    void loadFromUserInput();

    // ── Core algorithm ───────────────────────────────────────────────────────
    /**
     * @brief Phase 1 – Greedy backup selection.
     *
     * Steps:
     *  1. Compute priority = importance × risk / size for every file.
     *  2. Sort files in descending priority order.
     *  3. Iterate; select a file if it fits within remaining capacity.
     */
    void runGreedyBackup();

    // ── Accessors ────────────────────────────────────────────────────────────
    const std::vector<File>& getAllFiles()      const;
    const std::vector<File>& getBackedUpFiles() const;
    double                   getCapacity()      const;
    double                   getUsedStorage()   const;

    // ── Display ──────────────────────────────────────────────────────────────
    void displayAllFiles()      const;
    void displayBackedUpFiles() const;
    void displayStorageSummary() const;

private:
    double            storageCapacityMB_;
    double            usedStorageMB_;
    std::vector<File> allFiles_;
    std::vector<File> backedUpFiles_;
};

#endif // BACKUP_MANAGER_H
