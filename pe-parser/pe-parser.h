#pragma once

#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <vector>


enum class PARSER_OPERATION {
    IS_PE, IMPORT_FUNCTIONS, EXPORT_FUNCTIONS
};

struct PeParser {
    int open(const std::string &file_name) {
        fd = ::open(file_name.c_str(), O_RDONLY);
        return -1;
    }

    void close() {
        ::close(fd);
        fd = -1;
    }

    ssize_t is_pe() {
        assert(fd != -1);
        char buffer[4];
        memset(buffer, 0, 4);
        int32_t pe_start_address = signature_position();
        if (pe_start_address < 0) {
            return pe_start_address;
        }
        int res = pread(fd, buffer, 4, pe_start_address);
        if (res < 0) {
            return res;
        }
        return buffer[0] == 'P' && buffer[1] == 'E' && buffer[2] == 0 && buffer[3] == 0;
    }

    std::vector <std::pair<std::string, std::vector < std::string>>> import_dll() {
        assert(fd != -1);
        int current_address = signature_position();
        if (current_address < 0) {
            return {};
        }
        current_address += SIGN_SIZE + COFF_SIZE;
        char buffer[4];
        int res = pread(fd, buffer, 4, current_address + 0x78);
        int import_table_rva = *(int *) buffer;
        res = pread(fd, buffer, 4, current_address + 0x78 + 4);
        int import_table_size = *(int *) buffer;

        int import_table_addr = rva2addr(import_table_rva);
        std::vector < std::pair < std::string, std::vector < std::string>>> import;
        for (int i = 0; i < import_table_size; i += 20) {
            char import_directory_table[20];
            pread(fd, import_directory_table, 20, import_table_addr + i);
            int library_rva = *(int *) (import_directory_table + 0xC);
            if (library_rva == 0) {
                break;
            }
            int library_addr = rva2addr(library_rva);
            char library_name[40];
            pread(fd, library_name, 40, library_addr);
            import.emplace_back(library_name, import_functions(import_table_addr + i));
        }
        return import;
    }

    std::vector<std::string> export_function() {
        assert(fd != -1);
        int current_address = signature_position();
        if (current_address < 0) {
            return {};
        }
        current_address += SIGN_SIZE + COFF_SIZE;
        char buffer[4];
        int res = pread(fd, buffer, 4, current_address + 0x70);
        int export_table_rva = *(int *) buffer;
        res = pread(fd, buffer, 4, current_address + 0x70 + 4);
        int export_table_size = *(int *) buffer;
        if (export_table_rva == 0) {
            return {};
        }

        int export_table_addr = rva2addr(export_table_rva);
        pread(fd, buffer, 4, export_table_addr + 24);
        int entries = *(int *) buffer;
        pread(fd, buffer, 4, export_table_addr + 32);
        int names_table_rva = *(int *) buffer;
        int names_table_addr = rva2addr(names_table_rva);
        std::vector<std::string> exports;
        for (int i = 0; i < entries; ++i) {
            pread(fd, buffer, 4, names_table_addr + 4 * i);
            int export_name_addr = rva2addr(*(int *) buffer);
            char export_name[40];
            pread(fd, export_name, 40, export_name_addr);
            exports.emplace_back(export_name);
        }
        return exports;
    }



private:
    static constexpr int SIGN_SIZE = 4;
    static constexpr int COFF_SIZE = 20;
    static constexpr int OPTIONAL_HEADER_SIZE = 240;
    int fd = -1;

    std::vector <std::string> import_functions(int import_directory_table_address) {
        char import_directory_table[20];
        pread(fd, import_directory_table, 20, import_directory_table_address);
        int import_lookup_table_rva = *(int *) import_directory_table;
        if (import_lookup_table_rva == 0) {
            return {};
        }
        int import_lookup_table_address = rva2addr(import_lookup_table_rva);
        std::vector <std::string> functions;
        int entry = 0;
        while (true) {
            char table_entry[8];
            pread(fd, table_entry, 8, import_lookup_table_address + entry * 8);
            if ((table_entry[0] >> 7) == 1) {
                continue;
            }
            int function_name_rva = *(int *) (table_entry);
            if (function_name_rva == 0) {
                break;
            }
            int function_name_addr = rva2addr(function_name_rva) + 2;
            char function_name[50];
            pread(fd, function_name, 50, function_name_addr);
            functions.emplace_back(function_name);
            entry++;
            if (entry > 300) {
                exit(100);
            }
        }
        return functions;
    }

    int rva2addr(int address_rva) {
        int current_address = signature_position();
        if (current_address < 0) {
            return 0;
        }
        current_address += SIGN_SIZE + COFF_SIZE + OPTIONAL_HEADER_SIZE;
        int segment_address = find_segment_addr(current_address, address_rva);
        if (segment_address == 0) {
            return 0;
        }
        char buffer[4];
        int res = pread(fd, buffer, 4, segment_address + 0x14);
        int section_raw = *(int *) buffer;
        res = pread(fd, buffer, 4, segment_address + 0xC);
        int section_rva = *(int *) buffer;
        return section_raw + (address_rva - section_rva);
    }

    int find_segment_addr(int start_address, int import_table_rva) {
        int current_address = start_address;
        while (true) {
            int res = check_segment(current_address, import_table_rva);
            if (res == 1) {
                return current_address;
            }
            if (res == -1) {
                return 0;
            }
            current_address += 40;
        }
    }

    int check_segment(int start_address, int import_table_rva) {
        char buffer[4];
        pread(fd, buffer, 4, start_address + 0x8);
        int section_virtual_size = *(int *) buffer;
        pread(fd, buffer, 4, start_address + 0xC);
        int section_rva = *(int *) buffer;
        if (section_rva == 0 && section_virtual_size == 0) {
            return -1;
        } else if (section_rva <= import_table_rva && import_table_rva - section_rva < section_virtual_size) {
            return 1;
        } else {
            return 0;
        }
    }

    int32_t signature_position() {
        constexpr int bytes_start = 0x3C;
        char buffer[4];
        memset(buffer, 0, 4);
        int res = pread(fd, buffer, 4, bytes_start);
        if (res < 0) {
            return res;
        }
        return *(int *) buffer;
    }
};
