
#include <bits/stdc++.h>
#include <mpi.h>
#include <ctime>

using namespace std;

#define INF 100000
#define UDEF -1

// Tuple indexes of the edges
#define NODE 0
#define WEIGHT 1
#define TYPE 2

// States of the nodes
#define SLEEP 0
#define FIND 1
#define FOUND 2

// Types of the edges
#define BASIC_E 0
#define BRANCH_E 1
#define REJECT_E -1

vector<tuple<int, int, int>> edges; // (node, weight,type)
queue<pair<int,vector<int>>> deferred;

enum {CONNECT, INITIATE, TEST, ACCEPT, REJECT, REPORT, CHANGE_ROOT,TERMINATE};

int size, rank_p, rec_p;
int level = 0;
int nodeState = 0;
int fragmentName;
int father;
int bestch;
int bestwt;
int testch;
int termination=1;


// All the functions
void wakeUp();
void recvConnect(int q, int L);
void initiate(int L, int F, int S, int q);
void test();
void recvTest(int L, int F, int q);
void recvAccept(int q);
void recvReject(int q);
void report();
void recvReport(int W, int q);
void changeRoot();
void recvChangeRoot();
int getIndex(int node);
void terminate_p(int q);




int getIndex(int node)
{
    for(int i = 0;i < edges.size();i++)
    {
        if(get<NODE>(edges[i]) == node)
        {
            return i;
        }
    }
    return -1;
}

// 1. Initialisation 
void wakeUp()
{
    int node = get<NODE>(edges[0]);
    //cout << "Waking up node " << rank_p << ": with least incident edge to node " << node << endl;
    
    level = 0;
    get<TYPE>(edges[0]) = BRANCH_E;
    nodeState = FOUND;
    rec_p = 0;

    // something is missing here

    // Send CONNECT to node
    int buff = 0;
    //cout<< "Sending a CONNECT message to node " << node << " from node " << rank_p << endl;
    MPI_Send(&buff, 1, MPI_INT, node, CONNECT, MPI_COMM_WORLD);
}

// 2. receipt of CONNECT
void recvConnect(int q, int L)
{
    int index = getIndex(q);

    //cout<< "Received a CONNECT message from node " << q << " to node " << rank_p << ": with level = " << L <<  " and my level is " << level <<endl;
    
    if(L < level)
    {
        get<TYPE>(edges[index]) = BRANCH_E;

        int buff[3];
        buff[0] = level;
        buff[1] = fragmentName;
        buff[2] = nodeState;

        // Sending Initiate message to q
        //cout<< "Sending an INITIATE message to node " << q << " from node " << rank_p << ":with {level, fragmentName, state} = { " << level << "," << fragmentName << "," << nodeState << " }"  << endl;
        MPI_Send(&buff, 3, MPI_INT, q, INITIATE, MPI_COMM_WORLD);
    }
    else if(get<TYPE>(edges[index]) == BASIC_E)
    {
        vector<int> def;
        def.push_back(q);
        def.push_back(L);

        deferred.push(make_pair(CONNECT,def));
        //cout<< "Deferred CONNECT message from node " << q << " to node " << rank_p << ":as the edge is Basic" << endl;
    }
    else
    {
        int buff[3];
        buff[0] = level + 1;
        buff[1] = get<WEIGHT>(edges[index]);
        buff[2] = FIND;

        // Sending Initiate message to q
        //cout<< "Sending an INITIATE message to node " << q << " from node " << rank_p << ":with {level, fragmentName, state} = { " << buff[0] << ","<< buff[1] << "," << buff[2] << " }"  << endl;
        MPI_Send(&buff, 3, MPI_INT, q, INITIATE, MPI_COMM_WORLD);
    }
}

