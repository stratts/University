#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "text.h"

void writeWordsBST(stringBST words, FILE* file);

int main(void) {
    stringList urls = readCollection("collection.txt");
    stringBST words = NULL;

    // Iterate through each URL in collection
    for (stringNode curr = urls->start; curr != NULL; curr = curr->next) {
        char *url = curr->string;
        char *filename = stringJoin(url, ".txt");

        // Read all words from section into a string list
        char *sectionText = readSection(filename, "Section-2");
        stringList urlWords = readWords(sectionText);

        // Iterate through each word
        for (stringNode currW = urlWords->start; currW != NULL; currW = currW->next) {
            char *word = cleanString(currW->string);
            
            if (words == NULL) words = newStringBST(word);

            // Insert word into BST
            if (getKeyBST(words, word) == NULL) {
                insertKeyBST(words, word);
            }

            // Add URL to list of URLs containing word
            stringBST currNode = getKeyBST(words, word);

            if (!inStringList(currNode->list, url)) {
                insertSorted(currNode->list, url);
            }

            free(word);
        }

        free(filename);
        free(sectionText);
        freeStringList(urlWords);
    }

    FILE* output = fopen("invertedIndex.txt", "w");

    writeWordsBST(words, output);

    freeStringList(urls);
    freeStringBST(words);

    return 0;
}

// Traverse tree in-order and write each word and URLs to file
void writeWordsBST(stringBST words, FILE* file) {
    if (words == NULL) return;
    writeWordsBST(words->left, file);

    fprintf(file, "%s ", words->key);

    for (stringNode currU = words->list->start; currU != NULL; currU = currU->next) {
        fprintf(file, " %s", currU->string);
    }

    fprintf(file, "\n");

    writeWordsBST(words->right, file);
}
