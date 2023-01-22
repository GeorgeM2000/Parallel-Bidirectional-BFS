# Parallel-Bidirectional-BFS
A parallel implementation of the BFS(Breadth First Search) algorithm called bidirectional BFS.

# Description
The algorithm begins the search using a source and a destination node in a NxN matrix of nodes. The size of the matrix is arbitrary. Using the OpenMP API compiler directives, we fork two threads each of which executes an instance of the algorithm with the same parameters but in different order. In thread A, the BFS is called with the following parameters:

- BFS(source, destination)

Thread B calls BFS with parameters:

- BFS(destination, source)

As we can see, thread A begins searching the destination node from the source node and thread B does the exact opposite of that. If a thread(A or B) detects a node that has been visited by the opposite thread, the algorithm stops, and finally we print the path using the parent attribute of each node.
