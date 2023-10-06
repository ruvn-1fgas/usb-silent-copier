#ifndef DISK_INFO_HPP_
#define DISK_INFO_HPP_

class DiskInfo
{
public:
    /**
     * @brief Construct a new Disk Info object
     *
     */
    DiskInfo();
    /**
     * @brief Destroy the Disk Info object
     *
     */
    ~DiskInfo();
    /**
     * @brief Prints the disk info to the console.
     *
     */
    void Print();
    /**
     * @brief Converts the disk info to a string.
     *
     * @return std::string
     */
    std::string ToString();

    /**
     * @brief Create a Test file in disk
     *
     */
    void CreateTestFile();

    /**
     * @brief Copy files from disk to path_to
     *
     * @param path_to The path to copy the files to
     */
    void CopyFilesAndDirectories(std::wstring path_to);

    char disk_name = ' ';
    std::string vendor_id = "";
    std::string product_id = "";
    std::string product_revision = "";
    std::string serial_number = "";
    bool is_usb = false;
};

#endif // DISK_INFO_HPP_