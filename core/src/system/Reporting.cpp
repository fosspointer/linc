#include <linc/system/Reporting.hpp>

namespace linc
{
    Reporting::ReportList Reporting::s_reports = {};

    std::string Reporting::stageToString(Stage stage){
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

    void Reporting::push(const Report& report, bool log)
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

    void Reporting::clearReports()
    {
        s_reports.clear();
    }
}