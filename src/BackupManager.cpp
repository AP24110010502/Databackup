#include "BackupManager.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <stdexcept>

// ── Construction ──────────────────────────────────────────────────────────────

BackupManager::BackupManager(double storageCapacityMB)
    : storageCapacityMB_(storageCapacityMB), usedStorageMB_(0.0) {
    if (storageCapacityMB <= 0)
        throw std::invalid_argument("Storage capacity must be positive.");
}

// ── Dataset management ────────────────────────────────────────────────────────

void BackupManager::addFile(const File& file) {
    allFiles_.push_back(file);
}

void BackupManager::loadDefaultDataset() {
    allFiles_.clear();
    // id, name, size(MB), importance(1-10), risk(0-1)
    allFiles_ = {
        { 1,  "payroll_2024.xlsx",        120.0,  9.5, 0.85 },
        { 2,  "client_contracts.pdf",      80.0,  9.0, 0.90 },
        { 3,  "source_code_v3.zip",       200.0,  8.5, 0.60 },
        { 4,  "employee_records.db",       50.0,  8.0, 0.75 },
        { 5,  "product_designs.ai",       150.0,  7.5, 0.55 },
        { 6,  "marketing_assets.zip",     300.0,  5.0, 0.40 },
        { 7,  "server_config.tar",         30.0,  9.8, 0.95 },
        { 8,  "audit_logs_2024.log",       40.0,  8.5, 0.70 },
        { 9,  "backup_keys.enc",           10.0, 10.0, 0.99 },
        { 10, "financial_reports_Q4.pdf",  60.0,  9.2, 0.80 },
        { 11, "temp_cache.tmp",           250.0,  1.0, 0.30 },
        { 12, "meeting_recordings.mp4",   500.0,  4.0, 0.50 },
        { 13, "legal_documents.pdf",       45.0,  9.5, 0.88 },
        { 14, "database_snapshot.sql",    180.0,  8.8, 0.72 },
        { 15, "brand_guidelines.pdf",      25.0,  6.0, 0.45 },
    };
}

void BackupManager::loadFromUserInput() {
    allFiles_.clear();
    int n;
    std::cout << "\n  How many files would you like to enter? ";
    while (!(std::cin >> n) || n <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Please enter a positive integer: ";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 1; i <= n; ++i) {
        std::string name;
        double size, importance, risk;

        std::cout << "\n  --- File " << i << " ---\n";

        std::cout << "  Name       : ";
        std::getline(std::cin, name);

        std::cout << "  Size (MB)  : ";
        while (!(std::cin >> size) || size <= 0) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  Enter a positive number: ";
        }

        std::cout << "  Importance (1-10): ";
        while (!(std::cin >> importance) || importance < 1 || importance > 10) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  Enter a value between 1 and 10: ";
        }

        std::cout << "  Risk (0.0-1.0)   : ";
        while (!(std::cin >> risk) || risk < 0.0 || risk > 1.0) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  Enter a value between 0.0 and 1.0: ";
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        allFiles_.emplace_back(i, name, size, importance, risk);
    }
}

// ── Core algorithm ────────────────────────────────────────────────────────────

void BackupManager::runGreedyBackup() {
    backedUpFiles_.clear();
    usedStorageMB_ = 0.0;

    // Step 1: sort by priority descending  (importance × risk / size)
    std::vector<File> sorted = allFiles_;
    std::sort(sorted.begin(), sorted.end(), [](const File& a, const File& b) {
        return a.getPriority() > b.getPriority();
    });

    // Step 2: greedy selection
    for (const auto& f : sorted) {
        if (usedStorageMB_ + f.getSize() <= storageCapacityMB_) {
            backedUpFiles_.push_back(f);
            usedStorageMB_ += f.getSize();
        }
    }
}

// ── Accessors ─────────────────────────────────────────────────────────────────

const std::vector<File>& BackupManager::getAllFiles()       const { return allFiles_; }
const std::vector<File>& BackupManager::getBackedUpFiles()  const { return backedUpFiles_; }
double                   BackupManager::getCapacity()        const { return storageCapacityMB_; }
double                   BackupManager::getUsedStorage()     const { return usedStorageMB_; }

// ── Display ───────────────────────────────────────────────────────────────────

void BackupManager::displayAllFiles() const {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║                              ALL FILES IN SYSTEM                                         ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════════════════════════════════════╝\n";
    std::cout << "  " << std::string(92, '-') << "\n";
    std::cout << std::left
              << "  " << std::setw(5)  << "ID"
              << std::setw(30) << "Name"
              << std::setw(12) << "Size(MB)"
              << std::setw(14) << "Importance"
              << std::setw(10) << "Risk"
              << std::setw(12) << "Priority"
              << std::setw(8)  << "Value"
              << "\n";
    std::cout << "  " << std::string(92, '-') << "\n";
    for (const auto& f : allFiles_) f.display();
    std::cout << "  " << std::string(92, '-') << "\n";
    std::cout << "  Total files: " << allFiles_.size() << "\n";
}

void BackupManager::displayBackedUpFiles() const {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║                         FILES SELECTED FOR BACKUP (GREEDY)                               ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════════════════════════════════════╝\n";
    if (backedUpFiles_.empty()) {
        std::cout << "  No files were selected for backup.\n";
        return;
    }
    std::cout << "  " << std::string(92, '-') << "\n";
    for (const auto& f : backedUpFiles_) f.display();
    std::cout << "  " << std::string(92, '-') << "\n";
    displayStorageSummary();
}

void BackupManager::displayStorageSummary() const {
    double pct = (storageCapacityMB_ > 0)
                 ? (usedStorageMB_ / storageCapacityMB_) * 100.0
                 : 0.0;
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "  Files backed up : " << backedUpFiles_.size()
              << " / " << allFiles_.size() << "\n";
    std::cout << "  Storage used    : " << usedStorageMB_
              << " MB / " << storageCapacityMB_ << " MB"
              << "  (" << pct << "%)\n";
}
