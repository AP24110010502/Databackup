#include "File.h"
#include "BackupManager.h"
#include "RecoverySystem.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <sstream>
#include <vector>

// ── Banner ────────────────────────────────────────────────────────────────────

static void printBanner() {
    std::cout << "\n";
    std::cout << "  ██████╗  █████╗ ████████╗ █████╗     ██████╗  █████╗  ██████╗██╗  ██╗██╗   ██╗██████╗ \n";
    std::cout << "  ██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗    ██╔══██╗██╔══██╗██╔════╝██║ ██╔╝██║   ██║██╔══██╗\n";
    std::cout << "  ██║  ██║███████║   ██║   ███████║    ██████╔╝███████║██║     █████╔╝ ██║   ██║██████╔╝\n";
    std::cout << "  ██║  ██║██╔══██║   ██║   ██╔══██║    ██╔══██╗██╔══██║██║     ██╔═██╗ ██║   ██║██╔═══╝ \n";
    std::cout << "  ██████╔╝██║  ██║   ██║   ██║  ██║    ██████╔╝██║  ██║╚██████╗██║  ██╗╚██████╔╝██║     \n";
    std::cout << "  ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝    ╚═════╝ ╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝ ╚═════╝ ╚═╝     \n";
    std::cout << "\n";
    std::cout << "       ╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "       ║      BACKUP PRIORITY & RECOVERY PLANNER  v1.0               ║\n";
    std::cout << "       ║  Greedy Selection  +  Dynamic Programming Recovery          ║\n";
    std::cout << "       ╚═══════════════════════════════════════════════════════════════╝\n\n";
}

// ── Menu helpers ──────────────────────────────────────────────────────────────

static int getMenuChoice(int lo, int hi) {
    int choice;
    while (!(std::cin >> choice) || choice < lo || choice > hi) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Invalid input. Enter a number between " << lo << " and " << hi << ": ";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

static double getPositiveDouble(const std::string& prompt) {
    double val;
    std::cout << prompt;
    while (!(std::cin >> val) || val <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Please enter a positive number: ";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return val;
}

// ── Phase 1 menu ──────────────────────────────────────────────────────────────

static void runPhase1(BackupManager& bm) {
    std::cout << "\n  ┌─────────────────────────────────────────────┐\n";
    std::cout <<   "  │        PHASE 1 : BACKUP SELECTION            │\n";
    std::cout <<   "  │  Algorithm : Greedy (priority = imp×risk/sz) │\n";
    std::cout <<   "  └─────────────────────────────────────────────┘\n";

    std::cout << "\n  [1] Use default dataset (15 files)\n";
    std::cout <<   "  [2] Enter custom dataset\n";
    std::cout << "  Choice: ";
    int choice = getMenuChoice(1, 2);

    if (choice == 1)
        bm.loadDefaultDataset();
    else
        bm.loadFromUserInput();

    bm.displayAllFiles();

    std::cout << "\n  Running Greedy Backup Selection...\n";
    bm.runGreedyBackup();
    bm.displayBackedUpFiles();
}

// ── Phase 2 menu ──────────────────────────────────────────────────────────────

static void runPhase2(const BackupManager& bm, double bandwidthMB) {
    std::cout << "\n  ┌─────────────────────────────────────────────┐\n";
    std::cout <<   "  │        PHASE 2 : RECOVERY OPTIMISATION       │\n";
    std::cout <<   "  │  Algorithm : 0/1 Knapsack via DP             │\n";
    std::cout <<   "  └─────────────────────────────────────────────┘\n";

    RecoverySystem rs(bm.getBackedUpFiles(), bandwidthMB);

    std::cout << "\n  Loss Simulation Mode:\n";
    std::cout << "  [1] Random loss (uses file risk probability)\n";
    std::cout << "  [2] Predefined loss (you specify file IDs)\n";
    std::cout << "  Choice: ";
    int lossChoice = getMenuChoice(1, 2);

    if (lossChoice == 1) {
        unsigned int seed;
        std::cout << "  Enter random seed (e.g. 42): ";
        while (!(std::cin >> seed)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  Enter a non-negative integer: ";
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        rs.simulateLossRandom(seed);
    } else {
        std::cout << "  Enter file IDs to mark as lost (space-separated, then press Enter):\n  > ";
        std::string line;
        std::getline(std::cin, line);
        std::vector<int> ids;
        int id;
        std::istringstream ss(line);
        while (ss >> id) ids.push_back(id);
        rs.simulateLossPredefined(ids);
    }

    rs.displayLostFiles();

    if (!rs.getLostFiles().empty()) {
        std::cout << "\n  Running DP Recovery Optimisation...\n";
        rs.runDPRecovery();
        rs.displayRecoveredFiles();
    }

    rs.displayRecoverySummary();
}

// ── Final summary ─────────────────────────────────────────────────────────────

static void printFinalSummary(const BackupManager& bm) {
    const auto& all    = bm.getAllFiles();
    const auto& backed = bm.getBackedUpFiles();

    int totalValue  = 0, backedValue = 0;
    for (const auto& f : all)    totalValue  += f.getValue();
    for (const auto& f : backed) backedValue += f.getValue();

    double pct = (totalValue > 0)
                 ? (static_cast<double>(backedValue) / totalValue) * 100.0
                 : 0.0;

    std::cout << "\n  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║                     OVERALL SYSTEM SUMMARY                      ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════════════╝\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "  Total files in system     : " << all.size()    << "\n";
    std::cout << "  Files backed up (greedy)  : " << backed.size() << "\n";
    std::cout << "  Backup storage used       : " << bm.getUsedStorage()
              << " / " << bm.getCapacity() << " MB\n";
    std::cout << "  Total system value        : " << totalValue  << "\n";
    std::cout << "  Value protected by backup : " << backedValue
              << "  (" << pct << "%)\n";
    std::cout << "  " << std::string(66, '=') << "\n";
}

// ── Entry point ───────────────────────────────────────────────────────────────

int main() {
    // Need istringstream for predefined loss parsing
    // (already included via <string>; add the header explicitly below)
    printBanner();

    // ── Configuration ──────────────────────────────────────────────────────
    std::cout << "  SYSTEM CONFIGURATION\n";
    std::cout << "  " << std::string(40, '-') << "\n";
    double capacity  = getPositiveDouble("  Backup storage capacity (MB) : ");
    double bandwidth = getPositiveDouble("  Recovery bandwidth limit (MB) : ");

    // ── Phase 1 ────────────────────────────────────────────────────────────
    BackupManager bm(capacity);
    runPhase1(bm);

    if (bm.getBackedUpFiles().empty()) {
        std::cout << "\n  No files were backed up. "
                     "Consider increasing the storage capacity.\n";
        return 0;
    }

    // ── Phase 2 ────────────────────────────────────────────────────────────
    runPhase2(bm, bandwidth);

    // ── Summary ────────────────────────────────────────────────────────────
    printFinalSummary(bm);

    std::cout << "\n  Thank you for using the Data Backup Priority & Recovery Planner!\n\n";
    return 0;
}
