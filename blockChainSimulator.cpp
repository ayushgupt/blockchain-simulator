#include <bits/stdc++.h>
using namespace std;
//freopen("input.txt", "r", stdin);
//freopen("output.txt", "w", stdout);
#define DEBUG2(x) do { std::cerr << #x << ": " << x << std::endl; } while (0)
// Useful functions
#define bitcount(x) __builtin_popcount(x) 	// returns the number of 1-bits in x.
#define gcd(a,b) __gcd(a,b)					// gcd of a & b
#define max2(x)  __builtin_ctz(x)  			// returns max power of 2

//priority_queue --> top() returns largest element by default
//string2int= STOI
//int2string= TO_STRING

double randZeroOne()
{
    std::mt19937_64 rng;
    // initialize the random number generator with time-dependent seed
    uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
    rng.seed(ss);
    // initialize a uniform distribution between 0 and 1
    std::uniform_real_distribution<double> unif(0, 1);
    return unif(rng);
}

double exponentialDistValue(double lambda)
{
    /* Explanation can be found 
     * from answer of Amit in link:
     * https://stackoverflow.com/questions/11491458/how-to-generate-random-numbers-with-exponential-distribution-with-mean
     * */
    return log(1-randZeroOne())/((-1)*(lambda));
}


double totalTimeToSimulate;
double globalLambdaForBlockGeneration;
double globalLambdaForTransactionGeneration;
int numberOfNodes;
double z;// percentage of slow
double initialMaxAmount; // initial Max Money to be given
int globalTransactionIdCounter=0;
int globalBlockIdCounter=1;     //Genesis Block will have zero ID
double globalCurrentTime=0;
double nodeConnectivityProbability=0.5;
double blockMValue=8000000;
double transactionMValue=0;
double negval = -1;
string line_separator = "--------------------------------------------------";

vector< vector<double> > propDelay;
struct transaction
{
    int transactionId;
    int senderNodeId;
    int destinationNodeId;
    double coins;

    transaction()
    {

    }

    transaction(int transactionIdArg,int senderNodeIdArg,int destinationNodeIdArg,double coinsArg)
    {
        transactionId=transactionIdArg;
        senderNodeId=senderNodeIdArg;
        destinationNodeId=destinationNodeIdArg;
        coins=coinsArg;
    }
    bool operator<(const transaction& rhs) const
    {
        //so that we get earliest scheduled at the top
        return transactionId < rhs.transactionId;
    }

};

struct block
{
    int blockId;
    double timeOfCreation;
    int prevBlock;
    int createNodeId;
    int lengthInBlockchain;
    set<transaction> transactionSet;
    block()
    {

    }

    block(int blockIdArg, double timeOfCreationArg,
          int prevBlockArg, int createNodeIdArg, int lengthInBlockchainArg)
    {
        blockId=blockIdArg;
        timeOfCreation=timeOfCreationArg;
        prevBlock=prevBlockArg;
        createNodeId=createNodeIdArg;
        lengthInBlockchain=lengthInBlockchainArg;
    }

};

struct node
{
    int nodeId;
    vector<int> neighbourNodes;
    bool fast;
    double lambdaForBlockGeneration;
    double lambdaForTransactionGeneration;
    float coins;
    map<int,double> mapBlockidReceivetime;
    vector<block> blocks;
    set<transaction> unspentTransactions;
};

struct event
{
    int eventType;
    double scheduleTime;
    double createTime;
    int senderNode;
    int currNode;
    block currBlock;
    transaction currTransaction;
    event(int eventTypeArg,
            double scheduleTimeArg,
            double createTimeArg, 
            int senderNodeArg, 
            int currNodeArg, 
            block currBlockArg,
            transaction currTransactionArg)
    {
        eventType=eventTypeArg;
        scheduleTime=scheduleTimeArg;
        createTime=createTimeArg;
        senderNode=senderNodeArg;
        currNode=currNodeArg;
        currBlock=currBlockArg;
        currTransaction=currTransactionArg;
    }

    bool operator<(const event& rhs) const
    {
        //so that we get earliest scheduled at the top
        return scheduleTime > rhs.scheduleTime;
    }
};

priority_queue<event> eventsQueue;
vector<node> NodesVec;

