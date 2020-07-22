#ifndef SEARCH_H
#define SEARCH_H

#include "text.h"

stringList parseSearchTerms(int argc, char *argv[]);
stringList getMatchingUrls(stringList searchTerms, char* invertedIndex);

#endif