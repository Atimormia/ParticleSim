#pragma once
#include <vector>
#include <array>
#include <tuple>
#include <concepts>
#include <cstddef>

namespace core
{
    using namespace std;

    template <typename F>
    concept SoAField = requires(F f, size_t n) {
        { f.reserve(n) } -> same_as<void>;
        { f.resize(n) } -> same_as<void>;
        { f.size() } -> same_as<size_t>;
        { f.push_default() } -> same_as<void>;
    };

    template <size_t K, size_t Components>
    concept ComponentIndex = (K < Components);

    template <typename... Fs>
    concept AllSoAFields = (SoAField<Fs> && ...);

    template <typename T, size_t Components>
    struct SoAFieldBase
    {
        static_assert(Components > 0);

        using value_type = T;
        static constexpr size_t component_count = Components;

        array<vector<T>, Components> storage; // a try to convert AoS with nested data to SoA

        void reserve(size_t n)
        {
            for (auto &v : storage)
                v.reserve(n);
        }

        void resize(size_t n)
        {
            for (auto &v : storage)
                v.resize(n);
        }

        size_t size() const { return storage[0].size(); }

        template <size_t K>
            requires ComponentIndex<K, Components>
        T *data() noexcept
        {
            return storage[K].data();
        }

        template <size_t K>
            requires ComponentIndex<K, Components>
        const T *data() const noexcept
        {
            return storage[K].data();
        }
    };

    template <typename T>
    struct SoAFieldScalar : public SoAFieldBase<T, 1>
    {
        using Base = SoAFieldBase<T, 1>;

        void push_back(const T &value) { Base::storage[0].push_back(value); }

        void push_default() { Base::storage[0].push_back(T{}); }

        T &operator[](size_t i) { return Base::storage[0][i]; }

        T *data() noexcept { return Base::storage[0].data(); }
        const T *data() const noexcept { return Base::storage[0].data(); }
    };

    struct SoAFieldVector2D : public SoAFieldBase<float, 2>
    {
        using Base = SoAFieldBase<float, 2>;

        void push_back(float x, float y)
        {
            Base::storage[0].push_back(x);
            Base::storage[1].push_back(y);
        }

        void push_default()
        {
            Base::storage[0].push_back(0.f);
            Base::storage[1].push_back(0.f);
        }

        float *x() { return Base::data<0>(); }
        float *y() { return Base::data<1>(); }
    };

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

        template <size_t Index>
            requires(Index < sizeof...(Fields))
        auto &field()
        {
            return get<Index>(fields);
        }

    private:
        tuple<Fields...> fields;
    };
}