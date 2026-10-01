#include "argparser.h"
#include "type.h"

#include <string>
#include <iostream>

bool StrFromOther(const std::string& str, const std::string& other){
    for (int i = 0; i < str.size(); ++i){
        if (other.find(str[i]) == std::string::npos) { return false; }
    }
    return true;
}

void Flag::PrintHelp(){
    std::cout << short_flag_ << " " << long_flag_ << " " << description_ << '\n';
}

std::string Flag::GetLongArg(){
    return long_flag_;
}

std::string Flag::GetShortArg(){
    return short_flag_;
}


NamedArgument::NamedArgument (const NamedArgument& other) : 
    cur_type_(other.cur_type_), 
    long_arg_(other.long_arg_), 
    short_arg_(other.short_arg_), 
    value(other.value), 
    mode_(other.mode_), 
    description_(other.description_), 
    condition_(other.condition_),
    condition_description_(other.condition_description_),
    values_()
    {
        values_.resize(other.values_.size());
        if (other.cur_type_ == type::kIntType) {
            for (int i = 0; i < values_.size(); ++i){
                values_[i] = new int;
                *(int*)values_[i] = *(int*)other.values_[i];
            }
        }
        else if (other.cur_type_ == type::kFloatType) {
            for (int i = 0; i < values_.size(); ++i){
                values_[i] = new float;
                *(float*)values_[i] = *(float*)other.values_[i];
            }
        }
        else {
            for (int i = 0; i < values_.size(); ++i){
                values_[i] = new std::string;
                *(std::string*)values_[i] = *(std::string*)other.values_[i];
            }
        }
    }

NamedArgument& NamedArgument::operator= (const NamedArgument& other){
    if (this == &other) return *this;
    cur_type_ = other.cur_type_, 
    long_arg_ = other.long_arg_, 
    short_arg_ = other.short_arg_, 
    value = other.value, 
    mode_ = other.mode_, 
    description_ = other.description_, 
    condition_ = other.condition_,
    condition_description_ = other.condition_description_;

    for (void*& obj : values_){
        if (cur_type_ == type::kIntType) delete (int*)obj;
        else if (cur_type_ == type::kFloatType) delete (float*)obj;
        else delete (std::string*)obj;
    }

    values_.resize(other.values_.size());
    if (other.cur_type_ == type::kIntType) {
        for (int i = 0; i < values_.size(); ++i){
            values_[i] = new int;
            *(int*)values_[i] = *(int*)other.values_[i];
        }
    }
    else if (other.cur_type_ == type::kFloatType) {
        for (int i = 0; i < values_.size(); ++i){
            values_[i] = new float;
            *(float*)values_[i] = *(float*)other.values_[i];
        }
    }
    else {
        for (int i = 0; i < values_.size(); ++i){
            values_[i] = new std::string;
            *(std::string*)values_[i] = *(std::string*)other.values_[i];
        }
    }

    return *this;
}

NamedArgument::~NamedArgument(){
    for (void*& obj : values_){
        if (cur_type_ == type::kIntType) delete (int*)obj;
        else if (cur_type_ == type::kFloatType) delete (float*)obj;
        else delete (std::string*)obj;
    }
}


void NamedArgument::PrintHelp(){
    std::cout << short_arg_ << " " << long_arg_ << " " << description_ << '\n';
}

bool NamedArgument::CheckArgument(const std::string& description, int index, void* value){
    if (description == description_ && index < values_.size()){
        if (cur_type_ == type::kIntType){
            *(int*)value = *(int*)values_[index];
        }

        if (cur_type_ == type::kFloatType){
            *(float*)value = *(float*)values_[index];
        }

        if (cur_type_ == type::kStrType){
            *(std::string*)value = *(std::string*)values_[index];
        }
        return true;
    }
    return false;
}

std::string NamedArgument::GetDescription(){
    return description_;
}

int NamedArgument::GetValuesSize(){
    return values_.size();
}

int NamedArgument::GetMode(){
    return mode_;
}

type NamedArgument::GetCurType(){
    return cur_type_;
}

std::string NamedArgument::GetLongArg(){
    return long_arg_;
}

std::string NamedArgument::GetShortArg(){
    return short_arg_;
}


bool NamedArgument::IsSetFirstValue(){
    return is_set_first_value_;
}

template <typename T1> //int/float/string
void NamedArgument::Process(const T1& value, TypeOfErrors& error){
    if (condition_ && !((bool (*)(const T1&))condition_)(value)) {error = kNoSatisfyCondition;}
    if (is_set_first_value_ == false){
        *(static_cast<T1*>(this->value)) = value;
        is_set_first_value_ = true;
    }
    T1* value_ptr = new T1;
    *value_ptr = value;
    values_.push_back(value_ptr);
}

