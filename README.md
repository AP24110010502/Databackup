# Data Backup Priority & Recovery Planner

A C++ console application that simulates a real-world file backup system using a **Greedy Algorithm** for backup selection and **Dynamic Programming (0/1 Knapsack)** for recovery optimisation.

---

## Table of Contents

1. [Project Overview](#project-overview)  
2. [Features](#features)  
3. [Project Structure](#project-structure)  
4. [Algorithms Explained](#algorithms-explained)  
   - [Priority Formula](#priority-formula)  
   - [Phase 1 – Greedy Backup Selection](#phase-1--greedy-backup-selection)  
   - [Phase 2 – DP Recovery Optimisation](#phase-2--dp-recovery-optimisation)  
5. [Classes](#classes)  
6. [Build Instructions](#build-instructions)  
7. [Running the Program](#running-the-program)  
8. [Sample Output](#sample-output)  
9. [Running Tests](#running-tests)  
10. [Contributing](#contributing)  
11. [License](#license)

---

## Project Overview

Organisations routinely need to back up files under storage constraints and then recover the most valuable files after a data-loss event.  
This project models exactly that scenario:

| Phase | Problem | Solution |
|-------|---------|----------|
| 1 – Backup | Which files to back up within limited storage? | Greedy algorithm sorted by priority |
| 2 – Recovery | Which backed-up lost files to restore within bandwidth limits? | 0/1 Knapsack via Dynamic Programming |

---

## Features

- **File dataset** – 15-file default dataset or interactive custom input  
- **Risk-weighted priority** – `priority = importance × risk / size`  
- **Greedy backup selection** – O(n log n) sort then linear scan  
- **Loss simulation** – random (seed-controlled) or predefined file IDs  
- **DP recovery optimisation** – guaranteed optimal recovery within bandwidth  
- **Detailed console output** – tables, summaries, percentage metrics  
- **Unit tests** – no external framework required

---

## Project Structure

```
DataBackupPlanner/
├── include/
│   ├── File.h               # File entity with attributes & priority
│   ├── BackupManager.h      # Greedy backup selection
│   └── RecoverySystem.h     # Loss simulation + DP recovery
├── src/
│   ├── File.cpp
│   ├── BackupManager.cpp
│   ├── RecoverySystem.cpp
│   └── main.cpp             # Interactive entry point
├── tests/
│   └── test_main.cpp        # Unit tests (no external framework)
├── docs/
│   └── DESIGN.md            # Detailed design document
├── CMakeLists.txt           # CMake build script
├── Makefile                 # GNU Make alternative
├── .gitignore
└── README.md
```

---

## Algorithms Explained

### Priority Formula

```
priority = (importance × risk) / size
```

| Factor | Role |
|--------|------|
| `importance` (1–10) | How critical the file is to operations |
| `risk` (0.0–1.0) | Probability that the file will be lost without backup |
| `size` (MB) | Storage cost — penalises large files |

**Rationale:** A file that is highly important, likely to be lost, *and* small is the best candidate for backup. Dividing by size ensures the metric represents "value per megabyte" — exactly the greedy knapsack density heuristic.

**Integer value for DP:**  
`value = round(importance × risk × 100)`  
Scaling by 100 preserves fractional differences without floating-point DP.

---

### Phase 1 – Greedy Backup Selection

**Why Greedy?**

The backup selection problem is a variant of the 0/1 Knapsack problem (storage capacity is the knapsack weight).  
The exact DP solution is O(n × W), which becomes slow when n is large or W is a large floating-point number.

The greedy approach:
1. Compute `priority = importance × risk / size` for every file.
2. Sort files in **descending priority** order — O(n log n).
3. Iterate through the sorted list; select a file if it fits within remaining capacity.

This runs in O(n log n) and yields near-optimal results because:
- Files with the highest "bang per byte" are always preferred.
- The priority ratio naturally balances value against storage cost.
- In practice, greedy for fractional knapsack is **optimal**; for 0/1 knapsack it is a fast, well-accepted heuristic.

**Trade-off:** Greedy is not guaranteed to be globally optimal for 0/1 knapsack (where items cannot be split), but for real backup workloads the approximation quality is excellent and the speed benefit is significant.

---

### Phase 2 – DP Recovery Optimisation

**Why Dynamic Programming?**

After a loss event, recovery also has a constraint: recovery bandwidth (MB that can be transferred back from backup in the available time window).  
This is a *classic* 0/1 Knapsack problem where:

| Knapsack term | Backup meaning |
|---------------|----------------|
| Items | Backed-up files that were also lost |
| Weight | File size (bandwidth consumed during recovery) |
| Value | `round(importance × risk × 100)` |
| Capacity | Recovery bandwidth limit (MB) |

DP guarantees the **globally optimal** recovery selection in O(n × W) time.  
Unlike the backup phase (where speed dominates and greedy is acceptable), the recovery phase is a one-time decision where **maximising recovered value** is paramount.

**DP Recurrence:**

```
dp[i][w] = max value using the first i files with w MB of bandwidth remaining

dp[i][w] = max(
    dp[i-1][w],                         // skip file i
    dp[i-1][w - size_i] + value_i       // recover file i (if size_i ≤ w)
)

Base case: dp[0][w] = 0 for all w
```

After filling the table, back-tracking from `dp[n][W]` identifies exactly which files to recover.

**Size scaling:** File sizes are multiplied by 10 and rounded to integers, giving 0.1 MB resolution in the DP table while keeping weights integral.

---

## Classes

### `File`
Represents a single file.

| Member | Type | Description |
|--------|------|-------------|
| `id_` | `int` | Unique identifier |
| `name_` | `string` | File name |
| `size_` | `double` | Storage size in MB |
| `importance_` | `double` | Criticality (1–10) |
| `risk_` | `double` | Loss probability (0.0–1.0) |
| `getPriority()` | `double` | `importance × risk / size` |
| `getValue()` | `int` | `round(importance × risk × 100)` |

### `BackupManager`
Owns the file dataset and runs greedy backup selection.

| Method | Description |
|--------|-------------|
| `loadDefaultDataset()` | Loads 15 built-in files |
| `loadFromUserInput()` | Interactive dataset entry |
| `runGreedyBackup()` | Sorts by priority, selects greedily |
| `getBackedUpFiles()` | Returns selected files |

### `RecoverySystem`
Simulates loss and performs DP recovery.

| Method | Description |
|--------|-------------|
| `simulateLossRandom(seed)` | Random loss based on risk |
| `simulateLossPredefined(ids)` | Loss by specified file IDs |
| `runDPRecovery()` | 0/1 Knapsack DP, returns max value |
| `getRecoveredFiles()` | Returns optimally recovered files |

---

## Build Instructions

### Option A — CMake (recommended)

```bash
# From project root
mkdir build && cd build
cmake ..
cmake --build .
```

### Option B — GNU Make

```bash
make          # builds ./build/DataBackupPlanner
make run      # builds and runs
make test     # builds and runs unit tests
make clean    # removes build/
```

### Option C — Direct g++ compilation

```bash
g++ -std=c++17 -Iinclude \
    src/main.cpp src/File.cpp src/BackupManager.cpp src/RecoverySystem.cpp \
    -o DataBackupPlanner
./DataBackupPlanner
```

**Requirements:** GCC ≥ 7 or Clang ≥ 5 with C++17 support.

---

## Running the Program

```
./build/DataBackupPlanner
```

The program will prompt you for:
1. **Backup storage capacity** (MB)
2. **Recovery bandwidth limit** (MB)
3. **Dataset choice** – default (15 files) or custom input
4. **Loss simulation mode** – random (with seed) or predefined file IDs

---

## Sample Output

```
  SYSTEM CONFIGURATION
  ----------------------------------------
  Backup storage capacity (MB)  : 500
  Recovery bandwidth limit (MB) : 200

  ALL FILES IN SYSTEM
  --------------------------------------------------------------------------------------------
  ID    Name                         Size(MB)  Importance  Risk   Priority  Value
  --------------------------------------------------------------------------------------------
  [  9] backup_keys.enc               10.0 MB | Imp: 10.0 | Risk: 0.99 | Pri: 0.9900 | Val: 990
  [  7] server_config.tar             30.0 MB | Imp:  9.8 | Risk: 0.95 | Pri: 0.3097 | Val: 931
  ...

  FILES SELECTED FOR BACKUP (GREEDY)
  --------------------------------------------------------------------------------------------
  [  9] backup_keys.enc        ...
  [  7] server_config.tar      ...
  ...
  Files backed up : 9 / 15
  Storage used    : 465.0 MB / 500.0 MB  (93.0%)

  SIMULATED DATA LOSS
  [  9] backup_keys.enc  ...
  [  7] server_config.tar ...

  FILES RECOVERED VIA DP OPTIMISATION
  [  9] backup_keys.enc  ...

  RECOVERY SUMMARY
  Recovery bandwidth limit : 200.0 MB
  Files lost               : 5  (280.0 MB)
  Files recovered (DP opt) : 3  (130.0 MB)
  Value of lost files      : 3421
  Value recovered          : 2910  (85.1% of lost value)
```

---

## Running Tests

```bash
make test
# or
g++ -std=c++17 -Iinclude tests/test_main.cpp \
    src/File.cpp src/BackupManager.cpp src/RecoverySystem.cpp \
    -o build/RunTests && ./build/RunTests
```

Expected output:
```
  === File class tests ===
  [PASS] File id
  [PASS] File name
  ...
  Results : 20 passed, 0 failed.
```

---

## Contributing

1. Fork the repository  
2. Create a feature branch: `git checkout -b feature/my-feature`  
3. Commit your changes: `git commit -m "Add my feature"`  
4. Push and open a Pull Request

---

## License

This project is licensed under the MIT License — see [LICENSE](LICENSE) for details.
