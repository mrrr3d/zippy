#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <zip.h>

void compressFile(const char *zipFileName, const char *fileToAdd) {
  int err = 0;
  zip_t *zip = zip_open(zipFileName, ZIP_CREATE | ZIP_TRUNCATE, &err);
  if (!zip) {
    std::cerr << "Failed to open zip file: " << zip_strerror(zip) << std::endl;
    return;
  }

  zip_source_t *source = zip_source_file(zip, fileToAdd, 0, 0);
  if (!source) {
    std::cerr << "Failed to create zip source: " << zip_strerror(zip)
              << std::endl;
    zip_close(zip);
    return;
  }

  if (zip_file_add(zip, fileToAdd, source, ZIP_FL_OVERWRITE) < 0) {
    std::cerr << "Failed to add file to zip: " << zip_strerror(zip)
              << std::endl;
    zip_source_free(source);
    zip_close(zip);
    return;
  }

  zip_close(zip);
  std::cout << "File compressed successfully: " << zipFileName << std::endl;
}

void decompressFile(const std::string zipFileName) {
  int err = 0;
  zip_t *zip = zip_open(zipFileName.c_str(), 0, &err);
  if (!zip) {
    std::cerr << "Failed to open zip file: " << zip_strerror(zip) << std::endl;
    return;
  }
  auto numEntry = zip_get_num_entries(zip, ZIP_FL_UNCHANGED);
  std::cout << "numEntry: " << numEntry << std::endl;

  for (int i = 0; i < numEntry; i++) {
    const char *name = zip_get_name(zip, i, ZIP_FL_ENC_GUESS);
    std::cout << "name: " << name;

    unsigned int attr = 0;
    int rv = zip_file_get_external_attributes(zip, i, 0, 0, &attr);
    if (rv != 0) {
      std::cerr << "get external attr err, rv = " << rv << std::endl;
    }
    bool isDir = (attr >> 16) & 0x4000;
    uint32_t perm = (attr >> 16) & 0x0fff;
    std::cout << ", isDir: " << isDir << ", perm: " << std::hex << perm
              << std::endl;
    zip_stat_t stat;
    zip_stat_index(zip, i, 0, &stat);

    if (!isDir) {
      unsigned long long sum = 0;
      char buf[1000];
      auto zipFile = zip_fopen_index(zip, i, 0);
      // check if have parent
      if (std::filesystem::path(stat.name).has_parent_path()) {
        auto parentDir = std::filesystem::path(stat.name).parent_path();
        if (!std::filesystem::exists(parentDir)) {
          std::filesystem::create_directory(parentDir);
        }
      }
      std::ofstream outFile(stat.name, std::ios::app);
      if (!outFile.is_open()) {
        std::cout << "file not open!" << std::endl;
      }
      while (sum < stat.size) {
        const auto readByte = zip_fread(zipFile, buf, 1000);
        if (readByte < 0) {
          std::cerr << "Failed to read data" << std::endl;
          outFile.close();
          return;
        }
        outFile.write(buf, readByte);
        sum += readByte;
      }
      outFile.close();
    } else if (!std::filesystem::exists(stat.name)) {
      std::filesystem::create_directory(stat.name);
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " -c <zipfile> <file> | -x <zipfile>"
              << std::endl;
    return 1;
  }

  std::string mode = argv[1];
  if (mode == "-c") {
    if (argc < 4) {
      std::cerr << "Error: -c requires two parameters." << std::endl;
      return 1;
    }
    std::string zipfile = argv[2];
    std::string dir = argv[3];
  } else if (mode == "-x") {
    if (argc < 3) {
      std::cerr << "Error: -x requires one parameter." << std::endl;
      return 1;
    }
    std::string zipfile = argv[2];
    decompressFile(zipfile);
  } else {
    std::cerr << "Error: Unknown argument " << mode << std::endl;
    return 1;
  }

  return 0;
}