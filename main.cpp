#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <string>
#include <cmath>
#include <limits>

using namespace std;

// A structure to represent a node in the Decision Tree
struct TreeNode {
    string attribute; // the attribute this node splits on
    unordered_map<string, TreeNode*> children; // maps attribute values to child nodes
    string label; // label if this is a leaf node

    // Constructor for leaf node
    TreeNode(const string& lbl) : label(lbl) {}
    // Constructor for internal node
    TreeNode(const string& attr) : attribute(attr), label("") {}
};

// Helper class to represent the dataset
class Dataset {
public:
    vector<vector<string>> data; // 2D data matrix
    vector<string> attributes; // List of attribute names
    int labelIndex; // Index of the label column

    Dataset(const vector<vector<string>>& d, const vector<string>& attrs, int lblIdx)
        : data(d), attributes(attrs), labelIndex(lblIdx) {}

    // Get unique values of an attribute
    set<string> getUniqueValues(int attrIndex) const {
        set<string> values;
        for (const auto& row : data) {
            values.insert(row[attrIndex]);
        }
        return values;
    }

    // Count occurrences of each class label
    unordered_map<string, int> getClassCounts() const {
        unordered_map<string, int> counts;
        for (const auto& row : data) {
            counts[row[labelIndex]]++;
        }
        return counts;
    }

    // Filter dataset based on an attribute value
    Dataset filter(int attrIndex, const string& value) const {
        vector<vector<string>> filteredData;
        for (const auto& row : data) {
            if (row[attrIndex] == value) {
                filteredData.push_back(row);
            }
        }
        return Dataset(filteredData, attributes, labelIndex);
    }
};

// Calculate entropy of a dataset
double entropy(const Dataset& dataset) {
    auto counts = dataset.getClassCounts();
    double entropy = 0.0;
    int total = dataset.data.size();
    for (const auto& [label, count] : counts) {
        double p = static_cast<double>(count) / total;
        entropy -= p * log2(p);
    }
    return entropy;
}

// Calculate information gain for a given attribute
double informationGain(const Dataset& dataset, int attrIndex) {
    double baseEntropy = entropy(dataset);
    double gain = baseEntropy;
    int total = dataset.data.size();

    auto values = dataset.getUniqueValues(attrIndex);
    for (const string& value : values) {
        Dataset subset = dataset.filter(attrIndex, value);
        double subsetEntropy = entropy(subset);
        gain -= (static_cast<double>(subset.data.size()) / total) * subsetEntropy;
    }
    return gain;
}

// Find the best attribute to split on
int bestAttribute(const Dataset& dataset, const set<int>& remainingAttrs) {
    double bestGain = -1;
    int bestAttr = -1;
    for (int attr : remainingAttrs) {
        double gain = informationGain(dataset, attr);
        if (gain > bestGain) {
            bestGain = gain;
            bestAttr = attr;
        }
    }
    return bestAttr;
}

// Recursive function to build the decision tree
TreeNode* buildTree(const Dataset& dataset, set<int> remainingAttrs) {
    // Base case: if all instances have the same label, create a leaf node
    auto counts = dataset.getClassCounts();
    if (counts.size() == 1) {
        return new TreeNode(counts.begin()->first);
    }

    // Base case: if there are no attributes left, return the most common label
    if (remainingAttrs.empty()) {
        string mostCommonLabel;
        int maxCount = 0;
        for (const auto& [label, count] : counts) {
            if (count > maxCount) {
                maxCount = count;
                mostCommonLabel = label;
            }
        }
        return new TreeNode(mostCommonLabel);
    }

    // Find the best attribute to split on
    int bestAttr = bestAttribute(dataset, remainingAttrs);
    if (bestAttr == -1) {
        // No attribute provides a gain; return most common label as a leaf
        string mostCommonLabel;
        int maxCount = 0;
        for (const auto& [label, count] : counts) {
            if (count > maxCount) {
                maxCount = count;
                mostCommonLabel = label;
            }
        }
        return new TreeNode(mostCommonLabel);
    }

    // Create internal node and split on best attribute
    TreeNode* node = new TreeNode(dataset.attributes[bestAttr]);
    remainingAttrs.erase(bestAttr);
    auto values = dataset.getUniqueValues(bestAttr);

    for (const string& value : values) {
        Dataset subset = dataset.filter(bestAttr, value);
        node->children[value] = buildTree(subset, remainingAttrs);
    }

    return node;
}

// Function to classify a new instance
string classify(const TreeNode* root, const vector<string>& instance, const vector<string>& attributes) {
    if (root->children.empty()) return root->label;

    auto it = find(attributes.begin(), attributes.end(), root->attribute);
    int attrIndex = distance(attributes.begin(), it);
    string attrValue = instance[attrIndex];

    if (root->children.count(attrValue)) {
        return classify(root->children.at(attrValue), instance, attributes);
    }
    return "Unknown"; // No path for this attribute value in the tree
}

// Print the decision tree (preorder traversal)
void printTree(const TreeNode* node, string indent = "") {
    if (!node) return;
    if (node->children.empty()) {
        cout << indent << "Label: " << node->label << endl;
    } else {
        cout << indent << "Attribute: " << node->attribute << endl;
        for (const auto& [value, child] : node->children) {
            cout << indent << "  Value: " << value << endl;
            printTree(child, indent + "    ");
        }
    }
}

// Main function for testing
int main() {
    // Sample data: {Attribute1, Attribute2, ..., Label}
    vector<vector<string>> data = {
        {"sunny", "hot", "high", "weak", "no"},
        {"sunny", "hot", "high", "strong", "no"},
        {"overcast", "hot", "high", "weak", "yes"},
        {"rain", "mild", "high", "weak", "yes"},
        {"rain", "cool", "normal", "weak", "yes"},
        {"rain", "cool", "normal", "strong", "no"},
        {"overcast", "cool", "normal", "strong", "yes"},
        {"sunny", "mild", "high", "weak", "no"}
    };
    vector<string> attributes = {"Outlook", "Temperature", "Humidity", "Wind"};
    int labelIndex = 4;

    Dataset dataset(data, attributes, labelIndex);
    set<int> remainingAttrs = {0, 1, 2, 3};

    TreeNode* tree = buildTree(dataset, remainingAttrs);
    printTree(tree);

    vector<string> newInstance = {"sunny", "cool", "high", "strong"};
    cout << "Classification: " << classify(tree, newInstance, attributes) << endl;

    return 0;
}
