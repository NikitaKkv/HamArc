#include <vector>
#include <string>
#include <fstream>
#include "../lib/argparser.h"

enum class WorkMode{
    kList,
    kExtract,
    kDelet,
    kGetBlock
};

enum class Errors{
    kOk,
    kUnopened,
    kNotExistArch,
    kNotExistFile,
    kInvalidSizeBlock,
    kInvalidFile,
    kInvalidOperations
};

class Coder{
public:
    Coder (int size = 8) : size_block_(size) {}

    void SetSizeBlock(int size);

    void AddFile (std::ofstream& out, std::string path, const std::string from = "");

    void CodeWord (std::ofstream& out, const std::string& word);

    void CodeByte (std::ofstream& out, char input_byte, bool is_last_byte = false);

private:
    int size_block_;
};

class Decoder{
public:
    Decoder (int size = 8) : size_block_(size) {}

    void SetSizeBlock (int size);

    void ProcessArchive (const std::string& file_name, std::vector<std::string>& files, WorkMode mode, 
                        std::string directory = "");

private:
    int size_block_;
};

class Archiver{
public:
    static const std::string kNameCreateFlag;
    static const std::string kNameListFlag;
    static const std::string kNameConcatenateFlag;
    static const std::string kNameExtractFlag;

    static const std::string kNameSizeArg;
    static const std::string kNameArchiveArg;
    static const std::string kNameAppendArg;
    static const std::string kNameDeleteArg;

    static const std::string kFilesArgs;

    Archiver (int argc, char** argv);
    void AddOptions ();
    void Run ();

private:
    std::ofstream out_;

    bool is_create_ = false;
    bool is_list_ = false;
    bool is_extract_ = false;
    bool is_concatenate_ = false;

    int size_of_block_for_coder_ = 256;

    std::string file_for_delete_ = "";
    std::string file_for_append_ = "";
    std::string archive_name_ = "";
    std::string first_file_ = "";

    Decoder decoder_;
    Coder coder_;
    ArgumentParser parser_;
    std::vector<std::string>argv_;

    void Create (std::ofstream& out);

    void List ();

    void Append (std::ofstream& out);

    void Extract ();

    void Concatenate (std::ofstream& out);

    void Delete(std::ofstream& out);

    Errors Validate();
};
