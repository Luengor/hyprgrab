#include <array>
#include <format>
#include <iostream>
#include <memory>
#include <string>

const char USAGE[] = "Usage: hyprgrab screenshot/screencast [flags]";

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

enum RegionMode {
    OUTPUT,
    WINDOW,
    REGION
};

struct Args {
    bool video;
    RegionMode regionMode = OUTPUT;
};

std::string read_arg(int i, int argc, char *argv[]) {
    if (i >= argc) error("Expected argument");
    return std::string(argv[i]);
}

Args parse_args(int argc, char *argv[]) {
    Args args;

    // Get subcomand
    if (argc < 2) error(USAGE);
    
    const auto mode = std::string(argv[1]);
    if (mode == "screenshot") {
        args.video = false;
    } else if (mode == "screencast") {
        args.video = true;
    } else {
        error("Invalid mode. Valid values are: screenshot, screencast");
    }

    // Get flags
    for (int i = 2; i < argc; i++) {
        // All flags are -x
        const auto flag = std::string(argv[i]);
        if (flag.size() != 2 || flag[0] != '-')
            error(std::format("Unknown flag '{}'", flag));

        // Flags here
        switch (flag[1]) {
            case 'm': {
                // Read mode
                const auto arg = read_arg(++i, argc, argv);
                if (arg == "output") args.regionMode = OUTPUT;
                else if (arg == "window") args.regionMode = WINDOW;
                else if (arg == "region") args.regionMode = REGION;
                else error(std::format("Unknown region mode '{}'", arg));
                break;
            }

            default: error(std::format("Unknown flag '{}'", flag));
        }
    }

    return args;
}

int main(int argc, char *argv[]) {
    Args args = parse_args(argc, argv);
}


