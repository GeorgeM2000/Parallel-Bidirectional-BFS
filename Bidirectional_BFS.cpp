#include <bits/stdc++.h>
#include <sys/time.h>
#include <omp.h>
#include <queue>

#define NUMBER_OF_THREADS 2 // Number of nodes
#define N 1000    // Matrix size


// Basic structure of a node
struct Node {
    char visited_state;
    //u_int16_t weight;
    char thread;
    u_int16_t x;
    u_int16_t y;
    Node *parent;
};




void Serial_BFS(Node *A, u_int16_t src_node_pos[2], u_int16_t dest_node_pos[2]);   // Serial BFS algorithm
void Parallel_BFS(Node *A, u_int16_t src_node_pos[2], u_int16_t dest_node_pos[2], const char thread, bool *intersection_node_found, Node *thread_nodes);   // Parallel BFS algorithm
void Initialize_A(Node *A);    // Initialize node matrix A
void Find_Path(Node* current_node, bool print_path_mode); // Find path from source node to destination node
void Print_Grid(Node *A);  // Print nodes of matrix A
long usecs (void);  // Function to measure time
void Intersect_Path(Node *thread_A_nodes, Node *thread_B_nodes);    // Intersect the path stored in the intersection node(intersection node belongs to the opposite thread) and the node from which the intersection node was detected





int main(void) {


    // Position(x, y) of source and destination nodes
    u_int16_t src_pos[2];
    u_int16_t dest_pos[2];

    // Each thread stores two nodes before completion. The first node is the node from which we detected the intersection node. The second node is the intersection node. 
    Node thread_A_nodes[2];
    Node thread_B_nodes[2];


    
    bool intersection_node_found = false, *intersection_node_found_ptr;
    intersection_node_found_ptr = &intersection_node_found;

    // Time variables
    long t_start,t_end;
   	double time;

    // Create node matrix A
    Node* A = (Node*)malloc(sizeof(Node) * N * N);

    Initialize_A(A);    // Initialize A

    
    // Source and destination position input
    std::cout << "\nEnter the position of the source node: ";
    std::cin >> src_pos[0] >> src_pos[1];

    std::cout << "\nEnter the position of the destination node: ";
    std::cin >> dest_pos[0] >> dest_pos[1];



    t_start=usecs();    // Start timer

    //Serial_BFS(A, src_pos, dest_pos);

    #pragma omp parallel num_threads(NUMBER_OF_THREADS) shared(A, intersection_node_found_ptr, src_pos, dest_pos, thread_A_nodes, thread_B_nodes)
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            Parallel_BFS(A, src_pos, dest_pos, 'A', intersection_node_found_ptr, thread_A_nodes);

            #pragma omp section
            Parallel_BFS(A, dest_pos, src_pos, 'B', intersection_node_found_ptr, thread_B_nodes);
        }
    }

    t_end=usecs();  // End timer

    // Calculate the time elapsed and print it
    time = ((double)(t_end-t_start))/1000000;
    std::cout << "Computation Time: " << time << std::endl;

    //Intersect_Path(thread_A_nodes, thread_B_nodes); // Intersect path of thread A and thread B

    // Delete node matrix A
    free(A);

    return 0;
}

void Intersect_Path(Node *thread_A_nodes, Node *thread_B_nodes) {
    /*
    If the visited state('W' for White, 'B' for Black and 'G' for Gray) of the first node stored in each thread array is something other than the default('W'), then there is a path.
    When the thread arrays where created, the nodes were initialized to have default values. That means, there is no path because the nodes don't have parent nodes. So if the first node 
    has a parent node then there is a path.
    */ 
    if(thread_A_nodes[0].visited_state != 'W') {
        Find_Path(&thread_A_nodes[1], 0);
        Find_Path(&thread_A_nodes[0], 1);
    } 
    else if(thread_B_nodes[0].visited_state != 'W') {
        Find_Path(&thread_B_nodes[1], 0);
        Find_Path(&thread_B_nodes[0], 1);
    }
}


long usecs (void) {
    struct timeval t;
    gettimeofday(&t,NULL);
    return t.tv_sec*1000000+t.tv_usec;
}

void Initialize_A(Node *A) {
    for(int i=0; i<N; i++) {
        for(int j=0; j<N; j++) {
            A[i * N + j].x = i;
            A[i * N + j].y = j;
            A[i * N + j].visited_state = 'W';
        }
    }
}

void Print_Grid(Node *A) {
    for(int i=0; i<N; i++) {
        for(int j=0; j<N; j++) {
            std::cout << "(" << A[i * N + j].x << "," << A[i * N + j].y << ") ";
        }
        std::cout << std::endl;
    }
}

void Find_Path(Node* current_node, bool print_path_mode) {
    if(!current_node->parent) {
        std::cout << current_node->x << " " << current_node->y << std::endl;
    } else {

        if(print_path_mode) {
            std::cout << current_node->x << " " << current_node->y << std::endl;
            Find_Path(current_node->parent, print_path_mode);
        } else {
            Find_Path(current_node->parent, print_path_mode);
            std::cout << current_node->x << " " << current_node->y << std::endl;
        }   
    }
}

