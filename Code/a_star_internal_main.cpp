#include <algorithm>
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>
#include <time.h>

#include <system_error>

#include <boost/asio.hpp>
#include <boost/timer/timer.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>

#include "typedefs.h"
#include "dfs_node.h"
#include "sparse_parent_bitwise.h"
#include "doublelist.h"

#include <unistd.h>
#include <sys/time.h>

namespace po = boost::program_options;

/**
 * The tolerance allowed for floating point precision, measured in Ulps.
 * See: http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
 */
float epsilon = -1e2;

/**
 * The score cache file.
 */
std::string scoreFile;

/**
 * The sparse parent graphs for each variable.
 */
std::vector<bestscorecalculators::SparseParentBitwise*> spgs;

/**
 * The number of variables in the problem.
 */
int variableCount;

/**
 * The currently expanding node at each layer of the search.
 */
std::vector<DFSNode*> nodeStack;
/**
 * The valid parent sets for all variables at each layer of the search.
 */
std::vector<std::vector<bitset > > valid;

/**
 * The successors of the currently expanding nodes at each layer of the search.
 */
std::vector<std::vector<DFSNode* >> successors;

/**
 * Comparator to sort the successors before expanding.
 */
CompareDFSNodeStar cmp;

/**
 * All nodes which have been generated in the search.
 */
DFSNodeMap nodes;
/**
 * The current iteration of search.
 */
int iteration;

/**
 * The repair list for the current iteration of search.
 */
DFSNodeMap toRepair;

/**
 * The score of the best network found so far.
 */
float optimal;

/**
 * A pointer to the goal node, to make checking it easy.
 */
DFSNode *goalNode;

/**
 * The total number of nodes expanded, including re-expansions.
 */
int nodesExpanded;

/**
 * The file to write the learned network.
 */
std::string netFile;

/**
 * A timer to keep track of how long the algorithm has been running.
 */
boost::asio::io_service io;

/**
 * A variable to check if the user-specified time limit has expired.
 */
bool outOfTime;

/**
 * Timer to see how long the program has been running.
 */
boost::timer::auto_cpu_timer act;

/**
 * The maximum running time for the algorithm.
 */
int runningTime;

/**
 * Define the head node and flag node of the doubly linked list
 */
double_list *head, *inlink;

/**
 * Set the threshold and its coefficient
 */
float threshold, Fmin, Flimit=0;

/**
 * Check whether the target node is generated
 */
bool found = false;

/**
 * Get the memory occupied by the process
 */
unsigned int get_proc_mem(unsigned int pid){
	
	char file_name[64]={0};
	FILE *fd;
	char line_buff[512]={0};
	sprintf(file_name,"/proc/%d/status",pid);
	
	fd =fopen(file_name,"r");
	if(NULL == fd){
		return 0;
	}
	
	char name[64];
	int vmrss;
	for (int i=0; i<16;i++){
		fgets(line_buff,sizeof(line_buff),fd);
	}
	
	fgets(line_buff,sizeof(line_buff),fd);
	sscanf(line_buff,"%s %d",name,&vmrss);
	fclose(fd);
 
	return vmrss;
}

float h(varset &U, int layer) {
    float hVal = 0;
    // for each remaining variable
    for (byte Y = 0; Y < variableCount; Y++) {
        // make sure this variable was not already present
        if (!VARSET_GET(U, Y)) continue;
        int fsb = BITSET_FIRST_SET_BIT(valid[layer][Y]);
        float bestScore = spgs[Y]->getScore(fsb);
        hVal += bestScore;
    }
    return hVal;
}

