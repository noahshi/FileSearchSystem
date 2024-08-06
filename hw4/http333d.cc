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

#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <list>

#include "./ServerSocket.h"
#include "./HttpServer.h"

using std::cerr;
using std::cout;
using std::endl;
using std::list;
using std::string;

// Print out program usage, and exit() with EXIT_FAILURE.
static void Usage(string prog_name);

// Parse command-line arguments to get port, path, and indices to use
// for your http333d server.
//
// Params:
// - argc: number of argumnets
// - argv: array of arguments
// - port: output parameter returning the port number to listen on
// - path: output parameter returning the directory with our static files
// - indices: output parameter returning the list of index file names
//
// Calls Usage() on failure. Possible errors include:
// - path is not a readable directory
// - index file names are readable
static void GetPortAndPath(int argc,
                    char** argv,
                    uint16_t* const port,
                    string* const path,
                    list<string>* const indices);

int main(int argc, char** argv) {
  // Print out welcome message.
  cout << "Welcome to http333d, the UW cse333 web server!" << endl;
  cout << "  Copyright 2012 Steven Gribble" << endl;
  cout << "  http://www.cs.washington.edu/homes/gribble" << endl;
  cout << endl;
  cout << "initializing:" << endl;
  cout << "  parsing port number and static files directory..." << endl;

  // Ignore the SIGPIPE signal, otherwise we'll crash out if a client
  // disconnects unexpectedly.
  signal(SIGPIPE, SIG_IGN);

  // Get the port number and list of index files.
  uint16_t port_num;
  string static_dir;
  list<string> indices;
  GetPortAndPath(argc, argv, &port_num, &static_dir, &indices);
  cout << "    port: " << port_num << endl;
  cout << "    path: " << static_dir << endl;

  // Run the server.
  hw4::HttpServer hs(port_num, static_dir, indices);
  if (!hs.Run()) {
    cerr << "  server failed to run!?" << endl;
  }

  cout << "server completed!  Exiting." << endl;
  return EXIT_SUCCESS;
}


static void Usage(string prog_name) {
  cerr << "Usage: " << prog_name << " port staticfiles_directory indices+";
  cerr << endl;
  exit(EXIT_FAILURE);
}

static void GetPortAndPath(int argc,
                    char** argv,
                    uint16_t* const port,
                    string* const path,
                    list<string>* const indices) {
  // Here are some considerations when implementing this function:
  // - There is a reasonable number of command line arguments
  // - The port number is reasonable
  // - The path (i.e., argv[2]) is a readable directory
  // - You have at least 1 index, and all indices are readable files

  // STEP 1:
  // check if number of arguments is reasonable
  if (argc < 4) {
    cerr << endl;
    Usage(argv[0]);
  }

  // check if port is reasonable
  *port = std::atoi(argv[1]);
  if (*port < 1024 || *port > 49151) {
    cerr << endl << argv[1] << " isn't a valid port number." << endl;
    Usage(argv[0]);
  }

  // check if path is a readable directory
  *path = argv[2];
  char abs_path[PATH_MAX];
  if (!realpath(argv[2], abs_path)) {
    cerr << endl << argv[2] << " isn't a directory." << endl;
    Usage(argv[0]);
  }

  struct stat buf;
  if (stat(abs_path, &buf) == -1 ||
           !S_ISDIR(buf.st_mode)) {
    cerr << endl << argv[2] << " isn't a directory." << endl;
    Usage(argv[0]);
  }

  // check if index files are valid
  for (int i = 3; i < argc; i++) {
    string name = argv[i];
    // check if file has correct suffix
    if (name.size() < 4 || name.substr(name.size() - 4) != ".idx") {
      cout << name << " isn't a regular file. Skipping..." << endl;
      continue;
    }
    // check if it is an actual file
    if (!realpath(argv[i], abs_path)) {
      cerr << name << " isn't a regular file. Skipping..." << endl;
      Usage(argv[0]);
    }
    if (stat(abs_path, &buf) == -1 ||
             !S_ISREG(buf.st_mode)) {
      cout << name << " isn't a regular file. Skipping..." << endl;
      continue;
    }
    indices->push_back(name);
  }

  if (indices->size() == 0) {
    cerr << endl << "No valid index files found." << endl;
    Usage(argv[0]);
  }
}

