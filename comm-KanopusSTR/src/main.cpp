#include <array>
#include <fstream>
#include <iostream>

static std::istream * initializingStream(const std::string_view in_file1_name, bool & need_delete)
{
    if ("-" == in_file1_name) {
        return &std::cin;
    }
    else {
        need_delete = true;
        return new std::fstream(in_file1_name.data());
    }
}

static void outputLine(const std::string & line, int spaces, bool option)
{
    if (!option) {
        while (spaces-- > 0) {
            std::cout << '\t';
        }
        std::cout << line << '\n';
    }
}

static void outputRemainingFile(std::istream & in_file, std::string & line, int spaces, bool option)
{
    outputLine(line, spaces, option);
    while (std::getline(in_file, line)) {
        outputLine(line, spaces, option);
    }
}

static void safeDelete(bool need_delete, std::istream * input)
{
    if (need_delete) {
        delete input;
    }
}

int main(int argc, char ** argv)
{
    std::array<bool, 3> options{};
    for (int i = 1; i < argc - 2; ++i) {
        for (char * k = argv[i]; *k != '\0'; ++k) {
            if (*k <= '3' && *k >= '1') {
                options[*k - '1'] = true;
            }
        }
    }

    std::pair<bool, bool> need_delete{};

    std::pair<std::istream *, std::istream *> istream;
    istream.first = initializingStream(argv[argc - 2], need_delete.first);
    istream.second = initializingStream(argv[argc - 1], need_delete.second);

    std::array<int, 3> column_spaces_count{};
    for (int i = 1; i < 3; ++i) {
        column_spaces_count[i] = column_spaces_count[i - 1] + (options[i - 1] ? 0 : 1);
    }

    std::pair<std::string, std::string> lines;
    std::getline(*istream.first, lines.first);
    std::getline(*istream.second, lines.second);

    std::pair<bool, bool> file_ended;
    file_ended.first = istream.first->eof();
    file_ended.second = istream.second->eof();

    while (!file_ended.first && !file_ended.second) {
        int comp_result = lines.first.compare(lines.second);
        if (comp_result <= 0) {
            if (comp_result == 0) {
                outputLine(lines.second, column_spaces_count[2], options[2]);
            }
            else {
                outputLine(lines.first, column_spaces_count[0], options[0]);
            }
            file_ended.first = !std::getline(*istream.first, lines.first);
        }
        if (comp_result >= 0) {
            if (comp_result > 0) {
                outputLine(lines.second, column_spaces_count[1], options[1]);
            }
            file_ended.second = !std::getline(*istream.second, lines.second);
        }
    }
    if (!file_ended.second) {
        outputRemainingFile(*istream.second, lines.second, column_spaces_count[1], options[1]);
    }
    else if (!file_ended.first) {
        outputRemainingFile(*istream.first, lines.first, column_spaces_count[0], options[0]);
    }
    safeDelete(need_delete.first, istream.first);
    safeDelete(need_delete.second, istream.second);
}