void expand(int layer) {
    if (outOfTime) {
        return;
    }

    DFSNode *U = nodeStack[layer];
    successors[layer].clear();
#ifdef DEBUG
    //printf("\nExpanding: %s\tf=%f\n", U->toString().c_str(),U->getF());
	
#endif
    nodesExpanded++;

    for (byte leaf = 0; leaf < variableCount; leaf++) {
        // make sure this variable is still remaining
        if (!VARSET_GET(U->getSubnetwork(), leaf)) continue;

        // get the new variable set
        VARSET_COPY(U->getSubnetwork(), newVariables);
        VARSET_CLEAR(newVariables, leaf);
        //printf("%s\n",varsetToString(newVariables).c_str());

        // get BestScore(leaf, U) and calculate g
        int fsb = BITSET_FIRST_SET_BIT(valid[layer][leaf]);
        float bestScore = spgs[leaf]->getScore(fsb);
        float g = U->getG() + bestScore;

        DFSNode *successor = nodes[newVariables];
        // is this the first time we've ever seen this node
        
        if (successor == NULL) {
            float g = std::numeric_limits<float>::max();
            float hVal = h(newVariables, layer);
            successor = new DFSNode(g, hVal, newVariables, leaf);
            nodes[newVariables] = successor;
        }

        // check if we can use this node to find a better path to the goal
        float newHExact = bestScore + successor->getHExact();
        if (U->getHExact() > newHExact) {
#ifdef DEBUG
            printf("Setting hExact1: %s, newHExact: %f\n", varsetToString(U->getSubnetwork()).c_str(), newHExact);
#endif
            U->setHExact(newHExact);
        }

        // There are duplicate nodes in the same iteration, judge whether it is better
        bool seenThisIteration = (successor->getLastIterationSeen() == iteration);
        if (seenThisIteration) {
            if (g < successor->getG()) {
                float oldG = successor->getG();
                int intDiff = (*(int*) &g - *(int*) &oldG);

                if (intDiff < epsilon) {
#ifdef DEBUG
                    //printf("epsilon g: %f, old g: %f, intDiff: %d\n", g, successor->getG(), intDiff);
#endif
                    successor->setG(g);
                    successor->setLeaf(leaf);
                }
            }
            else
            {
                continue;
            }
            
        }
        
        // check if we have found a new best path to successor
        if (g < successor->getG()) {
            successor->setG(g);
            successor->setLeaf(leaf);
        } 
        else {
            // we have already found a better path, so just skip this
            continue;
        }
        
        // check if we can prune this node
        float f = successor->getH() + g;
        if (optimal <= f) {
            continue;
        }

        // so this is the first time we have seen successor this iteration
        successor->setLastIterationSeen(iteration);
        
        // if we make it here, then we have to expand the node
        if(newVariables==NULL)
            {
                found=true;//ftimes++;
                break;

            }
        successors[layer].push_back(successor);

        
    }
    std::sort(successors[layer].begin(), successors[layer].end(), cmp);

    for (auto it = successors[layer].begin(); it != successors[layer].end() ; ++it) 
    {
        DFSNode *successor = *it;
        varset newVariable=successor->getSubnetwork();
        
        for (byte Y = 0; Y < variableCount; Y++) {
            // make sure this variable is still remaining
            if (!VARSET_GET(successor->getSubnetwork(), Y)) continue;

            // update the valid parents in the next layer
            BITSET_CLEAR(valid[layer + 1][Y]);
            BITSET_OR(valid[layer + 1][Y], valid[layer][Y]);
            BITSET_AND(valid[layer + 1][Y], spgs[Y]->getUsedParents(successor->getLeaf()));
        }

		if(successor->getF()>Flimit){

			Fmin=std::min(successor->getF()+(float)layer*threshold,Fmin);

			if(successor->getexistdouble()==0)
			{
				insertlist_u(inlink,newVariable);
            	successor->setexistdouble(1);
			}
            
            continue;
        }
		if(successor->getexistdouble()==1)
		{
			continue;
		}

        nodeStack[layer + 1] = successor;
        expand(layer + 1);

        // check if we found a better path to the goal through U
        float bestScore = successor->getG() - U->getG();
        float newHExact = bestScore + successor->getHExact();
        if (U->getHExact() > newHExact) {
#ifdef DEBUG
            printf("Setting hExact2: %s, newHExact: %f\n", varsetToString(U->getSubnetwork()).c_str(), newHExact);
#endif
            U->setHExact(newHExact);
        }
    }

    // check if we found a better path to the goal
    float newPath = U->getHExact() + U->getG();
    
    if (newPath < optimal) {
        optimal = newPath;
        printf("New solution: %f, nodes expanded: %d\n", newPath, nodesExpanded);
        
        act.report();
    }
}