// 3. receipt of INITIATE
void initiate(int L, int F, int S, int q)
{
    level = L;
    fragmentName = F;
    nodeState = S;
    father = q;
    bestch = UDEF;
    bestwt = INF;
    // testch = UDEF;

    for(int i = 0;i < edges.size();i++)
    {
        if(get<TYPE>(edges[i]) == BRANCH_E and get<NODE>(edges[i]) != q)
        {
            int buff[3];

            buff[0] = L;
            buff[1] = F;
            buff[2] = S;

            int node = get<NODE>(edges[i]);

            // send INITIATE to node i
            //cout<< "Sending an INITIATE message to node " << node << " from node " << rank_p << "with {level, fragmentName, state} = { " << buff[0] << "," << buff[1] << "," << buff[2] << " }"  << endl;
            MPI_Send(&buff, 3, MPI_INT, node, INITIATE, MPI_COMM_WORLD);
        }
    }

    if(nodeState == FIND)
    {
        rec_p = 0;

        //cout<< "If nodeState == FIND, then test();" << endl;
        test();
    }

}

// 4. Procedure test
void test()
{
    // printf("%d: entered test \n",rank_p);
    int index = -1;

    for(int i = 0;i < edges.size();i++)
    {
        if(get<TYPE>(edges[i]) == BASIC_E)
        {
            index = i;
            break;
        }
    }

    if(index != -1)
    {
        testch = get<NODE>(edges[index]);

        int buff[2];
        buff[0] = level;
        buff[1] = fragmentName;

        // Send TEST message to testch
        //cout<< "Sending a TEST message to node " << testch << " from node " << rank_p << "with {level, fragmentName} = { " << level << "," << fragmentName << " }"  << endl;
        MPI_Send(&buff, 2, MPI_INT, testch, TEST, MPI_COMM_WORLD);

    }
    else
    {
        testch = UDEF;
        report();
    }



}

// 5. Receipt of TEST
void recvTest(int L, int F, int q)
{
    //cout<< "Received a TEST message from node " << q << " to node " << rank_p << " with {level, fragmentName} = { " << L << "," << F << " } and my level is " << level << endl;
    
    int index = getIndex(q);

    if(L > level)
    {
        vector<int> def;
        def.push_back(L);
        def.push_back(F);
        def.push_back(q);

        deferred.push(make_pair(TEST,def));
        //cout<< "Deferred TEST message from node " << q << " to node " << rank_p << "as the level recieved id higher" << endl;
    }
    else if(F == fragmentName)
    {
        if(get<TYPE>(edges[index]) == BASIC_E)
            get<TYPE>(edges[index]) = REJECT_E;
        
        if(q != testch)
        {
            int buff = 0;
            MPI_Send(&buff, 1, MPI_INT, q, REJECT, MPI_COMM_WORLD);
            //cout<< "Sending a REJECT message to node " << q << " from node " << rank_p << endl;
        }
        else
            test();
    }
    else
    {
        int buff = 0;
        MPI_Send(&buff, 1, MPI_INT, q, ACCEPT, MPI_COMM_WORLD);
        //cout<< "Sending an ACCEPT message to node " << q << " from node " << rank_p << endl;
    }

}

// 6. Receipt of ACCEPT
void recvAccept(int q)
{
    //cout<< "Received an ACCEPT message from node " << q << " to node " << rank_p << endl;
    int index = getIndex(q);

    testch = UDEF;

    if(get<WEIGHT>(edges[index]) < bestwt)
    {
        bestwt = get<WEIGHT>(edges[index]);
        bestch = q;
    }

    report();
}

// 7. Receipt of REJECT
void recvReject(int q)
{
    //cout<< "Received a REJECT message from node " << q << " to node " << rank_p << endl;
    int index = getIndex(q);

    if(get<TYPE>(edges[index]) == BASIC_E)
        get<TYPE>(edges[index]) = REJECT_E;

    test();
}

