#include <array>
#include <iostream>
#include <memory>
#include <string>

std::string exec_command(const std::string &command);

void error(const std::string &msg);

int main(int argc, char *argv[]) {
    std::cout << exec_command(argv[1]) << std::endl;
}

void error(const std::string &msg) {
    std::cerr << msg << std::endl;
    std::exit(1);
}

std::string exec_command(const std::string &command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, void (*)(FILE *)> pipe(popen(command.c_str(), "r"),
                                                 [](FILE *f) -> void {
                                                     // wrapper to ignore the
                                                     // return value from
                                                     // pclose() is needed with
                                                     // newer versions of gnu
                                                     // g++
                                                     std::ignore = pclose(f);
                                                 });
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) !=
           nullptr) {
        result += buffer.data();
    }
    return result;
}
