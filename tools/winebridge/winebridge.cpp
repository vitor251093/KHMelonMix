#include <windows.h>
#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include <array>

std::string winePathToUnix(const std::string& windowsPath)
{
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

void makeLinuxBinaryRunnable(const std::string& unixPath)
{
    std::string cmd = "start.exe /unix /usr/bin/chmod +x " + quote(unixPath);
    WinExec(cmd.c_str(), SW_HIDE);
}

static bool startWaitTitleProject(const std::string& cwdStr, PROCESS_INFORMATION& outPi)
{
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    outPi = {};

    std::string exePath = cwdStr + "\\WaitTitleProject.exe";
    std::string cmdLine = "\"" + exePath + "\"";

    std::vector<char> cmd(cmdLine.begin(), cmdLine.end());
    cmd.push_back('\0');

    std::vector<char> cwd(cwdStr.begin(), cwdStr.end());
    cwd.push_back('\0');

    return CreateProcessA(
        nullptr,
        (LPSTR)cmd.data(),
        nullptr, nullptr, FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        (LPSTR)cwd.data(),
        &si,
        &outPi
    );
}

static void closeWaitTitleProject(PROCESS_INFORMATION& waitPi, std::string& signalPath)
{
    // Delete the signal file if it exists from a previous run
    DeleteFileA(signalPath.c_str());

    bool appStarted = false;
    for (int i = 0; i < 100; ++i) // Wait up to 10 seconds
    {
        // Check if the file exists
        DWORD dwAttrib = GetFileAttributesA(signalPath.c_str());
        if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
        {
            appStarted = true;
            break;
        }
        Sleep(100);
    }

    // Optional: Small extra delay to let the UI finish rendering
    if (appStarted) Sleep(500);

    TerminateProcess(waitPi.hProcess, 0);
    WaitForSingleObject(waitPi.hProcess, 2000);
    CloseHandle(waitPi.hProcess);
    CloseHandle(waitPi.hThread);
}

int main(int argc, char* argv[])
{
    char cwd[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, cwd);

    std::string cwdStr = cwd;
    PROCESS_INFORMATION waitPi{};
    bool waitStarted = startWaitTitleProject(cwdStr, waitPi);

    std::string unixCwd = winePathToUnix(cwd);
    std::string appImage = unixCwd + "/Image/melon/MelonMix.AppImage";
    std::string assetsFolderPath = unixCwd + "/Image/melon/assets";
    std::string signalPath = cwdStr + "\\.melonmix_signal";
    bool enableLogFile = false;

    makeLinuxBinaryRunnable(appImage);

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
    if (enableLogFile) {
        linuxCmd += " > " + quote(unixCwd + "/melon_mix.log") + " 2>&1";
    }
    std::string cmd = "start /unix /usr/bin/bash -c " + quote(linuxCmd);

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::string commandLine = cmd;
    std::vector<char> commandLineLPSTR(commandLine.begin(), commandLine.end());
    commandLineLPSTR.push_back('\0');

    std::vector<char> cwdLPSTR(cwdStr.begin(), cwdStr.end());
    cwdLPSTR.push_back('\0');

    BOOL result = CreateProcessA(
        nullptr,
        (LPSTR)commandLineLPSTR.data(),
        nullptr, nullptr, FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        (LPSTR)cwdLPSTR.data(), &si, &pi
    );

    if (!result)
    {
        if (waitStarted)
        {
            closeWaitTitleProject(waitPi, signalPath);
        }
        return 1;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (waitStarted)
    {
        closeWaitTitleProject(waitPi, signalPath);
    }
    return 0;
}
