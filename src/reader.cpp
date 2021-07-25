#include "reader.h"

void CSVReader::read_file(const std::string& path) {
    /*  open file   */
    file_t file{new std::fstream(), [](std::fstream* file){file->close(); delete file;}};
    file.get()->open(path, std::fstream::in);

    /*  header parse */
    std::string line;
    getline(*file.get(), line);
    parse_header(line);

    /*  value parse  */
    while(getline(*file.get(), line)) {
        p_cell_t new_row = parse_line(line);
        m_data.insert(m_data.end(), new_row);
    }
}

void CSVReader::search_duplicate_header() {
    /*  to handle the dublicate header error 
            Ex: A,B,B,C                         */

    try {
        std::vector<std::string> copied_vector = m_vector_header;
        auto last = std::unique(copied_vector.begin(), copied_vector.end());
        if (last != copied_vector.end())
            throw "Error parse header [repeating headers]";
    }

    catch(const char* error_msg){
        std::cout << error_msg << std::endl;
        exit(0);
    }
}

std::string CSVReader::header_processing(std::string& line, size_t& pos_start, size_t& pos_end){
    try {
        std::string token = line.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + 1;

        if (token != ""  && m_header_number == 0)
            throw "The title must start with a separator";

        if (token == "" && m_header_number)
            throw "Title name must not be empty";
        
        if (std::any_of(token.begin(), token.end(), [](auto ch){return std::isdigit(ch); }))
            throw "There can be no numbers in the title";

        return token;
    }

     catch(const char* error_msg){
        std::cout << error_msg << std::endl;
        exit(0);
    }
}

void CSVReader::parse_header(std::string& line){
    size_t pos_start = 0, pos_end;
    while ((pos_end = line.find (separator, pos_start)) != std::string::npos) {
        std::string token = header_processing(line, pos_start, pos_end);
        
        if (m_header_number)
            m_vector_header.push_back (token);

        m_header_number++;
    }
    pos_end = line.length();
    std::string token = header_processing(line, pos_start, pos_end);
    m_vector_header.push_back(token);

    search_duplicate_header();
}

uint32_t CSVReader::get_index_line(std::string& line) {
    uint32_t index_line = 0;

    try {
        uint32_t pos_end = line.find(separator, 0);
        if (pos_end == std::string::npos || pos_end == 0) 
            throw "error line";

        std::string element = line.substr (0, pos_end);
        if (std::all_of(element.begin(), element.end(), [](auto ch){return std::isdigit(ch); }) ) {
            index_line = std::stoi(element);
        }
        else throw "index error";
        }

    catch(const char* error_msg) {
        std::cout << error_msg << std::endl;
        exit(0);
    }
    return index_line;
}

std::tuple<p_header_value_t, bool> CSVReader::value_processing(std::string& element, std::string& key) {
    try {
        if (element.empty())
            throw "value must not be empty";
        /*  check: is it a number  */
        if (std::all_of(element.begin(), element.end(), [](auto ch){return std::isdigit(ch) or ch == '-'; }) ) {
            int num = std::stoi(element);
            return std::tuple(p_header_value_t(key, num), true);
        }
    }

    catch(const char* error_msg) {
        std::cout << error_msg << std::endl;
        exit(0);
    }

    return std::tuple(p_header_value_t(key, element), false);
}

p_cell_t CSVReader::parse_line(std::string& line) {
    uint32_t index_line = get_index_line(line);

    header_value_t new_line;
    size_t number_parameters = 0;
    size_t pos_start = line.find (separator, 0) + 1, pos_end;
    std::tuple<p_header_value_t, bool> ret;

    try {
        /* split string into data */
        while ((pos_end = line.find (separator, pos_start)) != std::string::npos) {
            if (number_parameters == m_header_number)
                throw "number of values error"; 
            
            std::string element = line.substr (pos_start, pos_end - pos_start);
            pos_start = pos_end + 1;
            ret = value_processing(element, m_vector_header[number_parameters]);

            /*  If this is not a number, then we remember the address 
                of the expression into a vector */
            bool is_number =  std::get<1>(ret);
            if (is_number == false)
                m_expression.insert(m_expression.end(), std::pair<uint32_t, std::string>(index_line, m_vector_header[number_parameters]));

            new_line.insert(new_line.end(), std::get<0>(ret));
            number_parameters++;
        }
        if (number_parameters + 1 != m_header_number)
            throw "number of values error"; 

        std::string element = line.substr(pos_start, line.length() - pos_start);
        ret = value_processing(element, m_vector_header[number_parameters]);
        new_line.insert(new_line.end(), std::get<0>(ret));

        bool is_number =  std::get<1>(ret);
        if (is_number == false)
            m_expression.insert(m_expression.end(), std::pair<uint32_t, std::string>(index_line, m_vector_header[number_parameters]));
    }

    catch(const char* error_msg) {
        std::cout << error_msg << std::endl;
        exit(0);
    }

    return p_cell_t(index_line, new_line);
}

