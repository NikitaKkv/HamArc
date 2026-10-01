#pragma once

#include <string>
#include <vector>
#include <iostream>

enum TypeOfErrors{
    kOk,
    kInvalidTypeOfArgument,
    kNoSatisfyCondition,
    kMissRequiredArguments,
    kRedefinitionOfArgument,
    kInvalidrguments
};

enum class type{
    kIntType,
    kFloatType,
    kStrType
};

class Flag{
public:
    bool* is_used; 

    Flag(std::string short_flag, std::string long_flag, bool* is_used, std::string description, bool used) : 
    long_flag_(long_flag), short_flag_(short_flag), description_(description), is_used(is_used) {
        if (used) {*is_used = true;}
    }

    std::string GetLongArg();

    std::string GetShortArg();

    void PrintHelp();

private:
    std::string long_flag_;
    std::string short_flag_;
    std::string description_;    
};

class NamedArgument{ // named argument

public:
    void* value;

    template <typename T> 
    NamedArgument (std::string long_arg, std::string short_arg, T* value, std::string description, 
        int mode, bool (*condition)(const T&), std::string condition_description, type type)  : 
    cur_type_(type), 
    long_arg_(long_arg), 
    short_arg_(short_arg), 
    value(value), 
    mode_(mode), 
    description_(description), 
    condition_((void*)condition),
    condition_description_(condition_description) {}

    NamedArgument (const NamedArgument& other);

    NamedArgument& operator= (const NamedArgument& other);

    void PrintHelp();

    bool CheckArgument(const std::string& description, int index, void* value);

    std::string GetDescription();

    int GetValuesSize();

    int GetMode();

    std::string GetLongArg();

    std::string GetShortArg();

    type GetCurType();
    
    bool IsSetFirstValue();

    void DefinitionNamedArg(std::string cur_str, TypeOfErrors& error);

    template <typename T1> //int/float/string
    void Process(const T1& value, TypeOfErrors& error);

    ~NamedArgument();

private:
    type cur_type_ = type::kStrType;

    std::string long_arg_;
    std::string short_arg_;
    std::string description_;
    std::string condition_description_;

    void* condition_;

    std::vector<void*>values_;

    int mode_;
    
    bool is_set_first_value_ = false;

};

class PositionalArgument{ //positional argument

public:
    void* value;

    template <typename T>
    PositionalArgument (T* value, std::string description, int mode, 
    bool (*condition)(const T&), std::string condition_description, type type) :  
    cur_type_(type), 
    value(value), 
    description_(description), 
    mode_(mode), 
    condition_((void*)condition),
    condition_description_(condition_description) {}  

    PositionalArgument (const PositionalArgument& other);

    PositionalArgument& operator= (const PositionalArgument& other);

    void PrintHelp();

    bool CheckArgument(const std::string& description, int index, void* values);

    std::string GetDescription();

    int GetValuesSize();

    int GetMode();

    type GetCurType();

    bool IsSetFirstValue();

    bool DefinitionPosArg(std::string cur_str, TypeOfErrors& error);

    template <typename T1> //int/float/string
    void Process(const T1& value, TypeOfErrors& error);

    ~PositionalArgument();

private:
    type cur_type_ = type::kStrType;
    
    std::string condition_description_;
    std::string description_;

    std::vector<void*>values_;

    void* condition_;

    int mode_;
  
    bool is_set_first_value_ = false;

};
