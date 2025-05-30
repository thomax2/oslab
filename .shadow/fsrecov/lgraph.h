#ifndef __LGRAPH_H
#define __LGRAPH_H

#include <stdlib.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef struct BmpInfo {
    char name[50];
    u16 file_size;
}BmpInfo;

// node 
typedef struct ClusterNode {
    u32 cluster_id;
    u8  type;
    // char name[25];
    BmpInfo bmp_info;
}ClusterNode;


// edge
typedef struct ClusterEdge {
    u32 adj_id;
    double probability;
    struct ClusterEdge *next;
}ClusterEdge;


// graph
typedef struct ClusterGraph {
    int cluster_num;
    ClusterNode *clusters;
    ClusterEdge **adj_list;
}ClusterGraph;



#endif