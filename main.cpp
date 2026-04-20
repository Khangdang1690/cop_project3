// =====================================================================
// Project 3: Decision Trees (Group Assignment)
// Course: Data Structures
//
// Single-file C++ implementation of a general tree-based decision tree
// system using a linked structure. Builds the tree from a text file,
// writes an analysis file (about_tree.txt), and lets the user
// interactively explore nodes by their preorder position.
//
// Build (any machine with g++):
//     g++ -std=c++17 -Wall -O2 main.cpp -o tree
//
// Run:
//     ./tree tree-car.txt
//     ./tree tree-investment.txt
//
// If no filename is given on the command line, the program prompts
// for one, so it also works as simply: ./tree
// =====================================================================

#include <algorithm>
#include <climits>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------
// Node: one node in the general tree. Uses a vector of children so
// the tree can be N-ary; "binary-ness" is a property we check later,
// not an assumption we build in.
// ---------------------------------------------------------------------
struct Node {
    int level;                     // depth from the root (root is 0)
    int position;                  // preorder position from the file
    std::string edgeLabel;         // label on the edge from parent (empty for root)
    std::string content;           // text content of the node
    Node* parent;                  // pointer to parent (nullptr for root)
    std::vector<Node*> children;   // pointers to children in order

    Node(int lvl, int pos, std::string edge, std::string text)
        : level(lvl),
          position(pos),
          edgeLabel(std::move(edge)),
          content(std::move(text)),
          parent(nullptr) {}
};

// ---------------------------------------------------------------------
// LinkedTree: owns the root and provides all tree operations required
// by the assignment (build, write analysis, stats, interactive query).
// ---------------------------------------------------------------------
class LinkedTree {
public:
    LinkedTree() : root_(nullptr) {}
    ~LinkedTree() { destroy(root_); }

    // Parse the input file and build the linked structure.
    bool buildFromFile(const std::string& filename);

    // Write the required analysis file (visualization + properties).
    bool writeAboutFile(const std::string& filename) const;

    // Run the interactive console loop.
    void runInteractive() const;

    Node* root() const { return root_; }

private:
    Node* root_;
    std::map<int, Node*> byPosition_;  // preorder position -> node, for O(log n) lookup

    // ---- helpers ----
    static void destroy(Node* n);
    static void trimLeft(std::string& s);

    // traversal / stats
    void preorder(Node* n, std::vector<Node*>& out) const;
    int  heightOf(Node* n) const;
    int  countInternal(Node* n) const;
    int  countExternal(Node* n) const;

    // binary-tree style checks
    bool isBinary(Node* n) const;
    bool isProper(Node* n) const;
    bool isPerfect(Node* n) const;
    bool isBalanced(Node* n, int& outHeight) const;

    // output helpers
    void writeVisualization(std::ostream& os, Node* n) const;

    // interactive helpers
    static std::string describeAncestor(const Node* n);
    static std::string describeDescendant(const Node* n);
    static std::string describeSibling(const Node* n);
};

// ---------------------------------------------------------------------
// Destroy the subtree rooted at n (post-order delete).
// ---------------------------------------------------------------------
void LinkedTree::destroy(Node* n) {
    if (!n) return;
    for (Node* c : n->children) destroy(c);
    delete n;
}

// ---------------------------------------------------------------------
// Remove leading whitespace (spaces + tabs) from a string in place.
// ---------------------------------------------------------------------
void LinkedTree::trimLeft(std::string& s) {
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
    s.erase(0, i);
}

