#include <raylib.h>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <set>
#include <cmath>
#include <cctype>

struct BTreeNode {
    std::vector<int> keys;
    std::vector<BTreeNode*> children;
    bool leaf;
    
    BTreeNode(bool isLeaf) : leaf(isLeaf) {}
    
    ~BTreeNode() {
        for (auto child : children) {
            delete child;
        }
    }
};

class BTree {
    int t;
    BTreeNode* root;
    
public:
    BTree(int degree) : t(degree), root(nullptr) {}
    
    ~BTree() {
        delete root;
    }
    
    BTreeNode* getRoot() { return root; }
    
    void insert(int k);
    void remove(int k);
    bool search(int k);
    
private:
    void splitChild(BTreeNode* parent, int idx);
    void insertNonFull(BTreeNode* node, int k);
    void removeFromNode(BTreeNode* node, int k);
    void removeFromLeaf(BTreeNode* node, int idx);
    void removeFromNonLeaf(BTreeNode* node, int idx);
    int getPredecessor(BTreeNode* node, int idx);
    int getSuccessor(BTreeNode* node, int idx);
    void fill(BTreeNode* node, int idx);
    void borrowFromPrev(BTreeNode* node, int idx);
    void borrowFromNext(BTreeNode* node, int idx);
    void merge(BTreeNode* node, int idx);
    int findKey(BTreeNode* node, int k);
    bool searchNode(BTreeNode* node, int k);
};

bool BTree::search(int k) {
    return searchNode(root, k);
}

bool BTree::searchNode(BTreeNode* node, int k) {
    if (!node) return false;
    int i = 0;
    while (i < node->keys.size() && k > node->keys[i])
        i++;
    if (i < node->keys.size() && k == node->keys[i])
        return true;
    if (node->leaf)
        return false;
    return searchNode(node->children[i], k);
}

int BTree::findKey(BTreeNode* node, int k) {
    int idx = 0;
    while (idx < node->keys.size() && node->keys[idx] < k)
        idx++;
    return idx;
}

void BTree::splitChild(BTreeNode* parent, int idx) {
    BTreeNode* fullChild = parent->children[idx];
    BTreeNode* newChild = new BTreeNode(fullChild->leaf);
    
    int midIdx = t - 1;
    for (int j = 0; j < t - 1; j++) {
        newChild->keys.push_back(fullChild->keys[midIdx + 1 + j]);
    }
    
    if (!fullChild->leaf) {
        for (int j = 0; j < t; j++) {
            newChild->children.push_back(fullChild->children[midIdx + 1 + j]);
        }
    }
    
    int midKey = fullChild->keys[midIdx];
    fullChild->keys.resize(midIdx);
    if (!fullChild->leaf) {
        fullChild->children.resize(midIdx + 1);
    }
    
    parent->keys.insert(parent->keys.begin() + idx, midKey);
    parent->children.insert(parent->children.begin() + idx + 1, newChild);
}

void BTree::insert(int k) {
    if (!root) {
        root = new BTreeNode(true);
        root->keys.push_back(k);
        return;
    }
    
    if (root->keys.size() == 2 * t - 1) {
        BTreeNode* newRoot = new BTreeNode(false);
        newRoot->children.push_back(root);
        splitChild(newRoot, 0);
        
        int i = (newRoot->keys[0] < k) ? 1 : 0;
        insertNonFull(newRoot->children[i], k);
        root = newRoot;
    } else {
        insertNonFull(root, k);
    }
}

void BTree::insertNonFull(BTreeNode* node, int k) {
    int i = node->keys.size() - 1;
    
    if (node->leaf) {
        node->keys.push_back(0);
        while (i >= 0 && node->keys[i] > k) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = k;
    } else {
        while (i >= 0 && node->keys[i] > k)
            i--;
        i++;
        
        if (node->children[i]->keys.size() == 2 * t - 1) {
            splitChild(node, i);
            if (node->keys[i] < k)
                i++;
        }
        insertNonFull(node->children[i], k);
    }
}

int BTree::getPredecessor(BTreeNode* node, int idx) {
    BTreeNode* curr = node->children[idx];
    while (!curr->leaf)
        curr = curr->children[curr->keys.size()];
    return curr->keys[curr->keys.size() - 1];
}

int BTree::getSuccessor(BTreeNode* node, int idx) {
    BTreeNode* curr = node->children[idx + 1];
    while (!curr->leaf)
        curr = curr->children[0];
    return curr->keys[0];
}

