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

#include "./QueryProcessor.h"

#include <iostream>
#include <algorithm>
#include <list>
#include <string>
#include <vector>

extern "C" {
  #include "./libhw1/CSE333.h"
}

using std::list;
using std::sort;
using std::string;
using std::vector;

namespace hw3 {

QueryProcessor::QueryProcessor(const list<string>& index_list, bool validate) {
  // Stash away a copy of the index list.
  index_list_ = index_list;
  array_len_ = index_list_.size();
  Verify333(array_len_ > 0);

  // Create the arrays of DocTableReader*'s. and IndexTableReader*'s.
  dtr_array_ = new DocTableReader* [array_len_];
  itr_array_ = new IndexTableReader* [array_len_];

  // Populate the arrays with heap-allocated DocTableReader and
  // IndexTableReader object instances.
  list<string>::const_iterator idx_iterator = index_list_.begin();
  for (int i = 0; i < array_len_; i++) {
    FileIndexReader fir(*idx_iterator, validate);
    dtr_array_[i] = fir.NewDocTableReader();
    itr_array_[i] = fir.NewIndexTableReader();
    idx_iterator++;
  }
}

QueryProcessor::~QueryProcessor() {
  // Delete the heap-allocated DocTableReader and IndexTableReader
  // object instances.
  Verify333(dtr_array_ != nullptr);
  Verify333(itr_array_ != nullptr);
  for (int i = 0; i < array_len_; i++) {
    delete dtr_array_[i];
    delete itr_array_[i];
  }

  // Delete the arrays of DocTableReader*'s and IndexTableReader*'s.
  delete[] dtr_array_;
  delete[] itr_array_;
  dtr_array_ = nullptr;
  itr_array_ = nullptr;
}

// This structure is used to store a index-file-specific query result.
typedef struct {
  DocID_t doc_id;  // The document ID within the index file.
  int     rank;    // The rank of the result so far.
} IdxQueryResult;

// Helper function to convert lists from DocIDTableReaders into lists
// of IdxQueryResults
static list<IdxQueryResult> ConvertListFormat(
                const list<DocIDElementHeader> header_list);

vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuery(const vector<string>& query) const {
  Verify333(query.size() > 0);

  // STEP 1.
  // (the only step in this file)
  vector<QueryProcessor::QueryResult> final_result;

  for (int i = 0; i < array_len_; i++) {
    // first query
    list<IdxQueryResult> doc_id_results;
    DocIDTableReader *didtr = itr_array_[i]->LookupWord(query[0]);

    // skip index if word not found
    if (didtr == NULL) {
      continue;
    }

    doc_id_results = ConvertListFormat(didtr->GetDocIDList());
    delete didtr;

    // subsequent queries
    for (unsigned int j = 1; j < query.size(); j++) {
      DocIDTableReader *didtr = itr_array_[i]->LookupWord(query[j]);

      // skip index and clear index results if word not found
      if (didtr == NULL) {
        doc_id_results.clear();
        break;
      }

      // loop through current result documents & search for new queries
      for (auto idx_qr = doc_id_results.begin();
           idx_qr != doc_id_results.end(); ++idx_qr) {
        list<DocPositionOffset_t> positions;
        didtr->LookupDocID((*idx_qr).doc_id, &positions);

        if (positions.size() == 0) {
          // remove result if word not found in document
          idx_qr = doc_id_results.erase(idx_qr);
          --idx_qr;
        } else {
          // add to rank if word is found in document
          (*idx_qr).rank += positions.size();
        }
      }
      delete didtr;
    }

    // move results to final_result vector and change to correct format
    for (auto const& idx_qr : doc_id_results) {
      QueryProcessor::QueryResult qr;
      qr.rank = idx_qr.rank;
      dtr_array_[i]->LookupDocID(idx_qr.doc_id, &qr.document_name);
      final_result.push_back(qr);
    }
  }

  // Sort the final results.
  sort(final_result.begin(), final_result.end());
  return final_result;
}

static list<IdxQueryResult> ConvertListFormat(
                const list<DocIDElementHeader> header_list) {
  list<IdxQueryResult> idx_list;
  for (auto const& header : header_list) {
    IdxQueryResult idx_qr;
    idx_qr.doc_id = header.doc_id;
    idx_qr.rank = header.num_positions;
    idx_list.push_back(idx_qr);
  }
  return idx_list;
}

}  // namespace hw3
