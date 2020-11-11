#pragma once

#include <type_traits>

struct nullopt_t {
    enum class Construct {
        Token
    };

    explicit constexpr nullopt_t(Construct) {}
};

inline constexpr nullopt_t nullopt{nullopt_t::Construct::Token};

struct in_place_t {
    enum class Construct {
        Token
    };

    explicit constexpr in_place_t(Construct) {}
};

inline constexpr in_place_t in_place{in_place_t::Construct::Token};


template<typename T, typename = void>
class base_optional_destructor;

template<typename T>
class base_optional_destructor<
        T, std::enable_if_t<std::is_trivially_destructible<T>::value>> {
public:
    constexpr base_optional_destructor() noexcept {};

    constexpr base_optional_destructor(nullopt_t) noexcept {}

    constexpr base_optional_destructor(T t) : base_optional_destructor(in_place, std::move(t)) {}

    template<typename... Args>
    explicit constexpr base_optional_destructor(in_place_t, Args &&... args)
            : value(std::forward<Args>(args)...) {
        has_value = true;
    }

    constexpr base_optional_destructor(base_optional_destructor const &) = default;

    constexpr base_optional_destructor(base_optional_destructor &&) = default;

    base_optional_destructor &operator=(base_optional_destructor const &) = default;

    base_optional_destructor &operator=(base_optional_destructor &&) = default;

    constexpr explicit operator bool() const noexcept {
        return has_value;
    }

    constexpr T &operator*() noexcept {
        return value;
    }

    constexpr T const &operator*() const noexcept {
        return value;
    }

    constexpr T *operator->() noexcept {
        return &value;
    }

    constexpr T const *operator->() const noexcept {
        return &value;
    }

    template<typename... Args>
    void emplace(Args &&... args) {
        reset();
        new(&value) T(std::forward<Args>(args)...);
        has_value = true;
    }

    void reset() {
        dummy = '\0';
        has_value = false;
    }

    ~base_optional_destructor() = default;

    bool has_value = false;
    union {
        T value;
        char dummy{};
    };
};

template<typename T>
class base_optional_destructor<
        T, std::enable_if_t<!std::is_trivially_destructible<T>::value>> {
public:
    constexpr base_optional_destructor() noexcept {};

    constexpr base_optional_destructor(nullopt_t) noexcept {}

    constexpr base_optional_destructor(T t) : base_optional_destructor(in_place, std::move(t)) {}


    template<typename... Args>
    explicit constexpr base_optional_destructor(in_place_t, Args &&... args)
            : value(std::forward<Args>(args)...) {
        has_value = true;
    }

    constexpr base_optional_destructor(base_optional_destructor const &) = default;

    constexpr base_optional_destructor(base_optional_destructor &&) = default;

    base_optional_destructor &operator=(base_optional_destructor const &) = default;

    base_optional_destructor &operator=(base_optional_destructor &&) = default;

    constexpr explicit operator bool() const noexcept {
        return has_value;
    }

    constexpr T &operator*() noexcept {
        return value;
    }

    constexpr T const &operator*() const noexcept {
        return value;
    }

    constexpr T *operator->() noexcept {
        return &value;
    }

    constexpr T const *operator->() const noexcept {
        return &value;
    }

    template<typename... Args>
    void emplace(Args &&... args) {
        reset();
        new(&value) T(std::forward<Args>(args)...);
        has_value = true;
    }

    void reset() {
        if (has_value)
            value.~T();
        dummy = '\0';
        has_value = false;
    }

    ~base_optional_destructor() {
        if (has_value)
            value.~T();
    }

    bool has_value = false;
    union {
        T value;
        char dummy{};
    };
};

template<typename T, typename = void>
class base_optional_copy;

template<typename T>
class base_optional_copy<T,
        std::enable_if_t<std::is_trivially_copyable<T>::value>>
        : public base_optional_destructor<T> {
    using base = base_optional_destructor<T>;
public:
    using base::base;

    constexpr base_optional_copy(base_optional_copy const &) = default;

    constexpr base_optional_copy(base_optional_copy &&) = default;

    constexpr base_optional_copy &operator=(base_optional_copy const &) = default;

    constexpr base_optional_copy &operator=(base_optional_copy &&) = default;

    ~base_optional_copy() = default;
};