void BTree::borrowFromPrev(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx - 1];
    
    child->keys.insert(child->keys.begin(), node->keys[idx - 1]);
    
    if (!child->leaf) {
        child->children.insert(child->children.begin(), sibling->children.back());
        sibling->children.pop_back();
    }
    
    node->keys[idx - 1] = sibling->keys.back();
    sibling->keys.pop_back();
}

void BTree::borrowFromNext(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];
    
    child->keys.push_back(node->keys[idx]);
    
    if (!child->leaf) {
        child->children.push_back(sibling->children[0]);
        sibling->children.erase(sibling->children.begin());
    }
    
    node->keys[idx] = sibling->keys[0];
    sibling->keys.erase(sibling->keys.begin());
}

void BTree::merge(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];
    
    child->keys.push_back(node->keys[idx]);
    
    for (int key : sibling->keys)
        child->keys.push_back(key);
    
    if (!child->leaf) {
        for (BTreeNode* childPtr : sibling->children)
            child->children.push_back(childPtr);
        sibling->children.clear();
    }
    
    node->keys.erase(node->keys.begin() + idx);
    node->children.erase(node->children.begin() + idx + 1);
    
    delete sibling;
}

void BTree::fill(BTreeNode* node, int idx) {
    if (idx != 0 && node->children[idx - 1]->keys.size() >= t)
        borrowFromPrev(node, idx);
    else if (idx != node->keys.size() && node->children[idx + 1]->keys.size() >= t)
        borrowFromNext(node, idx);
    else {
        if (idx != node->keys.size())
            merge(node, idx);
        else
            merge(node, idx - 1);
    }
}

void BTree::removeFromLeaf(BTreeNode* node, int idx) {
    node->keys.erase(node->keys.begin() + idx);
}

void BTree::removeFromNonLeaf(BTreeNode* node, int idx) {
    int k = node->keys[idx];
    
    if (node->children[idx]->keys.size() >= t) {
        int pred = getPredecessor(node, idx);
        node->keys[idx] = pred;
        removeFromNode(node->children[idx], pred);
    } else if (node->children[idx + 1]->keys.size() >= t) {
        int succ = getSuccessor(node, idx);
        node->keys[idx] = succ;
        removeFromNode(node->children[idx + 1], succ);
    } else {
        merge(node, idx);
        removeFromNode(node->children[idx], k);
    }
}

void BTree::removeFromNode(BTreeNode* node, int k) {
    int idx = findKey(node, k);
    
    if (idx < node->keys.size() && node->keys[idx] == k) {
        if (node->leaf)
            removeFromLeaf(node, idx);
        else
            removeFromNonLeaf(node, idx);
    } else {
        if (node->leaf)
            return;
        
        bool isInSubtree = (idx == node->keys.size());
        
        if (node->children[idx]->keys.size() < t)
            fill(node, idx);
        
        if (isInSubtree && idx > node->keys.size())
            removeFromNode(node->children[idx - 1], k);
        else
            removeFromNode(node->children[idx], k);
    }
}

void BTree::remove(int k) {
    if (!root) return;
    
    removeFromNode(root, k);
    
    if (root->keys.empty()) {
        BTreeNode* tmp = root;
        root = root->leaf ? nullptr : root->children[0];
        if (!tmp->leaf) tmp->children.clear();
        delete tmp;
    }
}

float calculateWidth(BTreeNode* node, float keyWidth) {
    if (!node) return 0;
    
    float nodeWidth = node->keys.size() * keyWidth;
    
    if (node->leaf) {
        return nodeWidth;
    }
    
    float childrenWidth = 0;
    for (auto child : node->children) {
        childrenWidth += calculateWidth(child, keyWidth);
    }
    childrenWidth += (node->children.size() - 1) * 20;
    
    return std::max(nodeWidth, childrenWidth);
}

class Visualizer {
    BTree tree;
    std::set<int> values;
    std::mt19937 rng;
    
public:
    Visualizer(int degree) : tree(degree), rng(std::random_device{}()) {}
    
    void insertValue(int val) {
        if (values.find(val) == values.end()) {
            tree.insert(val);
            values.insert(val);
        }
    }
    
    void removeValue(int val) {
        if (values.find(val) != values.end()) {
            tree.remove(val);
            values.erase(val);
        }
    }
    
    void insertRandom() {
        int attempts = 0;
        while (attempts < 100) {
            int val = std::uniform_int_distribution<int>(1, 99)(rng);
            if (values.find(val) == values.end()) {
                insertValue(val);
                return;
            }
            attempts++;
        }
    }
    
