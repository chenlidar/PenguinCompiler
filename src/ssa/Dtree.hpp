#include "../backend/graph.hpp"
namespace DTREE{
    class Dtree:public GRAPH::Graph{
        public:
        Dtree(GRAPH::Graph* g);
        private:
        std::unordered_map<GRAPH::Node*,int> dfnum;
        std::unordered_map<int,GRAPH::Node*> vertex;
        std::unordered_map<GRAPH::Node*,GRAPH::Node*> parent;
        std::unordered_map<GRAPH::Node*,GRAPH::Node*> samedom;
        std::unordered_map<GRAPH::Node*,GRAPH::Node*> semi;
        std::unordered_map<GRAPH::Node*,GRAPH::Node*> idom;
        std::unordered_map<GRAPH::Node*,GRAPH::Node*> best;
        std::unordered_map<GRAPH::Node*,GRAPH::Node*> ancestor;
        std::unordered_map<GRAPH::Node*,std::vector<GRAPH::Node*> > bucket;
        int cnt;
        void dfs(GRAPH::Node* root,GRAPH::Node *fa);
        void link(GRAPH::Node* fa,GRAPH::Node* node);
        GRAPH::Node* findLowestSemiAncestor(GRAPH::Node* node);
    };
}