int32_t CSVReader::calculate( int32_t num_1,  int32_t num_2,  int32_t sign) {
    int32_t result = 0;
    try {
        switch (sign) {
            case '+':
                result = num_1 + num_2;
                break;

            case '-':
                result = num_1 - num_2;
                break;

            case '*':
                result = num_1 * num_2;
                break;

            case '/':
                if (num_2 == 0)
                    throw "division by zero";
                result = [num_1, num_2]()
                    {int result = num_1 / num_2;
                    if ((num_2 < 0 || num_1 < 0) && (num_1 % num_2))
                        result+=-1;
                    return result;}();

                break;
        }
    }
    catch(const char* error_msg) {
        std::cout << error_msg << std::endl;
        exit(0);
    }
    return result;
}

void CSVReader::calculate_expression(){

    auto find_sign = [](char ch) {
        char ret = 0;
        std::vector<char> signs{'+', '-', '*', '/'};
        for (auto sign: signs)
            if (sign == ch)
                return sign;
        return ret;
    };

    try {
        while (!m_expression.empty())
        for (iter idx = m_expression.begin(); idx !=m_expression.end(); ){
            std::string s_expr = std::get<1>(m_data[idx->first][idx->second]);
            if (s_expr[0]!='=')
                throw "expression must start with \"=\"";

            char sign = [s_expr, find_sign](){
                for (auto ch: s_expr)
                    if (find_sign(ch)!=0)
                        return ch;
                return static_cast<char>(0);}();

            if (sign == 0)
            throw "expression mush have operation signs";

            uint32_t idx_sep = s_expr.find(sign, 1);
            std::string term_1 = s_expr.substr(1, idx_sep - 1);
            std::string term_2 = s_expr.substr(idx_sep + 1, s_expr.length() - idx_sep - 1);
            int32_t num_1 = 0, num_2 = 0;
            if (get_value(term_1, num_1) == false or get_value(term_2, num_2) == false){
                idx++;
                //m_expression.insert(m_expression.end(),std::pair<uint32_t, std::string>(idx.first, idx.second));
                continue;
            }
            int32_t result = calculate(num_1, num_2, sign);
            
            m_data[idx->first][idx->second] = result;
            iter saved_iter = idx;
            saved_iter++;
            m_expression.erase(idx);
            idx = saved_iter;
        }
    }

    catch(const char* error_msg) {
        std::cout << error_msg << std::endl;
        exit(0);
    }
}

int CSVReader::get_number(std::string& s_number){
    int number = 0;
    try {
        if (std::all_of(s_number.begin(), s_number.end(), [](auto ch){return std::isdigit(ch); })==false ) 
            throw "error term";

        number = std::atoi(s_number.c_str());
    }

    catch(const char* error_msg){
        std::cout << error_msg << std::endl;
        exit(0);
    }

    return number;
}

bool CSVReader::get_value(std::string& term, int32_t& num){
    try {
    uint32_t idx = [term](){
        std::vector<char> digits{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
        uint32_t idx = term.length();
        uint32_t temp_idx = 0;
        for (auto i : digits)
            if ((temp_idx = term.find(i, 0))!=std::string::npos && temp_idx < idx )
                idx = temp_idx;
        return idx;}();

    if (idx == 0) {
        num = get_number(term);
        return true;
    }

    std::string name_head = term.substr(0, idx);
    if (std::any_of(m_vector_header.begin(), m_vector_header.end(), [name_head](std::string head){return name_head == head;})== false)
        throw "No such title";
    std::string s_num_line = term.substr(idx, term.length()-idx);
    int num_line = get_number(s_num_line);
    if (std::any_of(m_data.begin(), m_data.end(), [num_line](p_cell_t data){return num_line == data.first;})== false)
        throw "No such line";

    bool is_expression = std::any_of(m_expression.begin(), m_expression.end(), [num_line, name_head](std::pair<uint32_t, std::string> expr)
        { return expr.first == num_line && expr.second == name_head;});
    if (is_expression == false){
        num = std::get<0>(m_data[num_line][name_head]);
        return true;
        }
    }
    
    catch(const char* error_msg){
        std::cout << error_msg << std::endl;
        exit(0);
    }

    return false;
}