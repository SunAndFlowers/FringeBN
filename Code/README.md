# FringeBN
## Algorithm Introduction
We propose a Bayesian network learning method based on optimizing fringe search (FringeBN).

(1) In the reverse order graph, we use the depth first search method to traverse the nodes. The binary bit vector and bit operation are used to query the optimal parent sets for the nodes quickly and conveniently. In addition, we optimize the storage order of nodes in bidirectional linked list, which can help avoid sorting the nodes in the list. Then the better nodes can be expanded quickly and thus to improve the learning efficiency.

(2) According to the characteristics of Bayesian network, we adopt the dynamic threshold modification method to further enhance efficiency and accuracy. The threshold in the fringe search is updated dynamically according to BN score and layer number of the traversing nodes. This can help select a proper number of excellent nodes to do expansion. In addition, we prune the nodes according to the maximum number of times that the nodes are limited by the threshold. This can reduce the memory consumption and improve efficiency, so as to increase the maximum number of variables that can be processed to a certain extent.

(3) We use Child and Asia dataset to do experiments and validate the effectiveness of FringeBN. Experiment results show that FringeBN not only improves the accuracy and efficiency, but also costs less memory space. In addition, FringeBN can process the data with more number of variables, and have better learning efficiency and accuracy compared with the existing algorithms of A*, dynamic programming (DP), branch and bound (B&B) and DFBnB.
## Operating environment configuration
This project needs to be run on an Ubuntu system. The following configuration assumes that a pure Ubuntu 18.04 x64 is used
### 1. Install operation environment

```
▼ gcc-4.8.5 and g++-4.8.5
● First,make sure you are using gcc-4.8.5 and g++-4.8.5. You can use the commands "gcc --version" and "g++ --version" to check the version.
● Installation of gcc, g++ 4.8.5 version:
    	sudo apt-get install -y gcc-4.8 g++-4.8
● If there is only one version of gcc4.8 and g++ 4.8 in your system at this time, please ignore these steps. If there are two versions, please execute as follows:
	ls /usr/bin
	update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 50 #(50 means high priority)
	update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 50
	update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 20 #(20 means low priority,gcc-7 means another version of gcc)
	update-alternatives --config gcc
● Perform these steps regardless of whether you have multiple versions of gcc:
	vim ~/.bashrc # (Add the following 5 lines at the end of the file)
	export C_INCLUDE_PATH=$HOME/local/include:$C_INCLUDE_PATH
	export CPLUS_INCLUDE_PATH=$HOME/local/include:$C_INCLUDE_PATH
	export LD_LIBRARY_PATH=$HOME/local/lib:$HOME/local/lib64:$LD_LIBRARY_PATH
	export LIBRARY_PATH=$LD_LIBRARY_PATH
	export PATH=$HOME/local/bin:$PATH
	source $HOME/.bashrc
	
▼ boost1.63.0
● If your system already has an incompatible version of boost, please perform following steps to delete the existing boost. If not, please skip these steps and go directly to download boost1.63.0.
    Enter "/usr/lib" using the command "rm `ls | grep" boost "`"
    Enter "/usr/include" and use the command "rm -rf boost"
● Download the boost1.63.0 below:
	cd /home
	wget https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.tar.bz2/download
	mv download boost_1_63_0.tar.bz2
	tar --bzip2 -xvf boost_1_63_0.tar.bz2
	cd boost_1_63_0
	./bootstrap.sh --prefix=$HOME/local
	./b2 install
	mkdir $HOME/local/bin
```

### 2. Install bazel to run the URLearning score module

```
▼ bazel-0.4.5
● To install bazel, you can refer to (https://docs.bazel.build/versions/master/install-ubuntu.html#install-with-installer-ubuntu). You need to download bazel-0.4.5-installer-linux-x86_.sh(On the Download page). After downloading bazel-0.4.5-installer-linux-x86_.sh and putting it into the package, please enter the following commands:
	apt-get install pkg-config zip g++ zlib1g-dev unzip python
	add-apt-repository ppa:openjdk-r/ppa
	apt-get update
	apt-get install openjdk-8-jdk
	chmod +x bazel-0.4.5-installer-linux-x86_64.sh
	./bazel-0.4.5-installer-linux-x86_64.sh --user
```

## Score the sample to obtain the PSS file
This module will introduce how to obtain the score file based on the sample file.
First, you need to download the project package: urlearning-cpp-dev.zip, and then you also need to download the sample file used in the example: asia_s500_v1.csv.
Please go to the Download page for all downloads.
### 1. Compile the project

```
▼ When the gcc-4.8.5, g++-4.8.5 and boost 1.63.0 environment are configured and the project files are downloaded, you can compile the code. After successful compilation, you can use "score"
	cd urlearning-cpp-dev/
	./configure --prefix=$HOME/local
	make
	make install
```
### 2. Get PSS score file

