#include "archiver.h"
#include "../lib/argparser.h"

#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const std::string Archiver::kNameCreateFlag = "create";
const std::string Archiver::kNameListFlag = "list";
const std::string Archiver::kNameConcatenateFlag = "concatenate";
const std::string Archiver::kNameExtractFlag = "extract";

const std::string Archiver::kNameSizeArg = "size of block";
const std::string Archiver::kNameArchiveArg = "archive";
const std::string Archiver::kNameAppendArg = "append";
const std::string Archiver::kNameDeleteArg = "delete";

const std::string Archiver::kFilesArgs = "files";

Archiver::Archiver (int argc, char** argv) : argv_(argv, argv + argc), parser_("HamArc"), coder_(size_of_block_for_coder_) {}

int ReadHeader(std::ifstream& in);

void Archiver::AddOptions() {
    parser_.AddFlag("-c", "--create", &is_create_, kNameCreateFlag);
    parser_.AddFlag("-l", "--list", &is_list_, kNameListFlag);
    parser_.AddFlag("-A", "--concatenate", &is_concatenate_, kNameConcatenateFlag);
    parser_.AddFlag("-x", "--extract", &is_extract_, kNameExtractFlag);

    parser_.AddArgument("-s", "--size", &size_of_block_for_coder_, kNameSizeArg);
    parser_.AddArgument("-f", "--file", &archive_name_, kNameArchiveArg);
    parser_.AddArgument("-a", "--append", &file_for_append_, kNameAppendArg);
    parser_.AddArgument("-d", "--delete", &file_for_delete_, kNameDeleteArg);

    parser_.AddArgument(&first_file_, kFilesArgs, kNargsZeroOrMore);
}

void Archiver::Create(std::ofstream& out){

    coder_.CodeWord(out, std::to_string(size_of_block_for_coder_));
    coder_.CodeByte(out, 'A', true);        //archive header ------- ALWAYS 8/4
    out.flush();

    std::ifstream in(archive_name_);
    coder_.SetSizeBlock(ReadHeader(in));
    in.close();

    for (int i = 0; i < parser_.GetRepeatedCount(kFilesArgs); ++i){
        std::string value;
        parser_.GetRepeated(kFilesArgs, i, &value);
        coder_.AddFile(out, value);
    }
    out.flush();
}

void Archiver::List(){
    std::vector<std::string>list_all_files;
    decoder_.ProcessArchive(archive_name_, list_all_files, WorkMode::kList);
    for (const std::string& file : list_all_files){
        std::cout << "filename: " << file << std::endl;
    }
}

void Archiver::Append(std::ofstream& out){
    std::ifstream in(archive_name_);
    coder_.SetSizeBlock(ReadHeader(in));
    in.close();
    coder_.AddFile(out, file_for_append_);
}

void Archiver::Extract(){
    std::vector<std::string> extract_files;
    if (parser_.GetRepeatedCount(kFilesArgs)){
        for (int i = 0; i < parser_.GetRepeatedCount(kFilesArgs); ++i){
            std::string file;
            parser_.GetRepeated(kFilesArgs, i, &file);
            extract_files.push_back(file);
        }
    }
    decoder_.ProcessArchive(archive_name_, extract_files, WorkMode::kExtract);
}

void Archiver::Concatenate(std::ofstream& out){
    std::string archive1, archive2;
    parser_.GetRepeated(kFilesArgs, 0, &archive1);
    parser_.GetRepeated(kFilesArgs, 1, &archive2);

    coder_.CodeWord(out, std::to_string(size_of_block_for_coder_));
    coder_.CodeByte(out, 'A', true);       
    out.flush();

    std::ifstream in(archive_name_);
    coder_.SetSizeBlock(ReadHeader(in));
    in.close();

    std::string directory = "tmp" + std::to_string(rand());
    std::filesystem::create_directory(directory);

    directory += "/";

    std::vector<std::string> list;
    decoder_.ProcessArchive(archive1, list, WorkMode::kDelet, directory);
    decoder_.ProcessArchive(archive2, list, WorkMode::kDelet, directory);

    for (const std::string& file : list){
        coder_.AddFile(out, file, directory);
    }
    
    std::filesystem::remove_all(directory.substr(0, directory.size() - 1));
}

