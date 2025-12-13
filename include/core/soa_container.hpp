#pragma once
#include <vector>
#include <array>
#include <tuple>
#include <concepts>
#include <cstddef>
#include "soa_field.hpp"

namespace core
{
    using namespace std;

    template <size_t K, size_t Components>
    concept ComponentIndex = (K < Components);

    template <typename... Fs>
    concept AllSoAFields = (SoAField<Fs> && ...);

    template <typename... Fields>
        requires AllSoAFields<Fields...>
    class SoAContainer
    {
    public:
        void reserve(size_t n)
        {
            apply([&](auto &...f)
                  { (f.reserve(n), ...); }, fields);
        }

        void push_back()
        {
            apply([&](auto &...f)
                  { (f.push_default(), ...); }, fields);
        }

        size_t size() const
        {
            return get<0>(fields).size();
        }

        template <typename Tag>
            requires HasField_v<Tag, Fields...>
        auto &field()
        {
            constexpr size_t index = FieldIndex<Tag, Fields...>::value;
            return get<index>(fields);
        }

    private:
        tuple<Fields...> fields;
    };
}