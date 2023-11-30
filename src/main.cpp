#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <thread>

#include "disk_info.hpp"

/*
 * Some pre-defined constants.
 */
const std::wstring path_to_copy = L"C:/CopiedDisks/";
const std::wstring log_file_path = path_to_copy + L"logs.log";
const std::wstring config_file_name = L"config.txt";

// map of disk names to the time they were found
std::map<char, SYSTEMTIME> disks_found_times;

/**
 * @brief Get the connected disks names.
 *
 * @return std::string The connected disks names, like "CDE".
 */
std::string GetConnectedDisksNames()
{
    DWORD drives = GetLogicalDrives();
    std::string disk_names;
    for (int i = 0; i < 26; i++)
    {
        if (drives & (1 << i))
        {
            char drive = 'A' + i;
            disk_names.push_back(drive);
        }
    }
    return disk_names;
}

/**
 * @brief Get the vector of DiskInfo objects.
 *
 * @param disk_names The disks names to get the info of.
 * @return std::vector<DiskInfo>
 */
std::vector<DiskInfo> GetDisksInfo(std::string disk_names)
{
    std::vector<DiskInfo> disks_info;

    for (auto &disk : disk_names)
    {
        std::string disk_name = std::string(1, disk);

        std::string path = "\\\\.\\" + disk_name + ":";
        HANDLE h_device = CreateFileA(path.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (h_device == INVALID_HANDLE_VALUE)
        {
            std::cout << "Error opening device " << disk << '\n';
            continue;
        }

        STORAGE_PROPERTY_QUERY query;
        DWORD cb_bytes_returned = 0;
        char buffer[1024];

        memset(&query, 0, sizeof(query));
        query.PropertyId = StorageDeviceProperty;
        query.QueryType = PropertyStandardQuery;

        memset(buffer, 0, sizeof(buffer));

        if (!DeviceIoControl(h_device, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &buffer, sizeof(buffer), &cb_bytes_returned, NULL))
        {
            std::cout << "Error in DeviceIoControl\n";
            continue;
        }

        STORAGE_DEVICE_DESCRIPTOR *deviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR *)buffer;
        char *vendor_id = buffer + deviceDescriptor->VendorIdOffset;
        char *product_Id = buffer + deviceDescriptor->ProductIdOffset;
        char *product_revision = buffer + deviceDescriptor->ProductRevisionOffset;
        char *serial_number = buffer + deviceDescriptor->SerialNumberOffset;

        DiskInfo disk_info;
        disk_info.disk_name = disk;
        disk_info.vendor_id = vendor_id;
        disk_info.product_id = product_Id;
        disk_info.product_revision = product_revision;
        disk_info.serial_number = serial_number;
        disk_info.is_usb = deviceDescriptor->BusType == BusTypeUsb;

        // get path to exe file
        wchar_t path_to_exe[MAX_PATH];
        GetModuleFileNameW(NULL, path_to_exe, MAX_PATH);
        std::wstring path_to_exe_string(path_to_exe);

        // get path to exe directory
        std::wstring path_to_exe_directory = path_to_exe_string.substr(0, path_to_exe_string.find_last_of(L"\\/"));
        std::wstring path_to_config = path_to_exe_directory + config_file_name;

        // open file ignored_serial_numbers.txt and check if the serial number is in it
        HANDLE h_file = CreateFileW(path_to_config.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h_file != INVALID_HANDLE_VALUE)
        {
            // read file
            DWORD file_size = GetFileSize(h_file, NULL);
            char *file_buffer = new char[file_size + 1];
            DWORD bytes_read = 0;
            if (!ReadFile(h_file, file_buffer, file_size, &bytes_read, NULL))
            {
                std::cout << "Error reading file\n";
                continue;
            }
            file_buffer[file_size] = '\0';

            // check if serial number is in file
            std::string file_buffer_string(file_buffer);
            if (file_buffer_string.find(serial_number) != std::string::npos)
            {
                CloseHandle(h_file);
                CloseHandle(h_device);
                continue;
            }

            CloseHandle(h_file);
        }

        if (!disk_info.is_usb)
        {

            CloseHandle(h_device);
            continue;
        }

        disks_info.push_back(disk_info);
        CloseHandle(h_device);
    }

    return disks_info;
}

/**
 * @brief Create a Test file in the disk.
 *
 * @param disks The disks to create the test file in.
 */
