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

extern "C" {
  #include "./HashTable.h"
  #include "./HashTable_priv.h"
  #include "./LinkedList.h"
  #include "./LinkedList_priv.h"
}

#include "gtest/gtest.h"

#include "./test_suite.h"

namespace hw1 {

int ExtraLLPayloadComparator(LLPayload_t p1, LLPayload_t p2) {
  // A comparator used to test sort.
  if (p1 > p2)
    return 1;
  if (p1 < p2)
    return -1;
  return 0;
}

// Our payload structure
typedef struct payload_st {
  int magic_num;
  int payload_num;
} Payload;

class Test_Extra : public ::testing::Test {
 protected:
  static const int kMagicNum = 0xDEADBEEF;

  // Code here will be called before each test executes (ie, before
  // each TEST_F).
  virtual void SetUp() {
    freeInvocations_ = 0;
  }

  // A version of free() that verifies the payload before freeing it.
  static void VerifiedFree(HTValue_t payload) {
    ASSERT_EQ(kMagicNum, (static_cast<Payload *>(payload))->magic_num);
    free(payload);
  }

  // A version of VerifiedFree() which counts how many times it's been
  // invoked; this allows us to make assertions.  Note that the counter
  //  is reset in SetUp().
  static int freeInvocations_;
  static void InstrumentedFree(HTValue_t payload) {
    freeInvocations_++;
    VerifiedFree(payload);
  }

  // Linked List payload
  static LLPayload_t kOne;

  static void StubbedFree(LLPayload_t payload) {
    // Do nothing but verify the payload is non-NULL and
    // increment the free count.
    ASSERT_TRUE(payload != NULL);
    freeInvocations_++;
  }
};  // class Test_Extra

// statics:
int Test_Extra::freeInvocations_;
const int Test_Extra::kMagicNum;
LLPayload_t Test_Extra::kOne = (LLPayload_t)1;

static void NoOpFree(HTValue_t freeme) { }

TEST_F(Test_Extra, FNV) {
  char string_1[] = {'a','b','c','d','e','f','g','h','i'};
  char string_2[] = {'c','s','e','3','3','3'};
  char string_3[] = {'d','e','a','d','b','e','e','f'};

  ASSERT_EQ(0xfb321124e0e3a8ccULL, FNVHash64((unsigned char *)&string_1, 9));
  ASSERT_EQ(0xf9d93cecfb73bed9ULL, FNVHash64((unsigned char *)&string_2, 6));
  ASSERT_EQ(0xcd4f3b6f56d93515ULL, FNVHash64((unsigned char *)&string_3, 8));
}

TEST_F(Test_Extra, HTInvalidIteratorRemove) {
  HashTable *table = HashTable_Allocate(2);
  HTIterator *iter = HTIterator_Allocate(table);
  HTKeyValue_t kv;

  ASSERT_FALSE(HTIterator_Remove(iter, &kv));
  HTIterator_Free(iter);
  HashTable_Free(table, &NoOpFree);
}

TEST_F(Test_Extra, LLTrivialSort) {
  // Creating a list.
  LinkedList *llp = LinkedList_Allocate();
  ASSERT_TRUE(llp != NULL);
  ASSERT_EQ(0, LinkedList_NumElements(llp));
  ASSERT_EQ(NULL, llp->head);
  ASSERT_EQ(NULL, llp->tail);

  // Insert some elements.
  LinkedList_Append(llp, kOne);
  ASSERT_EQ(1, LinkedList_NumElements(llp));

  // Sort ascending.
  LinkedList_Sort(llp, true, &ExtraLLPayloadComparator);

  // Verify the sort.
  ASSERT_EQ(kOne, llp->head->payload);
  ASSERT_EQ(NULL, llp->head->next);

  // Resort descending.
  LinkedList_Sort(llp, false, &ExtraLLPayloadComparator);

  // Verify the sort.
  ASSERT_EQ(kOne, llp->head->payload);
  ASSERT_EQ(NULL, llp->head->next);

  // Delete the non-empty list.
  LinkedList_Free(llp, &Test_Extra::StubbedFree);
  llp = NULL;
}

}  // namespace hw1
