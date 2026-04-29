#ifndef FILE_H
#define FILE_H

#include <string>

/**
 * @class File
 * @brief Represents a file in the system with backup-relevant attributes.
 *
 * Each file has:
 *  - size       : Storage cost in MB (used as the "weight" in knapsack)
 *  - importance : Criticality score 1–10 (how vital the file is)
 *  - risk       : Probability of loss 0.0–1.0 (higher = more likely to be lost)
 *
 * Priority Formula:
 *   priority = (importance × risk) / size
 *
 * Rationale:
 *   - High importance → file is critical to operations.
 *   - High risk       → file is likely to be lost without backup.
 *   - Large size      → consumes more limited backup storage.
 *   Dividing by size penalizes large files, favouring smaller high-value files —
 *   exactly the greedy heuristic used for the fractional knapsack problem.
 */
class File {
public:
    // ── Constructors ────────────────────────────────────────────────────────
    File();
    File(int id,
         const std::string& name,
         double size,
         double importance,
         double risk);

    // ── Accessors ────────────────────────────────────────────────────────────
    int         getId()         const;
    std::string getName()       const;
    double      getSize()       const;   // MB
    double      getImportance() const;   // 1–10
    double      getRisk()       const;   // 0.0–1.0
    double      getPriority()   const;   // importance × risk / size
    int         getValue()      const;   // integer value for DP  (importance × risk × 100)

    // ── Mutators ─────────────────────────────────────────────────────────────
    void setId(int id);
    void setName(const std::string& name);
    void setSize(double size);
    void setImportance(double importance);
    void setRisk(double risk);

    // ── Utilities ────────────────────────────────────────────────────────────
    void display() const;

private:
    int         id_;
    std::string name_;
    double      size_;        // MB
    double      importance_;  // 1–10
    double      risk_;        // 0.0–1.0

    void validate() const;
};

#endif // FILE_H