// 8. Procedure report
void report()
{
    int report_num = 0;
    for(int i = 0;i < edges.size();i++)
    {
        if(get<TYPE>(edges[i]) == BRANCH_E and get<NODE>(edges[i]) != father)
        {
            report_num++;
        }
    }

    if(rec_p == report_num and testch == UDEF)
    {
        nodeState = FOUND;

        int buff = bestwt;

        MPI_Send(&buff, 1, MPI_INT, father, REPORT, MPI_COMM_WORLD);
        //cout<< "Sending a REPORT message to node " << father << " from node " << rank_p << " with bestwt = " << bestwt << endl;
    }
}

// 9. Receipt of REPORT
void recvReport(int W, int q)
{
    //cout<< "Received a REPORT message from node " << q << " to node " << rank_p << " with bestwt = " << W << endl;
    int index = getIndex(q);

    if(q != father)
    {
        if(W < bestwt)
        {
            bestwt = W;
            bestch = q;
        }
        rec_p++;
        report();
    }
    else
    {
        if(nodeState == FIND)
        {
            vector<int> def;
            def.push_back(W);
            def.push_back(q);

            deferred.push(make_pair(REPORT,def));
        }
        else if(W > bestwt)
        {
            changeRoot();
        }
        else if(W == bestwt and bestwt == INF)
        {
            // Terminate
            // //cout<< "Terminating the process " << rank_p << endl;
            // something is missing here
            // //cout << rank_p << ": I am the first one to detect the termination" << endl;

            terminate_p(-1);
            termination = 0;

        }
    }
}

// 10. Procedure changeRoot
void changeRoot()
{
    int index = getIndex(bestch);

    if(get<TYPE>(edges[index]) == BRANCH_E)
    {
        int buff = 0;
        MPI_Send(&buff, 1, MPI_INT, bestch, CHANGE_ROOT, MPI_COMM_WORLD);
    }
    else
    {
        get<TYPE>(edges[index]) = BRANCH_E;
        int buff = level;
        MPI_Send(&buff, 1, MPI_INT, bestch, CONNECT, MPI_COMM_WORLD);
    }
}

// 11. Receipt of CHANGEROOT
void recvChangeRoot()
{
    changeRoot();
}

// Propagate the TERMINATE message to your neighbours, whose status is BRANCH_E
void terminate_p(int q)
{
    for(int i = 0;i < edges.size();i++)
    {
        if(get<TYPE>(edges[i]) == BRANCH_E and get<NODE>(edges[i]) != q)
        {
            int buff = 0;
            int node = get<NODE>(edges[i]);
            MPI_Send(&buff, 1, MPI_INT, node, TERMINATE, MPI_COMM_WORLD);
        }
    }
}





