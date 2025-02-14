#include <gtest/gtest.h>
#include "simpleOrderedDict.h"
#include <string>
#include <stdexcept>

using namespace std;

TEST(SimpleOrderedDictTest, InitialEmpty) {
    SimpleOrderedDict<int, string> dict;
    EXPECT_EQ(dict.size(), 0);
    EXPECT_FALSE(dict.contains(1));
}

TEST(SimpleOrderedDictTest, BasicUpsertAndAccess) {
    SimpleOrderedDict<int, string> dict;
    dict.upsert(1, "one");
    dict.upsert(2, "two");
    
    EXPECT_EQ(dict.size(), 2);
    EXPECT_EQ(dict[1], "one");
    EXPECT_EQ(dict[2], "two");
}

TEST(SimpleOrderedDictTest, OrderPreservation) {
    SimpleOrderedDict<int, string> dict;
    dict.upsert(1, "one");
    dict.upsert(2, "two");
    dict.upsert(3, "three");
    
    EXPECT_EQ(dict.getKth(0), "one");
    EXPECT_EQ(dict.getKth(1), "two");
    EXPECT_EQ(dict.getKth(2), "three");
}

TEST(SimpleOrderedDictTest, UpdateExistingKey) {
    SimpleOrderedDict<int, string> dict;
    dict.upsert(1, "one");
    dict.upsert(1, "new_one");
    
    EXPECT_EQ(dict.size(), 1);
    EXPECT_EQ(dict[1], "new_one");
    EXPECT_EQ(dict.getKth(0), "new_one");
}

TEST(SimpleOrderedDictTest, RemoveOperations) {
    SimpleOrderedDict<int, string> dict;
    dict.upsert(1, "one");
    dict.upsert(2, "two");
    dict.upsert(3, "three");
    
    // Remove middle
    dict.remove(2);
    EXPECT_EQ(dict.size(), 2);
    EXPECT_EQ(dict.getKth(0), "one");
    EXPECT_EQ(dict.getKth(1), "three");
    
    // Remove head
    dict.remove(1);
    EXPECT_EQ(dict.size(), 1);
    EXPECT_EQ(dict.getKth(0), "three");
    
    // Remove tail
    dict.upsert(4, "four");
    dict.remove(4);
    EXPECT_EQ(dict.size(), 1);
}

TEST(SimpleOrderedDictTest, ConstAccess) {
    SimpleOrderedDict<int, string> dict;
    dict.upsert(42, "answer");
    const auto& const_dict = dict;
    
    EXPECT_EQ(const_dict[42], "answer");
    EXPECT_EQ(const_dict.size(), 1);
}

TEST(SimpleOrderedDictTest, ExceptionHandling) {
    SimpleOrderedDict<int, string> dict;
    
    EXPECT_THROW(dict.getKth(0), out_of_range);
    dict.upsert(1, "one");
    EXPECT_THROW(dict.getKth(1), out_of_range);
    
    EXPECT_NO_THROW(dict[1]);  // Existing key
    EXPECT_THROW(dict[999], out_of_range);  // Non-existent key
}