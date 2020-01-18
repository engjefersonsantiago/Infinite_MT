#include <string>
#include <iostream>

#include "multi_index_sorted_container.hpp"

template<typename T>
struct Greater
{
    constexpr auto operator()(const T& lhs, const T& rhs) { return lhs > rhs; }
};

struct Foo {
    std::string a;

    bool operator==(const Foo& other) const {
        return this->a  == other.a;
    }
};

namespace std {

template <> struct hash<Foo> {
    size_t operator()(const Foo& foo) const {
        return std::hash<std::string>()(foo.a);
    }
}; // hash<FiveTuple>

} // std

std::size_t hash_value(const Foo& foo)
{
    return std::hash<Foo>()(foo);
}

int main () {
    MultiIndexSortedContainer<2, Foo, int, std::greater<int>/*, std::less<int>*/> my_cont;
//    ({
//       { "a", 1 },
//       { "C", 1 },
//       { "b", 2 }
//    });

    my_cont.insert({{"a"}, 1 });
    my_cont.insert({{"C"}, 2 });
    my_cont.insert({{"b"}, 0 });

    Foo a = {"C"};
    std::cout << my_cont.highest_order()->key.a << ", " << my_cont.highest_order()->value << '\n';
    auto f = my_cont.find(a);
    //f->value = 14;
    if (f != my_cont.end())
    {
        //std::cout << "Found\n";
        //std::cout << f->key.a << ", " << f->value << '\n';
        auto f2 = *f;
        //my_cont.erase(f);
        my_cont.modify(f, f->value + 50);
        //my_cont.insert({f2.key, f2.value + 50 });
        //std::cout << my_cont.find(a)->key.a << ", " << my_cont.find(a)->value << '\n';
    }
    std::cout << my_cont.highest_order()->key.a << ", " << my_cont.highest_order()->value << '\n';
    //std::cout << my_cont.find(a)->key.a << ", " << my_cont.find(a)->value << '\n';
    for (auto i : my_cont)
       std::cout << i.key.a << ", " << i.value << '\n';
}
