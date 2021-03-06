#ifndef DFSNODE_H
#define	DFSNODE_H

#include "typedefs.h"

#include <limits>
#include <string>
#include <sstream>

class DFSNode {
public:
    DFSNode() : g(0), h(0), hExact(std::numeric_limits<float>::max()), subnetwork(varset(0)), leaf(0), lastIterationSeen(-1), existdouble(0) 
    {cs=0;}

    DFSNode(float g, float h, varset &subnetwork, byte &leaf) :
    g(g), h(h), hExact(std::numeric_limits<float>::max()), subnetwork(subnetwork), leaf(leaf), lastIterationSeen(-1), existdouble(0)
    {cs=0;}

    int getcs() const {
        return cs;
    }
    void setcs(){
        cs++;
    }

    float getF() const {
        return g + h;
    }

    varset getSubnetwork() const {
        return subnetwork;
    }

    int getLastIterationSeen() const {
        return lastIterationSeen;
    }

    byte getLeaf() const {
        return leaf;
    }

    float getH() const {
        return h;
    }

    float getG() const {
        return g;
    }

    float getHExact() const {
        return hExact;
    }

    int getexistdouble() const {
        return existdouble;
    }

    /**
     * Count (but do not cache) the layer of this node.
     * @return the layer of this node
     */
    byte getLayer() {
        return cardinality(subnetwork);
    }

    void copy(DFSNode* n) {
        g = n->g;
        h = n->h;
        leaf = n->leaf;
    }

    void setSubnetwork(varset subnetwork) {
        this->subnetwork = subnetwork;
    }

    void setLastIterationSeen(int lastIterationSeen) {
        this->lastIterationSeen = lastIterationSeen;
    }

    void setLeaf(byte leaf) {
        this->leaf = leaf;
    }

    void setG(float g) {
        this->g = g;
    }

    void setH(float h) {
        this->h = h;
    }
    
    void setHExact(float hExact) {
        this->hExact = hExact;
    }

    void setexistdouble(int existdouble) {
        this->existdouble=existdouble;
    }
    std::string toString() {
        std::ostringstream os;
        os << "subnetwork: " << varsetToString(subnetwork) << ", ";
        os << "leaf: " << (int)leaf << ", ";
        os << "g: " << g << ", ";
        os << "h: " << h << ", ";
        os << "hExact: " << hExact << ", ";
        os << "lastIterationSeen: " << lastIterationSeen<<",";
        return os.str();
    }
    
protected:
    float g;
    float h;
    float hExact;
    varset subnetwork;
    byte leaf;
    int lastIterationSeen;
    int existdouble;
    int cs;
};


struct CompareDFSNodeStar {

    bool operator()(DFSNode *a, DFSNode * b) {
        // I will always be comparing nodes at the same layer
        // so, just find the one with a closer estimated distance to the goal
        return a->getH() < b->getH();
    }
};

#endif	/* DFSNODE_H */

