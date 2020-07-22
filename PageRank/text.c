#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "text.h"

// Allocates and returns a new string list
stringList newStringList() {
    stringList list = malloc(sizeof(struct _stringList));
    list->start = NULL;
    list->end = NULL;
    return list;
}

stringNode newStringNode(char *contents) {
    stringNode node = malloc(sizeof(struct _stringNode));
    node->string = calloc(strlen(contents) + 1, sizeof(char));
    strcpy(node->string, contents);
    node->key = -1;
    node->list = newStringList();
    node->next = NULL;
    return node;
}

// Appends a new string to a string list
void appendToStringList(stringList l, char *contents) {
    stringNode new = newStringNode(contents);

    if (l->start == NULL) {
        l->start = new;
        l->end = new;
    } else {
        l->end->next = new;
        l->end = new;
    }
}

// Checks whether a string list contains a string
int inStringList(stringList l, char *string) {
    return (getNode(l, string) != NULL);
}

// Returns the node in a string list with the given value
stringNode getNode(stringList l, char* string) {
    
    for (stringNode curr = l->start; curr != NULL; curr = curr->next) {
        if (strcmp(curr->string, string) == 0) return curr;
    }
    return NULL;
}

// Inserts a node into a string list in sorted position
void insertSortedByKey(stringList l, char *contents, double key) {
    stringNode dest = NULL;
    stringNode new = newStringNode(contents);
    new->key = key;

    for (stringNode curr = l->start; curr != NULL; curr = curr->next) {
        // If node has no key, sort by contents of string
        if (key == NO_KEY || curr->key == key) {
            if (stringsSorted(curr->string, contents) &&
                (curr->next == NULL || !stringsSorted(curr->next->string, contents))) {
                dest = curr;
                break;
            }
        } else {
            if (curr->key >= key && (curr->next == NULL || curr->next->key <= key)) {
                dest = curr;
                break;
            }
        }
    }

    if (dest == NULL) {
        // Insert at front
        new->next = l->start;
        l->start = new;
    } else {
        // Insert after dest
        new->next = dest->next;
        dest->next = new;
    }

    if (l->end == dest) l->end = new;
}

void insertSorted(stringList l, char *contents) {
    insertSortedByKey(l, contents, NO_KEY);
}

// Returns a sorted copy of a string list
stringList sortStringList(stringList l) {
    stringList sorted = newStringList();

    for (stringNode n = l->start; n != NULL; n = n->next) {
        insertSortedByKey(sorted, n->string, n->key);
    }

    return sorted;
}

// Reads each word from a string into a string list
stringList readWords(char* string) {
    return splitString(string, " ");
}

// Splits string at delimiters into string list
stringList splitString(char *string, char *delimiters) {
    stringList head = newStringList();

    int readingWord = 0;
    int readLength = 0;

    char buffer[BUFFER_SIZE];

    // Read every character in sting
    for (int i = 0; string[i] != '\0' || readingWord; i++) {
        int c = string[i];
        int isDelimiter = 0;

        // Check whether current character is delimiter
        for (int j = 0; delimiters[j] != '\0'; j++) {
            if (delimiters[j] == c) {
                isDelimiter = 1;
                break;
            }
        }

        // If character is delimiter, and last character was a regular charatcer,
        // append current buffer to string list
        if (isDelimiter || c == '\n' || c == '\0') {
            if(readingWord) {
                buffer[readLength] = '\0';
                readingWord = 0;
                readLength = 0;
                appendToStringList(head, buffer);
            }           
        } else {
        // Otherwise, read character into buffer
            buffer[readLength] = c;
            readLength++;
            readingWord = 1;
        }

        if (c == '\0') break;
    }
    
    return head;
}

// Returns the length of a string list
int stringListLength(stringList l) {
    int count = 0;
    for (stringNode curr = l->start; curr != NULL; curr = curr->next) {
        count++;
    }
    return count;
}

// Prints each string in a string list on a single line
void printStringList(stringList l) {
    for (stringNode curr = l->start; curr != NULL; curr = curr->next) {
        printf("%s\n", curr->string);
    }
}

// Prints a two-dimensional string list 
void printStringList2D(stringList l) {
    for (stringNode curr = l->start; curr != NULL; curr = curr->next) {
        printf("%s: ", curr->string);
        for (stringNode curr2 = curr->list->start; curr2 != NULL; curr2 = curr2->next) {
            printf("%s ", curr2->string);
        }
        printf("\n");
    }
}