/**
 * Remove all parent sets at {@code layer} that have been removed from {@code U}.
 * 
 * @param U the node (presumably, a search will start from here)
 * @param layer the layer of U
 */
void initBitsets(DFSNode *U, int layer) {
    // first, set all of the bitsets completely
    for (int X = 0; X < variableCount; X++) {
        //bitset bs = ;
        BITSET_SET_ALL(valid[layer][X]);
    }

    // find variables which have been removed from U
    for (int X = 0; X < variableCount; X++) {
        if (VARSET_GET(U->getSubnetwork(), X)) continue;

        // X has already been removed from U
        // so remove it as a possible parent from everything that remains
        for (byte Y = 0; Y < variableCount; Y++) {
            // make sure this variable is remaining
            if (!VARSET_GET(U->getSubnetwork(), Y)) continue;

            // update the valid parents for Y to not include X
            BITSET_AND(valid[layer][Y], spgs[Y]->getUsedParents(X));
        }
    }
}

/**
 * Handler when out of time.
 */
void timeout(const boost::system::error_code& /*e*/) {
    printf("Out of time\n");
    outOfTime = true;
}

/*
    Output the shortest path found
 */
void bestpath(){
    printf("shortest path, nodes.size = %d\n",nodes.size());

    std::vector<varset> optimalParents;
    for (int i = 0; i < spgs.size(); i++) {
        optimalParents.push_back(VARSET(spgs.size()));
    }

    VARSET_COPY(goalNode->getSubnetwork(), remainingVariables);
    DFSNode *current = nodes[remainingVariables];
    float score = 0;
    int count = variableCount;//cardinality(scc);
    for (int i = 0; i < count; i++) {
        int leaf = current->getLeaf();
        score += spgs[leaf]->getScore(remainingVariables);
        varset parents = spgs[leaf]->getParents();
        optimalParents[leaf] = parents;
        printf("%d, ",leaf);
        VARSET_SET(remainingVariables, leaf);
        current = nodes[remainingVariables];
    }
    printf("\nbest parents :\n");
    for(int p=0;p<variableCount;p++)
        printf("<varset> optimalParents[%d] = %s\n", p,varsetToString(optimalParents[p]).c_str());
    printf("score: %f\n",score);    
    printf("END\n");
}


/**
 * The logic of the outer loop of DFS.
 */