void makeNodes()
{
    for(int i=0;i<numberOfNodes;i++)
    {
        node tempNode;
        tempNode.nodeId=i;
        bool fastFlag=true;
        if(randZeroOne()<(z/100))
        {
            fastFlag= false;
        }
        tempNode.fast=fastFlag;
        tempNode.lambdaForBlockGeneration=globalLambdaForBlockGeneration;
        tempNode.lambdaForTransactionGeneration=globalLambdaForTransactionGeneration;
        tempNode.coins=initialMaxAmount*randZeroOne();
        NodesVec.push_back(tempNode);
    }
}

void makeConnectedGraph()
{
    for(int i=0;i<numberOfNodes;i++)
    {
        for(int j=i+1;j<numberOfNodes;j++)
        {
            if(randZeroOne()<nodeConnectivityProbability)
            {
                NodesVec[i].neighbourNodes.push_back(j);
                NodesVec[j].neighbourNodes.push_back(i);
            }
        }
    }

    int numConnectedNodes=0;
    vector<bool> visited(numberOfNodes,false);
    queue<int> bfsQ;
    bfsQ.push(0);
    visited[0]=true;
    numConnectedNodes+=1;
    while(!bfsQ.empty())
    {
        int currNode=bfsQ.front();
        bfsQ.pop();
        for(auto neighIndex: NodesVec[currNode].neighbourNodes)
        {
            if(!visited[neighIndex])
            {
                visited[neighIndex]=true;
                numConnectedNodes+=1;
                bfsQ.push(neighIndex);
            }
        }
    }

    if(numConnectedNodes!=numberOfNodes)
    {
        for(int i=0;i<numberOfNodes;i++)
        {
            NodesVec[i].neighbourNodes.clear();
        }
        makeConnectedGraph();
    }


}


void makePropDelayVec()
{
    propDelay.resize(numberOfNodes, vector<double>(numberOfNodes));
    for(int i=0;i<numberOfNodes;i++)
    {
        for(int j=i+1;j<numberOfNodes;j++)
        {
            double tempDelay=10+490*randZeroOne();
            propDelay[i][j]=tempDelay/1000;
            propDelay[j][i]=tempDelay/1000;
        }
    }
}

double calculateLatency(int i,int j,double m)
{
    double cij;
    if(NodesVec[i].fast && NodesVec[j].fast){cij=100000000;}
    else{cij=5000000;}
    double dijLambda=cij/96000;
    return propDelay[i][j]+(m/cij)+exponentialDistValue(dijLambda);
}


