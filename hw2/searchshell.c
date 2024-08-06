/*
 * Copyright ©2024 Hannah C. Tang.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2024 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

// Feature test macro for strtok_r (c.f., Linux Programming Interface p. 63)
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "libhw1/CSE333.h"
#include "./CrawlFileTree.h"
#include "./DocTable.h"
#include "./MemIndex.h"

//////////////////////////////////////////////////////////////////////////////
// Helper function declarations, constants, etc
static void Usage(void);
static void PrintResults(LinkedList *ll, DocTable *dt);

//////////////////////////////////////////////////////////////////////////////
// Main
int main(int argc, char** argv) {
  if (argc != 2) {
    Usage();
  }

  // Implement searchshell!  We're giving you very few hints
  // on how to do it, so you'll need to figure out an appropriate
  // decomposition into functions as well as implementing the
  // functions.  There are several major tasks you need to build:
  //
  //  - Crawl from a directory provided by argv[1] to produce and index
  //  - Prompt the user for a query and read the query from stdin, in a loop
  //  - Split a query into words (check out strtok_r)
  //  - Process a query against the index and print out the results
  //
  // When searchshell detects end-of-file on stdin (cntrl-D from the
  // keyboard), searchshell should free all dynamically allocated
  // memory and any other allocated resources and then exit.
  //
  // Note that you should make sure the fomatting of your
  // searchshell output exactly matches our solution binaries
  // to get full points on this part.
  DocTable *dt;
  MemIndex *index;

  printf("Indexing \'%s\'\n", argv[1]);

  if (CrawlFileTree(argv[1], &dt, &index) == 0) {
    fprintf(stderr, "Path \'%s\' is not indexable\n", argv[1]);
    Usage();
  }

  while (1) {
    char *buffer = NULL;
    size_t size = 0;
    printf("enter query:\n");
    getline(&buffer, &size, stdin);

    if (!feof(stdin)) {
      // replace newline character will null character
      char *nl = strrchr(buffer, '\n');
      if (nl != NULL) {
        *nl = '\0';
      }

      // turn query into array of words
      int i;
      char *query[255];  // solution_binaries queries up to 255 words
      char *rest = buffer;
      for (i = 0; (query[i] = strtok_r(rest, " ", &rest)) && i < 256; i++) {
        // set query words to lowercase
        int j;
        char *lower = (char *) malloc(sizeof(char) * (strlen(query[i]) + 1));
        for (j = 0; j < strlen(query[i]) + 1; j++) {
          lower[j] = tolower(query[i][j]);
        }
        query[i] = lower;
      }
      // feed query into MemIndex
      LinkedList* results = MemIndex_Search(index, query, i);

      // print results and frees list
      if (results != NULL) {
        PrintResults(results, dt);
        LinkedList_Free(results, (LLPayloadFreeFnPtr)free);
      }

      // free search memory
      for (int j = 0; j < i; j++) {
        free(query[j]);
      }
      free(buffer);

    // EOF
    } else {
      printf("shutting down...\n");
      free(buffer);
      break;
    }
  }
  DocTable_Free(dt);
  MemIndex_Free(index);

  return EXIT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////
// Helper function definitions

static void Usage(void) {
  fprintf(stderr, "Usage: ./searchshell <docroot>\n");
  fprintf(stderr,
          "where <docroot> is an absolute or relative " \
          "path to a directory to build an index under.\n");
  exit(EXIT_FAILURE);
}

static void PrintResults(LinkedList *ll, DocTable *dt) {
  LLIterator *it = LLIterator_Allocate(ll);
  while (LLIterator_IsValid(it)) {
    SearchResult *result;
    char *name;

    LLIterator_Get(it, (LLPayload_t*) &result);
    if ((name = DocTable_GetDocName(dt, result->doc_id))) {
      printf("  %s ", name);
      printf("(%d)\n", result->rank);
    }
    LLIterator_Next(it);
  }
  LLIterator_Free(it);
}