void Archiver::Delete(std::ofstream& out){
    std::string directory = "tmp" + std::to_string(rand());
    std::filesystem::create_directory(directory);

    directory += "/";

    std::ifstream in(archive_name_);
    int size_of_block = ReadHeader(in);
    in.close();

    std::vector<std::string> list;
    decoder_.ProcessArchive(archive_name_, list, WorkMode::kDelet, directory);

    out.close();
    out.open(archive_name_, std::ios::trunc | std::ios::binary);
    coder_.CodeWord(out, std::to_string(size_of_block));
    coder_.CodeByte(out, 'A', true);  
    out.flush();

    coder_.SetSizeBlock(size_of_block);

    for (const std::string& file : list){
        if (file != file_for_delete_){
            coder_.AddFile(out, file, directory);
        }
    }
    std::filesystem::remove_all(directory.substr(0, directory.size() - 1));
}



Errors Archiver::Validate(){
    if (parser_.GetRepeatedCount(kNameArchiveArg) == 0) { 
        std::cerr << "No archive selected for operation";
        return Errors::kUnopened; 
    }
    if ((is_create_ || is_concatenate_) && std::filesystem::exists(archive_name_)){
        std::cerr << "Selected arhive already exists";
        return Errors::kInvalidFile;
    }
    if (!is_create_ && !is_concatenate_ && !std::filesystem::exists(archive_name_)) { 
        std::cerr << "No exists selected archive";
        return Errors::kNotExistArch; 
    }
    if (archive_name_.substr(archive_name_.rfind('.') + 1) != "haf" || archive_name_.find('.') == std::string::npos){
        std::cerr << "Selected file is not archive";
        return Errors::kInvalidFile;
    }
    if (size_of_block_for_coder_ < 8) { 
        std::cerr << "Too small size of block";
        return Errors::kInvalidSizeBlock; 
    }
    if (is_create_ && is_concatenate_){
        std::cerr << "Cannot create and concatenate at the same time";
        return Errors::kInvalidOperations;
    }
    if (parser_.GetRepeatedCount(kNameAppendArg) && !std::filesystem::exists(file_for_append_)){
        std::cerr << "No exists selected file";
        return Errors::kNotExistFile; 
    }
    for (int i = 0; i < parser_.GetRepeatedCount(kFilesArgs); ++i){
        std::string value;
        parser_.GetRepeated(kFilesArgs, i, &value);
        if (!std::filesystem::exists(value)) { 
            std::cerr << "No exists selected file";
            return Errors::kNotExistFile; 
        }
    }
    return Errors::kOk;
}

void Archiver::Run(){
    std::srand(time(nullptr));
    parser_.Parse(argv_.size(), argv_);

    Errors valid = Validate();
    if (static_cast<int>(valid)) { return; }

    out_.open(archive_name_, std::ios::app | std::ios::binary);

    if (is_create_){
        Create(out_);
    }

    if (is_list_){
        List();
    }

    if (is_extract_){
        Extract();
    }

    if (is_concatenate_){
        Concatenate(out_);
    }

    if (parser_.GetRepeatedCount(kNameAppendArg)){
        Append(out_);
    }

    if (parser_.GetRepeatedCount(kNameDeleteArg)){
        Delete(out_);
    }

    out_.close();

}


void Coder::SetSizeBlock(int size){
    size_block_ = size;
}

void Coder::AddFile (std::ofstream& out, std::string path, const std::string from){

    path = from + path;

    std::ifstream in(path, std::ios::binary);

    path = path.substr(path.rfind('/') + 1);

    in.seekg(0, std::ios::end);
    int size_file = in.tellg();
    in.seekg(0, std::ios::beg);

    CodeWord(out, std::to_string(path.size()));
    CodeByte(out, 'A');
    CodeWord(out, path);
    CodeWord(out, std::to_string(size_file));
    if (size_file == 0){
        CodeByte(out, 'A', true);
    }
    else{
        CodeByte(out, 'A');
    }

    char buffer[100];
    
    while(true){
        in.read(buffer, 100);
        int bytes_read = in.gcount();
        for (char* byte = buffer; byte < buffer + bytes_read; ++byte){
            if (in.eof() && byte == buffer + bytes_read - 1){
                CodeByte(out, *byte, true);
            }
            else{
                CodeByte(out, *byte);
            }
        }
        if (bytes_read != 100){
            break;
        }
    }

    in.close();

}

void Coder::CodeWord(std::ofstream& out, const std::string& word){
    for (const char& el : word){
        CodeByte(out, el);
    }
}

int GetCountParityBit(int data_block_size){
    for (int i = 0; i < 100; ++i){
        if ((1ll << i) >= data_block_size){
            return i;
        }
    }
    return 1;
}