template<typename T>
class base_optional_copy<
        T, std::enable_if_t<!std::is_trivially_copyable<T>::value>>
        : public base_optional_destructor<T> {
    using base = base_optional_destructor<T>;
public:
    using base::base;

    constexpr base_optional_copy(base_optional_copy const &other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        if (other.has_value) {
            new(&this->value) T(other.value);
            this->has_value = other.has_value;
        }
    }

    constexpr base_optional_copy &operator=(base_optional_copy const &other) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        if (this->has_value) {
            if (other.has_value) {
                this->value = other.value;
                this->has_value = other.has_value;
            } else {
                this->reset();
            }
        } else {
            if (other.has_value) {
                new(&this->value) T(other.value);
                this->has_value = other.has_value;
            }
        }
        return *this;
    }

    constexpr base_optional_copy(base_optional_copy &&) = default;

    constexpr base_optional_copy &operator=(base_optional_copy &&) = default;

    ~base_optional_copy() = default;
};

template<typename T, typename = void>
class base_optional_move;

template<typename T>
class base_optional_move<
        T, std::enable_if_t<std::is_trivially_move_constructible<T>::value>>
        : public base_optional_copy<T> {
    using base = base_optional_copy<T>;
public:
    using base::base;

    constexpr base_optional_move(base_optional_move const &) = default;

    constexpr base_optional_move(base_optional_move &&) = default;

    constexpr base_optional_move &operator=(base_optional_move const &) = default;

    constexpr base_optional_move &operator=(base_optional_move &&) = default;

    ~base_optional_move() = default;
};

template<typename T>
class base_optional_move<
        T, std::enable_if_t<!std::is_trivially_move_constructible<T>::value>>
        : public base_optional_copy<T> {
    using base = base_optional_copy<T>;
public:
    using base::base;

    constexpr base_optional_move(base_optional_move &&other) noexcept(std::is_nothrow_move_constructible_v<T>) {
        if (other.has_value) {
            new(&this->value) T(std::move(other.value));
            this->has_value = other.has_value;
        }
    }

    constexpr base_optional_move &operator=(base_optional_move &&other)
            noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>){
        if (this->has_value) {
            if (other.has_value) {
                this->value = std::move(other.value);
                this->has_value = true;
            } else {
                this->reset();
            }
        } else {
            if (other.has_value) {
                new(&this->value) T(std::move(other.value));
                this->has_value = true;
            }
        }
        return *this;
    }

    constexpr base_optional_move(base_optional_move const &) = default;

    constexpr base_optional_move &operator=(base_optional_move const &) = default;

    ~base_optional_move() = default;
};

template<typename T>
class optional : public base_optional_move<T> {
    using base = base_optional_move<T>;
public:
    using base::base;

    constexpr optional(optional const &) = default;

    constexpr optional(optional &&) = default;

    constexpr optional &operator=(optional const &) = default;

    constexpr optional &operator=(optional &&) = default;

    constexpr optional &operator=(nullopt_t) noexcept {
        this->reset();
        return *this;
    }

    ~optional() = default;
};

template<typename T>
constexpr bool operator==(optional<T> const &a, optional<T> const &b) {
    if (!static_cast<bool>(a) && !static_cast<bool>(b)) return true;
    if (static_cast<bool>(a) && static_cast<bool>(b)) {
        return *a == *b;
    }
    return false;
}

template<typename T>
constexpr bool operator<(optional<T> const &a, optional<T> const &b) {
    if (static_cast<bool>(a)) {
        if (static_cast<bool>(b)) {
            return *a < *b;
        }
        return false;
    }
    if (static_cast<bool>(b)) {
        return true;
    }
    return false;
}

template<typename T>
constexpr bool operator>(optional<T> const &a, optional<T> const &b) {
    return !(a <= b);
}


template<typename T>
constexpr bool operator!=(optional<T> const &a, optional<T> const &b) {
    return !(a == b);
}

template<typename T>
constexpr bool operator<=(optional<T> const &a, optional<T> const &b) {
    return a < b || a == b;
}

template<typename T>
constexpr bool operator>=(optional<T> const &a, optional<T> const &b) {
    return a > b || a == b;
}