    void removeRandom() {
        if (values.empty()) return;
        auto it = values.begin();
        std::advance(it, std::uniform_int_distribution<int>(0, values.size() - 1)(rng));
        removeValue(*it);
    }
    
    int drawNode(BTreeNode* node, float x, float y, float width);
    void draw();
    int getNodeCount(BTreeNode* node);
    std::string getValuesString();
    BTree& getTree() { return tree; }
    int getValueCount() { return values.size(); }
};

int Visualizer::drawNode(BTreeNode* node, float x, float y, float width) {
    if (!node) return -1;
    
    const float keyWidth = 50;
    const float keyHeight = 38;
    const float padding = 3;
    float totalNodeWidth = node->keys.size() * keyWidth;
    float startX = x - totalNodeWidth / 2;
    
    Vector2 mouse = GetMousePosition();
    int clickedKey = -1;
    
    // Draw keys with hover effect
    for (int i = 0; i < node->keys.size(); i++) {
        float keyX = startX + i * keyWidth;
        Rectangle keyRect = {keyX, y, keyWidth - padding, keyHeight};
        
        bool hover = CheckCollisionPointRec(mouse, keyRect);
        bool clicked = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        
        Color fillColor = hover ? RED : SKYBLUE;
        Color borderColor = hover ? MAROON : DARKBLUE;
        
        DrawRectangleRounded(keyRect, 0.2f, 6, fillColor);
        DrawRectangleRoundedLines(keyRect, 0.2f, 6, borderColor);
        
        std::string keyStr = std::to_string(node->keys[i]);
        int textWidth = MeasureText(keyStr.c_str(), 20);
        DrawText(keyStr.c_str(), keyX + (keyWidth - padding) / 2 - textWidth / 2, y + 9, 20, hover ? WHITE : BLACK);
        
        if (clicked) {
            clickedKey = node->keys[i];
        }
    }
    
    // Draw children
    if (!node->leaf && !node->children.empty()) {
        float verticalGap = 90;
        float childY = y + verticalGap;
        
        std::vector<float> childWidths;
        float totalChildWidth = 0;
        for (auto child : node->children) {
            float w = calculateWidth(child, keyWidth);
            childWidths.push_back(w);
            totalChildWidth += w;
        }
        
        float spacing = 30;
        totalChildWidth += (node->children.size() - 1) * spacing;
        
        float childStartX = x - totalChildWidth / 2;
        float currentX = childStartX;
        
        for (int i = 0; i < node->children.size(); i++) {
            float childCenterX = currentX + childWidths[i] / 2;
            
            float parentConnectX = x;
            if (i == 0 && node->keys.size() > 0) {
                parentConnectX = startX;
            } else if (i < node->keys.size()) {
                parentConnectX = startX + i * keyWidth;
            } else {
                parentConnectX = startX + totalNodeWidth;
            }
            
            DrawLineEx({parentConnectX, y + keyHeight}, {childCenterX, childY}, 2.5f, GRAY);
            
            int childClicked = drawNode(node->children[i], childCenterX, childY, childWidths[i]);
            if (childClicked != -1) clickedKey = childClicked;
            
            currentX += childWidths[i] + spacing;
        }
    }
    
    return clickedKey;
}

void Visualizer::draw() {
    if (tree.getRoot()) {
        float treeWidth = calculateWidth(tree.getRoot(), 50);
        int clickedKey = drawNode(tree.getRoot(), GetScreenWidth() / 2, 120, treeWidth);
        if (clickedKey != -1) {
            removeValue(clickedKey);
        }
    } else {
        DrawText("Tree is empty - Insert some values!", GetScreenWidth() / 2 - 160, 300, 22, GRAY);
    }
}

int Visualizer::getNodeCount(BTreeNode* node) {
    if (!node) return 0;
    int count = 1;
    for (auto child : node->children)
        count += getNodeCount(child);
    return count;
}

std::string Visualizer::getValuesString() {
    if (values.empty()) return "Values: []";
    
    std::string result = "Values: [";
    int count = 0;
    for (int val : values) {
        if (count > 0) result += ", ";
        result += std::to_string(val);
        count++;
        if (count >= 20) {
            result += ", ...";
            break;
        }
    }
    result += "]";
    return result;
}

struct Button {
    Rectangle rect;
    const char* label;
    Color baseColor;
    Color borderColor;
    