// Frees the memory occupied by a string list
void freeStringList(stringList l) {
    if (l == NULL) return;

    stringNode n = l->start;
    while (n != NULL) {
        freeStringList(n->list);
        free(n->string);
        stringNode temp = n->next;
        free(n);
        n = temp;
    }

    free(l);
}

// Reads all of the URLs in a collection file
stringList readCollection(char *filename) {
    FILE* f = fopen(filename, "r");

    if (f == NULL) {
        printf("ERROR: Could not read collection from file '%s'\n", filename);
        exit(1);
    }
    
    // Find size of file and rewind
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    // Allocate buffer and read entire file
    char *buffer = calloc(size + 1, sizeof(char));
    fread(buffer, size, 1, f);
    fclose(f);

    // Parse words from buffer
    stringList urls = readWords(buffer);
    free(buffer);
    return urls;
}

// Reads a section from a file and returns its contents
char *readSection(char *filename, char *section) {
    FILE* f = fopen(filename, "r");

    if (f == NULL) {
        printf("ERROR: Could not read section '%s' from file '%s'\n", section, filename);
        exit(1);
    }

    char buffer[MAX_LINE];
    int pos = 0;
    int start = -1;

    char *startTag = stringJoin("#start ", section);
    char *endTag = stringJoin("#end ", section);

    while(fgets(buffer, MAX_LINE, f)) { 
        if (stringStartsWith(buffer, startTag)) start = strlen(buffer) + pos;
        else if (stringStartsWith(buffer, endTag)) break;

        pos += strlen(buffer);     
    }

    int length = pos - start;

    free(startTag);
    free(endTag);

    fseek(f, start, SEEK_SET);
    char *sectionText = calloc(length + 1, sizeof(char));
    fread(sectionText, 1, length, f);

    return sectionText;
}

// Returns a new binary search tree node with the given key
stringBST newStringBST(char *key) {
    stringBST new = malloc(sizeof(struct _stringBST));
    new->left = NULL;
    new->right = NULL;
    new->list = newStringList();
    new->key = calloc(strlen(key) + 1, sizeof(char));
    strcpy(new->key, key);
    return new;
}

// Returns the BST node with the given key
stringBST getKeyBST(stringBST tree, char *key) {
    if (tree == NULL) return NULL;
    if (strcmp(tree->key, key) == 0) return tree;
    
    if (stringsSorted(key, tree->key)) return getKeyBST(tree->left, key);
    else return getKeyBST(tree->right, key);
}

// Inserts a node into a tree with the given key
void insertKeyBST(stringBST tree, char *key) {
    if (stringsSorted(key, tree->key)) {      
        if (tree->left == NULL) tree->left = newStringBST(key);
        else insertKeyBST(tree->left, key);
    }
    else {
        if (tree->right == NULL) tree->right = newStringBST(key);
        else insertKeyBST(tree->right, key);
    }
}

// Prints all nodes of a BST in-order
void printBST(stringBST tree) {
    if (tree == NULL) return;
    printBST(tree->left);
    printf("%s\n", tree->key);
    printBST(tree->right);
}

// Frees the memory associated with a BST
void freeStringBST(stringBST tree) {
    if (tree == NULL) return;
    freeStringBST(tree->left);
    freeStringBST(tree->right);
    freeStringList(tree->list);
    free(tree->key);
    free(tree);
}

// Checks whether string1 is alphabetically above string2
int stringsSorted(char *string1, char *string2) {
    int i = 0;
    while (string1[i] != '\0' && string2[i] != '\0') {
        char c1 = tolower(string1[i]);
        char c2 = tolower(string2[i]);
        if (c1 > c2) return 0; 
        else if (c1 < c2) return 1;
        i++;
    }
    return 1;
}

// Checks whether 'string' starts with 'match'
int stringStartsWith(char *string, char *match) {
    int i = 0;
    
    while(string[i] != '\0' && match[i] != '\0') {
        if (string[i] != match[i]) return 0;
        i++;
    }

    if (string[i] == '\0' && match[i] != '\0') return 0;
    return 1;
}

// Joins two strings together and returns the result
char *stringJoin(char *string1, char *string2) {
    char *new = calloc(strlen(string1) + strlen(string2) + 1, sizeof(char));
    sprintf(new, "%s%s", string1, string2);
    return new;
}

// Converts string to lowercase and removes special characters
char *cleanString(char *string) {
    int len = strlen(string);
    char *new = calloc(len + 1, sizeof(char));
    int pos = 0;
    for (int i = 0; i < len; i++) {
        if (isalnum(string[i])) {
            new[pos] = tolower(string[i]);
            pos++;
        }
    }
    return new;
}
