import phasephckr as ph

module_renames = { # straight forward rename / replace
    "RCHP" : "D_HP",
    "RCLP" : "D_LP",
    "PBLOSC" : "OSC_BLIT",
    "BIQUAD" : "BQ_FILTER",
    "BQLPF" : "BQ_LP",
    "BQPEAK" : "BQ_PEAK",
    "SVF" : "ZDF_SVF",
    "OSVF" : "ZDF_OSVF",
    "CHAMBFLT" : "ZDF_SVF", # not exactly correct on res/q ... but hey
    "OCHAMBFLT" : "ZDF_OSVF",
    "ENV" : "CAMELENV",
    "MULTRI" : "MUL_TRI",
    "MULQUAD" : "MUL_QUAD",
    "SATAN" : "NATAN",
    "SSATAN" : "SNATAN",
    "SCLSHFT" : "MULADD",
    "CINV" : "CLAMPINV",

    "@STEREOTAPE" : "@FACTORY.STEREOTAPE",
    "@ADSR" : "@FACTORY.ADSR"

}

port_renames = {
    "CAMELENV" : {
        "outputs" : {
            "value" : "out"
        }        
    },

    "@FACTORY.ADSR" : {
        "outputs" : {
            "value" : "out"
        }        
    },

    "ABS" : {
        "inputs" : {
            "input" : "in"
        },
        "outputs" : {
            "abs" : "out"
        }
    },

    "BQ_FILTER" : {
        "inputs" : {
            "input" : "in"
        },
        "outputs" : {
            "output" : "out"
        }        
    },

    "BQ_LP" : {
        "inputs" : {
            "f0" : "fc"
        }        
    },

    "BQ_PEAK" : {
        "inputs" : {
            "f0" : "fc"
        }    
    },

    "OSC_BLIT" : {
        "outputs" : {
            "output" : "out"
        }        
    },

    "ENV" : {
        "outputs" : {
            "value" : "out"
        }        
    },

    "XFADE" : {
        "inputs" : {
            "first" : "a",
            "second" : "b",
            "crossfade" : "mix"
        },
        "outputs" : {
            "output" : "out"
        }        
    },

    "FADEX" : {
        "inputs" : {
            "input" : "in",
            "fadecross" : "mix"
        },
        "outputs" : {
            "first" : "a",
            "second" : "b"
        }        
    },

    "FOLD" : {
        "inputs" : {
            "input" : "in"
        },
        "outputs" : {
            "output" : "out"
        }        
    },    

    "WRAP" : {
        "inputs" : {
            "input" : "in"
        },
        "outputs" : {
            "output" : "out"
        }        
    },

    "MULADD" : {
        "inputs" : {
            "input" : "in",
            "scale" : "mul",
            "shift" : "add"
        },
        "outputs" : {
            "output" : "out"
        }        
    },

    "CLAMPINV" : {
        "inputs" : {
            "input" : "in"
        },
        "outputs" : {
            "output" : "out"
        }        
    },  

    "TRESH" : {
        "inputs" : {
            "input" : "in"
        },
        "outputs" : {
            "binary" : "out"
        }        
    },        

    "COUNTER" : {
        "inputs" : {
            "trigger" : "in"
        },
        "outputs" : {
            "counter" : "out"
        }        
    },         

    "CLAM" : {
        "outputs" : {
            "clamp" : "out"
        }
    },   

    "MUL" : {
        "inputs" : {
            "in1" : "in"
        },
        "outputs" : {
            "prod" : "out"
        }        
    },

    "MUL_TRI" : {
        "inputs" : {
            "in1" : "in"
        },
        "outputs" : {
            "prod" : "out"
        }        
    },      

    "MUL_QUAD" : {
        "inputs" : {
            "in1" : "in"
        },
        "outputs" : {
            "prod" : "out"
        }        
    },      

    "D_HP" : {
        "inputs" : {
            "x1" : "in"
        },
        "outputs" : {
            "y1" : "out"
        }        
    },      

    "D_LP" : {
        "inputs" : {
            "x1" : "in"
        },
        "outputs" : {
            "y1" : "out"
        }            
    },        

    "SAMPHOLD" : {
        "inputs" : {
            "sample" : "in"
        },
        "outputs" : {
            "value" : "out"
        }            
    },          

    "SPOW" : {
        "outputs" : {
            "pow" : "out"
        }            
    },      

    "SLOG2" : {
        "inputs" : {
            "value" : "in"
        },
        "outputs" : {
            "log" : "out"
        }            
    },      

    "ZDF_1P" : {
        "outputs" : {
            "lp" : "low",
            "hp" : "high",
            "all" : "all"
        }
    },

    "ZDF_SVF" : {
        "inputs" : {
            "input" : "in",
            "q" : "res",
            "wc" : "fc"
        }
    },

    "ZDF_OSVF" : {
        "inputs" : {
            "input" : "in",
            "q" : "res",
            "wc" : "fc"
        }
    },

    "TANH" : {
        "outputs" : {
            "tanh" : "out"
        }
    },

    "NTANH" : {
        "outputs" : {
            "tanh" : "out"
        }
    },

    "CLAMP" : {
        "outputs" : {
            "clamp" : "out"
        }
    }
}