void SetupParityBits(std::ofstream& out, int count_parity_bits, int& parity_xor_result, int& bit_offset, 
    const unsigned char left_bound, int& current_bit_position, unsigned char& incomplete_byte, std::vector<unsigned char>& encoded_bytes, 
    int& extra_bit, bool is_last_byte = false){

    for (int parity_index = 0; parity_index < count_parity_bits; ++parity_index){
        int parity_bit_value = parity_xor_result & (1 << parity_index);
        if (parity_bit_value){
            extra_bit ^= 1;
            int parity_bit_position = 1 << parity_index;
            encoded_bytes[(bit_offset + parity_bit_position) >> 3] |= left_bound >> ((bit_offset + parity_bit_position) & 7);
        }
    }

    if (extra_bit){
        encoded_bytes[0] |= (left_bound >> bit_offset);
    }

    int num_incomplete_byte = (bit_offset + current_bit_position) >> 3;

    for (int i = 0; i < num_incomplete_byte; ++i){
        out << encoded_bytes[i];
        std::cout.flush();
    }

    if (is_last_byte){
        if (((bit_offset + current_bit_position) & 7) != 0)
            out << encoded_bytes[num_incomplete_byte];
        out.flush();
        return;
    }

    incomplete_byte = encoded_bytes[num_incomplete_byte];
    encoded_bytes.assign(encoded_bytes.size(), 0);
    encoded_bytes[0] = incomplete_byte;

    bit_offset = ((bit_offset + current_bit_position) & 7);
    current_bit_position = 3;
    parity_xor_result = 0;
    extra_bit = 0;
    
}

void Coder::CodeByte (std::ofstream& out, char input_byte, bool is_last_byte){

    static int check_size_block = size_block_;
    static unsigned char incomplete_byte = 0;
    static int bit_offset = 0;
    static int current_bit_position = 3;
    static int parity_xor_result = 0;
    static int extra_bit = 0;

    const static unsigned char left_bound = 1 << 7;

    static int count_parity_bits = GetCountParityBit(size_block_);
    static int need_byte = (size_block_ + 7) / 8 + 1;

    static std::vector<unsigned char>encoded_bytes(need_byte, 0);

    if (check_size_block != size_block_){
        check_size_block = size_block_;
        count_parity_bits = GetCountParityBit(size_block_);
        need_byte = (size_block_ + 7) / 8 + 1;
        encoded_bytes.assign(need_byte, 0);
    }

    for (int bit_input_byte = 0; bit_input_byte < 8; ++bit_input_byte){
        while (current_bit_position < size_block_ && (current_bit_position & (current_bit_position - 1)) == 0){
            ++current_bit_position;
        }

        if (current_bit_position == size_block_){
            SetupParityBits(out, count_parity_bits, parity_xor_result, bit_offset, left_bound, current_bit_position, 
                incomplete_byte, encoded_bytes, extra_bit);
        }

        int current_bit_value = input_byte & (left_bound >> bit_input_byte);

        if (current_bit_value){
            encoded_bytes[(bit_offset + current_bit_position) >> 3] |= left_bound >> ((bit_offset + current_bit_position) & 7);
            parity_xor_result ^= current_bit_position;
            extra_bit ^= 1;
        }
        
         ++current_bit_position;

        if (current_bit_position == size_block_){
            SetupParityBits(out, count_parity_bits, parity_xor_result, bit_offset, left_bound, current_bit_position, 
                incomplete_byte, encoded_bytes, extra_bit);
        }

    }

    if (is_last_byte){
        if (current_bit_position != 3){
            SetupParityBits(out, count_parity_bits, parity_xor_result, bit_offset, left_bound, size_block_, 
                            incomplete_byte, encoded_bytes, extra_bit, true);
        }
        else if(current_bit_position == 3 && bit_offset != 0){
            out << encoded_bytes[0];
            out.flush();
        }
        incomplete_byte = 0;
        bit_offset = 0;
        current_bit_position = 3;
        parity_xor_result = 0;
        extra_bit = 0;
        count_parity_bits = GetCountParityBit(size_block_);
        need_byte = (size_block_ + 7) / 8 + 1;
        encoded_bytes.assign(need_byte, 0);
        check_size_block = size_block_;
    }
}

void Decoder::SetSizeBlock(int size){
    size_block_ = size;
}

bool GetBit(const std::vector<unsigned char>& bytes, int idx){
    int byte_in_vector = idx >> 3;
    int bit_in_vector = idx & 7;
    const unsigned char left_bound = 1 << 7;
    return (left_bound >> bit_in_vector) & bytes[byte_in_vector];
}

void FlipBit(std::vector<unsigned char>& bytes, int idx){
    int byte_in_vector = idx >> 3;
    int bit_in_vector = idx & 7;
    const unsigned char left_bound = 1 << 7;
    bytes[byte_in_vector] ^= (left_bound >> bit_in_vector);
}

