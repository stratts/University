#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "text.h"
#include "search.h"

double getTfIdfSum(stringNode url);
double calculateTf(char *string, char *term);
double calculateIdf(char *collection, char *invertedIndex, char *term);

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("ERROR: No search terms given\n");
        exit(1);
    }

    stringList searchTerms = parseSearchTerms(argc, argv);
    stringList urls = getMatchingUrls(searchTerms, "invertedIndex.txt");

    // Calculate idf for every term given
    for (stringNode n = searchTerms->start; n != NULL; n = n->next) {
        n->key = calculateIdf("collection.txt", "invertedIndex.txt", n->string);
    }  

    // Calculate sum of tf-idf for each URL
    for (stringNode n = urls->start; n != NULL; n = n->next) {
        char *url = n->string;
        char *filename = stringJoin(url, ".txt");
        char *sectionText = readSection(filename, "Section-2");

        // Calculate tf-idf for every term found at the URL
        for (stringNode term = n->list->start; term != NULL; term = term->next) {
            double tf = calculateTf(sectionText, term->string);
            double idf;

            stringNode idfNode = getNode(searchTerms, term->string);

            if (idfNode != NULL) idf = idfNode->key;
            else idf = 0;

            term->key = tf * idf;
            n->key += tf * idf;
        }

        free(filename);
        free(sectionText);
    }

    stringList sorted = sortStringList(urls);

    int numOutput = 0;
    for (stringNode n = sorted->start; n != NULL; n = n->next) {
        printf("%s %lf\n", n->string, getTfIdfSum(getNode(urls, n->string)));
        numOutput++;
        if (numOutput == 30) break;
    }
    
    freeStringList(searchTerms);
    freeStringList(urls);
    freeStringList(sorted);

    return 0;
}

// Returns the sum of the tf-idf values for a URL
double getTfIdfSum(stringNode url) {
    double sum = 0;
    for (stringNode n = url->list->start; n != NULL; n = n->next) {
        sum += n->key;
    } 
    return sum;
}

// Calculates the term frequency of a term within a string
double calculateTf(char *string, char *term) {
    stringList words = readWords(string);

    double count = 0;
    double total = 0;

    for (stringNode n = words->start; n != NULL; n = n->next) {
        char *word = cleanString(n->string);

        if (strcmp(term, word) == 0) count++;
        total++;

        free(word);
    }

    freeStringList(words);

    return count / total;
}

// Calculates the inverse document frequency for a term
double calculateIdf(char *collection, char *invertedIndex, char *term) {
    stringList terms = newStringList();
    appendToStringList(terms, term);
    stringList all = readCollection(collection);
    stringList matches = getMatchingUrls(terms, invertedIndex);

    double numAll = stringListLength(all);
    double numMatches = stringListLength(matches);

    freeStringList(terms);
    freeStringList(all);
    freeStringList(matches);

    if (numMatches == 0) return 0;
    return log(numAll / numMatches);    
}
