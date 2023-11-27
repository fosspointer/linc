#pragma once
#include <linc/system/TextSpan.hpp>

namespace linc
{
    class Reporting final
    {
    public:
        using Type = Logger::Type;
        enum class Stage
        {
            Lexer, AST, ABT, Parser, Generator
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

        static std::string stageToString(Stage stage)
        {
            switch(stage)
            {
            case Stage::Lexer:     return "\e[4;30mLEXER\e[0m"; 
            case Stage::AST:       return "\e[4;30mAST\e[0m"; 
            case Stage::ABT:       return "\e[4;30mABT\e[0m"; 
            case Stage::Parser:    return "\e[4;30mPARSER\e[0m";
            case Stage::Generator: return "\e[4;30mGENERATOR\e[0m"; 
            default:
                throw LINC_EXCEPTION_OUT_OF_BOUNDS(Reporting::Stage);
            }
        }

        static void push(const Report& report, bool log = true)
        {
            s_reports.push_back(report);
            if(log) [[likely]] 
            {
                if(report.span.text.empty())
                    Logger::log(report.type, "[$] $", stageToString(report.stage), report.message);
                else
                    Logger::log(report.type, "[$] $\n in ``", stageToString(report.stage), report.message, report.span.get());
            }
        }

        static void clearReports()
        {
            s_reports.clear();
        }

        static const ReportList& getReports() { return s_reports; }
    
    private:
        static ReportList s_reports;
    };
}