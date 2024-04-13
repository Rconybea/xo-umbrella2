#include <string_view>
#include <array>

template <std::string_view const & ... Strings>
struct sv_concat
{
    static constexpr auto impl() noexcept {
        constexpr std::size_t n = (Strings.size() + ... + 0);

        std::array<char, n + 1> arr{};

        auto append = [i=0, &arr](const auto & s) mutable {
            for (auto c : s)
                arr[i++] = c;
        };
        (append(Strings), ...);
        arr[n] = '\0';

        return arr;
    }

    static constexpr auto arr = impl();
    static constexpr std::string_view value {
        arr.data(),
        arr.size() - 1
    };
};

template <std::string_view const & ... Strings>
static constexpr auto concat_v = sv_concat<Strings...>::value;
