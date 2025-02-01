#pragma once
#include <linc/Include.hpp>

namespace linc
{
    /// @brief File management utility class
    class Files final 
    {
    public:
        /// @brief STD wrapper to convert a filepath to absolute format.
        /// @param filepath_string The relative filepath.
        /// @return The absolute equivalent of the given filepath.
        static std::string toAbsolute(const std::string& filepath_string);

        /// @brief Add given file to the cache and open it.
        /// @param filepath_string The filepath to read in string format.
        /// @param write Whether to open the file with write permissions.
        /// @return Pointer to the file's file-stream object.
        static std::fstream* load(const std::string& filepath_string, bool write);
        
        /// @brief Load a file in memory, then read and return its contents.
        /// @param filepath_string The filepath to read in string format.
        /// @return The contents of the file.
        static std::string read(const std::string& filepath_string);
        
        /// @brief Check whether a given file exists, based on its filepath.
        /// @param filepath_string The filepath to read in string format.
        /// @return Boolean corresponding to the result of the test.
        static bool exists(const std::string& filepath_string);

        /// @brief Write contents to a specified file by its filepath.
        /// @param filepath_string The filepath to write to in string format.
        /// @param contents The new contents of the file.
        static void write(const std::string& filepath_string, const std::string& contents);

        [[nodiscard]] static inline std::vector<std::string>::const_iterator beginFilepaths() { return s_filepaths.begin(); }
        [[nodiscard]] static inline std::vector<std::string>::const_iterator endFilepaths() { return s_filepaths.end(); }
        [[nodiscard]] static inline const std::string& getFilepath(std::vector<std::string>::size_type index) { return s_filepaths.at(index); }
        static inline void pushFilepath(const std::string& filepath) { s_filepaths.push_back(filepath); }
    private:
        static std::vector<std::string> s_filepaths;
        static std::unordered_map<std::filesystem::path, std::fstream*> s_fileMap;
    };
}