#pragma once

#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

namespace sortlib {

    namespace detail {

        // Reorders *first, *mid, *last-1 into sorted order under comp, then moves
        // the resulting median value (from mid) into *first so it can serve as
        // the Hoare partition pivot.
        template <typename RandomIt, typename Compare>
        void MoveMedianOfThreeToFront(RandomIt first, RandomIt last, Compare comp) {
            auto size = std::distance(first, last);
            RandomIt mid = first + size / 2;
            RandomIt lastElem = last - 1;

            if (comp(*mid, *first)) std::iter_swap(mid, first);
            if (comp(*lastElem, *first)) std::iter_swap(lastElem, first);
            if (comp(*lastElem, *mid)) std::iter_swap(lastElem, mid);
            std::iter_swap(first, mid);
        }

        // Classic Hoare partition scheme with pivot = *first. Indices are used
        // instead of iterators positioned before `first`/at `last` so this stays
        // valid for debug-checked iterators (e.g. MSVC's checked vector
        // iterators), while preserving the exact index arithmetic (i starting
        // one below the first valid index, j one above the last) that the
        // algorithm's termination guarantees depend on.
        template <typename RandomIt, typename Compare>
        RandomIt Partition(RandomIt first, RandomIt last, Compare comp) {
            MoveMedianOfThreeToFront(first, last, comp);
            auto pivot = *first;

            auto size = std::distance(first, last);
            std::remove_const_t<decltype(size)> i = -1;
            std::remove_const_t<decltype(size)> j = size;

            while (true) {
                do { ++i; } while (comp(*(first + i), pivot));
                do { --j; } while (comp(pivot, *(first + j)));
                if (i >= j) return first + j;
                std::iter_swap(first + i, first + j);
            }
        }

        template <typename RandomIt, typename Compare>
        void QuickSortImpl(RandomIt first, RandomIt last, Compare comp) {
            if (std::distance(first, last) <= 1) {
                return;
            }
            RandomIt splitPoint = Partition(first, last, comp);
            QuickSortImpl(first, splitPoint + 1, comp);
            QuickSortImpl(splitPoint + 1, last, comp);
        }

    } // namespace detail

    // Sorts [first, last) in place using quicksort (median-of-three pivot,
    // Hoare partitioning), ordering elements so that comp(*it, *(it+1)) is
    // never true. Average O(n log n); requires RandomIt to be a random-access
    // iterator.
    template <typename RandomIt, typename Compare>
    void QuickSort(RandomIt first, RandomIt last, Compare comp) {
        detail::QuickSortImpl(first, last, comp);
    }

    // Sorts [first, last) in place using operator< for comparisons.
    template <typename RandomIt>
    void QuickSort(RandomIt first, RandomIt last) {
        using ValueType = typename std::iterator_traits<RandomIt>::value_type;
        QuickSort(first, last, std::less<ValueType>());
    }

} // namespace sortlib
