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

int godNode=-1; //for genesis and mining transaction giver Node
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

ofstream blockFile;
ofstream initialMoneyFile;
ofstream finalMoneyFile;


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

    bool operator==(const transaction& rhs) const
    {
        //so that we get earliest scheduled at the top
        return transactionId== rhs.transactionId;
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
    int localIndexOfPrevBlock;
    int createNodeId;
    int lengthInBlockchain;
    set<transaction> transactionSet;
    map<int,double> amountMap;
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
        for(int i=0;i<numberOfNodes;i++)
        {
            amountMap[i]=0;
        }
    }

};

struct node
{
    int nodeId;
    vector<int> neighbourNodes;
    bool fast;
    double lambdaForBlockGeneration;
    double lambdaForTransactionGeneration;
    //float coins;
    map<int,double> mapBlockidReceivetime;
    vector<block> blocks;
    set<transaction> allTransactions;
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
        //tempNode.coins=initialMaxAmount*randZeroOne();
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

int getlocalIndexOfLongestLenBlock(int nodeIndex)
{

    int longestLen=0;
    int localIndexOfLongestLenBlock;
    int actualBlockId;

    for(int i=0;i<NodesVec[nodeIndex].blocks.size();i++)
    {
        if(NodesVec[nodeIndex].blocks[i].lengthInBlockchain>longestLen){
            longestLen=NodesVec[nodeIndex].blocks[i].lengthInBlockchain;
            localIndexOfLongestLenBlock=i;
            actualBlockId=NodesVec[nodeIndex].blocks[i].blockId;
        }
    }
    double timestamp=negval;
    for(int i=0;i<NodesVec[nodeIndex].blocks.size();i++)
    {
        if(NodesVec[nodeIndex].blocks[i].lengthInBlockchain==longestLen){
            //When the 1st block with longestLen is found:
            if(timestamp<0){
                timestamp = NodesVec[nodeIndex].blocks[i].timeOfCreation;
                localIndexOfLongestLenBlock=i;
                actualBlockId=NodesVec[nodeIndex].blocks[i].blockId;
            }
                //When you have already found at least one block with the longestLen and found another one:
            else{
                if(timestamp<NodesVec[nodeIndex].blocks[i].timeOfCreation){
                    timestamp = NodesVec[nodeIndex].blocks[i].timeOfCreation;
                    localIndexOfLongestLenBlock=i;
                    actualBlockId=NodesVec[nodeIndex].blocks[i].blockId;
                }
            }
        }
    }
    return localIndexOfLongestLenBlock;

}

double getCoins(int hostIndex,int guestIndex)
{
    double currentCoins=0;
    int currBlockIndex=getlocalIndexOfLongestLenBlock(hostIndex);
    while(currBlockIndex!=(-1))
    {
        currentCoins+=NodesVec[hostIndex].blocks[currBlockIndex].amountMap[guestIndex];
        currBlockIndex=NodesVec[hostIndex].blocks[currBlockIndex].localIndexOfPrevBlock;
    }
    return currentCoins;
}

void generateTransactionEvent(event e)
{
    int senderId=e.currNode;
    int receipientId=randZeroOne()*numberOfNodes;
    while(receipientId==senderId)
    {
        receipientId=randZeroOne()*numberOfNodes;
    }

    double transactionAmount=randZeroOne()*getCoins(senderId,senderId);
    transaction newtrans(globalTransactionIdCounter,senderId,receipientId,transactionAmount);
    globalTransactionIdCounter+=1;
    NodesVec[senderId].allTransactions.insert(newtrans);

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
    for (auto curreTransaction:NodesVec[e.currNode].allTransactions )
    {
        if(curreTransaction.transactionId==e.currTransaction.transactionId)
        {
            transactionAlreadyRecieved=true;
            break;
        }
    }

    if(!transactionAlreadyRecieved)
    {
        NodesVec[e.currNode].allTransactions.insert(e.currTransaction);
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
                    localIndexOfLongestLenBlock=i;
                    actualBlockId=NodesVec[e.currNode].blocks[i].blockId;
                }
                    //When you have already found at least one block with the longestLen and found another one:
                else{
                    if(timestamp<NodesVec[e.currNode].blocks[i].timeOfCreation){
                        timestamp = NodesVec[e.currNode].blocks[i].timeOfCreation;
                        localIndexOfLongestLenBlock=i;
                        actualBlockId=NodesVec[e.currNode].blocks[i].blockId;
                    }
                }
            }
        }

        //create a new block that refers to this block
        block generatedBlock(globalBlockIdCounter,e.scheduleTime,actualBlockId,e.currNode,longestLen+1);



        DEBUG2("Block Being created");
        DEBUG2(globalBlockIdCounter);
        DEBUG2(e.scheduleTime);
        // DEBUG2(e.currNode);
        // DEBUG2("Block created\n");

        NodesVec[e.currNode].mapBlockidReceivetime[globalBlockIdCounter]=e.scheduleTime;
        globalBlockIdCounter+=1;

        //Mining Fee Given
        NodesVec[e.currNode].allTransactions.insert(transaction(globalTransactionIdCounter,godNode,e.currNode,50));
        globalTransactionIdCounter+=1;

        //add all unspent transactions to this block
        set<transaction> unspentTransactions=NodesVec[e.currNode].allTransactions;
        int currBlockIndex=localIndexOfLongestLenBlock;
        while(NodesVec[e.currNode].blocks[currBlockIndex].prevBlock!=(-1))
        {
            //DEBUG2(currBlockIndex);
            for(auto temTrans:NodesVec[e.currNode].blocks[currBlockIndex].transactionSet)
            {
                unspentTransactions.erase(temTrans);
            }
            currBlockIndex=NodesVec[e.currNode].blocks[currBlockIndex].localIndexOfPrevBlock;
        }

        vector<double> coinValuesAtHost(numberOfNodes,0);
        for(int i1=0;i1<numberOfNodes;i1++)
        {
            coinValuesAtHost[i1]=getCoins(e.currNode,i1);
        }

        for(auto txn:unspentTransactions)
        {
            if(txn.senderNodeId==godNode)
            {
                coinValuesAtHost[txn.destinationNodeId]+=txn.coins;
                generatedBlock.amountMap[txn.destinationNodeId]+=txn.coins;
            } else
            {
                if( txn.coins>coinValuesAtHost[txn.senderNodeId])
                {

                    //TODO: DIE
                    unspentTransactions.erase(txn);
                }
                else
                {
                    coinValuesAtHost[txn.senderNodeId]-=txn.coins;
                    coinValuesAtHost[txn.destinationNodeId]+=txn.coins;
                    generatedBlock.amountMap[txn.senderNodeId]-=txn.coins;
                    generatedBlock.amountMap[txn.destinationNodeId]+=txn.coins;
                }
            }
        }



        generatedBlock.transactionSet=unspentTransactions;

