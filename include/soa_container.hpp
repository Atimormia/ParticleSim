#pragma once
#include <vector>
#include <array>
#include <tuple>

template<typename T, int Components>
struct SoAFieldBase
{
    std::array<std::vector<T>, Components> storage;

    void reserve(size_t n)
    {
        for (auto& v : storage) v.reserve(n);
    }

    void resize(size_t n)
    {
        for (auto& v : storage) v.resize(n);
    }

    size_t size() const { return storage[0].size(); }

    T& component(int k, size_t i)       { return storage[k][i]; }
    const T& component(int k, size_t i) const { return storage[k][i]; }
};

template<typename T>
struct SoAFieldScalar : public SoAFieldBase<T,1>
{
    using Base = SoAFieldBase<T,1>;

    void push_back(const T& value)
    {
        Base::storage[0].push_back(value);
    }

    void push_default()
    {
        Base::storage[0].push_back({});
    }

    T& operator[](size_t i)
    {
        return Base::storage[0][i];
    }
};

struct SoAFieldVector2D : public SoAFieldBase<float,2>
{
    using Base = SoAFieldBase<float,2>;

    void push_back(float x, float y)
    {
        Base::storage[0].push_back(x);
        Base::storage[1].push_back(y);
    }

    void push_default()
    {
        Base::storage[0].push_back(0.0f);
        Base::storage[1].push_back(0.0f);
    }

    float& x(size_t i) { return Base::storage[0][i]; }
    float& y(size_t i) { return Base::storage[1][i]; }
};

template<typename... Fields>
class SoAContainer
{
public:
    void reserve(size_t n)
    {
        std::apply([&](auto&... f){ (f.reserve(n), ...); }, fields);
    }

    void push_back()
    {
        std::apply([&](auto&... f){ (f.push_default(), ...); }, fields);
    }

    size_t size() const
    {
        return std::get<0>(fields).size();
    }

    template<size_t Index>
    auto& field()
    {
        return std::get<Index>(fields);
    }

private:
    std::tuple<Fields...> fields;
};

