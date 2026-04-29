#include "File.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cmath>

// ── Constructors ──────────────────────────────────────────────────────────────

File::File()
    : id_(0), name_("unnamed"), size_(1.0), importance_(1.0), risk_(0.0) {}

File::File(int id,
           const std::string& name,
           double size,
           double importance,
           double risk)
    : id_(id), name_(name), size_(size), importance_(importance), risk_(risk) {
    validate();
}

// ── Validation ────────────────────────────────────────────────────────────────

void File::validate() const {
    if (size_ <= 0)
        throw std::invalid_argument("File size must be positive.");
    if (importance_ < 1.0 || importance_ > 10.0)
        throw std::invalid_argument("Importance must be between 1 and 10.");
    if (risk_ < 0.0 || risk_ > 1.0)
        throw std::invalid_argument("Risk must be between 0.0 and 1.0.");
}

// ── Accessors ─────────────────────────────────────────────────────────────────

int         File::getId()         const { return id_; }
std::string File::getName()       const { return name_; }
double      File::getSize()       const { return size_; }
double      File::getImportance() const { return importance_; }
double      File::getRisk()       const { return risk_; }

/**
 * Priority = importance × risk / size
 *
 * This is the "value density" metric used by the greedy algorithm.
 * It rewards files that are highly important and likely to be lost,
 * while penalising files that consume a lot of storage.
 */
double File::getPriority() const {
    return (importance_ * risk_) / size_;
}

/**
 * Integer value used by the DP knapsack.
 * Scaled by 100 so fractional importance/risk differences are preserved
 * without floating-point arithmetic in the DP table.
 */
int File::getValue() const {
    return static_cast<int>(std::round(importance_ * risk_ * 100.0));
}

// ── Mutators ──────────────────────────────────────────────────────────────────

void File::setId(int id)                     { id_ = id; }
void File::setName(const std::string& name)  { name_ = name; }
void File::setSize(double size)              { size_ = size;       validate(); }
void File::setImportance(double importance)  { importance_ = importance; validate(); }
void File::setRisk(double risk)              { risk_ = risk;       validate(); }

// ── Display ───────────────────────────────────────────────────────────────────

void File::display() const {
    std::cout << std::left
              << "  [" << std::setw(3) << id_ << "] "
              << std::setw(28) << name_
              << " | Size: "       << std::setw(8)  << std::fixed << std::setprecision(1) << size_       << " MB"
              << " | Importance: " << std::setw(5)  << std::fixed << std::setprecision(1) << importance_
              << " | Risk: "       << std::setw(5)  << std::fixed << std::setprecision(2) << risk_
              << " | Priority: "   << std::setw(8)  << std::fixed << std::setprecision(4) << getPriority()
              << " | Value: "      << getValue()
              << "\n";
}