void NamedArgument::DefinitionNamedArg(std::string cur_str, TypeOfErrors& error){
    int len = cur_str.size();

    if (values_.size() == 1 && mode_ == kNwithoutModificator){
        error = kRedefinitionOfArgument;
        return;
    }

    if (cur_type_ == type::kIntType){
        if (!StrFromOther(cur_str, "0123456789")) {
            error = TypeOfErrors::kInvalidTypeOfArgument;
            return;
        }
        int value = std::stoi(cur_str);
        Process(value, error);
    }
    else if (cur_type_ == type::kFloatType){
        if (!StrFromOther(cur_str, "0123456789.")) {
            error = TypeOfErrors::kInvalidTypeOfArgument;
            return;
        }
        float value = std::stof(cur_str);
        Process(value, error);
    }
    else{
        Process(cur_str, error);
    }

}

PositionalArgument::PositionalArgument (const PositionalArgument& other) : 
    cur_type_(other.cur_type_), 
    value(other.value), 
    mode_(other.mode_), 
    description_(other.description_), 
    condition_(other.condition_),
    condition_description_(other.condition_description_),
    values_()
    {
        values_.resize(other.values_.size());
        if (other.cur_type_ == type::kIntType) {
            for (int i = 0; i < values_.size(); ++i){
                values_[i] = new int;
                *(int*)values_[i] = *(int*)other.values_[i];
            }
        }
        else if (other.cur_type_ == type::kFloatType) {
            for (int i = 0; i < values_.size(); ++i){
                values_[i] = new float;
                *(float*)values_[i] = *(float*)other.values_[i];
            }
        }
        else {
            for (int i = 0; i < values_.size(); ++i){
                values_[i] = new std::string;
                *(std::string*)values_[i] = *(std::string*)other.values_[i];
            }
        }
    }

PositionalArgument& PositionalArgument::operator= (const PositionalArgument& other){
    if (this == &other) return *this;
    cur_type_ = other.cur_type_, 
    value = other.value, 
    mode_ = other.mode_, 
    description_ = other.description_, 
    condition_ = other.condition_,
    condition_description_ = other.condition_description_;

    for (void*& obj : values_){
        if (cur_type_ == type::kIntType) delete (int*)obj;
        else if (cur_type_ == type::kFloatType) delete (float*)obj;
        else delete (std::string*)obj;
    }

    values_.resize(other.values_.size());
    if (other.cur_type_ == type::kIntType) {
        for (int i = 0; i < values_.size(); ++i){
            values_[i] = new int;
            *(int*)values_[i] = *(int*)other.values_[i];
        }
    }
    else if (other.cur_type_ == type::kFloatType) {
        for (int i = 0; i < values_.size(); ++i){
            values_[i] = new float;
            *(float*)values_[i] = *(float*)other.values_[i];
        }
    }
    else {
        for (int i = 0; i < values_.size(); ++i){
            values_[i] = new std::string;
            *(std::string*)values_[i] = *(std::string*)other.values_[i];
        }
    }

    return *this;
}


PositionalArgument::~PositionalArgument(){
    for (void*& obj : values_){
        if (cur_type_ == type::kIntType) delete (int*)obj;
        else if (cur_type_ == type::kFloatType) delete (float*)obj;
        else delete (std::string*)obj;
    }
}

void PositionalArgument::PrintHelp(){
    std::cout << description_ << '\n';
}

bool PositionalArgument::CheckArgument(const std::string& description, int index, void* value){
    if (description == description_ && index < values_.size()){
        if (cur_type_ == type::kIntType){
            *(int*)value = *(int*)values_[index];
        }

        if (cur_type_ == type::kFloatType){
            *(float*)value = *(float*)values_[index];
        }

        if (cur_type_ == type::kStrType){
            *(std::string*)value = *(std::string*)values_[index];
        }
        return true;
    }
    return false;
}

std::string PositionalArgument::GetDescription(){
    return description_;
}

int PositionalArgument::GetValuesSize(){
    return values_.size();
}

int PositionalArgument::GetMode(){
    return mode_;
}

type PositionalArgument::GetCurType(){
    return cur_type_;
}

bool PositionalArgument::IsSetFirstValue(){
    return is_set_first_value_;
}

template <typename T1> //int/float/string
void PositionalArgument::Process(const T1& value, TypeOfErrors& error){
    if (condition_ && !((bool (*)(const T1&))condition_)(value)) {error = kNoSatisfyCondition;}
    if (is_set_first_value_ == false){
        *(static_cast<T1*>(this->value)) = value;
        is_set_first_value_ = true;
    }
    T1* value_ptr = new T1;
    *value_ptr = value;
    values_.push_back(value_ptr);
}

bool PositionalArgument::DefinitionPosArg(std::string cur_str, TypeOfErrors& error){

    int len = cur_str.size();
    
    if (cur_type_ == type::kIntType){
        if (!StrFromOther(cur_str, "0123456789")) {
            return false;
        }
        int value = std::stoi(cur_str);
        Process(value, error);
    }
    else if (cur_type_ == type::kFloatType){
        if (!StrFromOther(cur_str, "0123456789.")) {
            return false;
        }
        float value = std::stof(cur_str);
        Process(value, error);
    }
    else{
        Process(cur_str, error);
    }


    return true;
}