void generateTransactionEvent(event e)
{
    int senderId=e.currNode;
    int receipientId=randZeroOne()*numberOfNodes;
    while(receipientId==senderId)
    {
        receipientId=randZeroOne()*numberOfNodes;
    }

    double transactionAmount=randZeroOne()*(NodesVec[senderId].coins);
    transaction newtrans(globalTransactionIdCounter,senderId,receipientId,transactionAmount);
    globalTransactionIdCounter+=1;
    NodesVec[senderId].coins-=transactionAmount;
    NodesVec[receipientId].coins+=transactionAmount;
    NodesVec[senderId].unspentTransactions.insert(newtrans);

    event nextTransGen(
        1,
        e.scheduleTime+exponentialDistValue(NodesVec[senderId].lambdaForTransactionGeneration),
        e.scheduleTime,
        senderId,
        senderId,
        block(),
        transaction());
    eventsQueue.push(nextTransGen);

    for(int i=0;i< NodesVec[senderId].neighbourNodes.size();i++)
    {
        int recieverId=NodesVec[senderId].neighbourNodes[i];
        double latencyValue= calculateLatency(senderId,recieverId,transactionMValue);
        event recEvent(2,e.scheduleTime+latencyValue,e.scheduleTime,senderId,recieverId,block(),newtrans);
        eventsQueue.push(recEvent);
    }

}
void receiveTransactionEvent(event e)
{
    bool transactionAlreadyRecieved=false;
    for (auto curreTransaction:NodesVec[e.currNode].unspentTransactions )
    {
        if(curreTransaction.transactionId==e.currTransaction.transactionId)
        {
            transactionAlreadyRecieved=true;
            break;
        }
    }

    if(!transactionAlreadyRecieved)
    {
        NodesVec[e.currNode].unspentTransactions.insert(e.currTransaction);
        int senderId=e.currNode;
        for(int i=0;i<NodesVec[senderId].neighbourNodes.size();i++)
        {
            int destinId=NodesVec[senderId].neighbourNodes[i];
            double latencyValue=calculateLatency(senderId,destinId,transactionMValue);
            event recEvent(2,e.scheduleTime+latencyValue,e.scheduleTime,senderId,destinId,block(),e.currTransaction);
            eventsQueue.push(recEvent);
        }

    }

}
void generateBlockEvent(event e)
{

    bool receivedBlocksInMiddle=false;
    for( auto it = NodesVec[e.currNode].mapBlockidReceivetime.begin();
         it != NodesVec[e.currNode].mapBlockidReceivetime.end(); ++it )
    {
        double timeR = it->second;
        if(timeR<e.scheduleTime && timeR>e.createTime)
        {
            receivedBlocksInMiddle=true;
            break;
        }
    }
    if(!receivedBlocksInMiddle)
    {
        //Overall Picture: Create My Own Block and Broadcast It

        //Find the Id and Length of longest length block
        int longestLen=0;
        int localIndexOfLongestLenBlock;
        int actualBlockId;
//            Ayush's logic:
//        for(int i=0;i<NodesVec[e.currNode].blocks.size();i++)
//        {
//
//            if(NodesVec[e.currNode].blocks[i].lengthInBlockchain>longestLen)
//            {
//                longestLen=NodesVec[e.currNode].blocks[i].lengthInBlockchain;
//                localIndexOfLongestLenBlock=i;
//                actualBlockId=NodesVec[e.currNode].blocks[i].blockId;
//            }
//            
//        }
        
//            Ankit's logic:
        for(int i=0;i<NodesVec[e.currNode].blocks.size();i++)
        {
            if(NodesVec[e.currNode].blocks[i].lengthInBlockchain>longestLen){
                longestLen=NodesVec[e.currNode].blocks[i].lengthInBlockchain;
                localIndexOfLongestLenBlock=i;
                actualBlockId=NodesVec[e.currNode].blocks[i].blockId;
            }
        }
        double timestamp=negval;
        for(int i=0;i<NodesVec[e.currNode].blocks.size();i++)
        {
            if(NodesVec[e.currNode].blocks[i].lengthInBlockchain==longestLen){
                //When the 1st block with longestLen is found:
                if(timestamp<0){
                    timestamp = NodesVec[e.currNode].blocks[i].timeOfCreation;
                    actualBlockId=NodesVec[e.currNode].blocks[i].blockId;
                }
                //When you have already found at least one block with the longestLen and found another one:
                else{
                    if(timestamp<NodesVec[e.currNode].blocks[i].timeOfCreation){
                        timestamp = NodesVec[e.currNode].blocks[i].timeOfCreation;
                        actualBlockId=NodesVec[e.currNode].blocks[i].blockId;
                    }
                }
            }
        }

        //create a new block that refers to this block
        block generatedBlock(globalBlockIdCounter,e.scheduleTime,actualBlockId,e.currNode,longestLen+1);
        globalBlockIdCounter+=1;

        //add all unspent transactions to this block

        set<transaction> initialUnspentTransaction=NodesVec[e.currNode].unspentTransactions;
        for(auto currrTrans:initialUnspentTransaction)
        {
            if(NodesVec[e.currNode].blocks[localIndexOfLongestLenBlock].transactionSet.find(currrTrans)
               !=NodesVec[e.currNode].blocks[localIndexOfLongestLenBlock].transactionSet.end())
            {
                NodesVec[e.currNode].unspentTransactions.erase(currrTrans);
            } else
            {
                generatedBlock.transactionSet.insert(currrTrans);
            }
        }

        //TODO: Add Mining Fee
        NodesVec[e.currNode].coins+=50;
        //add block to my current node
        NodesVec[e.currNode].blocks.push_back(generatedBlock);

        //broadcast block to all my peers
        for(int i=0;i<NodesVec[e.currNode].neighbourNodes.size();i++)
        {
            int recieveNodeId=NodesVec[e.currNode].neighbourNodes[i];
            double latencyValue=calculateLatency(e.currNode,recieveNodeId,blockMValue);
            event recieveEvent(4,e.scheduleTime+latencyValue,e.scheduleTime,e.currNode,recieveNodeId,generatedBlock,transaction());
            //DEBUG2(e.scheduleTime+latencyValue);
            eventsQueue.push(recieveEvent);
        }

    }
}
void receiveBlockEvent(event e)
{
    bool alreadyRecievedBlock=false;
    for(int i=0;i< NodesVec[e.currNode].blocks.size();i++)
    {
        if(NodesVec[e.currNode].blocks[i].blockId==e.currBlock.blockId)
        {
            alreadyRecievedBlock=true;
            break;
        }
    }

    if(!alreadyRecievedBlock)
    {
        double t_K=exponentialDistValue( NodesVec[e.currNode].lambdaForBlockGeneration );

        //Trigger Block Gen Event
        event genBlockT(3,e.scheduleTime+t_K,e.scheduleTime,e.currNode,e.currNode,block(),transaction());
        eventsQueue.push(genBlockT);

        //find the len and blk id of the Node which is being referred here
        int longestLen=0;
        int localIndexOfLongestLenBlock;
        int actualBlockId;
        for(int i=0;i<NodesVec[e.currNode].blocks.size();i++)
        {
            if(NodesVec[e.currNode].blocks[i].blockId==e.currBlock.prevBlock)
            {
                longestLen=NodesVec[e.currNode].blocks[i].lengthInBlockchain;
                localIndexOfLongestLenBlock=i;
                actualBlockId=NodesVec[e.currNode].blocks[i].blockId;
            }
        }
        e.currBlock.lengthInBlockchain=longestLen+1;
        NodesVec[e.currNode].mapBlockidReceivetime[e.currBlock.blockId]=e.scheduleTime;
        NodesVec[e.currNode].blocks.push_back(e.currBlock);
        

        for(int i=0;i<NodesVec[e.currNode].neighbourNodes.size();i++)
        {
            int destinNode=NodesVec[e.currNode].neighbourNodes[i];
            if(destinNode!=e.senderNode)
            {
                double latencyValue= calculateLatency(e.currNode,destinNode,blockMValue);
                event recBevent(4,e.scheduleTime+latencyValue,e.scheduleTime,e.currNode,destinNode,e.currBlock,transaction());
                //DEBUG2(e.scheduleTime+latencyValue);
                eventsQueue.push(recBevent);

            }
        }

    }

}
void timeLoop()
{
    while(globalCurrentTime<=totalTimeToSimulate)
    {
        DEBUG2(globalCurrentTime);
        //for(int i=0;i<numberOfNodes;i++)
        {
            //DEBUG2(NodesVec[i].blocks.size());
            //DEBUG2(NodesVec[i].unspentTransactions.size());
        }
        DEBUG2(NodesVec[0].blocks.size());
        DEBUG2(NodesVec[0].unspentTransactions.size());

        event e=  eventsQueue.top();
        eventsQueue.pop();
        globalCurrentTime=e.scheduleTime;
        switch (e.eventType)
        {
            case 1:
                generateTransactionEvent(e);
                break;
            case 2:
                receiveTransactionEvent(e);
                break;
            case 3:
                generateBlockEvent(e);
                break;
            case 4:
                receiveBlockEvent(e);
                break;
        }
    }
}
void addGenesisBlock()
{
    block genBlock(0,0.0,-1,-1,1);
    for(int i=0;i<numberOfNodes;i++)
    {
        NodesVec[i].blocks.push_back(genBlock);
    }
}