void CreateTestFile(std::vector<DiskInfo> disks)
{
    for (auto disk : disks)
    {
        if (disk.is_usb)
        {
            disk.CreateTestFile();
        }
    }
}
/**
 * @brief Prints the disks info to the console.
 *
 * @param disks The disks info to print.
 */
void PrintDisksInfo(std::vector<DiskInfo> disks)
{
    std::cout << "There was " << disks.size() << " disks found\n";
    for (auto disk : disks)
    {
        disk.Print();
        std::cout << '\n';
    }
}
/**
 * @brief Logs the disks info to the log file.
 *
 * @param disks The disks info to log.
 */
void LogDisksInfo(std::vector<DiskInfo> disks)
{
    // open log file
    HANDLE h_file = CreateFileW(log_file_path.c_str(), FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h_file == INVALID_HANDLE_VALUE)
    {
        std::cout << "Error opening log file\n";
        return;
    }

    // write to log file
    for (auto disk : disks)
    {
        // add date and time to log
        SYSTEMTIME st;
        GetLocalTime(&st);
        disks_found_times[disk.disk_name] = st;

        char buffer[256];
        sprintf_s(buffer, "%04d-%02d-%02d %02d:%02d:%02d [main.logger] INFO: DISK %c WAS FOUND\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, disk.disk_name);
        std::string date_time(buffer);

        // write to log file
        DWORD bytes_written = 0;
        if (!WriteFile(h_file, date_time.c_str(), date_time.size(), &bytes_written, NULL))
        {
            std::cout << "Error writing to log file\n";
            return;
        }

        // write disk info to log file
        std::string disk_info = disk.ToString();
        bytes_written = 0;
        if (!WriteFile(h_file, disk_info.c_str(), disk_info.size(), &bytes_written, NULL))
        {
            std::cout << "Error writing to log file\n";
            return;
        }
    }

    // close log file
    CloseHandle(h_file);
}

/**
 * @brief Creates the logs directory and log file if they do not exist.
 *
 * @return true if the logs directory and log file were created successfully, false otherwise.
 */
bool CreateLogsDirectory()
{
    // Create directory to copy files
    std::wstring path_to_copy_full = path_to_copy;
    ExpandEnvironmentStringsW(path_to_copy.c_str(), &path_to_copy_full[0], path_to_copy.size() + 1);
    CreateDirectoryW(path_to_copy_full.c_str(), NULL);

    // Create log file if it does not exist
    HANDLE h_file = CreateFileW(log_file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h_file == INVALID_HANDLE_VALUE)
    {
        DWORD error_code = GetLastError();
        if (error_code == ERROR_FILE_NOT_FOUND)
        {
            h_file = CreateFileW(log_file_path.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
            if (h_file == INVALID_HANDLE_VALUE)
            {
                std::cout << "Error creating log file: " << GetLastError() << "\n";
                return false;
            }
            CloseHandle(h_file);
        }
        else
        {
            std::cout << "Error opening log file: " << error_code << "\n";
            return false;
        }
    }
    else
    {
        CloseHandle(h_file);
    }

    return true;
}

/**
 * @brief Copy files from the disks to the path_to_copy directory.
 *
 * @param disks The disks to copy files from.
 */
void CopyFiles(std::vector<DiskInfo> disks)
{
    std::vector<std::thread> threads;
    for (auto disk : disks)
    {
        // create directory
        auto time = disks_found_times[disk.disk_name];
        char folder_name[256];
        // also add disk name to folder name
        sprintf_s(folder_name, "%04d-%02d-%02d %02d-%02d-%02d %c", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, disk.disk_name);

        std::string folder_name_string(folder_name);
        std::wstring path_to_copy_full = path_to_copy + std::wstring(folder_name_string.begin(), folder_name_string.end());

        // create directory
        CreateDirectoryW(path_to_copy_full.c_str(), NULL);
        // disk.CopyFilesAndDirectories(path_to_copy_full);
        // copy files
        threads.emplace_back(&DiskInfo::CopyFilesAndDirectories, disk, path_to_copy_full);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }
}

int main(int argc, char **argv)
{
    if (!CreateLogsDirectory())
    {
        return 1;
    }

    // get disks info
    std::string disks_names = GetConnectedDisksNames();
    auto disks = GetDisksInfo(disks_names);

    PrintDisksInfo(disks);
    LogDisksInfo(disks);
    CopyFiles(disks);
}