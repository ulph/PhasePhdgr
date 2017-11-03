#include "DirectoryWatcher.hpp"

#include <algorithm>
#include <iostream>

using namespace std;

namespace PhasePhckrFileStuff {

    void createDirIfNeeded(File dir) {
        cout << "resource directory " + dir.getFullPathName();
        if (!dir.exists()) {
            auto res = dir.createDirectory();
            if(res.ok()){
                cout << " created" << endl;
            }
            else{
                cout << " was not created: " << endl;
                cerr << res.getErrorMessage() << endl;
            }
        }
        else{
            cout << " exists" << endl;
        }

    }

    StupidFileFilter g_fileFilter("json file");
    const FileFilter * getFilter() { 
        return &g_fileFilter; 
    }

}
