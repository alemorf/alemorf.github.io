#include <iostream>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <map>
 
std::string out;
std::map<time_t, std::string> history;

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

static bool NeedUpdate(struct stat& src_stat, const std::string& dest_name) {            
    struct stat dest_stat;
    return (lstat(dest_name.c_str(), &dest_stat) == -1) || (dest_stat.st_mtime < src_stat.st_mtime);
}

static std::string QuoteJs(const std::string& source) {
    static const char* abc = "0123456789ABCDEF";
    std::string out;
    out.reserve(source.size() * 4 + 2);
    out += "\"";
    for (auto c : source) {
        uint8_t code = uint8_t(c);
        if ((code < 32) || (c == '"')) {
            out += "\\x"; out += abc[code >> 4u]; out += abc[code & 0x0Fu];
        } else {
            out += c;
        }
    }
    out += "\"";
    return out;
}

static std::string LoadFile(const std::string& file_name) {            
    const int fd = open(file_name.c_str(), O_RDONLY, 0666);
    if (fd == -1) {
        throw std::runtime_error("Failed to open file " + file_name +
                                 ", errno " + std::to_string(errno));
    }    
    struct stat buff;
    auto result0 = fstat(fd, &buff);
    if (result0 != 0) {
        throw std::runtime_error("Failed to check file size " + file_name +
                                 ", errno " + std::to_string(errno));
    }
    
    std::string out;
    if (buff.st_size != 0) {
        out.resize(buff.st_size);
        
        if (read(fd, out.data(), out.size()) != out.size()) {
            throw std::runtime_error("Failed to read file " + file_name +
                                     ", errno " + std::to_string(errno));
        }
    }
    
    if (close(fd) != 0) {
        throw std::runtime_error("Failed to close file " + file_name +
                                 ", errno " + std::to_string(errno));
    }
    
    return out;
}

void level(uint32_t deep, std::string upPath, std::string relPath, unsigned photo)
{
    DIR *dir = opendir(upPath.c_str());
    if (dir == NULL)
    {
        //cerr << "error: opendir " << upDir << endl << "result path: " << resultPath.c_str() << endl;
        return;
    }

    int f = open((upPath+"/info").c_str(), O_RDONLY);
    if(f!=-1)
    {
        for(;;)
        {
            char buf[1024];        
            int l = read(f, buf, sizeof(buf)-1);
            if(l<=0) break;
            buf[l]=0;
            out += buf;
        }
        close(f);
    }
    out += " items:{\n";

    dirent *de;
    while ((de = readdir(dir)) != NULL)
    {
        if ((strcmp(de->d_name, ".")) == 0) continue;
        if ((strcmp(de->d_name, "..")) == 0) continue;
        if ((strcmp(de->d_name, "info")) == 0) continue;
        if ((strcmp(de->d_name, ".git")) == 0) continue;

        std::string fullPath = upPath + "/" + de->d_name;
        if(de->d_name[strlen(de->d_name)-1] == '~')
	    {
	        printf("kill %s\n", fullPath.c_str());
	        unlink(fullPath.c_str());	
	        continue;
	    }

        if(fullPath.size()>8 && 0==strcmp(fullPath.c_str()+fullPath.size()-8, ".jpg.jpg")) continue;
        if(fullPath.size()>8 && 0==strcmp(fullPath.c_str()+fullPath.size()-8, ".png.jpg")) continue;
        if(fullPath.size()>5 && 0==strcmp(fullPath.c_str()+fullPath.size()-5, ".info")) continue;

        struct stat destat;
        if (lstat(fullPath.c_str(), &destat) == -1) continue;

        const char* ext = fullPath.size()>4 ? (fullPath.c_str()+fullPath.size()-4) : "";
        if(photo) {
            if((0==strcmp(ext, ".jpg") || 0==strcmp(ext, ".png")))
            {
                const std::string small_photo_name = relPath + de->d_name;
                struct stat destat2;
                if (lstat(small_photo_name.c_str(), &destat2) == -1 || destat2.st_mtime < destat.st_mtime)
                {
                    RecursiveMkdir(relPath);
                    char cmd[1024];
                    sprintf(cmd, "convert %s -quality 85 -resize 300\\>x200\\> %s", fullPath.c_str(), small_photo_name.c_str());
                    printf("%s\n", cmd);
                    system(cmd);
                }
            }
        }
        
        //! Если были изменены файлы в папках doc и photo, то поставить html на изменение.
        bool html = fullPath.size()>5 && 0==strcmp(fullPath.c_str()+fullPath.size()-5, ".html");
//        bool jpg  = fullPath.size()>4 && 0==strcmp(fullPath.c_str()+fullPath.size()-4, ".jpg");
//        bool png  = fullPath.size()>4 && 0==strcmp(fullPath.c_str()+fullPath.size()-4, ".png");

        if(html)
        {
            history[destat.st_mtime] = fullPath.c_str() + 12;
        }        

        if((destat.st_mode & S_IFMT) == S_IFDIR)
        {        
            // "dir":{dir:1,info,items:{...}}
            out += "\"", out += de->d_name, out += "\":{\ndir:1,";
    	    unsigned photo1 = (0==strcmp(de->d_name, "photo"));
            level(deep + 1, fullPath, relPath + de->d_name + "/", photo1);
            out += "},\n";
        }
        else
        {            
            // "file":{}
            out += "\"", out += de->d_name, out += "\":{\n";

        if ((deep == 1) && (0 == strcmp(de->d_name, "russian.txt"))) {
            std::string text = LoadFile(fullPath);
            auto a = text.find("\n\n");
            if (a == std::string::npos) {
                throw std::runtime_error("Incorrect file " + fullPath);
            }
            auto b = text.find("\n\n", a + 1);
            if (b == std::string::npos) {
                throw std::runtime_error("Incorrect file " + fullPath);
            }
            out += "name:" + QuoteJs(text.substr(0, a)) + ",\n";
            out += "params:" + QuoteJs(text.substr(a + 2, b - a - 2)) + ",\n";
            out += "body:" + QuoteJs(text.substr(b + 2)) + ",\n";
        }

	    int f1 = open((upPath+"/"+de->d_name+".info").c_str(), O_RDONLY);
	    if(f1!=-1)
	    {
		out += "\n";
	        for(;;)
	        {
	            char buf[1024];        
	            int l = read(f1, buf, sizeof(buf)-1);
	            if(l<=0) break;
	            buf[l]=0;
                    out += buf;
	        }
	        close(f1);
	    }
	    out += "},\n";
        }
    }

    out += "}\n";
    
    closedir(dir);
}

int main()
{
    std::cout << "Make comps.js" << std::endl;

    out += "comps = {\n";
    level(0, "../../retro_computers", "", false);
    out += "};\n";

    // Save file
    int fd = open("comps.js", O_RDWR|O_CREAT|O_TRUNC, 0666);
    if (fd == -1) {
         std::cerr << "Failed to open file comps.js, errno " << errno << std::endl;
         return 1;
    }
    if (write(fd, out.c_str(), out.size()) != out.size()) {
         std::cerr << "Failed to write file comps.js, errno " << errno << std::endl;
         return 1;
    }
    if (close(fd) != 0) {
         std::cerr << "Failed to close file comps.js, errno " << errno << std::endl;
    }

    return 0;
}
