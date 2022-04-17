#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <filesystem>
#include <iostream>
#include <map>
#include <string>

#include "fs_tools.h"

static void RecursiveMkdir(const std::string& path) {
  size_t s = 0;
  for (;;) {
    s = path.find("/", s);
    const auto name = (s == std::string::npos) ? path : path.substr(0, s);
    if (!name.empty()) {
      if (mkdir(name.c_str(), 0777) != 0) {
        if (errno != EEXIST) {
          throw std::runtime_error("Failed to create directory " + name + ", errno " + std::to_string(errno));
        }
      }
    }
    if (s == std::string::npos) {
      break;
    }
    s++;
  }
}

//static bool NeedUpdate(struct stat& src_stat, const std::string& dest_name) {
//  struct stat dest_stat;
//  return (lstat(dest_name.c_str(), &dest_stat) == -1) || (dest_stat.st_mtime < src_stat.st_mtime);
//}

static std::string QuoteJs(const std::string& source) {
  static const char* abc = "0123456789ABCDEF";
  std::string out;
  out.reserve(source.size() * 4 + 2);
  out += "\"";
  for (auto c : source) {
    uint8_t code = uint8_t(c);
    if ((code < 32) || (c == '"')) {
      out += "\\x";
      out += abc[code >> 4u];
      out += abc[code & 0x0Fu];
    } else {
      out += c;
    }
  }
  out += "\"";
  return out;
}

static std::string GetFileExtension(const std::string& name) {
  auto pos_1 = name.rfind('.');
  const auto pos_2 = name.rfind('/');
  if (pos_2 != std::string::npos && pos_1 < pos_2) {
    pos_1 = std::string::npos;
  }
  return (pos_1 != std::string::npos) ? name.substr(pos_1 + 1) : "";
}

static void GetFileStat(const std::string& file_name, struct stat* out) {
  if (lstat(file_name.c_str(), out) == -1) {
    throw std::runtime_error("Failed to execute lstat(" + file_name + ")");
  }
}

class MakeCompsJsState {
public:
    void Start();
    
private:
    std::string out;
        
    void MakeLevel(uint32_t deep, std::string upPath, std::string relPath, bool photo_dir);
};

void MakeCompsJsState::Start() {
  static const auto output_file_name = "comps.js";
  static const auto retro_computers_repository_path = "../../retro_computers";
  std::cout << "Make " << output_file_name << std::endl;

  out = "const comps={\n";
  out += "dir:1,\n";
  MakeLevel(0, retro_computers_repository_path, "", false);
  out += "};\n";

  FsTools::SaveFile(output_file_name, out);
}

void MakeCompsJsState::MakeLevel(uint32_t deep, std::string upPath, std::string relPath, bool photo_dir) {
  FsTools::FindFiles dir(upPath.c_str());

  out += "items:{\n";

  while (dir.Next()) {
    const std::string name = dir.Item()->d_name;
    const std::string extension = GetFileExtension(name);

    // Ignore files
    if ((name == ".") && (name == "..")) {
      continue;
    }
    if ((deep == 0) && (name == ".git")) {
      continue;
    }
    if (extension == "info") {
      continue;
    }

    // Get file date and size
    const std::string full_path = upPath + "/" + name;
    struct stat file_stat {};
    GetFileStat(full_path, &file_stat);

    // Make preview
    if (photo_dir) {
      if ((extension == "jpg") || (extension == "png")) {
        const std::string preview_file_name = relPath + name;
        const bool preview_file_exists = std::filesystem::exists(preview_file_name);
        struct stat preview_file_stat {};
        if (preview_file_exists) {
          GetFileStat(preview_file_name, &preview_file_stat);
        }
        if (!preview_file_exists || (preview_file_stat.st_mtime < file_stat.st_mtime)) {
          RecursiveMkdir(relPath);
          const std::string command = "convert " + full_path + " -quality 85 -resize 300\\>x200\\> " + preview_file_name;
          std::cout << command << std::endl;
          if (system(command.c_str()) != 0) {
            throw std::runtime_error("Failed to execute system(" + command + ")");
          }
        }
      }
    }

    out += QuoteJs(name);
    out += ":{";

    if ((file_stat.st_mode & S_IFMT) == S_IFDIR) {
      out += "\ndir:1,\n";
      MakeLevel(deep + 1, full_path, relPath + dir.Item()->d_name + "/", name == "photo");
    } else {
      // Push description file
      if ((deep == 1) && ((name == "russian.txt") || (name == "english.txt"))) {
        std::string text;
        FsTools::LoadFile(full_path, SIZE_MAX, &text);

        auto a = text.find("\n\n");
        if (a == std::string::npos) {
          throw std::runtime_error("Incorrect file " + full_path);
        }
        auto b = text.find("\n\n", a + 1);
        if (b == std::string::npos) {
          throw std::runtime_error("Incorrect file " + full_path);
        }
        out += "\nname:";
        out += QuoteJs(text.substr(0, a));
        out += ",";
        out += "\nparams:";
        out += QuoteJs(text.substr(a + 2, b - a - 2));
        out += ",";
        out += "\nbody:";
        out += QuoteJs(text.substr(b + 2));
        out += ",";
      }

      // Push info file
      const std::string info_file_name = full_path + ".info";
      if (std::filesystem::exists(info_file_name)) {
        std::string info_file;
        FsTools::LoadFile(info_file_name, SIZE_MAX, &info_file);
        out += "\n";
        out += info_file;
        out += "\n";
      }
    }

    out += "},\n";
  }

  out += "}\n";
}

static void MakeCompJs() {
    MakeCompsJsState state;
    state.Start();
}

int main() {
  MakeCompJs(); //! try catch
  return 0;
}