void dfs() {

    printf("URLearning, DFBnB\n");
    printf("Dataset: '%s'\n", scoreFile.c_str());
    printf("Net file: '%s'\n", netFile.c_str());
    printf("Reading score cache: '%s'\n", scoreFile.c_str());
    scoring::ScoreCache cache;
    cache.read(scoreFile);
    threshold = cache.getThreshold();
    int cutset = cache.getCutset();
    variableCount = cache.getVariableCount();
    
    printf("Creating sparse parent graphs\n");
    for (int i = 0; i < variableCount; i++) {
        //spgs is a collection of sparse parent graphs, and spg is a sparse parent graph of each variable
        bestscorecalculators::SparseParentBitwise *spg = new bestscorecalculators::SparseParentBitwise(i, variableCount);;
        
        spg->initialize(cache);
        spgs.push_back(spg);

        nodeStack.push_back(new DFSNode());
        successors.push_back(std::vector<DFSNode*>());
    }

    printf("Creating bitset data structures\n");
    for (int i = 0; i <= variableCount; i++) {
        std::vector<bitset> vec;

        for (int j = 0; j < variableCount; j++) {
            bitset bs = BITSET_NEW(spgs[j]->size());
            vec.push_back(bs);
        }
        valid.push_back(vec);
    }

    printf("Initializing search variables\n");
    init_map(nodes);
    iteration = 1;

    VARSET_NEW(allVariables, variableCount);
    VARSET_SET_ALL(allVariables, variableCount);
    //Start node h is 0
    float hVal = h(allVariables, 0);
    
    byte leaf(0);
    DFSNode *startNode = new DFSNode(0.0f, hVal, allVariables, leaf);

    //Initialize doubly linked list
    head = createlist(allVariables);
    VARSET_NEW(empty, variableCount);
    goalNode = new DFSNode(std::numeric_limits<float>::max(), 0.0f, empty, leaf); 
	goalNode->setHExact(0.0f);
    nodes[allVariables]=startNode;
    optimal = std::numeric_limits<float>::max();
    nodesExpanded = 0;
    int layer = 0;

    while (found == false || enoughlist(head, variableCount) ){

        double_list *p;
        p = head;
        
        //Set the initial value of Fmin to the first element of the doubly linked list, f value = infinity
        Fmin=std::numeric_limits<float>::max();

		varset charu = allVariables;
		insertlist_f(head, 1, charu);
		p = p->next;
		inlink = head->next;
        //Traverse all nodes in the doubly linked list
        while(p->next != head)  
        {  
            p = p->next;
            varset variables = p->data;
            layer = variableCount - nodes[variables]->getLayer();
            
            float f=nodes[variables]->getF();
            if(f>Flimit)  
            {
				nodes[variables]->setcs();
				if(nodes[variables]->getcs() >= cutset)
				{
					deletelist_f(p);
					continue;
				}
				Fmin=std::min(f+(float)layer*threshold,Fmin);
                continue;
            }
            nodeStack[layer] = nodes[variables];//Expansion nodes at each layer
            //Adjust the bitset data structure
            initBitsets(nodes[variables], layer);           
            expand(layer);
            //Delete the expanded node in the doubly linked list
            deletelist_f(p);
			
            
        }
		deletelist_f(inlink);
        Flimit = Fmin;
        iteration++;
    }
    if(found == true)
    {
        bestpath();
        printf("Nodes expanded: %d\n", nodesExpanded);
        printf("shortpath, nodes.size = %d\n",nodes.size());
        io.stop();
    }
    else if(found==false)
    {
        printf("Cannot find best path!\n");
    }
    
    
}

int main(int argc, char** argv) {
    
    clock_t start,finish;
    double totaltime;
    start=clock();
    std::string description = std::string("Learn an optimal Bayesian network using DFBnB.  Example usage: ") + argv[0] + " iris.pss";
    po::options_description desc(description);
    
    desc.add_options()
            ("scoreFile", po::value<std::string > (&scoreFile)->required(), "The file containing the local scores in pss format. First positional argument.")
            ("runningTime,r", po::value<int> (&runningTime)->default_value(0), "The maximum running time for the algorithm.  0 means no running time.")
            ("netFile,n", po::value<std::string > (&netFile)->default_value(""), "The file to which the learned network is written.  Leave blank to not create the file.")
            ("help,h", "Show this help message.")
            ;

    po::positional_options_description positionalOptions;
    positionalOptions.add("scoreFile", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc)
            .positional(positionalOptions).run(),
            vm);

    if (vm.count("help") || argc == 1) {
        std::cout << desc;
        return 0;
    }

    po::notify(vm);

    outOfTime = false;
    
    boost::asio::deadline_timer t(io);
    if (argc > 4) {
        printf("Maximum running time: %d\n", runningTime);
        t.expires_from_now(boost::posix_time::seconds(runningTime));
        t.async_wait(timeout);
        boost::thread workerThread(dfs);
        io.run();
        workerThread.join();
    } else {
        dfs();
    }
    finish=clock();
    totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
    std::cout<<"\nrunning time: "<<totaltime<<" s!\n";
    unsigned int pid = getpid();
	printf("Memory footprint(KB)=%d\n",get_proc_mem(pid));
    return 0;
}