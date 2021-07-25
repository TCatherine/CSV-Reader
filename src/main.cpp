#include "reader.h"

using c_file_t = std::unique_ptr<FILE, std::function<void(FILE*)>>;

std::string parse_arguments(int count, char** values){
    try {
        if (count != 2){
            throw "Error number argument!";
        }

        std::string path_to_file = values[1];
        c_file_t test_file{std::fopen(path_to_file.c_str(), "r"),[](FILE* f) { std::fclose(f); }};

        if (test_file.get()==nullptr){
            throw "Error open file!";
        }

        return path_to_file;
    }

    catch(const char* error_msg){
        std::cout << error_msg << std::endl;
        exit(0);
    }
}

std::ostream& operator<<(std::ostream& os, const CSVReader& csv)
{
    os << "\t";
    for (auto name : csv.m_vector_header)
        os << name << "\t"; 
    os << std::endl;

    for (auto line : csv.m_data){
        os << line.first << "\t";
        for (auto name : csv.m_vector_header)
            os << std::get<int>(line.second[name]) << "\t";
        os << std::endl;
    }
    
    return os;
}

int main(int argc, char** argv) {
    std::string path = parse_arguments(argc, argv);

    //std::string path = "../tests/valid_expression3.csv";
    CSVReader reader;

    reader.read_file(path);
    reader.calculate_expression();

    std::cout << reader;
    
    return 0;
}