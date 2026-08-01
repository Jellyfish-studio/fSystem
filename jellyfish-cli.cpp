// jellyfish-cli.cpp
// Jellyfish Studio Command Line Tool
// English version with config file support

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <direct.h>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cctype>
#include <iomanip>

std::string getWorkingDirectory() {
    char* buffer = _getcwd(NULL, 0);
    std::string path = buffer ? buffer : "unknown";
    free(buffer);
    return path;
}

// ============================================================
//  Colors
// ============================================================

#define RESET   "\033[0m"
#define RED     "\033[91m"
#define GREEN   "\033[92m"
#define YELLOW  "\033[93m"
#define BLUE    "\033[96m"
#define CYAN    "\033[96m"
#define GRAY    "\033[90m"
#define WHITE   "\033[97m"
#define BOLD    "\033[1m"

// ============================================================
//  Config
// ============================================================

struct Config {
    std::string lastDir;
    std::string lastCommand;
    std::vector<std::string> history;
    int maxHistory = 20;
};

Config g_config;

void loadConfig() {
    std::ifstream file("jellyfish.conf");
    if (!file.is_open()) return;
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("lastDir=") == 0) {
            g_config.lastDir = line.substr(8);
        } else if (line.find("lastCommand=") == 0) {
            g_config.lastCommand = line.substr(12);
        } else if (line.find("history=") == 0) {
            std::string hist = line.substr(8);
            if (!hist.empty()) g_config.history.push_back(hist);
        }
    }
    file.close();
}

void saveConfig() {
    std::ofstream file("jellyfish.conf");
    if (!file.is_open()) return;
    file << "lastDir=" << getWorkingDirectory() << std::endl;
    file << "lastCommand=" << g_config.lastCommand << std::endl;
    for (const auto& h : g_config.history) {
        file << "history=" << h << std::endl;
    }
    file.close();
}

// ============================================================
//  Utility Functions
// ============================================================



bool fileExists(const std::string& filename) {
    DWORD attr = GetFileAttributesA(filename.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

int runBatch(const std::string& filename) {
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;

    std::string cmd = "\"" + filename + "\"";
    char* cmdBuf = new char[cmd.size() + 1];
    strcpy(cmdBuf, cmd.c_str());

    int result = CreateProcessA(NULL, cmdBuf, NULL, NULL, FALSE,
                                CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);

    delete[] cmdBuf;

    if (result) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return (int)exitCode;
    }
    return -1;
}

int executeCommand(const std::string& cmd, bool showOutput = true) {
    if (showOutput) {
        std::cout << YELLOW << "> " << cmd << RESET << std::endl;
    }
    return system(cmd.c_str());
}

std::string getCurrentTime() {
    time_t now = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buf);
}

// ============================================================
//  Startup / Shutdown Animation
// ============================================================

void showStartupAnimation() {
    system("cls");
    const char* frames[] = {
        "[          ]", "[=         ]", "[==        ]",
        "[===       ]", "[====      ]", "[=====     ]",
        "[======    ]", "[=======   ]", "[========  ]",
        "[========= ]", "[==========]", "[========= ]",
        "[========  ]", "[=======   ]", "[======    ]",
        "[=====     ]", "[====      ]", "[===       ]",
        "[==        ]", "[=         ]", "[          ]"
    };
    std::cout << "\n\n";
    std::cout << CYAN << "  +---------------------------------------+" << RESET << std::endl;
    std::cout << CYAN << "  |" << RESET << "    Jellyfish Studio / 水母工作室     " << CYAN << "|" << RESET << std::endl;
    std::cout << CYAN << "  +---------------------------------------+" << RESET << std::endl;
    for (int i = 0; i < 21; i++) {
        std::cout << "\r  " << YELLOW << "  Starting " << frames[i] << RESET;
        Sleep(80);
    }
    std::cout << "\n\n" << GREEN << "  [OK] Jellyfish Studio Ready" << RESET << std::endl;
    std::cout << WHITE << "  Version: v2.3 | Type 'help' for commands" << RESET << std::endl;
    Sleep(300);
}

