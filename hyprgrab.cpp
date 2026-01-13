#include "json.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <unistd.h>
using json = nlohmann::json;

const char USAGE[] = "Usage: hyprgrab screenshot|screencast [options]";
const char HELP[] = R"(Usage: hyprgrab screenshot|screencast [options]

Hyprgrab is a small utility program to easily capture the screen in Hyprland.

It will use hyprctl and slurp for region selection, then grim or wl-screenrec
to take the screenshot or record the screen. The result file will be stored
with the name 'hyprgrab_{shot|cast}_{time}.{ext}' in the chosen folder.

When recording, a new terminal window will be opened with the title
"hyprgrab-recorder". I recomend adding some rules to make the window float
in your Hyprland config like this:
  windowrulev2 = float, title: ^(hyprgrab-recorder)$

Options:
  -h        show this help message
  -m        one of: output, window, region
  -o        directory in which to save result. ~/{Pictures|Videos} by default
  -s        seconds to sleep before taking the screenshot/recording
  -t        command to open a named terminal when recording. Default is
            'kitty --title "hyprgrab-recorder"'

Modes:
  output:   take screenshot of an entire monitor (default)
  window:   take screenshot of an open window
  region:   take screenshot of selected region
)";

void error(const std::string &msg) {
    std::cerr << msg << std::endl;
    std::exit(1);
}

std::string exec_command(const std::string &command) {
    static std::array<char, 4096> buffer;
    std::string result;
    std::unique_ptr<FILE, void (*)(FILE *)> pipe(
        popen(command.c_str(), "r"),
        [](FILE *f) -> void { std::ignore = pclose(f); });

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) !=
           nullptr) {
        result += buffer.data();
    }
    if (!result.empty() && result.back() == '\n')
        result.pop_back();
    return result;
}

void notify(const std::string &msg, int expire=0) {
    std::string cmd;
    if (expire > 0)
        cmd = std::format("notify-send -t {} 'Hyprgrab' '{}'", expire, msg);
    else 
        cmd = std::format("notify-send 'Hyprgrab' '{}'", msg);

    exec_command(cmd);
}

bool another_running() {
    // Get process list
    std::string cmd = std::format("pgrep -x hyprgrab");
    std::string output = exec_command(cmd);

    // Get my own PID
    pid_t my_pid = getpid();

    for (const auto &line : std::views::split(output, '\n')) {
        std::string pid_str(line.begin(), line.end());
        if (pid_str.empty())
            continue;

        pid_t pid = static_cast<pid_t>(std::stoi(pid_str));
        if (pid != my_pid)
            return true;
    }

    return false;
}

enum RegionMode { OUTPUT, WINDOW, REGION };

struct Args {
    bool video;
    RegionMode regionMode = OUTPUT;
    std::string region;
    std::string terminal = "kitty --title hyprgrab-recorder";
    std::filesystem::path output_directory;
    std::filesystem::path output_path;
    int delay_seconds = 0;
};

std::string read_arg(int i, int argc, char *argv[], bool lower = false) {
    if (i >= argc)
        error("Expected argument");
    auto str = std::string(argv[i]);
    if (!lower)
        return str;

    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return str;
}

Args parse_args(int argc, char *argv[]) {
    Args args;

    // Get subcomand
    if (argc < 2)
        error(USAGE);

    const auto mode = std::string(argv[1]);
    if (mode == "screenshot" || mode == "shot") {
        args.video = false;
        args.output_directory = "~/Pictures";
    } else if (mode == "screencast" || mode == "cast") {
        args.video = true;
        args.output_directory = "~/Videos";
    } else if (mode == "-h") {
        std::cout << HELP;
        exit(0);
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
            case 'm':
                {
                    // Read mode
                    const auto arg = read_arg(++i, argc, argv, true);
                    if (arg == "output")
                        args.regionMode = OUTPUT;
                    else if (arg == "window")
                        args.regionMode = WINDOW;
                    else if (arg == "region")
                        args.regionMode = REGION;
                    else
                        error(std::format("Unknown region mode '{}'", arg));
                    break;
                }

            case 'h':
                {
                    // Show help and exit
                    std::cout << HELP;
                    exit(0);
                    break;
                }

            case 'o':
                {
                    // Set output directory
                    const auto arg = read_arg(++i, argc, argv);
                    args.output_directory = arg;
                    break;
                }

            case 't':
                {
                    // Set terminal
                    const auto arg = read_arg(++i, argc, argv);
                    args.terminal = arg;
                    break;
                }

            case 's':
                {
                    // Set sleep time
                    const auto arg = read_arg(++i, argc, argv);
                    args.delay_seconds = std::stoi(arg);
                    break;
                }

            default:
                error(std::format("Unknown flag '{}'", flag));
        }
    }

    return args;
}

