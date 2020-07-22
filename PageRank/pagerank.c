#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "graph.h"
#include "text.h"

graph buildInitialGraph();
double getWIn(graph g, int v, int u);
double getWOut(graph g, int v, int u);
int getNumIn(graph g, int id);
int getNumOut(graph g, int id);
void pageRankW(graph g, double d, double diffPR, int maxIterations);

int main(int argc, char *argv[]) { 
    graph g = buildInitialGraph();
    if (argc < 4) {
        printf("ERROR: Not enough arguments\n");
        printf("pagerank <damping> <diffPR> <maxIterations>\n");
        exit(1);
    }

    double damping = atof(argv[1]);
    double diffPR = atof(argv[2]);
    int maxIterations = atoi(argv[3]);

    pageRankW(g, damping, diffPR, maxIterations);
    freeGraph(g);
    return 0;
}

// Calculate and print the pagerank for each URL in a graph
void pageRankW(graph g, double d, double diffPR, int maxIterations) {
    int num = g->numVertexes;
    double prevPR[num];
    double currPR[num];

    // Set initial pagerank for each page
    for (int i = 0; i < num; i++) {
        prevPR[i] = 1.0 / num;
        currPR[i] = 0;
    } 

    int i = 0;
    double diff = diffPR;

    while (i < maxIterations && diff >= diffPR) {
        // For each page, transfer previous pagerank to connected pages
        for (int j = 0; j < num; j++) {
            vertex v = g->vertexes[j];
            for (edgeList e = v->edges; e != NULL; e = e->next) {
                int destID = getVertexNum(g, e->dest);
                currPR[destID] += prevPR[j] * getWIn(g, j, destID) * getWOut(g, j, destID);
            }
        }

        // Apply rest of formula
        for (int j = 0; j < num; j++) {
            currPR[j] = (currPR[j] * d) + ((1.0 - d) / num);
        }

        // Calculate diff for each page and sum 
        diff = 0;
        for (int j = 0; j < num; j++) {
            diff += fabs(currPR[j] - prevPR[j]);
        }

        // Store pagerank values calculated this iteration
        for (int j = 0; j < num; j++) {
            prevPR[j] = currPR[j];
            currPR[j] = 0;
        }

        i++;
    }

    // Sort pages by their final pagerank value
    stringList results = newStringList();

    for (int i = 0; i < num; i++) {
        insertSortedByKey(results, g->vertexes[i]->id, prevPR[i]);
    }

    FILE *output = fopen("pagerankList.txt", "w");

    // Print page name, outlinks, and pagerank value
    for (stringNode n = results->start; n != NULL; n = n->next) {
        int id =  getVertexNum(g, n->string);
        fprintf(output, "%s, %d, %.7f\n", n->string, getNumOut(g, id), prevPR[id]);
    }

    freeStringList(results);

    fclose(output);
}

double getWIn(graph g, int v, int u) {
    double refIn = 0;
    vertex vert = g->vertexes[v];
    for (edgeList e = vert->edges; e != NULL; e = e->next) {
        refIn += getNumIn(g, getVertexNum(g, e->dest));
    }
    return getNumIn(g, u) / refIn;
}

double getWOut(graph g, int v, int u) {
    double refOut = 0;
    vertex vert = g->vertexes[v];
    for (edgeList e = vert->edges; e != NULL; e = e->next) {
        double numOut = getNumOut(g, getVertexNum(g, e->dest));
        if (numOut == 0) numOut = 0.5;
        refOut += numOut;
    }
    double numOut = getNumOut(g, u);
    if (numOut == 0) numOut = 0.5;
    return numOut / refOut;
}

// Returns the number of pages that point to a page
int getNumIn(graph g, int id) {
    int count = 0;
    for (int i = 0; i <  g->numVertexes; i++) {
        vertex v = g->vertexes[i];
        for (edgeList e = v->edges; e != NULL; e = e->next) {
            if (getVertexNum(g, e->dest) == id) count ++;
        }
    }

    return count;
}

// Returns the number of pages a page points to
int getNumOut(graph g, int id) {
    return g->vertexes[id]->numEdges;
}

// Builds an initial graph with connections from a collection file
graph buildInitialGraph() {
    stringList urls = readCollection("collection.txt");
    graph linkGraph = newGraph(stringListLength(urls));

    // Populate initial vertexes
    for (stringNode curr = urls->start; curr != NULL; curr = curr->next) {
        if (!vertexInGraph(linkGraph, curr->string)) {
            addVertex(linkGraph, curr->string);
        }
    }

    // Iterate through each URLs
    for (stringNode curr = urls->start; curr != NULL; curr = curr->next) {
        char *url = curr->string;
        char *filename = stringJoin(url, ".txt");
        
        // Read links from page text
        char *linksText = readSection(filename, "Section-1");
        stringList links = splitString(linksText, " ");

        // Add connection from URL to linked page
        for (stringNode n = links->start; n != NULL; n = n->next) {
            if (strcmp(url, n->string) != 0 && !isConnection(linkGraph,  url, n->string)) {
                addConnection(linkGraph, url, n->string);
            }
        }
        
        free(filename);
        free(linksText);
        
        freeStringList(links);
    }

    freeStringList(urls);

    return linkGraph;
}
