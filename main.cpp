#include <iostream>
#include <vector>
#include <boost/container/small_vector.hpp>

#include "m32/SimpleNote.h"

template <std::size_t N>
struct small_vec_n {
    template <typename T>
    using vec = boost::container::small_vector<T, N>;
};

int main() {
    Interval<int> itv(4, 6);
    std::cout << itv << std::endl;
    std::cout << "Hello, World!" << std::endl;

    m32::SimpleNote<int, std::vector> v;
    boost::container::small_vector<int, 5> g {};
    m32::SimpleNote<int, small_vec_n<5>::vec> h{};
    return 0;
}
