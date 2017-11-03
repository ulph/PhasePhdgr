#include "DirectoryWatcher.hpp"

namespace PhasePhckrFileStuff {

    StupidFileFilter g_fileFilter("json file");
    const FileFilter * getFilter() { 
        return &g_fileFilter; 
    }

}
