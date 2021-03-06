# blockchain-simulator
## COL867: Assignment 1
The root directory contains these files:
1) blockChainSimulator.cpp : It contains the blockchain simulator code.
2) makeGraph.sh : It creates python scripts which can be used for visualization purposes,It generates Blockchain at first 10 nodes.  
3) run.sh : It is used to compile and run the file blockChainSimulator.cpp
4) README.md : This very file.
5) cleanFolder.sh : Cleans current folder of SVG, Python and Text Files. Required when you stop run.sh in the middle due to long execution time.  

Instructions to run the simulator:
./run.sh &lt;outputDirectory&gt; &lt;SimulationTime&gt; &lt;NumberOfNodes&gt; &lt;percentageSlowNodes&gt; &lt;nodeConnectionProbability&gt;  

Parameters:  
1. outputDirectory:Name of Directory where you want to dump the input  
2. SimulationTime:Time in Seconds for which you want to simulate  
3. NumberOfNodes: Number of Nodes in Network  
4. percentageSlowNodes: This should be b/w 0 and 100 and signifies percentage of Slow Nodes in Network  
5. nodeConnectionProbability: Probability of making any edge in the Network. Its value should be between 0 and 1  

OutputFiles:
1) SVG Files: Pictures of Node Structure and Blockchain structure on first 10 nodes.  
2) blockChainTreeOf<index>.py: Shows Blockchain and Other details of blocks inside each node.  
3) blockInfoFile.txt: Shows Info of generated blocks during full simulation.  
4) initialMoney.txt: Shows Coins with each Node initially.  
5) FinalMoney.txt: Shows Coins with each Node after end of Simulation.  
6) UnspentTransactionsOf<index>.py: Shows Transactions at each node which are not in longest chain at end of simulation.  
