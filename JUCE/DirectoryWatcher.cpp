#include "DirectoryWatcher.hpp"

#include <algorithm>

namespace PhasePhckrFileStuff {

    void createDirIfNeeded(File dir) {
        if (!dir.exists()) {
            auto res = dir.createDirectory();
            // somehow we can check if was ok
        }
    }

    StupidFileFilter g_fileFilter("json file");
    const FileFilter * getFilter() { 
        return &g_fileFilter; 
    }

}