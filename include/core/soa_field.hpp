#pragma once
#include <concepts>
#include <vector>
#include <array>
#include <cstddef>

namespace core
{
    using namespace std;

    template <size_t K, size_t Components>
    concept ComponentIndex = (K < Components);

    template <typename F>
    concept SoAField = requires(F f, size_t n) {
        { f.reserve(n) } -> same_as<void>;
        { f.resize(n) } -> same_as<void>;
        { f.size() } -> same_as<size_t>;
        { f.push_default() } -> same_as<void>;
    };

    template <typename Tag, typename... Fields>
    inline constexpr bool HasField_v =
    (is_same_v<typename Fields::tag, Tag> || ...);

    template <typename Tag>
    struct FieldTag
    {
        using tag = Tag;
    };

    template <typename Tag, typename... Fields>
    struct FieldIndex;

    template <typename Tag, typename First, typename... Rest>
    struct FieldIndex<Tag, First, Rest...>
    {
        static constexpr size_t value =
            is_same_v<typename First::tag, Tag>
                ? 0
                : 1 + FieldIndex<Tag, Rest...>::value;
    };

    template <typename Tag>
    struct FieldIndex<Tag>
    {
        static constexpr size_t value = 0;
    };

    template <typename T, size_t Components>
    struct SoAFieldBase
    {
        static_assert(Components > 0);

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
        T *data() noexcept { return storage[K].data();}

        template <size_t K>
            requires ComponentIndex<K, Components>
        const T *data() const noexcept { return storage[K].data();}
    };

    template <typename T, typename Tag>
    struct SoAFieldScalar : public SoAFieldBase<T, 1>, FieldTag<Tag>
    {
        using Base = SoAFieldBase<T, 1>;

        void push_back(const T &value) { Base::storage[0].push_back(value); }

        void push_default() { Base::storage[0].push_back(T{}); }

        T &operator[](size_t i) { return Base::storage[0][i]; }

        T *data() noexcept { return Base::storage[0].data(); }
        const T *data() const noexcept { return Base::storage[0].data(); }
    };

    template <typename Tag>
    struct SoAFieldVector2D : public SoAFieldBase<float, 2>, FieldTag<Tag>
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
}