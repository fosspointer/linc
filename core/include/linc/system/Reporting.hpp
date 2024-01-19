#pragma once
#include <linc/system/TextSpan.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class Reporting final
    {
    public:
        using Type = Logger::Type;
        enum class Stage
        {
            Lexer, AST, ABT, Parser, Generator, Preprocessor
        };

        struct Report
        {
            Type type;
            Stage stage;
            TextSpan span;
            std::string message;
        };

        using ReportList = std::vector<Report>; 
        using ReportSize = ReportList::size_type; 

        static void push(const Report& report, bool log = true);
        static void clearReports();
        static const ReportList& getReports() { return s_reports; }
    private:
        static std::string stageToString(Stage stage);
        static ReportList s_reports;
    };
}