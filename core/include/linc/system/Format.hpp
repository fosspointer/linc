#pragma once
#include <algorithm>
#include <cfloat>
#include <cstdint>
#include <cstdio>
#include <string>
#include <linc/system/Containers.hpp>

namespace linc
{
    class Format final
    {
    public:
        Format() = delete;
        template <typename Sink>
        class Appender final
        {
        public:
            Appender(Sink&& sink)
                :m_sink(std::forward<Sink>(sink))
            {}
            using SinkReturnType = decltype(std::declval<Sink&&>()(std::declval<std::string_view>()));
            inline void append(std::string_view string)
            {
                if constexpr(requires{m_aggregateYield.append(std::declval<SinkReturnType>());})
                    m_aggregateYield.append(m_sink(string));
                else
                    m_aggregateYield += m_sink(string);
            }
            inline void append(char character)
            {
                if constexpr(requires{m_aggregateYield.push_back(std::declval<char>());})
                    m_aggregateYield.push_back(character);
                else
                    m_aggregateYield += m_sink(std::string{&character, 1ul});
            }
            const SinkReturnType& getAggregate() const { return m_aggregateYield; }
            SinkReturnType& getAggregate() { return m_aggregateYield; }
        private:
            SinkReturnType m_aggregateYield{};
            Sink m_sink;
        };

        static constexpr auto printSink = [](std::string_view string) { return std::fwrite(string.data(), 1ul, string.length(), stdout); };
        static constexpr auto stringSink = [](std::string_view string) { return std::string{string}; };
        static constexpr auto identitySink = [](std::string_view string) { return string; };

        struct FileSink final
        {
        public:
            FileSink(const char* filename, const char* mode) noexcept
                :m_file{std::fopen(filename, mode)}, m_referenceCount(new std::size_t(1ul))
            {}

            FileSink(const FileSink& other) noexcept
                :m_file(other.m_file), m_referenceCount(other.m_referenceCount)
            {
                ++(*m_referenceCount);
            }
            
            FileSink(FileSink&& other) noexcept
                :m_file(other.m_file), m_referenceCount(other.m_referenceCount)
            {
                other.m_file = nullptr;
                other.m_referenceCount = nullptr;
            }

            FileSink& operator=(const FileSink& other) noexcept
            {
                if(m_referenceCount)
                    --(*m_referenceCount);
                m_file = other.m_file;
                m_referenceCount = other.m_referenceCount;
                ++(*m_referenceCount);
                return *this;
            }
            
            FileSink& operator=(FileSink&& other) noexcept
            {
                if(m_referenceCount)
                    --(*m_referenceCount);
                m_file = other.m_file;
                m_referenceCount = other.m_referenceCount;
                other.m_file = nullptr;
                other.m_referenceCount = nullptr;
                return *this;
            }

            ~FileSink()
            {
                if(!m_referenceCount) return;

                --(*m_referenceCount);
                if(!(*m_referenceCount) && m_file)
                {
                    std::fclose(m_file);
                    delete m_referenceCount;
                }
            }

            std::size_t operator()(std::string_view string) const
            {
                return m_file? std::fwrite(string.data(), 1ul, string.length(), m_file): 0u;
            };
        private:
            FILE* m_file;
            std::size_t* m_referenceCount;
        };

        inline static void printSigned(std::int64_t number, std::uint8_t base)
        {
            formatSigned(number, base, printSink);
        }

        inline static void printUnsigned(std::uint64_t number, std::uint8_t base)
        {
            formatUnsigned(number, base, printSink);
        }

        inline static void printFloating(double number, std::uint8_t precision)
        {
            formatFloating(number, precision, printSink);
        }

        inline static void printBoolean(bool boolean, bool is_lexical)
        {
            formatBoolean(boolean, is_lexical, printSink);
        }

        inline static std::string signedToString(std::int64_t number, std::uint8_t base)
        {
            return formatSigned(number, base, stringSink);
        }

        inline static std::string unsignedToString(std::uint64_t number, std::uint8_t base)
        {
            return formatUnsigned(number, base, stringSink);
        }

