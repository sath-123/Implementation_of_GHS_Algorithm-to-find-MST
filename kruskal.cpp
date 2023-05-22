#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
using namespace std;
#include <chrono>
 
// Data structure to store a graph edge
struct Edge {
    int src, dest, weight;
};
 
// Comparison object to be used to order the edges
struct compare
{
    bool operator() (Edge const &a, Edge const &b) const {
        return a.weight > b.weight;
    }
};
 
// A class to represent a disjoint set
class DisjointSet
{
    unordered_map<int, int> parent;
 
public:
    // perform MakeSet operation
    void makeSet(int n)
    {
        // create `n` disjoint sets (one for each vertex)
        for (int i = 0; i < n; i++) {
            parent[i] = i;
        }
    }
 
    // Find the root of the set in which element `k` belongs
    int Find(int k)
    {
        // if `k` is root
        if (parent[k] == k) {
            return k;
        }
 
        // recur for the parent until we find the root
        return Find(parent[k]);
    }
 
    // Perform Union of two subsets
    void Union(int a, int b)
    {
        // find the root of the sets in which elements
        // `x` and `y` belongs
        int x = Find(a);
        int y = Find(b);
 
        parent[x] = y;
    }
};
 
// Function to construct MST using Kruskal’s algorithm
vector<Edge> runKruskalAlgorithm(vector<Edge> edges, int n)        // no-ref, no-const
{
    // stores the edges present in MST
    vector<Edge> MST;
 
    // initialize `DisjointSet` class
    DisjointSet ds;
 
    // create a singleton set for each element of the universe
    ds.makeSet(n);
 
    // sort edges by increasing weight
    sort(edges.begin(), edges.end(), compare());
 
    // MST contains exactly `V-1` edges
    while (MST.size() != n - 1)
    {
        // consider the next edge with minimum weight from the graph
        Edge next_edge = edges.back();
        edges.pop_back();
 
        // find the root of the sets to which two endpoints
        // vertices of the next edge belongs
        int x = ds.Find(next_edge.src);
        int y = ds.Find(next_edge.dest);
 
        // if both endpoints have different parents, they belong to
        // different connected components and can be included in MST
        if (x != y)
        {
            MST.push_back(next_edge);
            ds.Union(x, y);
        }
    }
    return MST;
}
 
int main()
{
    // vector of graph edges as per the above diagram.
    int n;
    cin >> n;

    vector<Edge> edges ;
    struct Edge s;
    // {
    //     // (u, v, w) triplet represent undirected edge from
    //     // vertex `u` to vertex `v` having weight `w`
    //     {0, 1, 7}, {1, 2, 8}, {0, 3, 5}, {1, 3, 9}, {1, 4, 7}, {2, 4, 5},
    //     {3, 4, 15}, {3, 5, 6}, {4, 5, 8}, {4, 6, 9}, {5, 6, 11}
    // };
 
    // total number of nodes in the graph (labelled from 0 to 6)
    // int n = 7;
 
    // construct graph
    for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			int buff[2];
			buff[0] = j;
			cin >> buff[1];
			if (buff[1] != 100000)
            {
                s.src=i;
                s.dest=j;
                s.weight=buff[1];
				edges.push_back(s);
            }
			
			
				
		}
	}
    
    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
    vector<Edge> e = runKruskalAlgorithm(edges, n);
    std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();

    
    for (Edge &edge: e)
    {
        int temp;
        if (edge.src > edge.dest)
        {
            temp=edge.dest;
            edge.dest=edge.src;
            edge.src=temp;

        }

    }

    
    for (Edge &edge: e)
    {
        cout << edge.src << " " << edge.dest << " "
             << edge.weight  << endl;
    }
    std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time);
    // std::cout << "Execution time: " << duration.count() << " seconds" << std::endl;



 
    return 0;
}