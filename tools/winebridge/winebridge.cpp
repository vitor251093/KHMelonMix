#include <windows.h>
#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include <array>

std::string winePathToUnix(const std::string& windowsPath) {
    std::string command = "winepath -u \"" + windowsPath + "\"";

    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);

    if (!pipe) {
        return windowsPath;
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    if (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.erase(result.find_last_not_of("\r\n") + 1);
    }

    return result;
}

std::string quote(const std::string& arg)
{
    if (arg.find_first_of(" \t\"") == std::string::npos)
        return arg;

    std::string quoted = "\"";
    for (char c : arg)
    {
        if (c == '"')
            quoted += "\\\"";
        else
            quoted += c;
    }
    quoted += "\"";
    return quoted;
}

void makeExecutable(const std::string& unixPath) {
    std::string cmd = "start.exe /unix /usr/bin/chmod +x " + quote(unixPath);
    WinExec(cmd.c_str(), SW_HIDE);
}

int main(int argc, char* argv[])
{
    char cwd[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, cwd);

    std::string unixCwd = winePathToUnix(cwd);
    std::string appImage = unixCwd + "/Image/melon/MelonMix.AppImage";
    std::string assetsFolderPath = unixCwd + "/Image/melon/assets";
    makeExecutable(appImage);

    std::string linuxCmd = quote(appImage)+ " --appimage-extract-and-run";
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (!arg.empty() && arg[0] != '-')
        {
            if (arg[0] != '/' && arg.find(':') == std::string::npos)
            {
                arg = unixCwd + "/" + arg;
            }

            linuxCmd += " " + quote(arg);
        }
        else
        {
            linuxCmd += " " + arg;
        }
    }

    linuxCmd = "unset LD_PRELOAD; unset LD_LIBRARY_PATH; " + linuxCmd;
    linuxCmd = "export PATH=/usr/bin:/bin:/usr/local/bin; " + linuxCmd;
    linuxCmd = "export FUSERMOUNT_PROG=/usr/bin/fusermount; " + linuxCmd;
    linuxCmd = "export DISPLAY=:0; " + linuxCmd;
    linuxCmd = "export MELON_MIX_ASSETS=" + quote(assetsFolderPath) + "; " + linuxCmd;
    linuxCmd += " > " + quote(unixCwd + "/melon_error.log") + " 2>&1";
    std::string cmd = "start /unix /usr/bin/bash -c " + quote(linuxCmd);

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::string commandLine = cmd;
    std::vector<char> commandLineLPSTR(commandLine.begin(), commandLine.end());
    commandLineLPSTR.push_back('\0');

    std::string cwdStr = std::string(cwd);
    std::vector<char> cwdLPSTR(cwdStr.begin(), cwdStr.end());
    cwdLPSTR.push_back('\0');

    BOOL result = CreateProcessA(
        nullptr,
        (LPSTR)commandLineLPSTR.data(),
        nullptr, nullptr, FALSE,
        CREATE_NEW_CONSOLE,
        nullptr,
        (LPSTR)cwdLPSTR.data(), &si, &pi
    );

    if (!result)
    {
        return 1;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
