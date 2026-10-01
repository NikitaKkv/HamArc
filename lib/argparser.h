#pragma once

#include "type.h"
#include <vector>

enum mode{
    kNwithoutModificator,
    kNargsZeroOrMore,
    kNargsOneOrMore,
    kNargsRequired,
    kNargsOptional
};

class ArgumentParser{
    public: 
    ArgumentParser (const std::string& name);

    int GetRepeatedCount(const std::string& description);

    bool GetRepeated(const std::string& description, int index, void* value);

    bool Parse(int argc, std::vector<std::string>argv);

    void AddFlag(const char* short_flag, const char* long_flag, bool* is_used, 
        const std::string& description, bool used = false);

    void AddHelp();

    void PrintHelp();


    void AddArgument(int* value, const std::string& description, 
        int mode = kNwithoutModificator, bool (*condition)(const int&) = nullptr, 
        const std::string& condition_description = "", type type = type::kIntType);
    
    void AddArgument(float* value, const std::string& description, 
        int mode = kNwithoutModificator, bool (*condition)(const float&) = nullptr, 
        const std::string& condition_description = "", type type = type::kFloatType);

    void AddArgument(std::string* value, const std::string& description, 
        int mode = kNwithoutModificator, bool (*condition)(const std::string&) = nullptr, 
        const std::string& condition_description = "", type type = type::kStrType);
    

    void AddArgument(const char* short_named_arg, const char* long_named_arg, int* value, 
        const std::string& named_arg_description, int mode = kNwithoutModificator, 
    bool (*condition)(const int&) = nullptr, const std::string& condition_description = "", type type = type::kIntType);

    void AddArgument(const char* short_named_arg, const char* long_named_arg, float* value, 
        const std::string& named_arg_description, int mode = kNwithoutModificator, 
        bool (*condition)(const float&) = nullptr, const std::string& condition_description = "", type type = type::kFloatType);

    void AddArgument(const char* short_named_arg, const char* long_named_arg, std::string* value, 
        const std::string& named_arg_description, int mode = kNwithoutModificator, 
        bool (*condition)(const std::string&) = nullptr, const std::string& condition_description = "", type type = type::kStrType);


private:
    bool help_;
    std::string name_;
    std::vector<Flag> flags_;
    std::vector<PositionalArgument> pos_args_;
    std::vector<NamedArgument>named_args_;    
    
    bool PotentialNamedArg(std::vector<std::string> argv, int& i, TypeOfErrors& error);

    bool PotentialFlag(const std::string& cur_str);

    bool PotentialPosArg(std::vector<std::string> argv, int& i, int argc, TypeOfErrors& error);
};
