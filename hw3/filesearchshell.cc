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

#include <cstdlib>     // for EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>    // for std::cout, std::cerr, etc.
#include <algorithm>  // for std::transform

#include "./QueryProcessor.h"

using std::cerr;
using std::endl;
using std::cout;
using std::cin;

// Error usage message for the client to see
// Arguments:
// - prog_name: Name of the program
static void Usage(char* prog_name);

// Prints search results vector in a readable format
static void PrintResults(vector<hw3::QueryProcessor::QueryResult> results);

// Your job is to implement the entire filesearchshell.cc
// functionality. We're essentially giving you a blank screen to work
// with; you need to figure out an appropriate design, to decompose
// the problem into multiple functions or classes if that will help,
// to pick good interfaces to those functions/classes, and to make
// sure that you don't leak any memory.
//
// Here are the requirements for a working solution:
//
// The user must be able to run the program using a command like:
//
//   ./filesearchshell ./foo.idx ./bar/baz.idx /tmp/blah.idx [etc]
//
// i.e., to pass a set of filenames of indices as command line
// arguments. Then, your program needs to implement a loop where
// each loop iteration it:
//
//  (a) prints to the console a prompt telling the user to input the
//      next query.
//
//  (b) reads a white-space separated list of query words from
//      std::cin, converts them to lowercase, and constructs
//      a vector of c++ strings out of them.
//
//  (c) uses QueryProcessor.cc/.h's QueryProcessor class to
//      process the query against the indices and get back a set of
//      query results.  Note that you should instantiate a single
//      QueryProcessor  object for the lifetime of the program, rather
//      than  instantiating a new one for every query.
//
//  (d) print the query results to std::cout in the format shown in
//      the transcript on the hw3 web page.
//
// Also, you're required to quit out of the loop when std::cin
// experiences EOF, which a user passes by pressing "control-D"
// on the console.  As well, users should be able to type in an
// arbitrarily long query -- you shouldn't assume anything about
// a maximum line length.  Finally, when you break out of the
// loop and quit the program, you need to make sure you deallocate
// all dynamically allocated memory.  We will be running valgrind
// on your filesearchshell implementation to verify there are no
// leaks or errors.
//
// You might find the following technique useful, but you aren't
// required to use it if you have a different way of getting the
// job done.  To split a std::string into a vector of words, you
// can use a std::stringstream to get the job done and the ">>"
// operator. See, for example, "gnomed"'s post on stackoverflow for
// their example on how to do this:
//
//   http://stackoverflow.com/questions/236129/c-how-to-split-a-string
//
// (Search for "gnomed" on that page. They use an istringstream, but
// a stringstream gets the job done too.)
//
// Good luck, and write beautiful code!
int main(int argc, char** argv) {
  if (argc < 2) {
    Usage(argv[0]);
  }

  // STEP 1:
  // Implement filesearchshell!
  // Probably want to write some helper methods ...

  // Create index vector and query processor
  list<string> indices;
  for (int i = 1; i < argc; i++) {
    indices.push_back(argv[i]);
  }
  hw3::QueryProcessor qp(indices);

  // loop for queries
  while (1) {
    vector<string> query;
    printf("enter query:\n");
    string input;
    std::getline(cin, input);

    // exit if EOF detected
    if (cin.eof()) {
      cout << "^D" << endl;
      break;
    }

    // changes the input to lower case
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);

    // split input line into words
    while (input.find(" ") != string::npos) {
      int index = input.find(" ");
      string word = input.substr(0, index);
      query.push_back(word);
      input.erase(0, index + 1);
    }

    query.push_back(input);

    // feed query into ProcessQuery
    vector<hw3::QueryProcessor::QueryResult> results = qp.ProcessQuery(query);

    // print results
    PrintResults(results);
  }

  return EXIT_SUCCESS;
}

static void Usage(char* prog_name) {
  cerr << "Usage: " << prog_name << " [index files+]" << endl;
  exit(EXIT_FAILURE);
}

static void PrintResults(vector<hw3::QueryProcessor::QueryResult> results) {
  if (results.size() == 0) {
    cout << "  [no results]" << endl;
  } else {
    for (auto const& result : results) {
      cout << "  " << result.document_name
           << " (" << result.rank << ")" << endl;
    }
  }
}