import os, json
path = "."

SEP = "/"

def fix_stuff(data):
    instances_to_potentially_fix = []
    fixed_module_names = {}

    if "modules" in data:
        for i, module in enumerate(data["modules"]):
            if "@" in module[0]:
                new_name = module[0].replace("@", "_at_")
                fixed_module_names[module[0]] = new_name
                module[0] = new_name

            if module[1] in module_renames:
                print(module[1] + " -> " + module_renames[module[1]])
                data["modules"][i][1] = module_renames[module[1]]

            if module[1] in port_renames:
                instances_to_potentially_fix.append(module)

    if "connections" in data:
        for i, connection in enumerate(data["connections"]):
            if connection[0] in fixed_module_names:
                connection[0] = fixed_module_names[connection[0]]
            if connection[2] in fixed_module_names:
                connection[2] = fixed_module_names[connection[2]]

            from_name = connection[0]
            from_port = connection[1]
            to_name = connection[2]
            to_port = connection[3]

            for instance in instances_to_potentially_fix:
                name = instance[0]

                if name == to_name:
                    type = instance[1]
                    bay = "inputs"
                    if bay in port_renames[type]:
                        for port in port_renames[type][bay]:
                            if port == to_port:
                                data["connections"][i][3] = port_renames[type][bay][port]
                                print(type + "." + bay + "." + port + " -> " + port_renames[type][bay][port])

                if name == from_name:
                    type = instance[1]
                    bay = "outputs"
                    if bay in port_renames[type]:
                        for port in port_renames[type][bay]:
                            if port == from_port:
                                data["connections"][i][1] = port_renames[type][bay][port]
                                print(type + "." + bay + "." + port + " -> " + port_renames[type][bay][port])

    if "values" in data:
        for i, value in enumerate(data["values"]):
            if value[0] in fixed_module_names:
                value[0] = fixed_module_names[value[0]]

            value_name = value[0]
            value_port = value[1]
            for instance in instances_to_potentially_fix:
                name = instance[0]
                if value_name == name:
                    type = instance[1]
                    bay = "inputs"
                    if bay in port_renames[type]:
                        for port in port_renames[type][bay]:
                            if port == value_port:
                                data["values"][i][1] = port_renames[type][bay][port]
                                print(type + "." + bay + "." + port + " -> " + port_renames[type][bay][port])

    dive = ["voice", "effect", "root", "graph", "components"]

    for thing in dive:
        if thing in data:
            if thing == "components":
                for component in data[thing]:
                    fix_stuff(data[thing][component])
            else:
                fix_stuff(data[thing])

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
                content = json.loads(data)
                fix_stuff(content)
                open(filename, 'w').write(json.dumps(content, indent=2))

            except Exception as e:
                print("error {!s} - {!s}".format(filename, e.msg))
