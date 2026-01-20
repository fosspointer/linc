#include <linc/system/Formattable.hpp>
#include <cstdint>
#include <linc/system/Format.hpp>
#include <linc/system/Exception.hpp>

namespace linc
{
    void Formattable::print(const FormatOptions& options) const
    {
        switch(m_kind)
        {
        case Kind::Unsigned:
            Format::printUnsigned(m_unsigned, options.numericBase);
            break;
        case Kind::Signed:
            Format::printSigned(m_signed, options.numericBase);
            break;
        case Kind::Boolean:
            Format::printBoolean(m_boolean, options.lexicalBool);
            break;
        case Kind::Floating:
            Format::printFloating(m_floating, options.floatingPrecision);
            break;
        case Kind::String:
            std::fwrite(m_string.data(), 1ul, m_string.length(), stdout);
            break;
        case Kind::Character:
            std::fputc(m_character, stdout);
            break;
        case Kind::Pointer:
            Format::printUnsigned(std::bit_cast<std::uint64_t>(m_pointer), 16ul);
            break;
        } 
    }

    String Formattable::toString(const FormatOptions& options) const
    {
        switch(m_kind)
        {
        case Kind::Unsigned:
            return Format::unsignedToString(m_unsigned, options.numericBase);
        case Kind::Signed:
            return Format::signedToString(m_signed, options.numericBase);
        case Kind::Boolean:
            return Format::booleanToString(m_boolean, options.lexicalBool);
        case Kind::Floating:
            return Format::floatingToString(m_floating, options.floatingPrecision);
        case Kind::String:
            return String{m_string};
        case Kind::Character:
            return String{1ul, m_character};
        case Kind::Pointer:
            return Format::unsignedToString(std::bit_cast<std::uint64_t>(m_pointer), 16ul);
        default:
            throw LINC_EXCEPTION_OUT_OF_BOUNDS(m_kind);
        }
    }

    void Formattable::throwKindOutOfBounds() const
    {
        throw LINC_EXCEPTION_OUT_OF_BOUNDS(m_kind);
    }
}
