#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

// Globals
struct sockaddr_un HYPRLAND_SOCKET_ADDR;
size_t HYPRLAND_ADDR_SIZE;
char big_buffer[256];

// Write to the hyprland socket and read output back
std::string hyprctl(const std::string &command);

void global_init();

void error(const std::string &msg);

int main(int argc, char *argv[]) {
    global_init();
    std::cout << hyprctl(argv[1]) << std::endl;
}

void error(const std::string &msg) {
    std::cerr << msg << std::endl;
    std::exit(1);
}

void global_init() {
    // Get hyprland signature and socket path
    char *hyrpland_signature = std::getenv("HYPRLAND_INSTANCE_SIGNATURE");
    char *runtime_dir = std::getenv("XDG_RUNTIME_DIR");

    if (!hyrpland_signature)
        error("Couldn't get HYPRLAND_INSTANCE_SIGNATURE from env");

    if (!runtime_dir)
        error("Couldn't get XDG_RUNTIME_DIR from env");

    HYPRLAND_SOCKET_ADDR.sun_family = AF_UNIX;
    std::sprintf(HYPRLAND_SOCKET_ADDR.sun_path, "%s/hypr/%s/.socket.sock",
                 runtime_dir, hyrpland_signature);
    HYPRLAND_ADDR_SIZE = sizeof(HYPRLAND_SOCKET_ADDR.sun_family) +
                         std::strlen(HYPRLAND_SOCKET_ADDR.sun_path);
}

std::string hyprctl(const std::string &command) {
    // Open socket
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
        error("Couldn't create unix socket");

    if (fcntl(fd, O_NONBLOCK) != -1)
        error("Failed to set socket non-blocking");

    if (connect(fd, (struct sockaddr *)&HYPRLAND_SOCKET_ADDR,
                HYPRLAND_ADDR_SIZE) == -1) {
        close(fd);
        error("Couldn't connect to hyprland socket");
    }

    // Write command
    write(fd, command.c_str(), sizeof(char) * command.size());

    // Read response
    std::string output;
    while (true) {
        ssize_t read_size = recv(fd, big_buffer, sizeof(big_buffer) - 1, 0);
        
        if (read_size == -1) {
            // Check errno
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Done reading, break
                break;
            } else {
                // Another error, close and exit
                close(fd);
                error("Error reading from the hyprland socket");
            }
        }

        // Read something
        big_buffer[read_size] = 0;
        output.append(big_buffer);

        if (read_size != sizeof(big_buffer) - 1)
            break;
    }

    close(fd);

    return output; 
}

