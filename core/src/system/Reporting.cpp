#include <linc/system/Reporting.hpp>

namespace linc
{
    Reporting::ReportList Reporting::s_reports = {};
    Code::Source Reporting::s_source = {};
    bool Reporting::s_spansEnabled = true;


    std::string Reporting::stageToString(Stage stage){
        const static auto format_string = "\x1B[4;90m$\x1B[0m";
        switch(stage)
        {
        case Stage::Environment:  return Logger::format(format_string, "ENVIRONMENT");
        case Stage::Lexer:        return Logger::format(format_string, "LEXER");
        case Stage::Preprocessor: return Logger::format(format_string, "PREPROCESSOR");
        case Stage::Parser:       return Logger::format(format_string, "PARSER");
        case Stage::AST:          return Logger::format(format_string, "AST");
        case Stage::ABT:          return Logger::format(format_string, "ABT");
        case Stage::Generator:    return Logger::format(format_string, "GENERATOR");
        default:
            throw LINC_EXCEPTION_OUT_OF_BOUNDS(stage);
        }
    }

    void Reporting::push(const Report& report, bool log)
    {
        s_reports.push_back(report);
        if(log) [[likely]] 
        {
            if(report.isInvalid() || !s_spansEnabled)
                Logger::log(report.type, "$ $", stageToString(report.stage), report.message);
            else
                Logger::log(report.type, "$ $\n $:#4in$:#3 `$`", stageToString(report.stage), report.message, report.span.get(s_source,
                    report.type == Reporting::Type::Error? Colors::Color::Red: Colors::Color::Blue), Colors::pop(), Colors::push(Colors::Color::Yellow));
        }
    }

    void Reporting::clearReports()
    {
        s_reports.clear();
    }

    bool Reporting::hasError()
    {
        for(const auto& report: s_reports)
            if(report.type == Type::Error)
                return true;

        return false;
    }
    
    bool Reporting::hasWarning()
    {
        for(const auto& report: s_reports)
            if(report.type == Type::Warning)
                return true;

        return false;
    }
}