void triggerGenerationOfBlocksAndNodes()
{
    for(int i=0;i<numberOfNodes;i++)
    {
        //generate Block
        event genBlock(3,exponentialDistValue(NodesVec[i].lambdaForBlockGeneration),
        0.0,NULL,i,block(),transaction());
        eventsQueue.push(genBlock);
        //generate Transaction
        event genTransaction(1,exponentialDistValue(NodesVec[i].lambdaForTransactionGeneration),
                       0.0,NULL,i,block(),transaction());
        eventsQueue.push(genTransaction);
    }
}

void printBlockchainStructure()
{
    for(int i=0;i<numberOfNodes;i++)
    {
        ofstream outfile;
        outfile.open ("blockChainOn"+to_string(i)+".py");
        outfile<<"import networkx as nx"<<endl;
        outfile<<"G=nx.Graph()"<<endl;
        //DEBUG2(NodesVec[i].blocks.size());
        for(int j=0;j<NodesVec[i].blocks.size();j++)
        {
            //DEBUG2(NodesVec[i].blocks[j].prevBlock);
            if(NodesVec[i].blocks[j].prevBlock!=(-1))
            {
                outfile<<"G.add_edge("<<NodesVec[i].blocks[j].blockId<<","<<NodesVec[i].blocks[j].prevBlock<<")"<<endl;
            }
        }
        outfile<<"nx.draw(G,with_labels = True)"<<endl;
        outfile<<"import matplotlib.pyplot as plt"<<endl;
        //outfile<<"plt.show()"<<endl;
        string figureName="blockchaingraph"+to_string(i);
        outfile<<"plt.savefig('"<<figureName<<".png')";
        outfile.close();
    }
}

