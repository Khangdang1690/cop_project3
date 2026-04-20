# Project 3 — Decision Trees

Course: Data Structures
Group Number: **<fill in>**
Group Members: **<fill in>**

A general tree-based decision tree system implemented with a linked
structure in C++. The program reads a tree description from a text
file, builds the tree dynamically, writes an analysis file, and lets
the user explore nodes interactively.

---

## Files

| File | Purpose |
|---|---|
| `main.cpp` | Entire program (Node, LinkedTree, main) — single file, no other sources to link |
| `tree-car.txt` | Car-purchase decision tree (sample input) |
| `tree-investment.txt` | Investment-advice decision tree (sample input) |
| `about_tree.txt` | **Generated** at runtime — contains the tree visualization, properties, and binary-tree analysis |

---

## Build

Works on any machine with a recent `g++` (Linux, macOS, the student cluster, or Windows + MinGW):

```
g++ -std=c++17 -Wall -O2 main.cpp -o tree
```

Because everything lives in `main.cpp`, there are no `.h`/`.cpp` pairs to link — the command above is the only step needed.

## Run

```
./tree tree-car.txt
./tree tree-investment.txt
```

Without an argument the program prompts for a filename:

```
./tree
```

On Windows (MinGW), use `tree.exe` instead of `./tree`.

---

## What the program does

1. **Builds** the linked general tree from the input file. Input lines may be in preorder, level-order, or shuffled — the program sorts by the preorder-position field before linking.
2. **Writes `about_tree.txt`** containing:
   - Dashed tree visualization with edge labels (`--[Yes] content`).
   - Tree properties: root, internal/external counts, height, internal-node list (preorder), external-node list (preorder).
   - Binary-tree analysis: whether the tree is binary, and if so whether it is proper, perfect, and balanced.
3. **Prompts** `Which node would you like to explore (enter position or "exit"):` and for each valid position prints:
   - Node content
   - One ancestor
   - One descendant
   - One sibling

Invalid input → `Invalid input. Please try again.`
Typing `exit` → `Goodbye!`

---

## Input file format

- **First line** — root node: `<level> <position> <content>` (level 0, no edge label).
- **Subsequent lines** — `<level> <position> <edge label> <content>`, tab- or space-separated.

Example:

```
0 1 Are you nervous?
1 2 Yes Savings account
1 3 No Will you need to access most of the money within the next 5 years?
2 4 Yes Money market fund
...
```

---

## Work Distribution

| Member | Contribution | % |
|---|---|---|
| **<fill in>** | <fill in> | 25% |
| **<fill in>** | <fill in> | 25% |
| **<fill in>** | <fill in> | 25% |
| **<fill in>** | <fill in> | 25% |

---

## Academic Integrity Statement

We certify that this submission is the original work of our group. All
members contributed equally to the design, implementation, and testing
of the program. No code was copied from other students or from
unauthorized online sources. Any external references used are
standard-library documentation only.

---

## Submission Requirements

### 1. PDF Document

The accompanying PDF must include:

1. Group member names + group number
2. Instructions to compile/run on the student cluster
3. Work distribution (percentage per member)
4. Academic integrity statement

### 2. Code Folder

- Only necessary files (`main.cpp`, `tree-car.txt`, `tree-investment.txt`, `README.md`)
- Proper structure (single flat folder, nothing extraneous)
- Must be zipped as:

```
<GroupNumber>.zip
```

---

## Coding Standards

- Clear comments for all functions
- Meaningful variable and function names
- Clean, readable structure
- Follow C++ best practices (RAII, const-correctness, no raw new/delete leaks, no globals)

---

## Grading Rubric (100 Points)

| Criteria | Points |
|---|---:|
| Program runs correctly (cluster instructions) | 20 |
| Output correct & matches format | 20 |
| Code documentation (comments) | 15 |
| Variable & function naming | 15 |
| Code quality & readability | 10 |
| Correct linked tree implementation | 20 |
| **Total** | **100** |

---

## Important Constraints

- No hardcoding — the program works for any valid tree file.
- Handles any valid tree file (preorder, level-order, or shuffled line order).
- Output format matches the assignment specification exactly.
