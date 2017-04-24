#pragma once

#include "JuceLibraryCode/JuceHeader.h"
#include "docs.hpp"
#include <sstream>

struct LambdaTimer : Timer {
    LambdaTimer(std::function<void()> *callBack) {
        cb = callBack;
    }
    virtual void timerCallback() {
        (*cb)();
    }
    virtual ~LambdaTimer() {
        delete cb;
    }
private:
    const std::function<void(void)> *cb;
};

struct InterceptStringStream {
    InterceptStringStream(std::ostream & stream)
        : oldBuf(stream.rdbuf(newBuf.rdbuf()))
    {
    }
    ~InterceptStringStream() {
        std::cout.rdbuf(oldBuf);
    }
    void readAll(std::string & target) {
        char ch = newBuf.rdbuf()->sbumpc();
        while (ch != EOF) {
            target += ch;
            ch = newBuf.rdbuf()->sbumpc();
        }
    }
private:
    std::streambuf * oldBuf;
    std::stringstream newBuf;
};

class DocListModel : public ListBoxModel {
private:
    std::map<std::string, ModuleDoc> moduleDocs;
    std::vector<std::string> rows;
    TextEditor & docView;
public:
    DocListModel(const std::map<std::string, ModuleDoc> & moduleDocs, TextEditor & docView)
        : ListBoxModel()
        , moduleDocs(moduleDocs)
        , docView(docView)
    {
        for (const auto &kv : moduleDocs) {
            rows.push_back(kv.first);
        }
    }
    virtual int getNumRows() {
        return rows.size();
    }
    virtual void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) {
        const auto &key = rows[rowNumber];
        const auto &doc = moduleDocs.at(key);
        g.drawFittedText(doc.type, 0, 0, width, height, Justification::centred, 1);
    }
    virtual void listBoxItemClicked(int row, const MouseEvent &) {
        if (row >= 0) {
            const auto &key = rows[row];
            const auto &doc = moduleDocs[key];
            docView.clear();
            docView.insertTextAtCaret(doc.type + "\n\n");
            docView.insertTextAtCaret("inputs:\n");
            for (const auto & i : doc.inputs) {
                docView.insertTextAtCaret("  " + i.name + ((i.unit != "") ? (" [" + i.unit + "]") : "") + " " + std::to_string(i.value) + "\n");
            }
            docView.insertTextAtCaret("\noutputs:\n");
            for (const auto & o : doc.outputs) {
                docView.insertTextAtCaret("  " + o.name + ((o.unit != "") ? (" [" + o.unit + "]") : "") + "\n");
            }
            docView.insertTextAtCaret("\n\n" + doc.docString);
            docView.moveCaretToTop(false);
        }
    }
};

template <class T>
class SubValue {
private:
    T value;
    int ctr;
    std::map<int, std::function<void(const T&)>> listeners;
public:
    SubValue() : ctr(0) {}
    int subscribe(std::function<void(const T&)> callback) {
        listeners.emplace(ctr, callback);
        return ctr++;
    }
    void unsubscribe(int handle) {
        listeners.erase(handle);
    }
    void set(int handle, const T& newValue) {
        value = newValue;
        for (const auto &l : listeners) {
            if (l.first != handle) {
                l.second(value);
            }
        }
    }
};