void printBlockChainTree(){
    for(int i=0;i<numberOfNodes;i++){
        ofstream outfile;
        //Keeping the extension of the files as .py because of better readability in the text editor
        outfile.open ("blockChainTreeOf"+to_string(i)+".py");
        outfile<<"Node#"<<i<<"\n";
        outfile<<line_separator<<"\n";
        outfile<<line_separator<<"\n";
        for(int k=0; k<NodesVec[i].blocks.size();k++){
            outfile<<"Block#"<<k<<"\n";
            outfile<<line_separator<<"\n";
            outfile<<"Block ID:             "<<NodesVec[i].blocks[k].blockId<<"\n";
            outfile<<"Created by node:      "<<NodesVec[i].blocks[k].createNodeId<<"\n";
            outfile<<"Length in BlockChain: "<<NodesVec[i].blocks[k].lengthInBlockchain<<"\n";
            outfile<<"Previous Block:       "<<NodesVec[i].blocks[k].prevBlock<<"\n";
            outfile<<"Created at:           "<<NodesVec[i].blocks[k].timeOfCreation<<"\n";
            for(auto txn:NodesVec[i].blocks[k].transactionSet){
                outfile<<"\tTxnID "<<txn.transactionId<<": "<<txn.senderNodeId<<" pays "<<txn.destinationNodeId<<" "<<txn.coins<<" coins"<<"\n";
            }
        }
        outfile.close();
    }
}

void printAllUnspentTransactions(){
    for(int i=0;i<numberOfNodes;i++){
        ofstream outfile;
        //Keeping the extension of the files as .py because of better readability in the text editor
        outfile.open ("UnspentTransactionsOf"+to_string(i)+".py");
        outfile<<"Node#"<<i<<"\n";
        outfile<<line_separator<<"\n";
        for(auto txnSet:NodesVec[i].unspentTransactions){
            outfile<<"\tTxnID "<<txnSet.transactionId<<": "<<txnSet.senderNodeId<<" pays "<<txnSet.destinationNodeId<<" "<<txnSet.coins<<" coins"<<"\n";
        }
        outfile.close();
    }
}
void printNodesStructure()
{
    ofstream outfile;
    outfile.open ("nodesGraph.py");
    outfile<<"import networkx as nx"<<endl;
    outfile<<"G=nx.Graph()"<<endl;
    //DEBUG2(NodesVec[i].blocks.size());
    for(int j=0;j<numberOfNodes;j++)
    {
        for(int i=0;i<NodesVec[j].neighbourNodes.size();i++)
        {
            if(j<NodesVec[j].neighbourNodes[i])
            {
                outfile<<"G.add_edge("<<NodesVec[j].neighbourNodes[i]<<","<<j<<")"<<endl;
            }
        }
    }
    outfile<<"nx.draw(G,with_labels = True)"<<endl;
    outfile<<"import matplotlib.pyplot as plt"<<endl;
    //outfile<<"plt.show()"<<endl;
    string figureName="NodesGraph";
    outfile<<"plt.savefig('"<<figureName<<".png')";
    outfile.close();
}


int main()
{
    totalTimeToSimulate=100;
    numberOfNodes=10;
    z=60;
    initialMaxAmount=100;
    globalLambdaForBlockGeneration=0.1/(4*numberOfNodes);
    globalLambdaForTransactionGeneration=(10/numberOfNodes);
    nodeConnectivityProbability=0.1;
    makePropDelayVec();
    makeNodes();
    makeConnectedGraph();
    addGenesisBlock();
    triggerGenerationOfBlocksAndNodes();
    timeLoop();
    printNodesStructure();
    printBlockchainStructure();
    printBlockChainTree();
    printAllUnspentTransactions();
    return 0;
}
