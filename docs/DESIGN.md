# Design Document — Data Backup Priority & Recovery Planner

## 1. Problem Statement

Modern organisations store thousands of files. When backup storage is limited,
deciding *which* files to back up is a resource-allocation problem.  
After a data-loss event, deciding *which* backed-up files to restore first
(given limited recovery bandwidth) is a value-maximisation problem.

This project models both decisions with appropriate algorithms.

---

## 2. Data Model

### File Attributes

| Attribute  | Type   | Range     | Meaning                                         |
|------------|--------|-----------|-------------------------------------------------|
| size       | double | > 0 MB    | Storage consumed; the "cost" in both knapsacks  |
| importance | double | 1 – 10    | How critical the file is to business operations |
| risk       | double | 0.0 – 1.0 | Estimated probability of loss without backup    |

### Derived Metrics

```
priority = importance × risk / size   (greedy sort key)
value    = round(importance × risk × 100)   (integer DP value)
```

The priority metric combines both axes of "why back up this file" (high importance
and high risk) while penalising the storage cost.  Scaling by 100 in the value
formula preserves two decimal places of precision in the integer DP table.

---

## 3. Phase 1 — Greedy Backup Selection

### Algorithm

```
GREEDY-BACKUP(files, capacity):
  sort files by priority DESC          // O(n log n)
  used ← 0
  selected ← []
  for each file f in sorted order:
    if used + f.size ≤ capacity:
      selected.append(f)
      used ← used + f.size
  return selected
```

### Justification

The backup selection problem is a **0/1 Knapsack** variant:
- *Items* = files
- *Weight* = file size
- *Value* = importance × risk
- *Capacity* = backup storage limit

**Why not exact DP here?**  
The exact DP knapsack requires integer weights.  File sizes are real-valued (e.g.
123.7 MB), so scaling to integers would produce a table with capacity × 10 columns
— potentially millions of cells for large storage limits.

**Why greedy works well here:**  
For the *fractional* knapsack problem, greedy by value density is provably optimal.
For 0/1 knapsack it is a heuristic, but in practice:
1. The priority ratio (value density) is a strong predictor of selection.
2. Real backup workloads have many small high-priority files; the greedy
   approximation ratio is excellent empirically.
3. Speed matters: O(n log n) vs O(n × W).

### Complexity

| Step | Complexity |
|------|------------|
| Priority calculation | O(n) |
| Sort | O(n log n) |
| Selection scan | O(n) |
| **Total** | **O(n log n)** |

---

## 4. Phase 2 — DP Recovery Optimisation

### Algorithm

```
DP-RECOVERY(lost_files, bandwidth):
  // Scale sizes to integer units (×10 → 0.1 MB resolution)
  n ← |lost_files|
  W ← round(bandwidth × 10)

  // Build dp table
  dp[0..n][0..W] ← 0
  for i = 1 to n:
    for w = 0 to W:
      dp[i][w] ← dp[i-1][w]           // skip item i
      if weight[i] ≤ w:
        dp[i][w] ← max(dp[i][w],
                       dp[i-1][w - weight[i]] + value[i])

  // Back-track to find selection
  selected ← []
  w ← W
  for i = n down to 1:
    if dp[i][w] ≠ dp[i-1][w]:
      selected.prepend(lost_files[i])
      w ← w - weight[i]

  return dp[n][W], selected
```

### Justification

After a loss event there is a clear optimisation goal: **maximise the total value
of recovered files** subject to bandwidth.  This is exactly the 0/1 Knapsack
problem, for which DP provides the **guaranteed optimal** solution.

Unlike the backup phase, where speed is paramount and sizes are real-valued,
recovery:
1. Operates on a *subset* of backed-up files (typically much smaller).
2. Is a one-time decision — optimality matters more than speed.
3. Has file sizes that can be sensibly discretised to 0.1 MB without loss of
   practical accuracy.

### Complexity

| Step | Complexity |
|------|------------|
| Table fill | O(n × W) where W = bandwidth × 10 |
| Back-tracking | O(n) |
| **Total** | **O(n × W)** |

### Memory

The DP table stores (n+1) × (W+1) integers.  For n = 50 files and W = 5000
(500 MB bandwidth), this is ~250 KB — negligible on modern hardware.

---

## 5. Loss Simulation

### Random Mode

Each backed-up file is independently tested against its `risk` value:
```
for each file f:
    roll ← uniform_random(0, 1)
    if roll < f.risk:
        mark f as lost
```
A seed parameter ensures reproducibility for testing.

### Predefined Mode

The user specifies a list of file IDs to mark as lost, enabling deterministic
scenarios useful for demonstrations and regression tests.

---

## 6. Design Decisions

| Decision | Choice | Reason |
|----------|--------|--------|
| Weight representation | Integers (×10 scaled) | DP requires integer weights |
| Value representation | `int` | Avoids float DP, prevents rounding drift |
| DP table storage | 2-D vector | Enables back-tracking for item selection |
| Greedy sort | `std::sort` with lambda | STL efficiency, readable comparator |
| Input validation | Constructor throws | Fail-fast, prevents invalid state |

---

## 7. Future Enhancements

- **File versioning** — track multiple versions of the same file  
- **Incremental backup** — only back up changed blocks  
- **Multi-tier storage** — cold storage (cheap/slow) vs hot storage (fast/expensive)  
- **Network-aware recovery** — model bandwidth as time-varying  
- **GUI frontend** — visualise backup/recovery decisions  