// ---------------------------------------------------------------------
// buildFromFile
//
// The lines in the input file are not guaranteed to be in preorder
// (the sample car file, for example, lists nodes in level-order). The
// "position" field is the preorder index, so we:
//   1) parse every line into a Node (not yet linked);
//   2) sort those nodes by their preorder position;
//   3) walk them in that order with a stack of the current root-to-
//      deepest-node path -- for each new node at level L, pop until
//      the top of the stack is at level L-1, then attach there.
// This is O(n log n) and works regardless of how the file ordered
// its lines.
// ---------------------------------------------------------------------
bool LinkedTree::buildFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Error: could not open file '" << filename << "'\n";
        return false;
    }

    // Step 1: parse every line into a fresh Node with no links yet.
    std::vector<Node*> nodes;
    std::string line;
    while (std::getline(in, line)) {
        // drop a trailing carriage return if the file was saved on Windows
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        std::istringstream iss(line);
        int level = -1;
        int position = -1;
        if (!(iss >> level >> position)) continue;  // skip malformed lines

        std::string edgeLabel;
        std::string content;
        if (level == 0) {
            // root line has no edge label; content is the remainder
            std::getline(iss, content);
        } else {
            iss >> edgeLabel;
            std::getline(iss, content);
        }
        trimLeft(content);

        nodes.push_back(new Node(level, position, edgeLabel, content));
    }

    if (nodes.empty()) return false;

    // Step 2: sort by preorder position so the builder can rely on
    // ancestor-before-descendant line ordering.
    std::sort(nodes.begin(), nodes.end(),
              [](const Node* a, const Node* b) {
                  return a->position < b->position;
              });

    // Step 3: link the nodes using a stack of the current root-path.
    std::vector<Node*> path;
    for (Node* node : nodes) {
        byPosition_[node->position] = node;

        if (node->level == 0) {
            if (root_ != nullptr) {
                std::cerr << "Error: more than one root node found.\n";
                return false;
            }
            root_ = node;
            path.clear();
            path.push_back(node);
        } else {
            // The parent is the most recent node at level-1 that we
            // have already placed on the path stack.
            while (!path.empty() && path.back()->level >= node->level) {
                path.pop_back();
            }
            if (path.empty()) {
                std::cerr << "Error: could not find parent for position "
                          << node->position << ".\n";
                return false;
            }
            Node* parent = path.back();
            node->parent = parent;
            parent->children.push_back(node);
            path.push_back(node);
        }
    }

    return root_ != nullptr;
}

// ---------------------------------------------------------------------
// Preorder traversal: visit node, then each child in order.
// ---------------------------------------------------------------------
void LinkedTree::preorder(Node* n, std::vector<Node*>& out) const {
    if (!n) return;
    out.push_back(n);
    for (Node* c : n->children) preorder(c, out);
}

// ---------------------------------------------------------------------
// Height = longest root-to-leaf edge count. A single node has height 0.
// ---------------------------------------------------------------------
int LinkedTree::heightOf(Node* n) const {
    if (!n || n->children.empty()) return 0;
    int best = 0;
    for (Node* c : n->children) {
        int h = heightOf(c) + 1;
        if (h > best) best = h;
    }
    return best;
}

// ---------------------------------------------------------------------
// Internal node = at least one child. External (leaf) = zero children.
// ---------------------------------------------------------------------
int LinkedTree::countInternal(Node* n) const {
    if (!n) return 0;
    int total = n->children.empty() ? 0 : 1;
    for (Node* c : n->children) total += countInternal(c);
    return total;
}

int LinkedTree::countExternal(Node* n) const {
    if (!n) return 0;
    if (n->children.empty()) return 1;
    int total = 0;
    for (Node* c : n->children) total += countExternal(c);
    return total;
}

// ---------------------------------------------------------------------
// Binary: every node has at most 2 children.
// ---------------------------------------------------------------------
bool LinkedTree::isBinary(Node* n) const {
    if (!n) return true;
    if (n->children.size() > 2) return false;
    for (Node* c : n->children) {
        if (!isBinary(c)) return false;
    }
    return true;
}

// ---------------------------------------------------------------------
// Proper (a.k.a. full) binary tree: every internal node has exactly 2
// children. Leaves trivially satisfy the rule.
// ---------------------------------------------------------------------
bool LinkedTree::isProper(Node* n) const {
    if (!n) return true;
    if (!n->children.empty() && n->children.size() != 2) return false;
    for (Node* c : n->children) {
        if (!isProper(c)) return false;
    }
    return true;
}

