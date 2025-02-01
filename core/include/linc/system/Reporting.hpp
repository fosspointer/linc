#pragma once
#include <linc/system/TextSpan.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class Reporting final
    {
    public:
        Reporting() = delete;

        using Type = Logger::Type;
        enum class Stage
        {
            Environment, Lexer, Preprocessor, Parser, AST, ABT, Generator, 
        };

        struct Report
        {
            Type type;
            Stage stage;
            TextSpan span{.lineStart = 0ul, .lineEnd = 0ul, .spanStart = 0ul, .spanEnd = 0ul};
            std::string message;
            bool isInvalid() const
            {
                return message == std::string{} || span == Report{}.span || span.lineStart == 0ul || span.lineStart == -1ul || span.lineEnd > span.lineStart || span.file == -1ul;
            }

            std::string toString() const
            {
                if(isInvalid() || !s_spansEnabled)
                    return Logger::format("$ $", stageToString(stage), message);
                else
                    return Logger::format("$ $\n $:#4in$:#3 `$`", stageToString(stage), message, span.get(s_source, type == Reporting::Type::Error? Colors::Red: Colors::Blue), Colors::pop(), Colors::push(Colors::Yellow));
            }
        };

        using ReportList = std::vector<Report>; 
        using ReportSize = ReportList::size_type; 

        [[nodiscard]] inline static const ReportList& getReports() { return s_reports; }
        inline static void setSource(Code::Source source_code) { s_source = source_code; }
        inline static void setSpansEnabled(bool option) { s_spansEnabled = option; }

        static void push(const Report& report, bool log = true);
        static void clearReports();
        static bool hasError();
        static bool hasWarning();
    private:
        static std::string stageToString(Stage stage);
        static Code::Source s_source;
        static ReportList s_reports;
        static bool s_spansEnabled;
    };
}