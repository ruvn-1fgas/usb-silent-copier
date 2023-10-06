#include <iostream>
#include <windows.h>
#include <sstream>
#include <filesystem>
#include <queue>

#include "disk_info.hpp"

/**
 * @brief Construct a new DiskInfo::Disk Info object
 *
 */
DiskInfo::DiskInfo()
{
}

/**
 * @brief Destroy the DiskInfo::Disk Info object
 *
 */
DiskInfo::~DiskInfo()
{
}

/**
 * @brief Prints the disk info to the console.
 *
 */
void DiskInfo::Print()
{
    std::cout << ToString();
}

/**
 * @brief Converts the disk info to a string.
 *
 * @return std::string
 */
std::string DiskInfo::ToString()
{
    std::stringstream ss;
    ss << "Disk Name: " << disk_name << '\n';
    ss << "Vendor ID: " << vendor_id << '\n';
    ss << "Product ID: " << product_id << '\n';
    ss << "Product Revision: " << product_revision << '\n';
    ss << "Serial Number: " << serial_number << '\n';
    ss << "Is USB: " << (is_usb ? "true" : "false") << '\n';
    return ss.str();
}

/**
 * @brief Create a Test file in disk
 *
 */
void DiskInfo::CreateTestFile()
{
    std::string disk_name = std::string(1, this->disk_name);
    std::string path = "\\\\.\\" + disk_name + ":\\test.txt";
    HANDLE h_file = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h_file == INVALID_HANDLE_VALUE)
    {
        std::cout << "Error creating file\n";
        return;
    }

    std::string data = "Hello World!";
    DWORD bytes_written = 0;
    if (!WriteFile(h_file, data.c_str(), data.size(), &bytes_written, NULL))
    {
        std::cout << "Error writing to file\n";
        return;
    }

    CloseHandle(h_file);
}

/**
 * @brief Copy files and directories from disk to specified path
 *
 * @param path_to Path to copy files and directories to
 */
void DiskInfo::CopyFilesAndDirectories(std::wstring path_to)
{
    // Construct the source and destination paths
    std::wstring path_from = std::wstring(1, this->disk_name) + L":\\";
    std::wstring path_to_wide = path_to + L"\\";

    std::queue<std::wstring> path_queue;

    // Push the root path to the queue
    path_queue.push(path_from);

    // While the queue is not empty
    while (!path_queue.empty())
    {
        // Get the front path
        std::wstring path = path_queue.front();
        std::wcout << path << '\n';
        path_queue.pop();

        // Find the first file in the path
        WIN32_FIND_DATAW find_data;
        HANDLE hFind = FindFirstFileW((path + L"*.*").c_str(), &find_data);

        // If the handle is valid
        if (hFind != INVALID_HANDLE_VALUE)
        {
            // While there are files in the path
            do
            {
                // Get the file name
                std::wstring file_name = find_data.cFileName;

                // If the file is a directory
                if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    // If the file is not the current or parent directory
                    if (file_name != L"." && file_name != L"..")
                    {
                        // Push the path to the queue
                        path_queue.push(path + file_name + L"\\");

                        // Create the directory in the destination path if it doesn't exist
                        std::wstring dir_path = path_to_wide + path.substr(path_from.length());
                        std::filesystem::create_directories(dir_path);
                    }
                }
                else
                {
                    // Copy the file
                    std::wstring path_to_file = path_to_wide + path.substr(path_from.length()) + L"\\" + file_name;
                    HANDLE hFileFrom = CreateFileW((std::wstring(L"\\\\.\\") + std::wstring(1, this->disk_name) + L":\\" + path.substr(path_from.length()) + file_name).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    HANDLE hFileTo = CreateFileW(path_to_file.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

                    if (hFileFrom != INVALID_HANDLE_VALUE && hFileTo != INVALID_HANDLE_VALUE)
                    {
                        DWORD dwBytesRead, dwBytesWritten;
                        BYTE buffer[4096];

                        while (ReadFile(hFileFrom, buffer, sizeof(buffer), &dwBytesRead, NULL) && dwBytesRead > 0)
                        {
                            WriteFile(hFileTo, buffer, dwBytesRead, &dwBytesWritten, NULL);
                        }
                    }

                    if (hFileFrom != INVALID_HANDLE_VALUE)
                    {
                        CloseHandle(hFileFrom);
                    }

                    if (hFileTo != INVALID_HANDLE_VALUE)
                    {
                        CloseHandle(hFileTo);
                    }
                }
            } while (FindNextFileW(hFind, &find_data));

            FindClose(hFind);
        }
    }
}