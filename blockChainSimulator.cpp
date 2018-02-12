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
    vector<transaction> unspentTransactions;
};

struct event
{
    //TODO: Use Enum
    int eventType;
    double scheduleTime;
    double createTime;
    int senderNode;
    int currNode;
    block currBlock;
    transaction currTransaction;
    event(int eventTypeArg,double scheduleTimeArg,double createTimeArg, int senderNodeArg, int currNodeArg, block currBlockArg,transaction currTransactionArg)
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
    //TODO:check connectivity
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
}


void makePropDelayVec()
{
    propDelay.resize(numberOfNodes, vector<double>(numberOfNodes));
    for(int i=0;i<numberOfNodes;i++)
    {
        for(int j=i+1;j<numberOfNodes;j++)
        {
            double tempDelay=10+490*randZeroOne();
            if(randZeroOne()<nodeConnectivityProbability)
            {
                propDelay[i][j];
                propDelay[j][i];
            }
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
    NodesVec[senderId].unspentTransactions.push_back(newtrans);

    event nextTransGen(1,e.scheduleTime+exponentialDistValue(NodesVec[senderId].lambdaForTransactionGeneration),e.scheduleTime,senderId,senderId,block(),transaction());
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
    for(int i=0;i<NodesVec[e.currNode].unspentTransactions.size();i++)
    {
        if(NodesVec[e.currNode].unspentTransactions[i].transactionId==e.currTransaction.transactionId)
        {
            transactionAlreadyRecieved=true;
            break;
        }
    }
    if(!transactionAlreadyRecieved)
    {
        NodesVec[e.currNode].unspentTransactions.push_back(e.currTransaction);
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
        for(int i=0;i<NodesVec[e.currNode].blocks.size();i++)
        {
            if(NodesVec[e.currNode].blocks[i].lengthInBlockchain>longestLen)
            {
                longestLen=NodesVec[e.currNode].blocks[i].lengthInBlockchain;
                localIndexOfLongestLenBlock=i;
                actualBlockId=NodesVec[e.currNode].blocks[i].blockId;
            }
        }

        //create a new block that refers to this block
        block generatedBlock(globalBlockIdCounter,e.scheduleTime,actualBlockId,e.currNode,longestLen+1);
        globalBlockIdCounter+=1;

        //add all unspent transactions to this block

        for(int i=0;i< NodesVec[e.currNode].unspentTransactions.size() ;i++)
        {
            transaction currrTrans=NodesVec[e.currNode].unspentTransactions[i];

            if(NodesVec[e.currNode].blocks[localIndexOfLongestLenBlock].transactionSet.find(currrTrans)
                       !=NodesVec[e.currNode].blocks[localIndexOfLongestLenBlock].transactionSet.end())
            {
                NodesVec[e.currNode].blocks[localIndexOfLongestLenBlock].transactionSet.erase(currrTrans);
            } else
            {
                generatedBlock.transactionSet.insert(currrTrans);
            }
        }

        //add block to my current node
        NodesVec[e.currNode].blocks.push_back(generatedBlock);

        //broadcast block to all my peers
        for(int i=0;i<NodesVec[e.currNode].neighbourNodes.size();i++)
        {
            int recieveNodeId=NodesVec[e.currNode].neighbourNodes[i];
            double latencyValue=calculateLatency(e.currNode,recieveNodeId,blockMValue);
            event recieveEvent(4,e.scheduleTime+latencyValue,e.scheduleTime,e.currNode,recieveNodeId,generatedBlock,transaction());
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
                eventsQueue.push(recBevent);

            }
        }

    }

}
void timeLoop()
{
    while(globalCurrentTime<=totalTimeToSimulate)
    {
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
    block genBlock(0,0.0,NULL,-1,1);
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

int main()
{
    totalTimeToSimulate=10;
    numberOfNodes=10;
    z=60;
    initialMaxAmount=100;
    globalLambdaForBlockGeneration=100;
    globalLambdaForTransactionGeneration=1000;
    nodeConnectivityProbability=0.5;
    makePropDelayVec();
    makeNodes();
    makeConnectedGraph();
    addGenesisBlock();
    triggerGenerationOfBlocksAndNodes();
    timeLoop();
    return 0;
}