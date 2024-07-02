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
            TextSpan span{.lineIndex = -1ul, .spanStart = -1ul, .spanEnd = -1ul};
            std::string message;
        };

        using ReportList = std::vector<Report>; 
        using ReportSize = ReportList::size_type; 

        [[nodiscard]] inline static const ReportList& getReports() { return s_reports; }
        inline static void setSource(Code::Source source_code) { s_source = source_code; }
        
        static void push(const Report& report, bool log = true);
        static void clearReports();
        static bool hasError();
        static bool hasWarning();
    private:
        static std::string stageToString(Stage stage);
        static Code::Source s_source;
        static ReportList s_reports;
    };
}