//        set<transaction> initialUnspentTransaction=NodesVec[e.currNode].allTransactions;
//        for(auto currrTrans:initialUnspentTransaction)
//        {
//            if(NodesVec[e.currNode].blocks[localIndexOfLongestLenBlock].transactionSet.find(currrTrans)
//               !=NodesVec[e.currNode].blocks[localIndexOfLongestLenBlock].transactionSet.end())
//            {
//                NodesVec[e.currNode].allTransactions.erase(currrTrans);
//            } else
//            {
//                generatedBlock.transactionSet.insert(currrTrans);
//            }
//        }

        blockFile<<line_separator<<endl;
        blockFile<<"BlockId="<<generatedBlock.blockId<<endl;
        blockFile<<"CreatedByNode="<<generatedBlock.createNodeId<<endl;
        blockFile<<"TimeCreated="<<generatedBlock.timeOfCreation<<endl;
        blockFile<<"NumberOfTransactions="<<generatedBlock.transactionSet.size()<<endl;
        blockFile<<"PrevBlockId="<<generatedBlock.prevBlock<<endl;
        blockFile<<line_separator<<endl;

        //add block to my current node
        generatedBlock.localIndexOfPrevBlock=localIndexOfLongestLenBlock;
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
                break;
            }
        }
        e.currBlock.lengthInBlockchain=longestLen+1;
        NodesVec[e.currNode].mapBlockidReceivetime[e.currBlock.blockId]=e.scheduleTime;
        e.currBlock.localIndexOfPrevBlock=localIndexOfLongestLenBlock;
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
        //DEBUG2(globalCurrentTime);
        //for(int i=0;i<numberOfNodes;i++)
        {
            //DEBUG2(NodesVec[i].blocks.size());
            //DEBUG2(NodesVec[i].allTransactions.size());
        }
        //DEBUG2(NodesVec[0].blocks.size());
        //DEBUG2(NodesVec[0].allTransactions.size());

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

    genBlock.localIndexOfPrevBlock=-1;

    initialMoneyFile.open("initialMoney.txt");
    for(int i=0;i<numberOfNodes;i++)
    {
        double randAmou=initialMaxAmount*randZeroOne();
        initialMoneyFile<<"Amount with Node "<<i<<" was "<<randAmou<<endl;
        genBlock.transactionSet.insert(transaction(globalTransactionIdCounter,godNode,i,randAmou));
        genBlock.amountMap[i]+=randAmou;
        globalTransactionIdCounter+=1;
    }
    initialMoneyFile.close();

    //globalBlockIdCounter+=1;
    blockFile<<line_separator<<endl;
    blockFile<<"BlockId="<<genBlock.blockId<<endl;
    blockFile<<"CreatedByNode="<<genBlock.createNodeId<<endl;
    blockFile<<"TimeCreated="<<genBlock.timeOfCreation<<endl;
    blockFile<<"NumberOfTransactions="<<genBlock.transactionSet.size()<<endl;
    blockFile<<"PrevBlockId="<<genBlock.prevBlock<<endl;
    blockFile<<line_separator<<endl;

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
        outfile<<"plt.savefig('"<<figureName<<".svg', format='svg', dpi=1200)";
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
            outfile<<"Received at:           "<<NodesVec[i].mapBlockidReceivetime[NodesVec[i].blocks[k].blockId]<<"\n";
            for(auto txn:NodesVec[i].blocks[k].transactionSet){
                outfile<<"\tTxnID "<<txn.transactionId<<": "<<txn.senderNodeId<<" pays "<<txn.destinationNodeId<<" "<<txn.coins<<" coins"<<"\n";
            }
        }
        outfile.close();
    }
}