void CheckErrors(int size_block, int bit_offset, const unsigned char left_bound, std::vector<unsigned char>& encoded_bytes) {
    int xor_result = 0;
    int extra_bit = 0;
    for (int hamming_bit = 1; hamming_bit < size_block; ++hamming_bit){ 
        if (GetBit(encoded_bytes, hamming_bit + bit_offset)){
            extra_bit ^= 1;
            xor_result ^= hamming_bit;
        }
    }

    if (xor_result){
        if ((extra_bit ^ GetBit(encoded_bytes, bit_offset)) == 0){
            std::cout << "Detected 2-bit error: cannot correct\n";
            //exit(1);
        }
        else{
            FlipBit(encoded_bytes, bit_offset + xor_result);
        }
    }

    if (xor_result == 0 && (extra_bit ^ GetBit(encoded_bytes, bit_offset))){
        FlipBit(encoded_bytes, bit_offset);
    }
}

bool Decode256BlockHamm(int& current_bit, const unsigned char left_bound, unsigned char& byte, 
                        const std::vector<unsigned char>& encoded_bytes, std::string& size){
    for (int encoded_bit = 1; encoded_bit < 256; ++encoded_bit){
        if ((encoded_bit & (encoded_bit - 1)) != 0){
            int val = GetBit(encoded_bytes, encoded_bit);
            if (val){
                byte |= (left_bound >> current_bit);
            }
            ++current_bit;
        }
        if (current_bit == 8){
            if (byte == 'A'){
                return true;
            }
            size += byte;
            byte = 0;
            current_bit = 0;
        }
    }
    return false;
}

int ReadHeader(std::ifstream& in){
    std::string size = "";
    std::vector<unsigned char> encoded_bytes(32, 0);
    unsigned char byte = 0;
    const unsigned char left_bound = 1 << 7;
    int current_bit = 0;

    unsigned char* u_start = &encoded_bytes[0];
    char* start = reinterpret_cast<char*>(u_start);

    for (int i = 0; true; ++i) {
        in.read(start, encoded_bytes.size());

        CheckErrors(256, 0, left_bound, encoded_bytes);

        if (Decode256BlockHamm(current_bit, left_bound, byte, encoded_bytes, size)){
            break;
        }
    }
    return std::stoi(size);
}



void WorkEndHammigBlock(int& bit_offset, unsigned char& incomplete_byte, int& idx_curr_encod_byte, int bit_in_vector, 
                        std::vector<unsigned char>& encoded_bytes){
    bit_offset = (bit_in_vector + 1) & 7;
    incomplete_byte = encoded_bytes[idx_curr_encod_byte - 1];
    encoded_bytes.assign(encoded_bytes.size(), 0);
    encoded_bytes[0] = incomplete_byte;
    idx_curr_encod_byte = (bit_offset == 0 ? 0 : 1);
}

bool ProcessCompleteDataByte(bool& is_find_size, bool& is_find_file_name_in_archive, int& size, 
                            unsigned char& data_byte, std::string& str_size, std::string& file_name_in_archive, 
                            unsigned char& incomplete_byte, int& bit_offset, int& idx_curr_encod_byte,
                            int& cur_bit_pos_data_byte, std::vector<std::string>& files, WorkMode mode, bool& is_out,
                            std::ofstream& out, std::string directory){
    if (is_find_size){
        if (data_byte == 'A'){
            size = std::stoi(str_size);
            is_find_size = false;
            str_size = "";
            if (size == 0){ 
                is_find_file_name_in_archive = 1 - is_find_file_name_in_archive;
                is_find_size = true;
                incomplete_byte = 0;
                bit_offset = 0;
                idx_curr_encod_byte = 0;
                data_byte = 0;
                cur_bit_pos_data_byte = 0;
                is_out = false;
                out.close();
                return true;
            }
        }
        else{
            str_size += data_byte;
        }
    }
    else{
        if (is_find_file_name_in_archive){
            file_name_in_archive += data_byte;
        }
        if ((mode == WorkMode::kExtract || mode == WorkMode::kDelet) && !is_find_file_name_in_archive){
            if (is_out){
                out << data_byte;
            }
        }
        size--;
        if (size == 0){
            if (!is_find_file_name_in_archive){
                is_find_file_name_in_archive = 1 - is_find_file_name_in_archive;
                is_find_size = true;
                incomplete_byte = 0;
                bit_offset = 0;
                idx_curr_encod_byte = 0;
                data_byte = 0;
                cur_bit_pos_data_byte = 0;
                is_out = false;
                out.close();
                return true;
            }
            is_find_file_name_in_archive = !is_find_file_name_in_archive;
            is_find_size = true;
            if (mode == WorkMode::kList || mode == WorkMode::kDelet){
                files.push_back(file_name_in_archive);
            }
            if (mode == WorkMode::kExtract){
                if (find(files.begin(), files.end(), file_name_in_archive) != files.end() || files.empty()){
                    out.open(file_name_in_archive, std::ios::binary);
                    is_out = true;
                }
            }
            if (mode == WorkMode::kDelet){
                out.open(directory + file_name_in_archive, std::ios::binary);
                is_out = true;
            }
            file_name_in_archive = "";
        }
    }
    cur_bit_pos_data_byte = 0;
    data_byte = 0;
    return false;
}

