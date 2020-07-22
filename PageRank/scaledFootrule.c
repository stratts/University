#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "text.h"

#define NO_RANK 0

typedef struct _unionList *unionList;
typedef struct _unionItem *unionItem;

struct _unionList {
    int length;
    unionItem *items;
};

struct _unionItem {
    char *name;
    int numRanks;
    int *ranks;
    int *rankLen;
    double *footrule;
    double *score;
};

struct result {
    double footrule;
    int *ranking;
};

unionList createUnion(int numRanks, stringList ranks[]);

double scaledFootruleItem(unionItem item, int newPos, int newLength);
double scaledFootrule(unionList list, int newPos[]);
void calculateUnionFootrule(unionList list);
void calculateScore(unionList list);

struct result getBestScaledFootrule(unionList list);
void permutePotentials(int i, int length, int** potentials, int* potentialCount, 
    int *current, int *minimum, int *taken, double *minFootrule, unionList list);

int getNodePos(stringList l, char *string);
void freeUnion(unionList list);
void freeUnionItem(unionItem item);

int main(int argc, char *argv[]) {

    int numRanks = argc - 1;

    if (numRanks == 0) {
        printf("ERROR: No rank files given\n");
        exit(1);
    }

    // Read each ranked list into a string list
    stringList *ranks = calloc(numRanks, sizeof(stringList));
    for (int i = 1; i < argc; i++) ranks[i - 1] = readCollection(argv[i]);

    // Create union from lists, and calculate best scaled footrule
    unionList list = createUnion(numRanks, ranks);
    calculateUnionFootrule(list);
    calculateScore(list);
    struct result results = getBestScaledFootrule(list);

    printf("%.6f\n", results.footrule);

    for (int i = 0; i < list->length; i++) {
        int index = results.ranking[i] - 1;
        printf("%s\n", list->items[index]->name);
    }  

    // Free memory 
    for (int i = 0; i < numRanks; i++) freeStringList(ranks[i]);
    free(ranks);
    free(results.ranking);
    freeUnion(list);
    
    return 0;
}

// Creates a union between two ranked lists
unionList createUnion(int numRanks, stringList ranks[]) {
    stringList unionItems = newStringList();

    int numItems = 0;

    // Combine all items into single string list
    for (int i = 0; i < numRanks; i++) { 
        for (stringNode n = ranks[i]->start; n != NULL; n = n->next) {
            if (!inStringList(unionItems, n->string)) {
                appendToStringList(unionItems, n->string);
                numItems++;
            }
        }
    }

    unionList list = malloc(sizeof(struct _unionList));
    list->items = calloc(numItems, sizeof(struct _unionItem));
    list->length = 0;

    // Convert each string list item into a union item
    for (stringNode n = unionItems->start; n != NULL; n = n->next) {
        unionItem item = malloc(sizeof(struct _unionItem));
        item->name = calloc(strlen(n->string) + 1, sizeof(char));
        strcpy(item->name, n->string); 

        item->numRanks = numRanks;
        item->ranks = calloc(numRanks, sizeof(int));
        item->rankLen = calloc(numRanks, sizeof(int));
        item->footrule = calloc(numItems, sizeof(double));
        item->score = calloc(numItems, sizeof(double));

        // Save position in and size of original ranked lists
        for (int i = 0; i < numRanks; i++) {
            item->ranks[i] = getNodePos(ranks[i], item->name);
            item->rankLen[i] = stringListLength(ranks[i]);
        }

        list->items[list->length] = item;
        list->length++;
    }

    freeStringList(unionItems);

    return list;
}

// Calculates the scaled footrule of a union item, given the new position and length
double scaledFootruleItem(unionItem item, int newPos, int newLength) {
    double footrule = 0;
    double new = (double)newPos / (double)newLength;

    for (int i = 0; i < item->numRanks; i++) {
        if (item->ranks[i] == NO_RANK) continue;

        double old = (double)item->ranks[i] / (double)item->rankLen[i];
        footrule += fabs(new - old);
    }

    return footrule;
}

// Calculates the total scaled footrule of a new rank
double scaledFootrule(unionList list, int newPos[]) {
    double footrule = 0;

    for (int i = 0; i < list->length; i++) {
        footrule += scaledFootruleItem(list->items[i], newPos[i], list->length);
    }

    return footrule;
}