    bool isClicked() {
        Vector2 mouse = GetMousePosition();
        return CheckCollisionPointRec(mouse, rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    }
    
    void draw() {
        Vector2 mouse = GetMousePosition();
        bool hover = CheckCollisionPointRec(mouse, rect);
        
        Color fillColor = hover ? Fade(baseColor, 0.7f) : Fade(baseColor, 0.4f);
        DrawRectangleRounded(rect, 0.2f, 6, fillColor);
        DrawRectangleRoundedLines(rect, 0.2f, 6, borderColor);
        
        int textWidth = MeasureText(label, 19);
        DrawText(label, rect.x + rect.width / 2 - textWidth / 2, rect.y + rect.height / 2 - 10, 19, BLACK);
    }
};

struct TextInput {
    Rectangle rect;
    std::string text;
    bool active;
    
    void update() {
        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            active = CheckCollisionPointRec(mouse, rect);
        }
        
        if (active) {
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= 48 && key <= 57 && text.length() < 5) { // digits only, max 5 chars
                    text += (char)key;
                }
                key = GetCharPressed();
            }
            
            if (IsKeyPressed(KEY_BACKSPACE) && text.length() > 0) {
                text.pop_back();
            }
        }
    }
    
    void draw(const char* label) {
        // Label
        DrawText(label, rect.x, rect.y - 22, 18, DARKGRAY);
        
        // Input box
        Color borderColor = active ? BLUE : GRAY;
        Color bgColor = active ? Fade(SKYBLUE, 0.2f) : Fade(LIGHTGRAY, 0.3f);
        
        DrawRectangleRounded(rect, 0.2f, 6, bgColor);
        DrawRectangleRoundedLines(rect, 0.2f, 6, borderColor);
        
        // Text
        const char* displayText = text.empty() ? "0" : text.c_str();
        int textWidth = MeasureText(displayText, 20);
        DrawText(displayText, rect.x + 10, rect.y + 8, 20, text.empty() ? GRAY : BLACK);
        
        // Cursor
        if (active && ((int)(GetTime() * 2) % 2 == 0)) {
            int cursorX = rect.x + 10 + MeasureText(text.c_str(), 20);
            DrawLine(cursorX, rect.y + 8, cursorX, rect.y + 28, BLACK);
        }
    }
    
    int getValue() {
        if (text.empty()) return 0;
        return std::stoi(text);
    }
    
    void clear() {
        text.clear();
    }
};

int main() {
    InitWindow(1400, 800, "B-Tree Visualizer (Degree = 3) - Click nodes to delete!");
    SetTargetFPS(60);
    
    Visualizer vis(3);
    
    Button addRandom{{30, 20, 140, 45}, "Insert Random", GREEN, DARKGREEN};
    Button delRandom{{190, 20, 140, 45}, "Delete Random", RED, MAROON};
    Button clear{{350, 20, 140, 45}, "Clear Tree", ORANGE, BROWN};
    Button insertCustom{{650, 20, 140, 45}, "Insert Value", BLUE, DARKBLUE};
    
    TextInput customInput{{510, 20, 120, 45}, "", false};
    
    float lastActionTime = 0;
    const float actionCooldown = 0.15f;
    
    while (!WindowShouldClose()) {
        float currentTime = GetTime();
        bool canAct = (currentTime - lastActionTime) >= actionCooldown;
        
        customInput.update();
        
        if (canAct) {
            if (addRandom.isClicked()) {
                vis.insertRandom();
                lastActionTime = currentTime;
            }
            else if (delRandom.isClicked()) {
                vis.removeRandom();
                lastActionTime = currentTime;
            }
            else if (clear.isClicked()) {
                vis = Visualizer(3);
                customInput.clear();
                lastActionTime = currentTime;
            }
            else if (insertCustom.isClicked()) {
                int val = customInput.getValue();
                if (val > 0 && val <= 99999) {
                    vis.insertValue(val);
                    customInput.clear();
                    lastActionTime = currentTime;
                }
            }
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        addRandom.draw();
        delRandom.draw();
        clear.draw();
        customInput.draw("Custom:");
        insertCustom.draw();
        
        vis.draw();
        
        // Stats
        DrawText(TextFormat("Total Nodes: %d | Values: %d", 
                 vis.getNodeCount(vis.getTree().getRoot()), vis.getValueCount()), 
                 30, GetScreenHeight() - 65, 19, DARKGRAY);
        
        std::string valStr = vis.getValuesString();
        DrawText(valStr.c_str(), 30, GetScreenHeight() - 35, 17, DARKGRAY);
        
        // Instructions
        DrawText("Hover over nodes to DELETE them | Type number and click 'Insert Value'", 
                 GetScreenWidth() - 580, GetScreenHeight() - 35, 17, GRAY);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}