// ---------------------------------------------------------------------
// Perfect: proper AND every leaf is at the same depth.
// ---------------------------------------------------------------------
bool LinkedTree::isPerfect(Node* n) const {
    if (!isProper(n)) return false;

    // Collect all leaf depths; they must all match.
    std::vector<Node*> all;
    preorder(n, all);
    int leafDepth = -1;
    for (Node* p : all) {
        if (p->children.empty()) {
            if (leafDepth == -1) leafDepth = p->level;
            else if (p->level != leafDepth) return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------
// Balanced binary tree: for every node, |height(left) - height(right)| <= 1.
// If a node has fewer than 2 children, the missing side has height -1
// by convention (so a node with one child of height 0 is still balanced).
// outHeight returns the height of the subtree rooted at n.
// ---------------------------------------------------------------------
bool LinkedTree::isBalanced(Node* n, int& outHeight) const {
    if (!n) { outHeight = -1; return true; }

    int leftH = -1, rightH = -1;
    if (n->children.size() >= 1) {
        if (!isBalanced(n->children[0], leftH)) return false;
    }
    if (n->children.size() >= 2) {
        if (!isBalanced(n->children[1], rightH)) return false;
    }
    if (std::abs(leftH - rightH) > 1) return false;

    outHeight = 1 + std::max(leftH, rightH);
    return true;
}

// ---------------------------------------------------------------------
// Write the dash-indented visualization to an output stream.
// Level n  ->  2*n dashes, then "[edgeLabel] content" (edge label in
// brackets is only shown for non-root nodes).
// ---------------------------------------------------------------------
void LinkedTree::writeVisualization(std::ostream& os, Node* n) const {
    if (!n) return;
    os << std::string(2 * n->level, '-');
    if (!n->edgeLabel.empty()) os << "[" << n->edgeLabel << "] ";
    os << n->content << "\n";
    for (Node* c : n->children) writeVisualization(os, c);
}

// ---------------------------------------------------------------------
// Produce the required about_tree.txt file containing:
//   A. Dashed tree visualization with edge labels.
//   B. Tree properties (root, internal/external counts, height,
//      preorder lists of internal and external nodes).
//   C. Binary-tree analysis (binary? proper? perfect? balanced?).
// ---------------------------------------------------------------------
bool LinkedTree::writeAboutFile(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Error: could not open '" << filename << "' for writing.\n";
        return false;
    }
    if (!root_) {
        out << "(empty tree)\n";
        return true;
    }

    // --- Section A: visualization ---
    out << "Tree Visualization:\n";
    writeVisualization(out, root_);
    out << "\n";

    // --- Section B: properties ---
    std::vector<Node*> all;
    preorder(root_, all);

    out << "Tree Properties:\n";
    out << "Root node: " << root_->content << "\n";
    out << "Number of internal nodes: " << countInternal(root_) << "\n";
    out << "Number of external nodes: " << countExternal(root_) << "\n";
    out << "Tree height: " << heightOf(root_) << "\n";

    out << "Internal nodes (preorder):";
    for (Node* p : all) {
        if (!p->children.empty()) out << " " << p->content << ";";
    }
    out << "\n";

    out << "External nodes (preorder):";
    for (Node* p : all) {
        if (p->children.empty()) out << " " << p->content << ";";
    }
    out << "\n\n";

    // --- Section C: binary-tree analysis ---
    bool binary = isBinary(root_);
    out << "Binary Tree: " << (binary ? "Yes" : "No") << "\n";
    if (binary) {
        int h = 0;
        out << "Proper Tree: "   << (isProper(root_)           ? "Yes" : "No") << "\n";
        out << "Perfect Tree: "  << (isPerfect(root_)          ? "Yes" : "No") << "\n";
        out << "Balanced Tree: " << (isBalanced(root_, h)      ? "Yes" : "No") << "\n";
    }

    return true;
}

// ---------------------------------------------------------------------
// Interactive helpers: pick ONE ancestor / descendant / sibling to
// show, per the assignment spec. If none exists, show "None".
// ---------------------------------------------------------------------
std::string LinkedTree::describeAncestor(const Node* n) {
    if (n && n->parent) return n->parent->content;
    return "None";
}

std::string LinkedTree::describeDescendant(const Node* n) {
    if (n && !n->children.empty()) return n->children.front()->content;
    return "None";
}

std::string LinkedTree::describeSibling(const Node* n) {
    if (!n || !n->parent) return "None";
    for (Node* c : n->parent->children) {
        if (c != n) return c->content;
    }
    return "None";
}

// ---------------------------------------------------------------------
// Interactive console loop:
//   - Prompt for a preorder position (or "exit").
//   - On valid input: print node content, one ancestor, one
//     descendant, one sibling.
//   - On invalid input: "Invalid input. Please try again."
//   - On "exit": print "Goodbye!" and stop.
// ---------------------------------------------------------------------
void LinkedTree::runInteractive() const {
    std::string line;
    while (true) {
        std::cout << "Which node would you like to explore (enter position or \"exit\"): ";
        if (!std::getline(std::cin, line)) break;

        // trim whitespace on both ends so stray spaces don't break parsing
        size_t a = 0, b = line.size();
        while (a < b && std::isspace(static_cast<unsigned char>(line[a]))) ++a;
        while (b > a && std::isspace(static_cast<unsigned char>(line[b - 1]))) --b;
        std::string cmd = line.substr(a, b - a);

        if (cmd == "exit") {
            std::cout << "Goodbye!\n";
            return;
        }

        // Must be a pure integer. std::stoi would accept "12abc", so
        // we check every character is a digit first, and we catch
        // out_of_range for integers too large to fit in an int.
        if (cmd.empty() ||
            !std::all_of(cmd.begin(), cmd.end(),
                         [](unsigned char c){ return std::isdigit(c); })) {
            std::cout << "Invalid input. Please try again.\n";
            continue;
        }

        int pos = 0;
        try {
            pos = std::stoi(cmd);
        } catch (const std::exception&) {
            std::cout << "Invalid input. Please try again.\n";
            continue;
        }
        auto it = byPosition_.find(pos);
        if (it == byPosition_.end()) {
            std::cout << "Invalid input. Please try again.\n";
            continue;
        }

        const Node* n = it->second;
        std::cout << "Node's content: " << n->content << "\n";
        std::cout << "Ancestor: "       << describeAncestor(n)   << "\n";
        std::cout << "Descendant: "     << describeDescendant(n) << "\n";
        std::cout << "Sibling: "        << describeSibling(n)    << "\n";
    }
}

// ---------------------------------------------------------------------
// main: take a filename from argv (or prompt for one), build the tree,
// write the analysis file, then enter the interactive loop.
// ---------------------------------------------------------------------
int main(int argc, char* argv[]) {
    std::string inputFile;
    if (argc >= 2) {
        inputFile = argv[1];
    } else {
        std::cout << "Enter tree file name: ";
        if (!std::getline(std::cin, inputFile) || inputFile.empty()) {
            std::cerr << "No file provided. Exiting.\n";
            return 1;
        }
    }

    LinkedTree tree;
    if (!tree.buildFromFile(inputFile)) {
        std::cerr << "Failed to build tree from '" << inputFile << "'.\n";
        return 1;
    }

    const std::string outputFile = "about_tree.txt";
    if (!tree.writeAboutFile(outputFile)) {
        std::cerr << "Failed to write '" << outputFile << "'.\n";
        return 1;
    }
    std::cout << "Tree analysis written to " << outputFile << "\n";

    tree.runInteractive();
    return 0;
}
