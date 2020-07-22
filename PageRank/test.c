#include <assert.h>
#include <stdio.h>

#include "text.h"
#include "graph.h"

#include "string.h"

void testCleanString();
void testStringSort();
void testInsertSorted();
void testStringOps();
void testGraph();
void testBST();

int main(void) {
    testCleanString();
    testStringSort();
    testInsertSorted();
    //testStringOps();
    testGraph();
    testBST();
    return 0;
}

void testCleanString() {
    assert(strcmp(cleanString("Test123"), "test123") == 0);
    assert(strcmp(cleanString("TEST123"), "test123") == 0);
    assert(strcmp(cleanString("TEST'123'"), "test123") == 0);
    assert(strcmp(cleanString(" TEST'123' "), "test123") == 0);
    assert(strcmp(cleanString(" TEST'123'. "), "test123") == 0);
}

void testStringSort() {
    assert(stringsSorted("a", "b") == 1);
    assert(stringsSorted("b", "a") == 0);
    assert(stringsSorted("abc", "abc") == 1);
    assert(stringsSorted("Abc", "abc") == 1);
    assert(stringsSorted("url12", "url13") == 1);
    assert(stringsSorted("url22", "url19") == 0);
    assert(stringsSorted("mars", "jupiter") == 0);
    assert(stringsSorted("jupiter", "mars") == 1);
    assert(stringsSorted("mars", "mers") == 1);
}

void testInsertSorted() {
    stringList l = newStringList();
    appendToStringList(l, "a");
    appendToStringList(l, "c");
    appendToStringList(l, "f");
    appendToStringList(l, "b");
    /*printStringList(l);
    l = newStringList();
    insertSortedByKey(l, "b", 5.5);
    insertSortedByKey(l, "c", 3.3);
    insertSortedByKey(l, "f", 1.1);
    insertSortedByKey(l, "a", 5.3);
    printStringList(l);*/

    //printStringList(sortStringList(l));
}

void testStringOps() {

}

void testGraph() {
    graph test = newGraph(100);
    addVertex(test, "1");
    addVertex(test, "2");
    addConnection(test, "1", "2");
    //printGraph(test);
}

void testBST() {
    stringBST test = newStringBST("b");  
    insertKeyBST(test, "a");
    assert(test->left != NULL);
    assert(strcmp(test->left->key, "a") == 0);  
    assert(getKeyBST(test, "a") != NULL);
    assert(strcmp(getKeyBST(test, "a")->key, "a") == 0);
    
    insertKeyBST(test, "e");
    insertKeyBST(test, "c");

    printBST(test);
}