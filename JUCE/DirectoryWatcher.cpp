#include "DirectoryWatcher.hpp"

#include <algorithm>

namespace PhasePhckrFileStuff {

    void createDirIfNeeded(File dir) {
        if (!dir.exists()) {
            auto res = dir.createDirectory();
            // somehow we can check if was ok
        }
    }

    TimeSliceThread g_fileWatchThread("fileWatchThread");
    TimeSliceThread & getThread() { 
        g_fileWatchThread.startThread();
        return g_fileWatchThread; 
    }

    StupidFileFilter g_fileFilter("json file");
    const FileFilter * getFilter() { 
        return &g_fileFilter; 
    }

}