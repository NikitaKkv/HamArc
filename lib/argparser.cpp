#include "argparser.h"
#include "type.h"

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>


ArgumentParser::ArgumentParser(const std::string& name) : name_(name) {}

bool ArgumentParser::PotentialFlag(const std::string& cur_str){
    for (int num_of_flag = 0; num_of_flag < flags_.size(); ++num_of_flag){
        if (cur_str == flags_[num_of_flag].GetLongArg() || cur_str == flags_[num_of_flag].GetShortArg() ){
            *flags_[num_of_flag].is_used = true;
            return true;
        }
    }
    return false;
}

bool ArgumentParser::PotentialNamedArg(std::vector<std::string> argv, int& i, TypeOfErrors& error){

    std::string cur_str = argv[i];
    for (int num_of_named_arg = 0; num_of_named_arg < named_args_.size(); ++num_of_named_arg){
        NamedArgument named_arg = named_args_[num_of_named_arg];
        const std::string& long_arg = named_arg.GetLongArg();

        if (argv[i] == long_arg || argv[i] == named_arg.GetShortArg()){
            ++i;

            named_args_[num_of_named_arg].DefinitionNamedArg(argv[i], error);
            return true;
            
        }else if (argv[i].size() > long_arg.size() & cur_str.find(long_arg + "=") == 0 ){
            std::string buffer = argv[i].substr(long_arg.size() + 1, argv[i].size() - long_arg.size() - 1); // + 1 т.к. =
            named_args_[num_of_named_arg].DefinitionNamedArg(buffer, error);
            return true;
        }
    }
    return false;
}

bool ArgumentParser::PotentialPosArg (std::vector<std::string> argv, int& i, int argc, TypeOfErrors& error){
    for (static int num_of_pos_arg = 0; num_of_pos_arg < pos_args_.size(); ++num_of_pos_arg){
        if (pos_args_[num_of_pos_arg].IsSetFirstValue() == false){
            if (!pos_args_[num_of_pos_arg].DefinitionPosArg(argv[i], error)) {return false;} //позиционный аргумент не может не существовать
            ++i;
            if (pos_args_[num_of_pos_arg].GetMode() == kNargsZeroOrMore || 
                pos_args_[num_of_pos_arg].GetMode() == kNargsOneOrMore){
                while (i < argc && pos_args_[num_of_pos_arg].DefinitionPosArg(argv[i], error)){
                    ++i;
                }
            }
            --i;
            return true;
        }
    }
    return false;

}



bool ArgumentParser::Parse(int argc, std::vector<std::string> argv){

    TypeOfErrors error = kOk;

    for (int i = 1; i < argc; ++i){

        if (help_ & (std::string)"--help" == argv[i] == 0) {
            PrintHelp();
        }
        else if(PotentialFlag(argv[i])) {}
        else if(PotentialNamedArg(argv, i, error)){}
        else if(PotentialPosArg(argv, i, argc, error)) {}
        else error = kInvalidrguments;
        
        if (error) {
            std::cout << error << '\n';
            return false;
        }

    }

    for (int i = 0; i < named_args_.size(); ++i){
        NamedArgument check = named_args_[i];
        if ((check.GetMode() == kNargsRequired || check.GetMode() == kNargsOneOrMore) & check.GetValuesSize() == 0){
                error = kMissRequiredArguments;
                std::cout << "named " << error << '\n';
                return false;
            }
    }

    for (int i = 0; i < pos_args_.size(); ++i){
        PositionalArgument check = pos_args_[i];
        if (check.GetMode() == kNargsOneOrMore & check.GetValuesSize() == 0){
                error = kMissRequiredArguments;
                std::cout << "pos " << error << '\n';
                return false;
            }
    }

    return true;

}

int ArgumentParser::GetRepeatedCount(const std::string& description){
    for (int i = 0; i < pos_args_.size(); ++i){
        if (description == pos_args_[i].GetDescription()){
            return pos_args_[i].GetValuesSize();
        }
    }
    for (int i = 0; i < named_args_.size(); ++i){
        if (description == named_args_[i].GetDescription()){
            return named_args_[i].GetValuesSize();
        }
    }
    return 0;
}

bool ArgumentParser::GetRepeated(const std::string& description, int index, void* value){
    for (int i = 0; i < named_args_.size(); ++i){
        if (named_args_[i].CheckArgument(description, index, value)){
            return true;
        }
    }

    for (int i = 0; i < pos_args_.size(); ++i){
        if (pos_args_[i].CheckArgument(description, index, value)){
            return true;
        }
    }

    return false;
}

void ArgumentParser::AddHelp(){
    help_ = "--help";
}

void ArgumentParser::PrintHelp(){
    std::cout << "Help: \n\n";
    std::cout << "Flags: \n";
    for (int i = 0; i < flags_.size(); i++){
        std::cout << i << " ";
        flags_[i].PrintHelp();
    }
    std::cout << "\nNamed Arguments: \n";
    for (int i = 0; i < named_args_.size(); i++){
        std::cout << i << " ";
        named_args_[i].PrintHelp();
    }
    std::cout << "\nPoistional Arguments: \n";
    for (int i = 0; i < pos_args_.size(); i++){
        std::cout << i << " ";
        pos_args_[i].PrintHelp();
    }
}

void ArgumentParser::AddFlag(const char* short_flag, const char* long_flag, bool* is_used, 
    const std::string& description, bool used){
    flags_.push_back(Flag(short_flag == nullptr ? "" : short_flag, long_flag == nullptr ? "" : long_flag, is_used, description, used));
}

//positional argument

void ArgumentParser::AddArgument(int* value, const std::string& description, int mode, bool (*condition)(const int&), 
const std::string& condition_description, type type){ 
    pos_args_.push_back(PositionalArgument(value, description, mode, condition, condition_description, type));
}

void ArgumentParser::AddArgument(float* value, const std::string& description, int mode, bool (*condition)(const float&), 
const std::string& condition_description, type type){ 
    pos_args_.push_back(PositionalArgument(value, description, mode, condition, condition_description, type));
}

void ArgumentParser::AddArgument(std::string* value, const std::string& description, int mode, bool (*condition)(const std::string&), 
const std::string& condition_description, type type){ 
    pos_args_.push_back(PositionalArgument(value, description, mode, condition, condition_description, type));
}

//named argument
void ArgumentParser::AddArgument(const char* short_arg, const char* long_arg, int* value, 
    const std::string& description, int mode, bool (*condition)(const int&), const std::string& condition_description, type type){
    named_args_.push_back(NamedArgument(long_arg == nullptr ? "" : long_arg, short_arg == nullptr ? "" : short_arg, value, 
    description, mode, condition, condition_description, type));
}

void ArgumentParser::AddArgument(const char* short_arg, const char* long_arg, float* value, 
    const std::string& description, int mode, bool (*condition)(const float&), const std::string& condition_description, type type){ 
    named_args_.push_back(NamedArgument(long_arg == nullptr ? "" : long_arg, short_arg == nullptr ? "" : short_arg, value, 
    description, mode, condition, condition_description, type));
}

void ArgumentParser::AddArgument(const char* short_arg, const char* long_arg, std::string* value, 
    const std::string& description, int mode, bool (*condition)(const std::string&), 
    const std::string& condition_description, type type){ 
    named_args_.push_back(NamedArgument(long_arg == nullptr ? "" : long_arg, short_arg == nullptr ? "" : short_arg, value, 
    description, mode, condition, condition_description, type));
}