void Serial_BFS(Node *A, u_int16_t src_node_pos[2], u_int16_t dest_node_pos[2]) {

    A[src_node_pos[0] * N + src_node_pos[1]].visited_state = 'G';
    A[src_node_pos[0] * N + src_node_pos[1]].parent = NULL;

    std::queue<Node> queue;
    queue.push(A[src_node_pos[0] * N + src_node_pos[1]]);

    std::cout << "Started!" << std::endl;

    while(!queue.empty()) {
        Node u = queue.front();
        queue.pop();

        if(u.x == dest_node_pos[0] && u.y == dest_node_pos[1]) {
            std::cout << "Node Found!" << std::endl;
            //Find_Path(&A[u.x * N + u.y], 0);
            break;

        }
        
        // Moving up
        if(u.x-1 >= 0 && A[(u.x-1) * N + u.y].visited_state == 'W') {
            A[(u.x-1) * N + u.y].visited_state = 'G';
            A[(u.x-1) * N + u.y].parent = &A[u.x * N + u.y];
            queue.push(A[(u.x-1) * N + u.y]);
        }
        // Moving Down
        if(u.x+1 < N && A[(u.x+1) * N + u.y].visited_state == 'W') {
            A[(u.x+1) * N + u.y].visited_state = 'G';
            A[(u.x+1) * N + u.y].parent = &A[u.x * N + u.y];
            queue.push(A[(u.x+1) * N + u.y]);
        }
        // Moving left
        if(u.y-1 >= 0 && A[u.x * N + (u.y-1)].visited_state == 'W') {
            A[u.x * N + (u.y-1)].visited_state = 'G';
            A[u.x * N + (u.y-1)].parent = &A[u.x * N + u.y];
            queue.push(A[u.x * N + (u.y-1)]);
        }
        // Moving right
        if(u.y+1 < N && A[u.x * N + (u.y+1)].visited_state == 'W') {
            A[u.x * N + (u.y+1)].visited_state = 'G';
            A[u.x * N + (u.y+1)].parent = &A[u.x * N + u.y];
            queue.push(A[u.x * N + (u.y+1)]);
        }

        u.visited_state = 'B';
    }

}


void Parallel_BFS(Node *A, u_int16_t src_node_pos[2], u_int16_t dest_node_pos[2], const char thread, bool *intersection_node_found, Node *thread_nodes) {

    A[src_node_pos[0] * N + src_node_pos[1]].visited_state = 'G';
    A[src_node_pos[0] * N + src_node_pos[1]].parent = NULL;
    A[src_node_pos[0] * N + src_node_pos[1]].thread = thread;


    std::queue<Node> queue;
    queue.push(A[src_node_pos[0] * N + src_node_pos[1]]);

    std::cout << "Started!" << std::endl;

    while(!queue.empty() && !*intersection_node_found) {
        Node u = queue.front();
        queue.pop();

        // If the destination node is found from thread A or B 
        if(u.x == dest_node_pos[0] && u.y == dest_node_pos[1]) {
            std::cout << "Destination Node Found for thread " << thread << std::endl;
            *intersection_node_found = true;
            break;

        }
        


        // Moving up
        if(u.x-1 >= 0) {
            if(A[(u.x-1) * N + u.y].visited_state == 'W') {
                A[(u.x-1) * N + u.y].visited_state = 'G';
                A[(u.x-1) * N + u.y].thread = thread;
                A[(u.x-1) * N + u.y].parent = &A[u.x * N + u.y];
                queue.push(A[(u.x-1) * N + u.y]);
            } 
            else if((A[(u.x-1) * N + u.y].visited_state == 'G' || A[(u.x-1) * N + u.y].visited_state == 'B') && A[(u.x-1) * N + u.y].thread != thread) {
                *intersection_node_found = true;
                thread_nodes[0] = A[u.x * N + u.y]; thread_nodes[1] = A[(u.x-1) * N + u.y];
                break;
            }
        } 

        // Moving Down
        if(u.x+1 < N) {
            if(A[(u.x+1) * N + u.y].visited_state == 'W') {
                A[(u.x+1) * N + u.y].visited_state = 'G';
                A[(u.x+1) * N + u.y].thread = thread;
                A[(u.x+1) * N + u.y].parent = &A[u.x * N + u.y];
                queue.push(A[(u.x+1) * N + u.y]);
            }
            else if((A[(u.x+1) * N + u.y].visited_state == 'G' || A[(u.x+1) * N + u.y].visited_state == 'B') && A[(u.x+1) * N + u.y].thread != thread) {
                *intersection_node_found = true;
                thread_nodes[0] = A[u.x * N + u.y]; thread_nodes[1] = A[(u.x+1) * N + u.y];
                break;
            }
        }

        // Moving left
        if(u.y-1 >= 0) {
            if(A[u.x * N + (u.y-1)].visited_state == 'W') {
                A[u.x * N + (u.y-1)].visited_state = 'G';
                A[u.x * N + (u.y-1)].thread = thread;
                A[u.x * N + (u.y-1)].parent = &A[u.x * N + u.y];
                queue.push(A[u.x * N + (u.y-1)]);
            } 
            else if((A[u.x * N + (u.y-1)].visited_state == 'G' || A[u.x * N + (u.y-1)].visited_state == 'B') && A[u.x * N + (u.y-1)].thread != thread) {
                *intersection_node_found = true;
                thread_nodes[0] = A[u.x * N + u.y]; thread_nodes[1] = A[u.x * N + (u.y-1)];
                break;
            }
        }
        // Moving right
        if(u.y+1 < N) {
            if(A[u.x * N + (u.y+1)].visited_state == 'W') {
                A[u.x * N + (u.y+1)].visited_state = 'G';
                A[u.x * N + (u.y+1)].thread = thread;
                A[u.x * N + (u.y+1)].parent = &A[u.x * N + u.y];
                queue.push(A[u.x * N + (u.y+1)]);
            } 
            else if((A[u.x * N + (u.y+1)].visited_state == 'G' || A[u.x * N + (u.y+1)].visited_state == 'B') && A[u.x * N + (u.y+1)].thread != thread) {
                *intersection_node_found = true;
                thread_nodes[0] = A[u.x * N + u.y]; thread_nodes[1] = A[u.x * N + (u.y+1)];                 
                break;
            }
        }

        u.visited_state = 'B';
    }

}