        inline static std::string floatingToString(double number, std::uint8_t precision)
        {
            return formatFloating(number, precision, stringSink);
        }

        inline static std::string booleanToString(bool boolean, bool is_lexical)
        {
            return formatBoolean(boolean, is_lexical, stringSink);
        }

        template<typename Sink>
        static decltype(std::declval<Sink&&>()(std::declval<std::string_view>())) formatSigned(std::int64_t number, std::uint8_t base, Sink&& sink)
        {
            static char buffer[21ul];
            buffer[20ul] = '\0';

            std::size_t position{20ul};
            bool negative{false};
            if(number == 0l)
                return sink("0");
            else if(number < 0l)
            {
                number = -number;
                negative = true;
            }

            do
            {
                buffer[--position] = s_digitBuffer[number % base];
                number /= base;
            } while(number != 0l);
            if(negative)
                buffer[--position] = '-';
            
            return sink(buffer + position);
        }

        template <typename Sink>
        static decltype(std::declval<Sink&&>()(std::declval<std::string_view>())) formatUnsigned(std::uint64_t number, std::uint8_t base, Sink&& sink)
        {
            static char buffer[21ul];
            buffer[20ul] = '\0';

            std::size_t position{20ul};
            if(number == 0ul)
                return sink("0");

            do
            {
                buffer[--position] = s_digitBuffer[number % base];
                number /= base;
            } while(number != 0ul);

            return sink(buffer + position);
        }

        template<typename Sink>
        static decltype(std::declval<Sink&&>()(std::declval<std::string_view>())) formatFloating(double number, std::uint8_t precision, Sink&& sink)
        {
            static char buffer[32ul];
            buffer[31ul] = '\0';
            precision = std::min<std::uint8_t>(precision, 9u);
            static constinit double powers_ten[] = { 1.0, 10.0, 100.0, 1000.0, 10'000.0, 100'000.0, 1'000'000.0, 10'000'000.0, 100'000'000.0, 1'000'000'000.0 };

            if(number != number)
                return sink("NaN");
            else if(number < -DBL_MAX)
                return sink("-Inf");
            else if(number > DBL_MAX)
                return sink("+Inf");
            else if(number > 1e9 || number < -1e9)
                return formatSigned(static_cast<std::int64_t>(number), 10u, sink);
            
            bool is_negative{false};
            if(number < 0.0)
            {
                is_negative = true;
                number = 0.0 - number;
            }

            auto integral = static_cast<std::int64_t>(number);
            auto temporary = (number - static_cast<double>(integral)) * powers_ten[precision];
            auto fraction = static_cast<std::uint_least32_t>(temporary);
            auto difference = temporary - static_cast<double>(fraction);

            std::size_t position{31ul}; // Initial position is set at the null terminator

            if(difference >= 0.5) // Round past the precision cutoff
            {
                ++fraction;
                if(fraction >= powers_ten[precision])
                {
                    fraction = 0.0;
                    ++integral;
                }
            }

            if(precision == 0u) // Round the digits place if precision is 0
            {
                difference = number - static_cast<double>(integral);
                if(difference >= 0.5) ++integral;
            }
            else
            {
                std::uint8_t count{precision};
                do
                {
                    --count;
                    buffer[--position] = '0' + static_cast<char>(fraction % 10u);
                } while(fraction /= 10u);

                while(position != 0ul && count-- != 0ul)
                    buffer[--position] = '0';

                buffer[--position] = '.';
            }

            do
            {
                buffer[--position] = '0' + static_cast<char>(integral % 10ul);
            } while(integral /= 10ul);
            if(is_negative) buffer[--position] = '-';

            return sink(buffer + position);
        }

        template<typename Sink>
        static decltype(std::declval<Sink&&>()(std::declval<std::string_view>())) formatBoolean(bool boolean, bool is_lexical, Sink&& sink)
        {
            if(is_lexical) [[likely]]
                return sink(boolean? "true": "false");
            else
                return sink(boolean? "1": "0");
        }
    private:
        static constexpr const char* s_digitBuffer = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    };
}
