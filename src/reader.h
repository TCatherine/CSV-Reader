#include <iostream>
#include <memory>
#include <functional>
#include <variant>
#include <map>
#include <fstream>
#include <algorithm>  
#include <tuple>

using file_t = std::unique_ptr<std::fstream, std::function<void(std::fstream*)>>;

using p_header_value_t = std::pair<std::string, std::variant<int32_t, std::string>>;
using header_value_t = std::map<std::string, std::variant<int32_t, std::string>>;

using p_cell_t = std::pair<uint32_t, header_value_t>;
using cell_t = std::map<uint32_t, header_value_t>;

using iter = std::multimap<uint32_t, std::string>::iterator;

class CSVReader {
    public:
        void read_file(const std::string& path);
        void calculate_expression();

        friend std::ostream& operator<<(std::ostream&, const CSVReader&);

    private:
            std::vector<std::string> m_vector_header;
            size_t m_header_number = 0;

            cell_t m_data;
            
            // keys to expressions
            std::multimap<uint32_t, std::string> m_expression;

            char separator = ',';

            // methods of the header processing
            void parse_header(std::string& line);
            std::string header_processing(std::string& line, size_t& pos_start, size_t& pos_end);
            void search_duplicate_header();
            bool is_correct_header();

            //methods of the line processing
            p_cell_t parse_line(std::string& line);
            uint32_t get_index_line(std::string& line);
            std::tuple<p_header_value_t, bool> value_processing(std::string& line, std::string& key);
            
            //methods for calculating expressions
            bool get_value(std::string& term, int32_t& num);
            int32_t get_number(std::string& s_number);
            int32_t calculate( int32_t term_1,  int32_t term_2,  int32_t sign);
};