int main(int argc, char *argv[])
{
    // Intializing the mpi
    MPI_Init(&argc, &argv);
    
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_p);

    // reading the input, at the root node
    int nodes;
    if (rank_p == 0)
    {
        cin >> nodes;

        if (nodes != size)
        {
            //cout<< "Number of nodes and processes must be equal" << endl;
            return 0;
        }

        for (int i = 0; i < nodes; i++)
        {
            for (int j = 0; j < nodes; j++)
            {
                int buff[2];
                buff[0] = j;
                cin >> buff[1];

                if (i == 0)
                {
                    if (buff[1] != INF)
                        edges.push_back(make_tuple(j, buff[1], 0));
                }
                else
                    MPI_Send(&buff, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
    }
    // double start_time = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);

    // broadcasting number of nodes
    MPI_Bcast(&nodes, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // receiving the node edges
    int buffer[2];
    if (rank_p != 0)
    {
        for (int i = 0; i < nodes; i++)
        {
            MPI_Recv(&buffer, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (buffer[1] != INF)
                edges.push_back(make_tuple(buffer[0], buffer[1], 0));
        }
    }
    
    // sort edges based on weight, ascending, which is the second element of the tuple
    sort(edges.begin(), edges.end(), [](const tuple<int, int, int> &a, const tuple<int, int, int> &b) {
        return get<WEIGHT>(a) < get<WEIGHT>(b);
    });

    // printing the edge, with greater weight first
    // for (int i = 0; i < edges.size(); i++)
    // {
    //     //cout << rank_p << " " << get<NODE>(edges[edges.size()-1]) << " " << get<WEIGHT>(edges[edges.size()-1]) << endl;
    // }

    MPI_Barrier(MPI_COMM_WORLD);

    // Activating the nodes
    wakeUp();

   
    int MSG;
    int flag=0;
    int source;
    int n_count;

    
    
    while(termination)
    {
        int flag;
        MPI_Status status;
        // non blocking function,if there is any msg from any source we will have information in status
        // if there is no any msg ,it won't wait
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,&flag, &status);
        // //cout << flag << "flag" << endl;
        if(flag)          // on receving the msg
        {
            source = status.MPI_SOURCE;
            MSG = status.MPI_TAG;
            // //cout << MSG << " msg info" << endl;

            MPI_Get_count(&status, MPI_INT, &n_count);
            
            int* number_buf = (int*)malloc(sizeof(int) * n_count);
            
            MPI_Recv(number_buf, n_count, MPI_INT, source,MSG,MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if(MSG == CONNECT)
            {
                // //cout<< rank_p << ": CONNECT from " << source << endl;
                recvConnect(source,number_buf[0]);
            }
            else if(MSG == INITIATE)
            {
                // //cout<< rank_p << " INITIATE from " << source << endl;

                int L = number_buf[0];
                int F = number_buf[1];
                int S = number_buf[2];
                
                initiate(L,F,S,source);
            }
            else if(MSG == TEST)
            {
                // //cout<< rank_p << ": TEST from " << source << endl;

                int L = number_buf[0];
                int F = number_buf[1];

                recvTest(L,F,source);
            }
            else if(MSG == ACCEPT)
            {
                // //cout<< rank_p << ": ACCEPT from " << source << endl;

                recvAccept(source);
            }
            else if(MSG == REJECT)
            {
                // //cout<< rank_p << ": REJECT from " << source << endl;

                recvReject(source);
            }
            else if(MSG == REPORT)
            {
                // //cout<< rank_p << ": REPORT from " << source << endl;

                int w = number_buf[0];
                recvReport(w,source);
            }
            else if(MSG == CHANGE_ROOT)
            {
                // //cout<< rank_p << ": CHANGEROOT from " << source << endl;
                recvChangeRoot();
            }
            else if(MSG == TERMINATE)
            {
                terminate_p(source);
                break;
            }
            


            
        }

        if(deferred.size() > 0)
        {
            pair<int,vector<int>> msg = deferred.front();
            deferred.pop();

            if(msg.first == CONNECT)
            {
                // //cout<< "Deferred " << rank_p << ": CONNECT from " << msg.second[0] << endl;
                
                recvConnect(msg.second[0],msg.second[1]);
            }
            else if(msg.first == REPORT)
            {
                // //cout<< "Deferred " << rank_p << ": REPORT from " << msg.second[1] << endl;

                recvReport(msg.second[0],msg.second[1]);
            }
            else if(msg.first == TEST)
            {
                // //cout<< " " << rank_p << ": TEST from " << msg.second[2] << endl;

                recvTest(msg.second[0],msg.second[1],msg.second[2]);
            }
            
        }


    }
    // double local_time = MPI_Wtime() - start_time;
    // double max_time = -1;
    // MPI_Reduce(&local_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    

    for(int i = 0;i < edges.size();i++)
    {
        if(get<TYPE>(edges[i]) == BRANCH_E and get<NODE>(edges[i]) > rank_p)    
        {
            cout << rank_p << " " << get<NODE>(edges[i]) << " " << get<WEIGHT>(edges[i]) << endl;
        }
    }
    // if (rank_p == 0) {
    //     // double end_time = MPI_Wtime();
    //     // double computation_time = end_time - start_time;

    //     // printf("Computation time: %f seconds\n", computation_time);
    //     printf("Slowest process time: %f seconds\n", max_time/CLOCKS_PER_SEC);
    // }
    

   
    MPI_Finalize();
    return 0;
}