```
▼ Use "score" to convert asia_s500_v1.csv to asia_s500_v1.pss
	xxx@xxx:~$ score asia_s500_v1.csv asia_s500_v1.pss
	URLearning, Score Calculator
	Input file: 'asia_s500_v1.csv'
	Output file: 'asia_s500_v1.pss'
	Delimiter: ','
	Constraints file: ''
	r_min: '5'
	Scoring function: 'BIC'
	ESS: '1.000000'
	Maximum parents: '0'
	Threads: '1'
	Running time (per variable): '-1'
	Has header: 'false'
	Enable end-of-scoring pruning: 'true'
	Enable deCampos-style pruning (experimental for BDeu): 'False'
	Parsing input file.
	Initializing data specifications.
	recordFile.getRecords()[0].size(): 8
	Creating AD-tree.
	Thread: 0, Variable: 0, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 0, Size before pruning: 120, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 0, Size after pruning: 1, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 1, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 1, Size before pruning: 120, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 1, Size after pruning: 7, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 2, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 2, Size before pruning: 120, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 2, Size after pruning: 1, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 3, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 3, Size before pruning: 120, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 3, Size after pruning: 7, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 4, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 4, Size before pruning: 120, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 4, Size after pruning: 7, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 5, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 5, Size before pruning: 120, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 5, Size after pruning: 8, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 6, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 6, Size before pruning: 120, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 6, Size after pruning: 5, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 7, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 7, Size before pruning: 120, Time: Mon Aug 17 04:48:19 2020
	Thread: 0, Variable: 7, Size after pruning: 10, Time: Mon Aug 17 04:48:19 2020
	 0.050580s wall, 0.030000s user + 0.010000s system = 0.040000s CPU (40.1%)

	Result: We got asia_s500_v1.pss, You can download it on the Download page.
```
### 3. Postscript
- Please note that the running path does not include Chinese
- You can get the complete Asia (sample sizes of 500, 1000) and Child data sets (sample sizes of 500, 1000, and 5000 respectively) on the download page. Asia has 8 variables and Child has 20 variables. The original CSV data provided in the data package needs to be converted into a runnable PSS file through the "score" method
- If you want to add constraints, you can use the command "score --help" to find the format of the command
## Get Bayesian network based on score file
This module will introduce how to generate Bayesian network in the FringeBN algorithm. First, you need to download the code package:FringeBN.zip, also see the Download page.
### 1. Building a Bayesian network

```
▼ Use the "make" command to compile the project
	cd FringeBN
	make
▼ Put the pss file in the directory:FringeBN/dist/Debug_astar/GNU-Linux-x86. Of course, you can also specify files in other paths at runtime
▼ Use astar-debug.exe to get network
	./astar-debug asia_s500_v1.pss
	xxx@xxx:~/FringeBN/dist/Debug_astar/GNU-Linux-x86$ ./astar-debug asia_s500_v1.pss
	URLearning, DFBnB
	Dataset: 'asia_s500_v1.pss'
	Net file: ''
	Reading score cache: 'asia_s500_v1.pss'
	threshold = 0.500000	Cutset = 6
	Creating sparse parent graphs
	Creating bitset data structures
	Initializing search variables
	shortest path, nodes.size = 256
	3, 0, 2, 5, 1, 4, 6, 7, 
	best parents :
	 optimalParents[0] = {}
	 optimalParents[1] = {5, }
	 optimalParents[2] = {}
	 optimalParents[3] = {}
	 optimalParents[4] = {1, }
	 optimalParents[5] = {2, 3, }
	 optimalParents[6] = {5, }
	 optimalParents[7] = {3, 4, }
	score: 1133.481079
	END
	Nodes expanded: 261
	shortpath, nodes.size = 256

	running time: 0.003509 s!
	 0.004340s wall, 0.000000s user + 0.000000s system = 0.000000s CPU (n/a%)
```
### 2. Postscript

```
▼ The main function of the program is in the file:a_star_internal_main.cpp
▼ If you want to modify the threshold setting, please pay extra attention to the modification of files score_cache.h
▼ If you want to add constraints, you can use the command "./astar-debug --help" to find the format of the command
```

## Confusion matrix
This module will guide you to obtain a confusion matrix based on Bayesian networks to understand indicators such as network accuracy. First you need to download the file:Cmatrix.zip. See the Download page.
### 1. Compile code

```
▼ Use the following command to compile the file, you can refer to readme.txt
	cd Cmatrix
	gcc Cmatrix.cpp -lstdc++ -o Cmatrix
	
	result:We get the executable file:Cmatrix.exe
```
### 2. Generate confusion matrix

```
▼ First copy the Bayesian network structure generated in the "Network file" page into a "txt" text
	asia_s500_v1_network.txt A file opens as follows:
		8
		<varset> optimalParents[0] = {}
		<varset> optimalParents[1] = {5, }
		<varset> optimalParents[2] = {}
		<varset> optimalParents[3] = {}
		<varset> optimalParents[4] = {1, }
		<varset> optimalParents[5] = {2, 3, }
		<varset> optimalParents[6] = {5, }
		<varset> optimalParents[7] = {3, 4, }
▼ Then put the standard network "asiagraph.txt" and the above files in the "Cmatrix/" directory
▼ Finally, run the following command to get the result
	./Cmatrix Parameter1.txt Parameter1.txt
			Standard network    Current network
	xxx@xxx:~/Cmatrix$ ./Cmatrix asiagraph.txt asia_s500_v1_network.txt
	5  3  

	2  54  

	acc = 0.921875	pre = 0.714286	rec = 0.625	f1 = 0.666667
```
