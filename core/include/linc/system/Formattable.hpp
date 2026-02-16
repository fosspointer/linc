#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <linc/system/Format.hpp>
#include <linc/system/Arena.hpp>

namespace linc
{
    template<typename T>
    concept HasToStringMethod = std::is_convertible<decltype(std::declval<T>().toString()), std::string>::value;

    template<typename T>
    concept HasCStringMethod = std::is_convertible<decltype(std::declval<T>().c_str()), const char*>::value;

    template<typename T>
    concept Integral = std::is_integral_v<T>;

    struct FormatOptions final
    {
        bool lexicalBool = true;
        std::size_t floatingPrecision = 6ul;
        std::uint8_t numericBase = 10ul;
    };

    class Formattable final
    {
    public:
        template<typename Sink>
        decltype(std::declval<Sink&&>()(std::declval<std::string_view>())) formatSink(Sink&& sink, const FormatOptions& options = FormatOptions{}) const
        {
            switch(m_kind)
            {
            case Kind::Unsigned:
                return Format::formatUnsigned(m_unsigned, options.numericBase, std::forward<Sink>(sink));
            case Kind::Signed:
                return Format::formatSigned(m_signed, options.numericBase, std::forward<Sink>(sink));
            case Kind::Boolean:
                return Format::formatBoolean(m_boolean, options.lexicalBool, std::forward<Sink>(sink));
            case Kind::Floating:
                return Format::formatFloating(m_floating, options.floatingPrecision, std::forward<Sink>(sink));
            case Kind::String:
                return sink(m_string);
            case Kind::Character:
                return sink(std::string_view{&m_character, 1ul});
            case Kind::Pointer:
                return Format::formatUnsigned(std::bit_cast<std::uint64_t>(m_pointer), 16ul, std::forward<Sink>(sink));
            default:
                throwKindOutOfBounds();
                return Format::formatUnsigned(0, 0ul, std::forward<Sink>(sink));
            } 
        }
        void print(const FormatOptions& options = FormatOptions{}) const;
        std::string toString(const FormatOptions& options = FormatOptions{}) const;

        enum class Kind : std::uint_least8_t
        {
            Unsigned, Signed, Boolean, Floating, String, Character, Pointer
        };
        template<HasToStringMethod T>
        Formattable(const T& formattable)
            :m_kind(Kind::String)
        {
            new(&m_string) std::string_view{formattable.toString()};
        }

        template<HasCStringMethod T>
        Formattable(const T& formattable)
            :m_kind(Kind::String)
        {
            new(&m_string) std::string_view{formattable.c_str()};
        }

        Formattable(const char* string)
            :m_kind(Kind::String)
        {
            new(&m_string) std::string_view{string};
        }

        Formattable(std::string_view string)
            :m_kind(Kind::String)
        {
            new(&m_string) std::string_view{string};
        }

        Formattable(const std::string& string)
            :m_kind(Kind::String)
        {
            new(&m_string) std::string_view{string};
        }

        template<Integral INT>
        Formattable(INT integral)
        {
            if constexpr(std::is_signed_v<INT>)
            {
                m_kind = Kind::Signed;
                m_signed = integral;
            }
            else
            {
                m_kind = Kind::Unsigned;
                m_unsigned = integral;
            }
        }

        Formattable(bool boolean)
            :m_kind(Kind::Boolean), m_boolean(boolean)
        {}

        Formattable(float floating)
            :m_kind(Kind::Floating), m_floating(static_cast<double>(floating))
        {}

        Formattable(double floating)
            :m_kind(Kind::Floating), m_floating(floating)
        {}

        Formattable(char character)
            :m_kind(Kind::Character), m_character(character)
        {}

        template<typename T>
        Formattable(const T* pointer)
            :m_kind(Kind::Pointer), m_pointer(pointer)
        {}
    private:
        void throwKindOutOfBounds() const;
        Kind m_kind;
        union
        {
            std::uint64_t m_unsigned;
            std::int64_t m_signed;
            bool m_boolean;
            double m_floating;
            std::string_view m_string;
            char m_character;
            const void* m_pointer;
        };
    };
}
