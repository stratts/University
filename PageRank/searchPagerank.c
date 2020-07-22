#include <stdio.h>
#include <stdlib.h>

#include "text.h"
#include "search.h"

#define MAX_LINE 1024

void addPagerank(stringList urls, char *pagerankList);

int main(int argc, char* argv[]) {

    if (argc == 1) {
        printf("ERROR: No search terms given\n");
        exit(1);
    }
    
    // Find all URLs that match search terms
    stringList searchTerms = parseSearchTerms(argc, argv);
    stringList urls = getMatchingUrls(searchTerms, "invertedIndex.txt");

    // Add corresponding pagerank to each URL key
    addPagerank(urls, "pagerankList.txt");
    stringList sorted = sortStringList(urls);

    // Print out matching URLs, sorted by number of terms found and pagerank
    int numOutput = 0;
    for (stringNode n = sorted->start; n != NULL; n = n->next) {
        printf("%s\n", n->string);
        numOutput++;
        if (numOutput == 30) break;
    }

    freeStringList(searchTerms);
    freeStringList(urls);
    freeStringList(sorted);

    return 0;
}

void addPagerank(stringList urls, char *pagerankList) {
    char buffer[1024];
    FILE* rank = fopen(pagerankList, "r");

    if (rank == NULL) {
        printf("ERROR: Could not open pagerank list '%s'", pagerankList);
        exit(1);
    }

    while (fgets(buffer, MAX_LINE, rank)) {
        // Split line into string list
        stringList line = splitString(buffer, ", ");
        
        if (line->start != NULL) {
            // First word in string list = url name
            char *url = line->start->string;
            stringNode urlNode = getNode(urls, url);

            // Last word in string list = pagerank value
            if (urlNode != NULL) {
                double prValue = atof(line->end->string);
                urlNode->key += prValue;
            }
        }

        freeStringList(line);
    }

    fclose(rank);
}