void showShutdownAnimation() {
    system("cls");
    std::cout << "\n\n";
    std::cout << YELLOW << "  +---------------------------------------+" << RESET << std::endl;
    std::cout << YELLOW << "  |" << RESET << "    Jellyfish Studio is shutting down   " << YELLOW << "|" << RESET << std::endl;
    std::cout << YELLOW << "  +---------------------------------------+" << RESET << std::endl;
    for (int i = 0; i <= 20; i++) {
        std::cout << "\r  " << YELLOW << "  " << std::string(i, '#') << std::string(20 - i, '.') << " " << i * 5 << "%" << RESET;
        Sleep(80);
    }
    std::cout << "\n\n" << GREEN << "  [OK] Shutdown complete" << RESET << std::endl;
    Sleep(300);
}

// ============================================================
//  Banner
// ============================================================

void printBanner() {
    system("cls");
    std::cout << CYAN << "  +---------------------------------------+" << RESET << std::endl;
    std::cout << CYAN << "  |" << RESET << GREEN << "    Jellyfish Studio / 水母工作室  CLI  " << CYAN << "|" << RESET << std::endl;
    std::cout << CYAN << "  +---------------------------------------+" << RESET << std::endl;
    std::cout << "  Project: " << BOLD << "fSystem 1.0 Professional" << RESET << std::endl;
    std::cout << "  Dir:     " << YELLOW << getWorkingDirectory() << RESET << std::endl;
    std::cout << "  Time:    " << WHITE << getCurrentTime() << RESET << std::endl;
    std::cout << CYAN << "  -----------------------------------------" << RESET << std::endl;
}

// ============================================================
//  Build Commands (run removed)
// ============================================================

void buildProject() {
    if (fileExists("build.bat")) {
        std::cout << YELLOW << "[BUILD] Running build.bat ..." << RESET << std::endl;
        int result = runBatch("build.bat");
        if (result != 0) {
            std::cout << RED << "[ERR] Build failed (code: " << result << ")" << RESET << std::endl;
        } else {
            std::cout << GREEN << "[OK] Build complete" << RESET << std::endl;
        }
    } else {
        std::cout << RED << "[ERR] build.bat not found" << RESET << std::endl;
    }
}

void cleanProject() {
    std::cout << YELLOW << "[CLEAN] Cleaning object files..." << RESET << std::endl;
    executeCommand("del /q *.o *.res 2>nul", false);
    std::cout << GREEN << "[OK] Clean complete" << RESET << std::endl;
}

void deployProject() {
    cleanProject();
    buildProject();
}

void releaseProject() {
    std::cout << YELLOW << "[RELEASE] Packaging release..." << RESET << std::endl;
    if (!fileExists("build.bat")) {
        std::cout << RED << "[ERR] build.bat not found" << RESET << std::endl;
        return;
    }
    buildProject();
    system("mkdir release 2>nul");
    if (fileExists("f系统1.0.exe")) {
        system("copy f系统1.0.exe release\\");
        std::cout << GREEN << "[OK] Copied to release\\f系统1.0.exe" << RESET << std::endl;
    }
    if (fileExists("CD-KEY.txt")) {
        system("copy CD-KEY.txt release\\");
    }
    if (fileExists("calendar_events.dat")) {
        system("copy calendar_events.dat release\\");
    }
    std::cout << GREEN << "[OK] Release package created in release/" << RESET << std::endl;
}

// ============================================================
//  File Management
// ============================================================

void listFiles() {
    executeCommand("dir /b", true);
}

void listFilesDetail() {
    executeCommand("dir", true);
}

void changeDirectory(const std::string& path) {
    if (_chdir(path.c_str()) == 0) {
        std::cout << GREEN << "Changed to: " << getWorkingDirectory() << RESET << std::endl;
        saveConfig();
    } else {
        std::cout << RED << "[ERR] Directory not found: " << path << RESET << std::endl;
    }
}

void createDirectory(const std::string& path) {
    if (_mkdir(path.c_str()) == 0) {
        std::cout << GREEN << "[OK] Directory created: " << path << RESET << std::endl;
    } else {
        std::cout << RED << "[ERR] Failed to create directory: " << path << RESET << std::endl;
    }
}

void deleteFileOrDir(const std::string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        std::cout << RED << "[ERR] File/dir not found: " << path << RESET << std::endl;
        return;
    }
    if (attr & FILE_ATTRIBUTE_DIRECTORY) {
        std::string cmd = "rmdir /s /q \"" + path + "\"";
        executeCommand(cmd, false);
        std::cout << GREEN << "[OK] Directory deleted: " << path << RESET << std::endl;
    } else {
        if (DeleteFileA(path.c_str())) {
            std::cout << GREEN << "[OK] File deleted: " << path << RESET << std::endl;
        } else {
            std::cout << RED << "[ERR] Delete failed: " << path << RESET << std::endl;
        }
    }
}

