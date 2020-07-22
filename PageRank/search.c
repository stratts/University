#include <stdio.h>
#include <stdlib.h>

#include "search.h"

// Returns a string list containing the search terms given in the arguments
stringList parseSearchTerms(int argc, char *argv[]) {
    stringList searchTerms = newStringList();

    for (int i = 1; i < argc; i++) {
        char *cleaned = cleanString(argv[i]);
        appendToStringList(searchTerms, cleaned);
        free(cleaned);
    }

    return searchTerms;
}

// Using an inverted index file, returns the URLs that contain
// one or more search terms
stringList getMatchingUrls(stringList searchTerms, char* invertedIndex) { 
    FILE* index = fopen(invertedIndex, "r");

    if (index == NULL) {
        printf("ERROR: Could not open inverted index file '%s'\n", invertedIndex);
        exit(1);
    }

    char buffer[1024];
    stringList urls = newStringList();

    while (fgets(buffer, MAX_LINE, index)) {
        stringList line = readWords(buffer);

        // Check that line contains a term, and that it's one of the search terms
        if (line->start != NULL && inStringList(searchTerms, line->start->string)) {
            // Add all URLs on same line to list of URLs
            for (stringNode n = line->start->next; n != NULL; n = n->next) {
                if (!inStringList(urls, n->string)) {
                    appendToStringList(urls, n->string);
                }    

                // Increase URL key when term found
                stringNode current = getNode(urls, n->string);
                appendToStringList(current->list, line->start->string);
                if (current->key == NO_KEY) current->key = 1;
                else current->key++;
            }
        }

        freeStringList(line);
    }

    fclose(index);

    return urls;
}