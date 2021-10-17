// Copyright 2021 Chris E. Holloway
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef XTR_FORMATTERS_HPP
#define XTR_FORMATTERS_HPP

#include <cstddef>
#include <utility>

template<typename T>
concept iterable = requires(T t) { std::begin(t); std::end(t); };

template<typename T>
concept associative_container = requires(T t) { typename T::mapped_type; };

template<typename T>
concept tuple_like = requires(T t) { std::tuple_size<T>(); };

namespace fmt
{
    // Note that https://gcc.gnu.org/bugzilla/show_bug.cgi?id=92944 prevents
    // writing fmt::formatter

    template<typename T>
    requires tuple_like<T> && (!iterable<T>)
    struct formatter<T>
    {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx)
        {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(const T& value, FormatContext& ctx)
        {
            return format_impl(value, ctx, std::make_index_sequence<std::tuple_size_v<T>>{});
        }

        template<typename FormatContext, std::size_t Index, std::size_t... Indexes>
        auto format_impl(
            const T& value,
            FormatContext& ctx,
            std::index_sequence<Index, Indexes...>)
        {
            format_to(ctx.out(), "(");
            if (std::tuple_size_v<T> > 0)
            {
                format_to(ctx.out(), "{}", std::get<0>(value));
                ((format_to(ctx.out(), ", {}", std::get<Indexes>(value))), ...);
            }
            return format_to(ctx.out(), ")");
        }
    };

    template<associative_container T>
    struct formatter<T>
    {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx)
        {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(const T& value, FormatContext& ctx)
        {
            format_to(ctx.out(), "{{");
            if (!value.empty())
            {
                auto it = std::begin(value);;
                format_to(ctx.out(), "{}: {}", it->first, it->second);
                ++it;
                while (it != std::end(value))
                {
                    format_to(ctx.out(), ", {}: {}", it->first, it->second);
                    ++it;
                }
            }
            return format_to(ctx.out(), "}}");
        }
    };

    template<typename T>
    requires
        iterable<T> &&
        (!std::constructible_from<T, const char*>) &&
        (!associative_container<T>)
    struct formatter<T>
    {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx)
        {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(const T& value, FormatContext& ctx)
        {
            format_to(ctx.out(), "[");
            if (!value.empty())
            {
                auto it = std::begin(value);;
                format_to(ctx.out(), "{}", *it++);
                while (it != std::end(value))
                    format_to(ctx.out(), ", {}", *it++);
            }
            return format_to(ctx.out(), "]");
        }
    };
}

#endif
