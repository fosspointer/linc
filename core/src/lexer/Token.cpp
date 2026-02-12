#include <linc/lexer/Token.hpp>

namespace linc
{
    std::string_view Token::kindToString(Token::Kind kind)
    {
        switch(kind)
        {
        case Kind::EndOfFile: return "End of File";
        case Kind::FormatStringStart: return "I-String Start";
        case Kind::FormatStringEnd: return "I-String End";
        case Kind::FormatStringExpressionStart: return "F-String Expression Start";
        case Kind::FormatStringExpressionEnd: return "E-String Expression End";
        case Kind::KeywordReturn: return "Return Keyword";
        case Kind::KeywordFunction: return "Function Keyword";
        case Kind::KeywordIf: return "If Keyword";
        case Kind::KeywordElse: return "Else Keyword";
        case Kind::KeywordWhile: return "While Keyword";
        case Kind::KeywordMutability: return "Mutability Keyword";
        case Kind::KeywordAs: return "As Keyword";
        case Kind::KeywordFor: return "For Keyword";
        case Kind::KeywordIn: return "In Keyword";
        case Kind::KeywordBreak: return "Break Keyword";
        case Kind::KeywordContinue: return "Continue Keyword";
        case Kind::KeywordRecord: return "Record";
        case Kind::KeywordCase: return " KeywordCase";
        case Kind::KeywordChoice: return "Choice Keyword";
        case Kind::KeywordGeneric: return "Generic Keyword";
        case Kind::KeywordAlias: return "Alias Keyword";
        case Kind::KeywordNamespace: return "Namespace Keyword";
        case Kind::KeywordDefault: return "Default Keyword";
        case Kind::KeywordFinally: return "Finally Keyword";
        case Kind::Tilde: return "Tilde";
        case Kind::Bang: return "Bang";
        case Kind::At: return "At";
        case Kind::Pound: return "Pound";
        case Kind::Percent: return "Percent";
        case Kind::Caret: return "Caret";
        case Kind::Ampersand: return "Ampersand";
        case Kind::Asterisk: return "Asterisk";
        case Kind::ParenthesisLeft: return "Left Parenthesis";
        case Kind::ParenthesisRight: return "Right Parenthesis";
        case Kind::Minus: return "Minus";
        case Kind::Equals: return "Equals";
        case Kind::Plus: return "Plus";
        case Kind::SquareLeft: return "Left Squared Bracket";
        case Kind::BraceLeft: return "Left Brace";
        case Kind::SquareRight: return "Right Squared Bracket";
        case Kind::BraceRight: return "Right Brace";
        case Kind::Semicolon: return "Semicolon";
        case Kind::Colon: return "Colon";
        case Kind::Bar: return "Bar";
        case Kind::Comma: return "Comma";
        case Kind::AngledLeft: return "Left Angled Bracket";
        case Kind::Dot: return "Dot";
        case Kind::AngledRight: return "Right Angled Bracket";
        case Kind::Slash: return "Slash";
        case Kind::DoubleBang: return "Double Bang";
        case Kind::DoublePound: return "Double Pound";
        case Kind::DoubleAmpersand: return "Double Ampersand";
        case Kind::DoubleMinus: return "Double Minus";
        case Kind::DoubleEquals: return "Double Equals";
        case Kind::DoublePlus: return "Double Plus";
        case Kind::DoubleColon: return "Double Colon";
        case Kind::DoubleBar: return "Double Bar";
        case Kind::DoubleAngledLeft: return "Double Angled Left Bracket";
        case Kind::DoubleDot: return "Double Dot";
        case Kind::DoubleAngledRight: return "Double Angled Right Bracket";
        case Kind::BangEquals: return "Bang Equals";
        case Kind::AsteriskEquals: return "Asterisk Equals";
        case Kind::MinusEquals: return "Minus Equals";
        case Kind::PlusEquals: return "Plus Equals";
        case Kind::ColonEquals: return "Colon Equals";
        case Kind::SlashEquals: return "Slash Equals";
        case Kind::LiteralNumber: return "Number Literal";
        case Kind::LiteralCharacter: return "Character Literal";
        case Kind::LiteralString: return "String Literal";
        case Kind::LiteralTrue: return "True Literal";
        case Kind::LiteralFalse: return "False Literal";
        case Kind::Identifier: return "Identifier";
        }
    }
}