// Calculates the footrule for every item in a union, at every position
void calculateUnionFootrule(unionList list) {
    for (int i = 0; i < list->length; i++) {
        unionItem item = list->items[i];
       
        for (int j = 0; j < list->length; j++) {
            item->footrule[j] = scaledFootruleItem(item, j + 1, list->length);
        }
    }
}

// Calculates the score for a every item at every position in a union,
// which is the sum of the 'item score' and the 'position score'
void calculateScore(unionList list) {
    // Item score
    // Difference at each position from average footrule for item
    for (int i = 0; i < list->length; i++) {
        unionItem item = list->items[i];

        double total = 0;

        for (int j = 0; j < list->length; j++) {
            total += item->footrule[j];
        }

        for (int j = 0; j < list->length; j++) {
            item->score[j] = item->footrule[j] - (total / list->length);
        }
    } 
    
    // Position score
    // Difference at each position from average footrule for position
    for (int i = 0; i < list->length; i++) {
        double total = 0;

        for (int j = 0; j < list->length; j++) {
            unionItem item = list->items[j];
            total += item->footrule[i];
        }

        for (int j = 0; j < list->length; j++) {
            unionItem item = list->items[j];
            item->score[i] += item->footrule[i] - (total / list->length);
        }
    }
}

// Returns the best rank for a union
struct result getBestScaledFootrule(unionList list) {
    int size = list->length;
    int **potentials = calloc(size, sizeof(int*));
    int *potentialCount = calloc(size, sizeof(int));

    // Allocate list of potentials for each position
    for (int i = 0; i < size; i++) {
        potentials[i] = calloc(size, sizeof(int));
    } 

    // Get potential items for each position
    for (int r = 0; r < list->length; r++) {
        int count = 0;
        for (int i = 0; i < list->length; i++) {
            unionItem item = list->items[i];

            // Only add item if it has a score < 0
            if (item->score[r] <= 0) {        
                potentials[r][count] = i;
                count++;
            }
        }

        potentialCount[r] = count;
    }

    int *taken = calloc(size, sizeof(int));             // Items that are already assignment to a rank
    int *minimum = calloc(list->length, sizeof(int));   // Best rank found so far
    int *current = calloc(list->length, sizeof(int));   // Current rank to check
    double minFootrule = -1;                            // Smallest scaled footrule found

    permutePotentials(0, list->length, potentials, potentialCount, current, minimum, taken, &minFootrule, list);

    struct result results = {
        .ranking = minimum,
        .footrule = minFootrule
    };

    for (int i = 0; i < size; i++) free(potentials[i]);
    free(potentials);
    free(taken);
    free(current);
    free(potentialCount);

    return results;
}

// Given a list of potential items at each position, checks each rank,
// calculates the score, and finds the best rank
void permutePotentials(int i, int length, int** potentials, int* potentialCount, 
    int *current, int *minimum, int *taken, double *minFootrule, unionList list) {
    if (i == length) {
        // Found one possibile rank, so calculate footrule
        double score = scaledFootrule(list, current);

        if (score < *minFootrule || *minFootrule == -1) {
            *minFootrule = score;

            for (int i = 0; i < length; i++) {
                minimum[i] = current[i];     
            }
        }
    }
    else {
        // Continue building new possible rank
        // Iterate through every potential item at position
        for (int p = 0; p < potentialCount[i]; p++) {     
            int num = potentials[i][p];
            // If item is not already assigned to a previous position
            if (!taken[num]) {
                current[i] = potentials[i][p] + 1; 
                taken[num] = 1;
                permutePotentials(i + 1, length, potentials, potentialCount,
                current, minimum, taken, minFootrule, list);
                taken[num] = 0;
            } 
        }
    }
}

int getNodePos(stringList l, char *string) {
    int i = 1;

    for (stringNode n = l->start; n != NULL; n = n->next) {
        if (strcmp(n->string, string) == 0) return i;
        i++;
    }

    return NO_RANK;
}

void freeUnion(unionList list) {
    for (int i = 0; i < list->length; i++) freeUnionItem(list->items[i]);
    free(list->items);
    free(list);
}

void freeUnionItem(unionItem item) {
    free(item->name);
    free(item->footrule);
    free(item->ranks);
    free(item->rankLen);
    free(item->score);
    free(item);
}