#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"

// Initialises and returns a new graph object
graph newGraph(int size) {
    graph g = malloc(sizeof(struct _graph));
    g->vertexes = calloc(size, sizeof(struct _vertex));
    g->numVertexes = 0;
    return g;
}

//Initialises and returns a new vertex object
vertex newVertex(char *id) {
    vertex v = malloc(sizeof(struct _vertex));
    v->id = calloc(strlen(id) + 1, sizeof(char));
    v->num = 0;
    v->numEdges = 0;
    strcpy(v->id, id);
    v->edges = NULL;
    return v;
}

// Initialises and returns a new edge object
edgeList newEdge(char *dest) {
    edgeList new = malloc(sizeof(struct _edgeList));
    new->dest = calloc(strlen(dest) + 1, sizeof(char));
    strcpy(new->dest, dest);
    new->next = NULL;
    return new;
}

int vertexInGraph(graph g, char *id) {
    if (getVertex(g, id) != NULL) return 1;

    return 0;
}

// Returns the edge with the given ID in the graph
vertex getVertex(graph g, char *id) {
    for (int i = 0; i < g->numVertexes; i++) {
        vertex v = g->vertexes[i];
        if (strcmp(v->id, id) == 0) return v;
    }

    return NULL;
}

// Gets the number of the vertex with the given ID
int getVertexNum(graph g, char *id) {
    return getVertex(g, id)->num;
}

// Checks whether there is an edge from src -> dest
int isConnection(graph g, char *src, char *dest) {
    vertex v = getVertex(g, src);

    for (edgeList e = v->edges; e != NULL; e = e->next) {
        if (strcmp(e->dest, dest) == 0) return 1;
    }

    return 0;
}

// Adds a new vertex with the given ID to the graph
void addVertex(graph g, char *id) {
    int i = 0;
    while (i < g->numVertexes) {
        i++;
    }

    g->vertexes[i] = newVertex(id);
    g->vertexes[i]->num = g->numVertexes;
    g->numVertexes++;
}

// Adds an edge to an edge list that points to the given vertex ID
void addEdge(edgeList e, char *dest) {
    edgeList curr = e;
    edgeList last = e;

    while (curr != NULL) {
        if (curr->dest == dest) return;
        last = curr;
        curr = curr->next;
    }

    last->next =newEdge(dest);
}

// Adds an edge to a vertex that points to a given vertex ID 
void addVertexEdge(vertex v, char *dest) {
    if (v->edges == NULL) {
        v->edges = newEdge(dest);
    } else {
        addEdge(v->edges, dest);
    }

    v->numEdges++;
}

// Creates a one way connection between two vertexes in a graph
void addConnection(graph g, char *src, char *dest) {
    vertex v = getVertex(g, src);
    addVertexEdge(v, dest);
}

// Lists the edges contained in an edge list
void listEdges(edgeList e) {
    for (edgeList curr = e; curr != NULL; curr = curr->next) {
        printf("%s\n", curr->dest);
    }
}

// Prints a graph's vertexes and their edges
void printGraph(graph g) {
    for (int i = 0; i < g->numVertexes; i++) {
        vertex v = g->vertexes[i];

        // Print vertex
        printf("%s: {", v->id);

        // Print edges
        edgeList e = v->edges;

        while (e != NULL) {
            printf("%s", e->dest);
            e = e->next;
            if (e != NULL) printf(", ");
        }

        printf("}\n");
    }
}

// Frees the memory associated with a vertex and its edges
void freeVertex(vertex v) {
    edgeList e = v->edges;

    while (e != NULL) {
        free(e->dest);
        edgeList temp = e->next;
        free(e);
        e = temp;
    }

    free(v->id);
    free(v);
}

// Frees the memory occupied by a graph
void freeGraph(graph g) {
    if (g == NULL) return;

    for (int i = 0; i < g->numVertexes; i++) freeVertex(g->vertexes[i]);
    free(g->vertexes);
    free(g);
}

