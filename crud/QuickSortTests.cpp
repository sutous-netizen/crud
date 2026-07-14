#include <algorithm>
#include <array>
#include <functional>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "QuickSort.h"

using ::testing::ElementsAre;
using ::testing::IsEmpty;

TEST(QuickSortTest, SortsEmptyRange) {
    std::vector<int> v;
    sortlib::QuickSort(v.begin(), v.end());
    EXPECT_THAT(v, IsEmpty());
}

TEST(QuickSortTest, SortsSingleElementRange) {
    std::vector<int> v{42};
    sortlib::QuickSort(v.begin(), v.end());
    EXPECT_THAT(v, ElementsAre(42));
}

TEST(QuickSortTest, SortsAlreadySortedInput) {
    std::vector<int> v{1, 2, 3, 4, 5};
    sortlib::QuickSort(v.begin(), v.end());
    EXPECT_THAT(v, ElementsAre(1, 2, 3, 4, 5));
}

TEST(QuickSortTest, SortsReverseSortedInput) {
    std::vector<int> v{5, 4, 3, 2, 1};
    sortlib::QuickSort(v.begin(), v.end());
    EXPECT_THAT(v, ElementsAre(1, 2, 3, 4, 5));
}

TEST(QuickSortTest, SortsRandomOrderInput) {
    std::vector<int> v{8, 3, 9, 1, 6, 2, 7, 4, 0, 5};
    sortlib::QuickSort(v.begin(), v.end());
    EXPECT_THAT(v, ElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(QuickSortTest, SortsInputWithDuplicates) {
    std::vector<int> v{3, 1, 3, 2, 1, 3, 2};
    sortlib::QuickSort(v.begin(), v.end());
    EXPECT_THAT(v, ElementsAre(1, 1, 2, 2, 3, 3, 3));
}

TEST(QuickSortTest, SortsInputWithAllEqualElements) {
    std::vector<int> v{7, 7, 7, 7, 7};
    sortlib::QuickSort(v.begin(), v.end());
    EXPECT_THAT(v, ElementsAre(7, 7, 7, 7, 7));
}

TEST(QuickSortTest, SortsWithCustomDescendingComparator) {
    std::vector<int> v{5, 1, 4, 2, 3};
    sortlib::QuickSort(v.begin(), v.end(), std::greater<int>());
    EXPECT_THAT(v, ElementsAre(5, 4, 3, 2, 1));
}

TEST(QuickSortTest, SortsStrings) {
    std::vector<std::string> v{"banana", "apple", "cherry", "date"};
    sortlib::QuickSort(v.begin(), v.end());
    EXPECT_THAT(v, ElementsAre("apple", "banana", "cherry", "date"));
}

TEST(QuickSortTest, SortsFixedSizeArray) {
    std::array<int, 6> a{9, 5, 1, 4, 3, 7};
    sortlib::QuickSort(a.begin(), a.end());
    EXPECT_THAT(a, ElementsAre(1, 3, 4, 5, 7, 9));
}

TEST(QuickSortTest, SortsRawPointerRange) {
    int values[] = {4, 2, 5, 1, 3};
    sortlib::QuickSort(std::begin(values), std::end(values));
    EXPECT_THAT(values, ElementsAre(1, 2, 3, 4, 5));
}

TEST(QuickSortTest, MatchesStdSortOnLargerInput) {
    std::vector<int> expected{
        34, 7, 23, 32, 5, 62, 32, 12, 90, 1, 0, -5, 18, 45, 3, 3, 99, 21, 8, 8
    };
    std::vector<int> actual = expected;

    std::sort(expected.begin(), expected.end());
    sortlib::QuickSort(actual.begin(), actual.end());

    EXPECT_EQ(actual, expected);
}