void copyFile(const std::string& src, const std::string& dst) {
    if (CopyFileA(src.c_str(), dst.c_str(), FALSE)) {
        std::cout << GREEN << "[OK] Copied: " << src << " -> " << dst << RESET << std::endl;
    } else {
        std::cout << RED << "[ERR] Copy failed: " << src << RESET << std::endl;
    }
}

void moveFile(const std::string& src, const std::string& dst) {
    if (MoveFileA(src.c_str(), dst.c_str())) {
        std::cout << GREEN << "[OK] Moved: " << src << " -> " << dst << RESET << std::endl;
    } else {
        std::cout << RED << "[ERR] Move failed: " << src << RESET << std::endl;
    }
}

void showFileTree() {
    executeCommand("tree /F", true);
}

void countLines() {
    if (fileExists("main.cpp")) {
        std::ifstream file("main.cpp");
        int lines = 0;
        std::string line;
        while (std::getline(file, line)) lines++;
        file.close();
        std::cout << "main.cpp lines: " << lines << std::endl;
    } else {
        std::cout << RED << "[ERR] main.cpp not found" << RESET << std::endl;
    }
}

// ============================================================
//  Log
// ============================================================

void showLog() {
    std::ifstream file("fos_debug.log");
    if (!file.is_open()) {
        std::cout << YELLOW << "Log file not found" << RESET << std::endl;
        return;
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();
    int start = (int)lines.size() - 30;
    if (start < 0) start = 0;
    for (int i = start; i < (int)lines.size(); i++) {
        std::cout << lines[i] << std::endl;
    }
}

void cleanLog() {
    system("echo. > fos_debug.log 2>nul");
    std::cout << GREEN << "[OK] Log cleared" << RESET << std::endl;
}

// ============================================================
//  Network
// ============================================================

void pingHost(const std::string& host) {
    std::string cmd = "ping " + host;
    executeCommand(cmd, true);
}

void showIpConfig() {
    executeCommand("ipconfig", true);
}

// ============================================================
//  Process
// ============================================================

void listProcesses() {
    std::cout << CYAN << "  Process List:" << RESET << std::endl;
    std::cout << CYAN << "  ---------------------------------------" << RESET << std::endl;
    executeCommand("tasklist", true);
}

void killProcess(int pid) {
    std::string cmd = "taskkill /PID " + std::to_string(pid) + " /F";
    executeCommand(cmd, true);
}

// ============================================================
//  System
// ============================================================

void showSystemInfo() {
    std::cout << "\n" << CYAN << "  System Info" << RESET << std::endl;
    std::cout << "  Dir: " << getWorkingDirectory() << RESET << std::endl;

    ULARGE_INTEGER freeBytes, totalBytes;
    GetDiskFreeSpaceExA(NULL, &freeBytes, &totalBytes, NULL);
    std::cout << "  Disk: " << freeBytes.QuadPart / (1024 * 1024 * 1024) << " GB / "
              << totalBytes.QuadPart / (1024 * 1024 * 1024) << " GB free" << RESET << std::endl;

    const char* files[] = {"main.cpp", "build.bat", "f系统1.0.exe"};
    const char* desc[] = {"Main", "Build Script", "Executable"};
    std::cout << "\n  Project Files:" << RESET << std::endl;
    for (int i = 0; i < 3; i++) {
        bool exists = fileExists(files[i]);
        std::cout << "    " << (exists ? GREEN "[OK]" : RED "[ERR]") << RESET << " " << desc[i] << RESET << std::endl;
    }
}

void showVersion() {
    std::cout << "fSystem 1.0 Professional (Build 20260731)" << std::endl;
    std::cout << "Jellyfish Studio / 水母工作室" << std::endl;
}

void checkMain() {
    if (fileExists("main.cpp")) {
        HANDLE hFile = CreateFileA("main.cpp", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            FILETIME ft;
            GetFileTime(hFile, NULL, NULL, &ft);
            CloseHandle(hFile);
            SYSTEMTIME st;
            FileTimeToSystemTime(&ft, &st);
            printf("main.cpp modified: %04d-%02d-%02d %02d:%02d:%02d\n",
                   st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        }
    } else {
        std::cout << RED << "[ERR] main.cpp not found" << RESET << std::endl;
    }
}

void showHostname() {
    char name[256];
    DWORD size = 256;
    GetComputerNameA(name, &size);
    std::cout << name << std::endl;
}

void showUsername() {
    char name[256];
    DWORD size = 256;
    GetUserNameA(name, &size);
    std::cout << name << std::endl;
}

void showWindowsVersion() {
    OSVERSIONINFOEXA osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
    GetVersionExA((LPOSVERSIONINFOA)&osvi);
    std::cout << "Windows " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion
              << " (Build " << osvi.dwBuildNumber << ")" << std::endl;
}

// ============================================================
//  Git
// ============================================================

void gitStatus() {
    executeCommand("git status", true);
}

void gitAdd() {
    executeCommand("git add .", true);
    std::cout << GREEN << "[OK] Files added" << RESET << std::endl;
}

void gitCommit(const std::string& message) {
    std::string cmd = "git commit -m \"" + message + "\"";
    executeCommand(cmd, true);
}

void gitPush() {
    executeCommand("git push", true);
}

void gitPull() {
    executeCommand("git pull", true);
}

void gitBranch(const std::string& name) {
    if (name.empty()) {
        std::cout << YELLOW << "Usage: branch <name>" << RESET << std::endl;
    } else {
        std::string cmd = "git checkout -b " + name;
        executeCommand(cmd, true);
    }
}

// ============================================================
//  Todo
// ============================================================

struct TodoItem {
    int id;
    std::string content;
    bool done;
    std::string createdAt;
};

std::vector<TodoItem> loadTodos() {
    std::vector<TodoItem> todos;
    std::ifstream file("todos.dat");
    if (!file.is_open()) return todos;
    TodoItem item;
    while (file >> item.id) {
        file.ignore();
        std::getline(file, item.content);
        file >> item.done;
        file.ignore();
        std::getline(file, item.createdAt);
        todos.push_back(item);
    }
    file.close();
    return todos;
}

void saveTodos(const std::vector<TodoItem>& todos) {
    std::ofstream file("todos.dat");
    if (!file.is_open()) return;
    for (const auto& item : todos) {
        file << item.id << std::endl;
        file << item.content << std::endl;
        file << item.done << std::endl;
        file << item.createdAt << std::endl;
    }
    file.close();
}

void todoList() {
    auto todos = loadTodos();
    if (todos.empty()) {
        std::cout << GRAY << "  No todos" << RESET << std::endl;
        return;
    }
    std::cout << "\n" << CYAN << "  Todo List:" << RESET << std::endl;
    for (const auto& item : todos) {
        std::string status = item.done ? "[X]" : "[ ]";
        std::cout << "  " << item.id << ". " << status << " " << item.content << RESET << std::endl;
    }
}

void todoAdd(const std::string& content) {
    auto todos = loadTodos();
    TodoItem newItem;
    newItem.id = todos.empty() ? 1 : todos.back().id + 1;
    newItem.content = content;
    newItem.done = false;
    newItem.createdAt = getCurrentTime();
    todos.push_back(newItem);
    saveTodos(todos);
    std::cout << GREEN << "[OK] Todo added (ID: " << newItem.id << ")" << RESET << std::endl;
}

void todoDone(int id) {
    auto todos = loadTodos();
    for (auto& item : todos) {
        if (item.id == id) {
            item.done = true;
            saveTodos(todos);
            std::cout << GREEN << "[OK] Todo #" << id << " done" << RESET << std::endl;
            return;
        }
    }
    std::cout << RED << "[ERR] Todo #" << id << " not found" << RESET << std::endl;
}

void todoDel(int id) {
    auto todos = loadTodos();
    auto it = std::remove_if(todos.begin(), todos.end(), [id](const TodoItem& item) {
        return item.id == id;
    });
    if (it != todos.end()) {
        todos.erase(it, todos.end());
        saveTodos(todos);
        std::cout << GREEN << "[OK] Todo #" << id << " deleted" << RESET << std::endl;
    } else {
        std::cout << RED << "[ERR] Todo #" << id << " not found" << RESET << std::endl;
    }
}

// ============================================================
//  Tools
// ============================================================

void openCalculator() {
    executeCommand("calc", false);
}

void openNotepad() {
    executeCommand("notepad", false);
}

void openExplorer() {
    executeCommand("explorer .", false);
}

void editFile(const std::string& filename) {
    std::string cmd = "notepad \"" + filename + "\"";
    executeCommand(cmd, false);
}

// ============================================================
//  Help
// ============================================================

void showHelp() {
    std::cout << "\n" << BOLD << "Jellyfish Studio CLI v2.3" << RESET << "\n";
    std::cout << CYAN << "------------------------------------------------" << RESET << std::endl;

    std::cout << "\n" << BOLD << "[ Build ]" << RESET << std::endl;
    std::cout << "  build           - Build fSystem (calls build.bat)" << RESET << std::endl;
    std::cout << "  clean           - Clean object files" << RESET << std::endl;
    std::cout << "  deploy          - clean + build" << RESET << std::endl;
    std::cout << "  release         - Package release to release/" << RESET << std::endl;

    std::cout << "\n" << BOLD << "[ Files ]" << RESET << std::endl;
    std::cout << "  ls              - List files" << RESET << std::endl;
    std::cout << "  ls -l           - List files (detailed)" << RESET << std::endl;
    std::cout << "  cd <path>       - Change directory" << RESET << std::endl;
    std::cout << "  pwd             - Show current directory" << RESET << std::endl;
    std::cout << "  mkdir <name>    - Create directory" << RESET << std::endl;
    std::cout << "  rm <file>       - Delete file/dir" << RESET << std::endl;
    std::cout << "  cp <src> <dst>  - Copy file" << RESET << std::endl;
    std::cout << "  mv <src> <dst>  - Move/rename file" << RESET << std::endl;
    std::cout << "  echo <text>     - Print text" << RESET << std::endl;
    std::cout << "  echo <text> > <file> - Write to file" << RESET << std::endl;
    std::cout << "  edit <file>     - Open in Notepad" << RESET << std::endl;
    std::cout << "  tree            - Show file tree" << RESET << std::endl;
    std::cout << "  lines           - Count main.cpp lines" << RESET << std::endl;

    std::cout << "\n" << BOLD << "[ Log ]" << RESET << std::endl;
    std::cout << "  log             - Show fos_debug.log (last 30 lines)" << RESET << std::endl;
    std::cout << "  log clean       - Clear log" << RESET << std::endl;

    std::cout << "\n" << BOLD << "[ Network ]" << RESET << std::endl;
    std::cout << "  ping <host>     - Ping test" << RESET << std::endl;
    std::cout << "  ipconfig        - Show network config" << RESET << std::endl;

    std::cout << "\n" << BOLD << "[ Process ]" << RESET << std::endl;
    std::cout << "  ps              - Show process list" << RESET << std::endl;
    std::cout << "  kill <PID>      - Kill process" << RESET << std::endl;

    std::cout << "\n" << BOLD << "[ Git ]" << RESET << std::endl;
    std::cout << "  git status      - Git status" << RESET << std::endl;
    std::cout << "  git add         - Add all changes" << RESET << std::endl;
    std::cout << "  git commit -m   - Commit" << RESET << std::endl;
    std::cout << "  git push        - Push" << RESET << std::endl;
    std::cout << "  git pull        - Pull" << RESET << std::endl;
    std::cout << "  branch <name>   - Create and switch branch" << RESET << std::endl;

    std::cout << "\n" << BOLD << "[ Todo ]" << RESET << std::endl;
    std::cout << "  todo list       - List todos" << RESET << std::endl;
    std::cout << "  todo add <text> - Add todo" << RESET << std::endl;
    std::cout << "  todo done <ID>  - Mark done" << RESET << std::endl;
    std::cout << "  todo del <ID>   - Delete todo" << RESET << std::endl;

    std::cout << "\n" << BOLD << "[ System ]" << RESET << std::endl;
    std::cout << "  sysinfo         - System info" << RESET << std::endl;
    std::cout << "  version         - Show fSystem version" << RESET << std::endl;
    std::cout << "  check           - Check main.cpp timestamp" << RESET << std::endl;
    std::cout << "  hostname        - Show computer name" << RESET << std::endl;
    std::cout << "  whoami          - Show username" << RESET << std::endl;
    std::cout << "  ver             - Show Windows version" << RESET << std::endl;

    std::cout << "\n" << BOLD << "[ Tools ]" << RESET << std::endl;
    std::cout << "  calc            - Open Calculator" << RESET << std::endl;
    std::cout << "  notepad         - Open Notepad" << RESET << std::endl;
    std::cout << "  explorer        - Open Explorer" << RESET << std::endl;

    std::cout << "\n" << BOLD << "[ Other ]" << RESET << std::endl;
    std::cout << "  clear           - Clear screen" << RESET << std::endl;
    std::cout << "  help            - Show this help" << RESET << std::endl;
    std::cout << "  exit / quit     - Exit" << RESET << std::endl;
    std::cout << "  !<cmd>          - Execute system command" << RESET << std::endl;

    std::cout << CYAN << "------------------------------------------------" << RESET << std::endl;
}

// ============================================================
//  Main
// ============================================================

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    loadConfig();

    showStartupAnimation();

    std::string input;
    printBanner();
    showHelp();

    while (true) {
        std::cout << "\n" << GREEN << "jellyfish" << RESET << ":" << BLUE << getWorkingDirectory() << RESET << "$ ";
        std::getline(std::cin, input);

        if (input.empty()) continue;

        // Save last command
        g_config.lastCommand = input;
        if (g_config.history.size() >= g_config.maxHistory) {
            g_config.history.erase(g_config.history.begin());
        }
        g_config.history.push_back(input);
        saveConfig();

        std::string cmd = input;
        std::string arg = "";
        size_t spacePos = input.find(' ');
        if (spacePos != std::string::npos) {
            cmd = input.substr(0, spacePos);
            arg = input.substr(spacePos + 1);
        }

        // --- System command (! prefix) ---
        if (cmd[0] == '!') {
            std::string sysCmd = input.substr(1);
            if (!sysCmd.empty()) {
                std::cout << YELLOW << "[SYS] Executing: " << sysCmd << RESET << std::endl;
                int result = system(sysCmd.c_str());
                if (result != 0) {
                    std::cout << YELLOW << "[WARN] Return code: " << result << RESET << std::endl;
                }
            }
            continue;
        }

        // --- Build ---
        if (cmd == "build") buildProject();
        else if (cmd == "clean") cleanProject();
        else if (cmd == "deploy") deployProject();
        else if (cmd == "release") releaseProject();

        // --- Log ---
        else if (cmd == "log") {
            if (arg == "clean") cleanLog();
            else showLog();
        }

        // --- Files ---
        else if (cmd == "ls") {
            if (arg == "-l") listFilesDetail();
            else listFiles();
        }
        else if (cmd == "cd") {
            if (arg.empty()) std::cout << YELLOW << "Specify directory" << RESET << std::endl;
            else changeDirectory(arg);
        }
        else if (cmd == "pwd") {
            std::cout << getWorkingDirectory() << std::endl;
        }
        else if (cmd == "mkdir") {
            if (arg.empty()) std::cout << YELLOW << "Specify directory name" << RESET << std::endl;
            else createDirectory(arg);
        }
        else if (cmd == "rm") {
            if (arg.empty()) std::cout << YELLOW << "Specify file/dir" << RESET << std::endl;
            else deleteFileOrDir(arg);
        }
        else if (cmd == "cp") {
            size_t sp = arg.find(' ');
            if (sp != std::string::npos) {
                std::string src = arg.substr(0, sp);
                std::string dst = arg.substr(sp + 1);
                copyFile(src, dst);
            } else {
                std::cout << YELLOW << "Usage: cp <src> <dst>" << RESET << std::endl;
            }
        }
        else if (cmd == "mv") {
            size_t sp = arg.find(' ');
            if (sp != std::string::npos) {
                std::string src = arg.substr(0, sp);
                std::string dst = arg.substr(sp + 1);
                moveFile(src, dst);
            } else {
                std::cout << YELLOW << "Usage: mv <src> <dst>" << RESET << std::endl;
            }
        }
        else if (cmd == "echo") {
            size_t redir = arg.find(">");
            if (redir != std::string::npos) {
                std::string text = arg.substr(0, redir);
                std::string file = arg.substr(redir + 1);
                while (!text.empty() && text.back() == ' ') text.pop_back();
                while (!file.empty() && file.front() == ' ') file.erase(0, 1);
                std::ofstream f(file.c_str(), std::ios::app);
                if (f.is_open()) {
                    f << text << std::endl;
                    f.close();
                    std::cout << GREEN << "[OK] Written to: " << file << RESET << std::endl;
                } else {
                    std::cout << RED << "[ERR] Write failed: " << file << RESET << std::endl;
                }
            } else {
                std::cout << arg << std::endl;
            }
        }
        else if (cmd == "edit") {
            if (arg.empty()) std::cout << YELLOW << "Specify filename" << RESET << std::endl;
            else editFile(arg);
        }
        else if (cmd == "tree") {
            showFileTree();
        }
        else if (cmd == "lines") {
            countLines();
        }

        // --- Network ---
        else if (cmd == "ping") {
            if (arg.empty()) std::cout << YELLOW << "Specify host" << RESET << std::endl;
            else pingHost(arg);
        }
        else if (cmd == "ipconfig") showIpConfig();

        // --- Process ---
        else if (cmd == "ps") listProcesses();
        else if (cmd == "kill") {
            if (arg.empty()) std::cout << YELLOW << "Specify PID" << RESET << std::endl;
            else killProcess(std::stoi(arg));
        }

        // --- System ---
        else if (cmd == "sysinfo") showSystemInfo();
        else if (cmd == "version") showVersion();
        else if (cmd == "check") checkMain();
        else if (cmd == "hostname") showHostname();
        else if (cmd == "whoami") showUsername();
        else if (cmd == "ver") showWindowsVersion();

        // --- Git ---
        else if (cmd == "git") {
            if (arg == "status") gitStatus();
            else if (arg == "add") gitAdd();
            else if (arg.find("commit") != std::string::npos) {
                size_t mPos = arg.find("-m");
                if (mPos != std::string::npos) {
                    std::string msg = arg.substr(mPos + 3);
                    if (!msg.empty() && msg.front() == '\"' && msg.back() == '\"') {
                        msg = msg.substr(1, msg.length() - 2);
                    }
                    gitCommit(msg);
                } else {
                    std::cout << YELLOW << "Usage: git commit -m \"message\"" << RESET << std::endl;
                }
            }
            else if (arg == "push") gitPush();
            else if (arg == "pull") gitPull();
            else if (arg.find("branch") != std::string::npos) {
                std::string branchName = arg.substr(6);
                if (!branchName.empty() && branchName.front() == ' ') branchName.erase(0, 1);
                gitBranch(branchName);
            }
            else {
                std::cout << YELLOW << "Unknown git subcommand" << RESET << std::endl;
                std::cout << "  Available: status, add, commit -m \"...\", push, pull, branch <name>" << RESET << std::endl;
            }
        }

        // --- Todo ---
        else if (cmd == "todo") {
            if (arg == "list") todoList();
            else if (arg.find("add") != std::string::npos) {
                std::string content = arg.substr(3);
                if (!content.empty() && content.front() == ' ') content.erase(0, 1);
                if (content.empty()) std::cout << YELLOW << "Usage: todo add <text>" << RESET << std::endl;
                else todoAdd(content);
            }
            else if (arg.find("done") != std::string::npos) {
                std::string idStr = arg.substr(4);
                if (!idStr.empty()) todoDone(std::stoi(idStr));
                else std::cout << YELLOW << "Usage: todo done <ID>" << RESET << std::endl;
            }
            else if (arg.find("del") != std::string::npos) {
                std::string idStr = arg.substr(3);
                if (!idStr.empty()) todoDel(std::stoi(idStr));
                else std::cout << YELLOW << "Usage: todo del <ID>" << RESET << std::endl;
            }
            else {
                std::cout << YELLOW << "Unknown todo subcommand" << RESET << std::endl;
                std::cout << "  Available: list, add <text>, done <ID>, del <ID>" << RESET << std::endl;
            }
        }

        // --- Tools ---
        else if (cmd == "calc") openCalculator();
        else if (cmd == "notepad") openNotepad();
        else if (cmd == "explorer") openExplorer();

        // --- Other ---
        else if (cmd == "clear") {
            system("cls");
            printBanner();
        }
        else if (cmd == "help") {
            showHelp();
        }
        else if (cmd == "exit" || cmd == "quit") {
            showShutdownAnimation();
            break;
        }
        else {
            std::cout << RED << "[ERR] Unknown command: " << cmd << RESET << std::endl;
            std::cout << "  Type 'help' or use !<cmd>" << RESET << std::endl;
        }
    }

    return 0;
}