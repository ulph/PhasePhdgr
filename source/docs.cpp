#include "docs.hpp"
#include "moduleregister.hpp"
#include "connectiongraph.hpp"
#include "module.hpp"

std::vector<ModuleDoc> *g_moduleDocs = nullptr;

void PhasePhckr::RefreshDocs() {
    delete g_moduleDocs;
    g_moduleDocs = new std::vector<ModuleDoc>;
    ConnectionGraph cg;
    ModuleRegister::registerAllModules(cg);
    cg.makeModuleDocs(*g_moduleDocs);
}

const std::vector<ModuleDoc> & PhasePhckr::GetModuleDocs() {
    if (!g_moduleDocs) PhasePhckr::RefreshDocs();
    return *g_moduleDocs;
}
