#ifndef TEXT_H
#define TEXT_H

#define BUFFER_SIZE 256
#define MAX_LINE 1024

#define NO_KEY -1

typedef struct _stringList *stringList;
typedef struct _stringNode *stringNode;
typedef struct _stringBST *stringBST;

struct _stringBST {
    char *key;
    stringBST left;
    stringBST right;
    stringList list;
};

struct _stringNode {
    char *string;
    double key;
    stringList list;
    stringNode next;
};

struct _stringList {
    stringNode start;
    stringNode end;
};

stringList newStringList();
stringNode newStringNode(char *contents);

int inStringList(stringList l, char *string);
stringNode getNode(stringList l, char *string);

void appendToStringList(stringList l, char *contents);
void insertSorted(stringList l, char *contents);
void insertSortedByKey(stringList l, char *contents, double key);
stringList sortStringList(stringList l);

int stringListLength(stringList l);
void printStringList(stringList l);
void printStringList2D(stringList l);

void freeStringList(stringList l);

stringBST newStringBST();
stringBST getKeyBST(stringBST tree, char *key);
void insertKeyBST(stringBST tree, char *key);
void printBST(stringBST tree);
void freeStringBST(stringBST tree);

stringList readWords(char* string);
stringList splitString(char *string, char *delimiters);
stringList readCollection(char *filename);
char *readSection(char *filename, char *section);

int stringsSorted(char *string1, char *string2);
int stringStartsWith(char *string, char *match);
char *stringJoin(char *string1, char *string2);
char *cleanString(char *string);


#endif