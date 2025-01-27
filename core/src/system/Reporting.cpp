#include <linc/system/Reporting.hpp>
#include <linc/system/Colors.hpp>

namespace linc
{
    Reporting::ReportList Reporting::s_reports = {};
    Code::Source Reporting::s_source = {};
    bool Reporting::s_spansEnabled = true;


    std::string Reporting::stageToString(Stage stage){
        using namespace std::string_literals;
        const auto color = Colors::toANSI(Colors::Black | Colors::HighIntensity | Colors::Underline);
        switch(stage)
        {
        case Stage::Environment:  return color + "ENVIRONMENT"s + Colors::toANSI(Colors::Reset);
        case Stage::Lexer:        return color + "LEXER"s + Colors::toANSI(Colors::Reset);
        case Stage::Preprocessor: return color + "PREPROCESSOR"s + Colors::toANSI(Colors::Reset);
        case Stage::Parser:       return color + "PARSER"s + Colors::toANSI(Colors::Reset);
        case Stage::AST:          return color + "AST"s + Colors::toANSI(Colors::Reset);
        case Stage::ABT:          return color + "ABT"s + Colors::toANSI(Colors::Reset);
        case Stage::Generator:    return color + "GENERATOR"s + Colors::toANSI(Colors::Reset);
        default:
            throw LINC_EXCEPTION_OUT_OF_BOUNDS(stage);
        }
    }

    void Reporting::push(const Report& report, bool show_log)
    {
        s_reports.push_back(report);
        if(show_log) [[likely]] 
            Logger::log(report.type, "$", report);
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