void BuildDataByte(int hamming_bit, int byte_in_vector, const unsigned char left_bound, int bit_in_vector, unsigned char& data_byte, 
                    int& cur_bit_pos_data_byte, const std::vector<unsigned char>& encoded_bytes){
    if ((hamming_bit & (hamming_bit - 1)) != 0){
        int value_bit = encoded_bytes[byte_in_vector] & (left_bound >> bit_in_vector);
        if (value_bit){
            data_byte |= (left_bound >> cur_bit_pos_data_byte);
        }
        ++cur_bit_pos_data_byte;
    }
}

void DecodeHammingBlock(int& bit_offset, int size_block, const unsigned char left_bound, 
                        int& idx_curr_encod_byte, std::vector<unsigned char>& encoded_bytes,
                        std::vector<std::string>& files, int& cur_bit_pos_data_byte, bool& is_find_size, 
                        bool& is_find_file_name_in_archive, int& size, unsigned char& data_byte, 
                        std::string& file_name_in_archive, unsigned char& incomplete_byte, std::string& str_size, 
                        WorkMode mode, bool& is_out, std::ofstream& out, std::string directory){

    for (int hamming_bit = 1; hamming_bit < size_block; ++hamming_bit){
        int byte_in_vector = (bit_offset + hamming_bit) >> 3;
        int bit_in_vector = (bit_offset + hamming_bit) & 7;

        BuildDataByte(hamming_bit, byte_in_vector, left_bound, bit_in_vector, data_byte, cur_bit_pos_data_byte, encoded_bytes);

        if (cur_bit_pos_data_byte == 8){
            if (ProcessCompleteDataByte(is_find_size, is_find_file_name_in_archive, size, data_byte, str_size, 
                                    file_name_in_archive, incomplete_byte, bit_offset, idx_curr_encod_byte, 
                                    cur_bit_pos_data_byte, files, mode, is_out, out, directory))
                                    break;
        }

        if (hamming_bit == size_block - 1){
            WorkEndHammigBlock(bit_offset, incomplete_byte, idx_curr_encod_byte, bit_in_vector, encoded_bytes);
        }
    }
}

void Decoder::ProcessArchive(const std::string& file_name, std::vector<std::string>& files, WorkMode mode, 
                            std::string directory){
    std::ifstream in (file_name, std::ios::binary);
    SetSizeBlock(ReadHeader(in));

    std::ofstream out;
    int size;
    unsigned char incomplete_byte = 0;
    unsigned char data_byte = 0;
    bool is_find_size = true;
    bool is_find_file_name_in_archive = true;
    std::string str_size = "";
    std::string file_name_in_archive = "";
    int cur_bit_pos_data_byte = 0;
    int bit_offset = 0;
    int idx_curr_encod_byte = 0;
    int need_byte = (size_block_ + 7) / 8 + 1;
    bool is_out = false;
    
    unsigned char left_bound = 1 << 7;
    
    std::vector<unsigned char>encoded_bytes(need_byte, 0);

    char buffer[100];
    while (true){
        in.read(buffer, 100);
        int read_bytes = in.gcount();
        for (int i = 0; i < read_bytes; ++i){
            encoded_bytes[idx_curr_encod_byte] = buffer[i];
            idx_curr_encod_byte++;

            if (idx_curr_encod_byte * 8 - bit_offset >= size_block_){
                CheckErrors(size_block_, bit_offset, left_bound, encoded_bytes);
                DecodeHammingBlock(bit_offset, size_block_, left_bound, idx_curr_encod_byte, encoded_bytes,
                                   files, cur_bit_pos_data_byte, is_find_size, 
                                   is_find_file_name_in_archive, size, data_byte, file_name_in_archive,
                                   incomplete_byte, str_size, mode, is_out, out, directory);
            }
        }

        if (read_bytes != 100){
            break;
        }
    }

    out.close();
    in.close();
    
}