void printAllUnspentTransactions()
{
    for(int abc=0;abc<numberOfNodes;abc++)
    {

        int longestLen=0;
        int localIndexOfLongestLenBlock;
        int actualBlockId;

        for(int i=0;i<NodesVec[abc].blocks.size();i++)
        {
            if(NodesVec[abc].blocks[i].lengthInBlockchain>longestLen){
                longestLen=NodesVec[abc].blocks[i].lengthInBlockchain;
                localIndexOfLongestLenBlock=i;
                actualBlockId=NodesVec[abc].blocks[i].blockId;
            }
        }
        double timestamp=negval;
        for(int i=0;i<NodesVec[abc].blocks.size();i++)
        {
            if(NodesVec[abc].blocks[i].lengthInBlockchain==longestLen){
                //When the 1st block with longestLen is found:
                if(timestamp<0){
                    timestamp = NodesVec[abc].blocks[i].timeOfCreation;
                    localIndexOfLongestLenBlock=i;
                    actualBlockId=NodesVec[abc].blocks[i].blockId;
                }
                    //When you have already found at least one block with the longestLen and found another one:
                else{
                    if(timestamp<NodesVec[abc].blocks[i].timeOfCreation){
                        timestamp = NodesVec[abc].blocks[i].timeOfCreation;
                        localIndexOfLongestLenBlock=i;
                        actualBlockId=NodesVec[abc].blocks[i].blockId;
                    }
                }
            }
        }


        set<transaction> unspentTransactions=NodesVec[abc].allTransactions;
        int currBlockIndex=localIndexOfLongestLenBlock;
        while(NodesVec[abc].blocks[currBlockIndex].prevBlock!=(-1))
        {
            //DEBUG2(currBlockIndex);
            for(auto temTrans:NodesVec[abc].blocks[currBlockIndex].transactionSet)
            {
                unspentTransactions.erase(temTrans);
            }
            currBlockIndex=NodesVec[abc].blocks[currBlockIndex].localIndexOfPrevBlock;
        }
        //DEBUG2("bahar aa gaya!");

        ofstream outfile;
        //Keeping the extension of the files as .py because of better readability in the text editor
        outfile.open ("UnspentTransactionsOf"+to_string(abc)+".py");
        outfile<<"Node#"<<abc<<"\n";
        outfile<<line_separator<<"\n";
        for(auto txnSet:unspentTransactions)
        {
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
    outfile<<"plt.savefig('"<<figureName<<".svg', format='svg', dpi=1200)";
    outfile.close();
}


void printFinalAmountOfMoney()
{
    finalMoneyFile.open("FinalMoney.txt");
    for(int i=0;i<numberOfNodes;i++)
    {
        finalMoneyFile<<"Amount with Node "<<i<<" is Finally "<<getCoins(i,i)<<endl;
    }
    finalMoneyFile.close();
}





int main(int argc, char** argv)
{
    blockFile.open("blockInfoFile.txt");
    totalTimeToSimulate=1000;
    numberOfNodes=10;
    z=60;
    initialMaxAmount=100;
    globalLambdaForBlockGeneration=0.1/(4*numberOfNodes);
    globalLambdaForTransactionGeneration=(20/numberOfNodes);
    nodeConnectivityProbability=0.3;

    if (argc == 5)
    {
        totalTimeToSimulate=atof(argv[1]);
        numberOfNodes=atoi(argv[2]);
        z=atof(argv[3]);
        nodeConnectivityProbability=atof(argv[4]);
    }
    DEBUG2(totalTimeToSimulate);
    DEBUG2(numberOfNodes);
    DEBUG2(z);
    DEBUG2(nodeConnectivityProbability);


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
    printFinalAmountOfMoney();
    blockFile.close();
    return 0;
}