std::string get_region(const Args &args) {
    // If region just throw it to slurp
    if (args.regionMode == REGION) {
        // Slurp
        return exec_command("slurp -d");
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
        bool is_on_active_workspace =
            std::ranges::any_of(workspaces, [&](int id) {
                return id == client["workspace"]["id"];
            });

        if (is_on_active_workspace) {
            auto [x, y] = client["at"].get<std::pair<int, int>>();
            auto [w, h] = client["size"].get<std::pair<int, int>>();
            boxes.push_back(std::format("{},{} {}x{}", x, y, w, h));
        }
    }

    std::string boxes_text = boxes[0];
    for (unsigned i = 1; i < boxes.size(); i++) {
        boxes_text += '\n' + boxes[i];
    }

    return exec_command(std::format("slurp -r <<< '{}'", boxes_text));
}

std::filesystem::path get_output_path(const Args &args) {
    // Join the output folder with the file name
    std::filesystem::path out_dir_path = args.output_directory;
    auto now = time(NULL);
    std::string filename =
        std::format("hyprgrab_{}_{}.{}", args.video ? "cast" : "shot", now,
                    args.video ? "mp4" : "png");

    out_dir_path.append(filename);

    // Resolve output_directory if needed
    std::string out_dir = out_dir_path;
    if (out_dir.starts_with('~')) {
        const char *home = std::getenv("HOME");
        if (!home)
            error("$HOME is not set");
        out_dir_path = home;
        out_dir_path.append(out_dir.substr(2));
    }

    return out_dir_path;
}

void screenshot(const Args &args) {
    // Sleep if needed
    if (args.delay_seconds > 0) {
        notify(std::format("Sleeping for {} seconds...", args.delay_seconds), args.delay_seconds * 1000 - 100);
        sleep(args.delay_seconds);
    }

    // Save screenshot to output path
    exec_command(std::format("grim -g '{}' - | magick - -shave 1x1 '{}'",
                             args.region, args.output_path.string()));

    // Copy to clipboard
    exec_command(std::format("wl-copy < {}", args.output_path.string()));
}

void screencast(const Args &args) {
    // Get terminal from args or from env if not set
    std::string terminal = args.terminal;

    // Execute screenrecording command
    std::string command =
        std::format("echo 'Recording... Press Ctrl+C to stop.';"
                    "wl-screenrec -g '{}' -f '{}';",
                    args.region, args.output_path.string());

    if (args.delay_seconds > 0) {
        command =
            std::format("echo 'Waiting {} {}...';sleep {};", args.delay_seconds,
                        args.delay_seconds == 1 ? "second" : "seconds",
                        args.delay_seconds) +
            command;
    }

    std::string final_cmd =
        std::format("{} -e sh -c \"{}\"", terminal, command);

    std::ignore = exec_command(final_cmd.c_str());
}

int main(int argc, char *argv[]) {
    // Check if another instance with the same argv[0] is running
    if (another_running()) {
        error("Another instance of hyprgrab is already running.");
    }

    // Get the args and region and things
    Args args = parse_args(argc, argv);
    args.region = get_region(args);

    if (args.region == "")
        return 0;

    args.output_path = get_output_path(args);

    // Execute the command
    if (args.video)
        screencast(args);
    else
        screenshot(args);

    notify("Done!");
}
