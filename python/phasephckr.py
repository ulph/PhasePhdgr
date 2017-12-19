#!/usr/bin/env python3

import json
import os
import os.path
import argparse

SEP = "/"
COMPONENT = "component"
VOICE = "voice"
EFFECT = "effect"
PRESET = "preset"

ALL = [COMPONENT, VOICE, EFFECT, PRESET]

missing = set()
broken = set()

def discover_all(path):
    result = {}
    for dirpath, dirnames, filenames in os.walk(path):
        root = os.path.relpath(dirpath, path)
        for f in filenames:
            n, e = os.path.splitext(f)
            if e == ".json":
                filename = os.path.join(dirpath, f)
                prefix = SEP.join(dirpath.split(os.sep)) + SEP
                name = prefix + n
                data = open(filename,'r').read()
                try:
                    result[name] = json.loads(data)
                except:
                    broken.add(filename)
    return result

FLAT_BLOB = dict()
BLOB = dict()

def scan():
    BLOB[COMPONENT] = discover_all(COMPONENT)
    BLOB[VOICE] = discover_all(VOICE)
    BLOB[EFFECT] = discover_all(EFFECT)
    BLOB[PRESET] = discover_all(PRESET)

    FLAT_BLOB.update(BLOB[COMPONENT])
    FLAT_BLOB.update(BLOB[VOICE])
    FLAT_BLOB.update(BLOB[EFFECT])
    FLAT_BLOB.update(BLOB[PRESET])

def find_dependencies_on(something, include_modules=False):
    who = set()
    for k, v in FLAT_BLOB.items():
        dep, local = find_dependencies_off(v, include_modules=include_modules)
        if something in dep:
            who.add(k)
    return who

def find_dependencies_off(data, include_modules=False):
    deps = set()
    locals = set()
    if "root" in data:
        d, l = find_dependencies_off(data["root"], include_modules=include_modules)
        deps.update(d)
        locals.update(l)
    if "graph" in data:
        d, l = find_dependencies_off(data["graph"], include_modules=include_modules)
        deps.update(d)
        locals.update(l)
    if "components" in data:
        for k, v in data["components"].items():
            locals.add(k)
    if "modules" in data:
        for k, v in data["modules"]:
            if v[0] == "@":
                deps.add(v)
            elif include_modules:
                deps.add(v)

    return deps, locals

def resolve_component_or_module(cmp):
    if cmp.split(SEP)[0] == COMPONENT: return "@"+SEP.join(cmp.split(SEP)[1:])
    return cmp

def only_components(arg):
    if not arg.startswith(COMPONENT):
        print("Only applicable for "+COMPONENT+"\nSee --module")
        return False
    return True

if __name__ == "__main__":
    import sys 

    p = argparse.ArgumentParser(description="PhasePhckr tool - analyse, fix and manipulate user data")
    p.add_argument('--overview', help="overview", action="store_const", const=1, default=0)
    p.add_argument('--list', choices=ALL)
    p.add_argument('--doff', help="list dependecies of")
    p.add_argument('--don', help="list dependecies on")
    p.add_argument('--modules', help="include modules in analysis/operation", action="store_const", const=1, default=0)
    args = p.parse_args()

    if not sys.argv[1:]:
        p.print_help()
        exit()

    scan()

    if args.overview:
        print("")
        for x in ALL:
            print(x+": "+str(len(BLOB[x])))
        if len(broken):
            print("")
            print("possibly broken:\n"+"\n".join(broken))

    if args.list:
        print("listing "+args.list+":\n")
        print("\n".join(BLOB[args.list].keys()))

    if args.doff:
        if(args.doff in FLAT_BLOB):
            print(args.doff)
            deps, locals = list(find_dependencies_off(FLAT_BLOB[args.doff], include_modules=args.modules))
            if deps: print("depends on:\n"+("\n".join(deps)))
            if locals: print("defines:\n"+("\n".join(locals)))
        else:
            print("No such thing!")

    if args.don:
        if args.modules or only_components(args.don):
            print(args.don)
            what = resolve_component_or_module(args.don)
            if what[0] == "@":
                print("aka "+what)
                if args.don not in FLAT_BLOB:
                    print("is not defined")            
            who = list(find_dependencies_on(what, include_modules=args.modules))
            if who:
                print("is a dependency to:\n")
                print("\n".join(who))
            else:
                print("is not a dependency to anything else")