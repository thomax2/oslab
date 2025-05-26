#ifndef __LGRAPH_H
#define __LGRAPH_H

#include <stdlib.h>

#define MaxVertexNum 100
typedef int Vertex;
typedef int WeightType;
typedef char DataType;

//边定义
typedef struct ENode *PtrToENode;
struct ENode
{
    Vertex V1, V2;     //有向边<v1,v2>
    WeightType Weight; //权重
};
typedef PtrToENode Edge;

//邻接点定义
typedef struct AdjVNode *PtrToAdjVNode;
struct AdjVNode
{
    Vertex AdjV;        //邻接点下标
    WeightType Weight;  //边权重
    PtrToAdjVNode Next; //下个节点位置
};

//顶点表头节点
typedef struct Vnode
{
    PtrToAdjVNode FirstEdge;
    DataType Data;
} AdjList[MaxVertexNum];

//图定义
typedef struct GNode *PtrToGNode;
struct GNode
{
    int Nv;    //顶点数
    int Ne;    //边数
    AdjList G; //邻接表
};
typedef PtrToGNode LGraph;




#endif