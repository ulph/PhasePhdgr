#include "DirectoryWatcher.hpp"

namespace PhasePhdgrFileStuff {

    StupidFileFilter g_fileFilter("json file");
    const FileFilter * getFilter() { 
        return &g_fileFilter; 
    }

}
