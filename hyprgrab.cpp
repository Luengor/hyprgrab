#include <algorithm>
#include <array>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include "json.hpp"
using json = nlohmann::json;

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
    return result.substr(0, result.size() - 1);
}

enum RegionMode {
    OUTPUT,
    WINDOW,
    REGION
};

struct Args {
    bool video;
    RegionMode regionMode = OUTPUT;
    std::string region;
};

std::string read_arg(int i, int argc, char *argv[], bool lower = false) {
    if (i >= argc) error("Expected argument");
    auto str = std::string(argv[i]);
    if (!lower) return str;

    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return str;
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
                const auto arg = read_arg(++i, argc, argv, true);
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

std::string get_region(const Args &args) {
    // If region just throw it to slurp
    if (args.regionMode == REGION) {
        // Slurp
        return exec_command("slurp");
    }

    // If output, make slurp get it
    else if (args.regionMode == OUTPUT) {
        return exec_command("slurp -o -r");
    }

    // If not, get the monitors to get the active workspaces
    auto monitors = json::parse(exec_command("hyprctl monitors -j"));
    std::vector<int> workspaces;
    workspaces.reserve(monitors.size());

    for (const auto &monitor : monitors)
        workspaces.push_back(monitor["activeWorkspace"]["id"]);

    // Now, all the clients on those workspaces
    auto clients = json::parse(exec_command("hyprctl clients -j"));
    std::vector<std::string> boxes;
    for (auto &client : clients) {
        if (std::find(workspaces.begin(), workspaces.end(),
                      client["workspace"]["id"]) != workspaces.end()) {
            const auto &at = client["at"];
            const auto &size = client["size"];
            const std::string title = client["title"];
            boxes.push_back(std::format("{},{} {}x{} \"{}\"", (int)at[0], (int)at[1],
                                        (int)size[0], (int)size[1], title));
        }
    }

    std::string boxes_text = boxes[0];
    for (unsigned i = 1; i < boxes.size(); i++) {
        boxes_text += '\n' + boxes[i];
    }

    return exec_command(std::format("slurp -r <<< '{}'", boxes_text));
}

void screenshot(const Args &args) {
    exec_command(std::format("grim -g '{}' - | wl-copy", args.region));
}

void screencast(const Args &args) {
    error("TODO");
}

int main(int argc, char *argv[]) {
    // Get the args and region and things
    Args args = parse_args(argc, argv);
    args.region = get_region(args);

    if (args.region == "")
        return 0;

    // Execute the command
    if (args.video)
        screencast(args);
    else
        screenshot(args);
}

