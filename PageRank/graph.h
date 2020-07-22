#ifndef GRAPH_H
#define GRAPH_H

typedef struct _graph *graph;
typedef struct _vertex *vertex;
typedef struct _edgeList *edgeList;

struct _graph {
    vertex *vertexes;
    int numVertexes;
};

struct _vertex {
    char *id;
    int num;
    edgeList edges;
    int numEdges;
};

struct _edgeList {
    char *dest;
    int destNum;
    double weight;
    edgeList next;
};

graph newGraph(int size);
vertex newVertex(char *id);
edgeList newEdge(char *dest);

int vertexInGraph(graph g, char *id);
vertex getVertex(graph g, char *id);
int getVertexNum(graph g, char *id);

int isConnection(graph g, char *src, char *dest);

void addVertex(graph g, char *id);
void addEdge(edgeList e, char *dest);
void addVertexEdge(vertex v, char *dest);
void addConnection(graph g, char *src, char *dest);

void listEdges(edgeList e);
void printGraph(graph g);

void freeGraph(graph g);

#endif