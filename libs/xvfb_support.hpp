/*
    xvfb_support.hpp - source code

    ===================================
    code added for PIwebVNC in C++ only
    ===================================

    Free to use, free to modify, free to redistribute.
    Created by : Jishan Ali Mondal

    This is a header-only library.
    created for only PIwebVNC
    * This code was created entirely for the most optimized performance for PIwebVNC *
    * May not be suitable for other projects *
    version 1.0.1
*/

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <cstring>

void startXinit();
void startDisplayManager();
    // Function to read a password from the terminal without echoing characters
    std::string readPassword()
{
    std::string password;
    struct termios oldt, newt;

    // Disable echo
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Read password
    std::getline(std::cin, password);

    // Enable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return password;
}

// Function to install Xvfb
void installXvfb()
{
    // Check if Xvfb is installed
    if (system("Xvfb -help") != 0)
    {
        // Xvfb is not installed, prompt the user for their password
        std::cout << "Xvfb is not installed. Root permission is required to install Xvfb." << std::endl;
        std::string preCondition = "";
        if (system("sudo true") != 0) {
            std::cout << "Please enter your password: ";
            std::string password = readPassword();
            preCondition = "echo \"" + password + "\" | ";
        }

        // Execute the installation command with sudo
        std::string installCommand = "sudo apt-get install xvfb";
        std::string fullCommand =  preCondition + installCommand + " 2>&1";
        if (system(fullCommand.c_str()) != 0)
        {
            std::cerr << "Failed to install Xvfb." << std::endl;
            exit(1);
        }

        // Verify installation
        if (system("Xvfb -help") != 0)
        {
            std::cerr << "Xvfb installation verification failed." << std::endl;
            exit(1);
        }

        std::cout << "Xvfb installed successfully." << std::endl;
    }
    else
    {
        std::cout << "Xvfb is already installed." << std::endl;
    }
}

void initXvfb()
{
    std::string executeXvfb = "Xvfb :1 -screen 0 1280x720x24 &";
    if (system(executeXvfb.c_str()) != 0)
    {
        std::cerr << "Failed to start Xvfb." << std::endl;
        exit(1);
    }
    std::cout << "Xvfb started successfully." << std::endl;
    startXinit();
}

void startXinit()
{
    // Wait for Xvfb to start (adjust delay as needed)
    sleep(3); // Wait for 3 seconds (example delay)

    // Set the DISPLAY environment variable
    setenv("DISPLAY", ":1", 1); // ":1" corresponds to the display provided by Xvfb
    startDisplayManager();
}

void startDisplayManager()
{
    const char *desktop_environments[] = {
        "which gnome-session > /dev/null 2>&1",
        "which mate-session > /dev/null 2>&1",
        "which cinnamon-session > /dev/null 2>&1",
        "which pantheon-session > /dev/null 2>&1",
        "which lxqt-session > /dev/null 2>&1",
        "which startkde > /dev/null 2>&1",
        "which startxfce4 > /dev/null 2>&1",
        "which lxsession > /dev/null 2>&1",
        "which enlightenment_start > /dev/null 2>&1",
        NULL // Terminating null pointer
    };

    const char *desktop_environments_start_command[] = {
        "gnome-session &",
        "mate-session &",
        "cinnamon-session &",
        "pantheon-session &",
        "lxqt-session &",
        "startkde &",
        "startxfce4 &",
        "lxsession &",
        "enlightenment_start &",
        NULL // Terminating null pointer
    };    

    for (int i = 0; desktop_environments[i] != NULL; i++)
    {
        if(system(desktop_environments[i]) == 0)
        {
            if (system(desktop_environments_start_command[i]) == 0)
            {
                std::cout << "Desktop environment started successfully." << std::endl;
                return;
            }
            else
            {
                std::cerr << "Failed to start desktop environment." << std::endl;
                exit(1);
            }
        }
    }
}