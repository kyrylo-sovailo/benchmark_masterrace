#!/usr/bin/env python3
import os, pickle, platform, shutil, subprocess, sys

#ALL_COMPILERS = [ "CMP_GCC", "CMP_LLVM", "CMP_GPP", "CMP_LLVMPP" ]
#ALL_COMPILER_OPTIONS = [ "OPT_DEFAULT", "OPT_NATIVE", "OPT_RTTI", "OPT_LTO", "OPT_BASE", "OPT_DEFAULT32", "OPT_NATIVE32", "OPT_RTTI32", "OPT_LTO32", "OPT_BASE32" ]
#ALL_VECTOR_STRUCTURES = [ "VEC_CPP", "VEC_POINTER", "VEC_SIZE_T", "VEC_UINT", "VEC_POINTER_NO_CAPACITY", "VEC_SIZE_T_NO_CAPACITY", "VEC_UINT_OFFSET", "VEC_STATIC" ]
#ALL_ALLOCATORS = [ "ALC_ALLOCATOR", "ALC_NEW", "ALC_NEW_NO_DELETE", "ALC_MALLOC", "ALC_REALLOC", "ALC_MALLOC_NO_FREE", "ALC_MMAP", "ALC_STATIC" ]
#ALL_MULTITHREADINGS = [ "MLT_NO", "MLT_CPP", "MLT_C", "MLT_PTHREAD", "MLT_CLONE" ]
#ALL_IOS = [ "IO_CPP_LINE", "IO_CPP_ERROR", "IO_CPP_STATE", "IO_C_LINE", "IO_C_ERROR", "IO_C_STATE", "IO_READ_STATE", "IO_MMAP_ERROR", "IO_MMAP_STATE" ]
#ALL_LIBCS = [ "LC_YES", "LC_NO" ]
#ALL_KNOWLEDGES = [ "PK_YES", "PK_NO" ]

ALL_COMPILERS = [ "CMP_GPP" ]
ALL_COMPILER_OPTIONS = [ "OPT_DEFAULT", "OPT_NATIVE", "OPT_RTTI", "OPT_LTO", "OPT_BASE", "OPT_DEFAULT32", "OPT_NATIVE32", "OPT_RTTI32", "OPT_LTO32", "OPT_BASE32" ]
ALL_VECTOR_STRUCTURES = [ "VEC_CPP", "VEC_POINTER", "VEC_SIZE_T", "VEC_UINT", "VEC_POINTER_NO_CAPACITY", "VEC_SIZE_T_NO_CAPACITY" ]
ALL_ALLOCATORS = [ "ALC_ALLOCATOR", "ALC_NEW", "ALC_NEW_NO_DELETE", "ALC_MALLOC", "ALC_REALLOC", "ALC_MALLOC_NO_FREE" ]
ALL_MULTITHREADINGS = [ "MLT_NO" ]
ALL_IOS = [ "IO_CPP_LINE", "IO_C_LINE" ]
ALL_LIBCS = [ "LC_YES" ]
ALL_KNOWLEDGES = [ "PK_NO" ]

ALL_STANDARDS = [
    ("CMP_GPP", "OPT_DEFAULT", "VEC_CPP", "ALC_ALLOCATOR", "MLT_NO", "IO_CPP_LINE", "LC_YES", "PK_NO"),
    ("CMP_GPP", "OPT_DEFAULT", "VEC_POINTER", "ALC_REALLOC", "MLT_NO", "IO_CPP_LINE", "LC_YES", "PK_NO")
]

RUNS_NUMBER = 10

def generate_all_configurations():
    configurations = { (compiler, compiler_options, vector_structure, allocator, multithreading, io, libc, knowledge)
        #for compiler in [ "CMP_GPP" ]
        #for compiler_options in [ "OPT_DEFAULT" ]
        #for vector_structure in [ "VEC_CPP", "VEC_POINTER", "VEC_SIZE_T", "VEC_UINT", "VEC_POINTER_NO_CAPACITY", "VEC_SIZE_T_NO_CAPACITY" ]
        #for allocator in [ "ALC_ALLOCATOR", "ALC_NEW", "ALC_NEW_NO_DELETE", "ALC_MALLOC", "ALC_REALLOC", "ALC_MALLOC_NO_FREE" ]
        #for multithreading in [ "MLT_NO" ]
        #for io in [ "IO_CPP_LINE" ]
        #for libc in [ "LC_YES" ]
        #for knowledge in [ "PK_NO" ]

        for compiler in ALL_COMPILERS
        for compiler_options in ALL_COMPILER_OPTIONS
        for vector_structure in ALL_VECTOR_STRUCTURES
        for allocator in ALL_ALLOCATORS
        for multithreading in ALL_MULTITHREADINGS
        for io in ALL_IOS
        for libc in ALL_LIBCS
        for knowledge in ALL_KNOWLEDGES
    }
    return configurations

def generate_configurations_around(configuration, include_configuration):
    configurations = []
    (compiler, compiler_options, vector_structure, allocator, multithreading, io, libc, knowledge) = configuration
    configurations += [ (r, compiler_options, vector_structure, allocator, multithreading, io, libc, knowledge) for r in ALL_COMPILERS ]
    configurations += [ (compiler, r, vector_structure, allocator, multithreading, io, libc, knowledge) for r in ALL_COMPILER_OPTIONS ]
    configurations += [ (compiler, compiler_options, r, allocator, multithreading, io, libc, knowledge) for r in ALL_VECTOR_STRUCTURES ]
    configurations += [ (compiler, compiler_options, vector_structure, r, multithreading, io, libc, knowledge) for r in ALL_ALLOCATORS ]
    configurations += [ (compiler, compiler_options, vector_structure, allocator, r, io, libc, knowledge) for r in ALL_MULTITHREADINGS ]
    configurations += [ (compiler, compiler_options, vector_structure, allocator, multithreading, r, libc, knowledge) for r in ALL_IOS ]
    configurations += [ (compiler, compiler_options, vector_structure, allocator, multithreading, io, r, knowledge) for r in ALL_LIBCS ]
    configurations += [ (compiler, compiler_options, vector_structure, allocator, multithreading, io, libc, r) for r in ALL_KNOWLEDGES ]
    if not include_configuration: configurations = [ c for c in configurations if c != configurations ]
    return configurations

def filter_plausible_configurations(configurations):
    plausible_configurations = []

    for configuration in configurations:
        compiler, compiler_options, vector_structure, allocator, multithreading, io, libc, knowledge = configuration

        # Check constraints
        cpp_required = (compiler_options in [ "OPT_RTTI", "OPT_RTTI32" ] or
            vector_structure in [ "VEC_CPP" ] or
            allocator in [ "ALC_ALLOCATOR", "ALC_NEW", "ALC_NEW_NO_DELETE" ] or
            multithreading in [ "MLT_CPP", "MLT_C", "MLT_CLONE" ] or
            io in [ "IO_CPP_LINE", "IO_CPP_ERROR", "IO_CPP_STATE" ]
            )
        if cpp_required and not compiler in [ "CMP_GPP", "CMP_LLVMPP" ]: continue

        knowledge_required = allocator in [ "VEC_STATIC" ] or allocator in [ "ALC_STATIC" ]
        if knowledge_required and not knowledge in [ "PK_YES" ]: continue

        cpp_vector_required = vector_structure in [ "VEC_CPP" ]
        if cpp_vector_required != (allocator in [ "ALC_ALLOCATOR" ]): continue

        uint_not_same_as_sizet = platform.architecture()[0] == '64bit'
        if uint_not_same_as_sizet and vector_structure in [ "VEC_UINT", "VEC_UINT_OFFSET" ]: continue

        libc_required = (vector_structure in [ "VEC_CPP" ] or
            not allocator in [ "ALC_MMAP", "ALC_STATIC" ] or
            not multithreading in [ "MLT_NO", "MLT_CLONE" ] or
            not io in [ "IO_READ_STATE", "IO_MMAP_STATE" ]
            )
        if libc_required and libc in [ "LC_NO" ]: continue

        has_capacity = vector_structure in [ "VEC_CPP", "VEC_POINTER", "VEC_SIZE_T", "VEC_UINT" ]
        has_free = allocator in [ "ALC_ALLOCATOR", "ALC_NEW", "ALC_MALLOC", "ALC_REALLOC" ]
        if (not has_capacity) and (not has_free): continue
        
        plausible_configurations.append(configuration)

    return plausible_configurations

def filter_unique_configurations(configurations):
    set_configurations = set()
    filtered_configurations = []
    for configuration in configurations:
        if configuration in set_configurations: continue
        set_configurations.add(configuration)
        filtered_configurations.append(configuration)
    return filtered_configurations

def print_configurations(configurations, infos=None):
    MIN_SPACE_BETWEEN = 2
    MIN_SPACE = 4

    def print_space(string, alignment):
        print(string, end=" " * max([ 0, alignment - len(string) ]))

    def remove_prefix(option):
        return option[option.find("_") + 1:]

    def find_space(options):
        return max([len(remove_prefix(r)) + MIN_SPACE_BETWEEN for r in options ] + [ MIN_SPACE ])

    max_cmp = find_space(ALL_COMPILERS)
    max_opt = find_space(ALL_COMPILER_OPTIONS)
    max_vec = find_space(ALL_VECTOR_STRUCTURES)
    max_alc = find_space(ALL_ALLOCATORS)
    max_mlt = find_space(ALL_MULTITHREADINGS)
    max_io = find_space(ALL_IOS)
    max_lc = find_space(ALL_LIBCS)
    max_pk = find_space(ALL_KNOWLEDGES)

    print(" " * MIN_SPACE, end='')
    print_space("CMP", max_cmp)
    print_space("OPT", max_opt)
    print_space("VEC", max_vec)
    print_space("ALC", max_alc)
    print_space("MLT", max_mlt)
    print_space("IO", max_io)
    print_space("LC", max_lc)
    print_space("PK", max_pk)
    print()
    
    if infos is None: infos = [ "" for _ in configurations ]
    for i, (configuration, info) in enumerate(zip(configurations, infos)):
        compiler, compiler_options, vector_structure, allocator, multithreading, io, libc, knowledge = configuration
        print_space(str(i+1), MIN_SPACE)
        print_space(remove_prefix(compiler), max_cmp)
        print_space(remove_prefix(compiler_options), max_opt)
        print_space(remove_prefix(vector_structure), max_vec)
        print_space(remove_prefix(allocator), max_alc)
        print_space(remove_prefix(multithreading), max_mlt)
        print_space(remove_prefix(io), max_io)
        print_space(remove_prefix(libc), max_lc)
        print_space(remove_prefix(knowledge), max_pk)
        print(info, end='')
        if configuration in ALL_STANDARDS: print(" (standard)", end='')
        print()

class Masterrace:
    def __init__(self):
        # Create build directory
        build_directory = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), "build")
        os.makedirs(build_directory, exist_ok=True)
        os.chdir(build_directory)

        # Copy source
        if not os.path.isfile("dijkstra_c.c") or (os.path.getmtime("../source/dijkstra_cpp.cpp") > os.path.getmtime("dijkstra_c.c")):
            command = "cp ../source/dijkstra_cpp.cpp dijkstra_c.c"
            print(command)
            command = command.split()
            shutil.copyfile(command[1], command[2])

        # Create benchmark
        if not os.path.isfile("create_dijkstra") or (os.path.getmtime("../source/create_dijkstra.cpp") > os.path.getmtime("create_dijkstra")):
            command = "g++ ../source/create_dijkstra.cpp -O3 -o create_dijkstra"
            print(command)
            subprocess.run(command, shell=True, check=True)
        if not os.path.isfile("dijkstra.txt") or (os.path.getmtime("create_dijkstra") > os.path.getmtime("dijkstra.txt")):
            command = "./create_dijkstra"
            print(command)
            subprocess.run(command, shell=True, check=True)

        # Create list of configurations
        explore_all = False
        if explore_all:
            self.configurations = generate_all_configurations()
        else:
            self.configurations = []
            for standard in ALL_STANDARDS: self.configurations += generate_configurations_around(standard, True)
        print(f"Total configurations    : {len(self.configurations)}")
        self.plausible_configurations = filter_plausible_configurations(self.configurations)
        self.plausible_configurations = filter_unique_configurations(self.plausible_configurations)
        print(f"Plausible configurations: {len(self.plausible_configurations)}")
        
        # Load results
        self.change = False
        try:
            with open("results.pkl", "rb") as f: self.results = pickle.load(f)
        except:
            self.results = dict()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.save()

    def save(self):
        if self.change:
            with open("results.pkl", "wb") as f: pickle.dump(self.results, f)
            self.change = False

    def print_configurations(self):
        print_configurations(self.plausible_configurations)
    
    def print_results(self):
        infos = []
        for configuration in self.plausible_configurations:
            result = self.results.get(configuration)
            if result is None: continue
            average = { "real":0, "user":0, "sys":0, "parse":0, "solve":0 }
            for key in average: average[key] += sum(result[key]) / len(result[key])
            infos.append((average["real"], configuration, f"{average["parse"]:0.4f} + {average["solve"]:0.4f} = {average["real"]:0.4f}"))
        infos = sorted(infos, key=lambda info: info[0])
        configurations = [ info[1] for info in infos ]
        infos = [ info[2] for info in infos ]
        print_configurations(configurations, infos)
    
    def build(self):
        for i, configuration in enumerate(self.plausible_configurations):
            compiler, compiler_options, vector_structure, allocator, multithreading, io, libc, knowledge = configuration
            binary = "_".join([ option[option.find("_") + 1:].replace("_", "").lower() for option in configuration ])
            if os.path.isfile(binary): continue
            command = {
                "CMP_GCC":"gcc dijkstra_c.c", "CMP_LLVM":"clang dijkstra_c.c", "CMP_GPP":"g++ ../source/dijkstra_cpp.cpp", "CMP_LLVMPP":"clang++ ../source/dijkstra_cpp.cpp"
            }[compiler]
            command += " -o " + binary
            command += " -O3 -DNDEBUG -Wall -Wextra -pedantic -Werror -Wno-unused-variable"
            #command += " -g -Wall -Wextra -pedantic -Werror -Wno-unused-variable"
            command += " " + {
                "OPT_DEFAULT":"", "OPT_NATIVE":"-march=native", "OPT_RTTI":"-fno-rtti", "OPT_LTO":"-flto", "OPT_BASE":"-fomit-frame-pointer"
            }[compiler_options[:-2] if compiler_options.endswith("32") else compiler_options]
            if compiler_options.endswith("32"): command += " -m32"
            command += " " + " ".join([ "-D" + option for option in configuration ])
            print(f"[{i+1}/{len(self.plausible_configurations)}] {command}")
            subprocess.run(command, shell=True, check=True)

    def test(self):
        def decode_time(string):
            units = { "h": 3600, "m": 60, "s": 1, "ms": 1e-3, "us": 1e-6, "ns": 1e-9 }
            numbers = ""
            letters = ""
            last_letter = False
            time = 0
            for c in string:
                if c in [ " ", "\t", "\n", "\r" ]: continue
                if c in [ '.',',' ] or c.isnumeric():
                    if last_letter:
                        time += float(numbers) * units[letters]
                        numbers = ""
                        letters = ""
                    numbers += c
                    last_letter = False
                else:
                    letters += c
                    last_letter = True
            if last_letter:
                time += float(numbers) * units[letters]
                numbers = ""
                letters = ""
            return time

        for i, configuration in enumerate(self.plausible_configurations):
            if configuration in self.results: continue
            binary = "_".join([ option[option.find("_") + 1:].replace("_", "").lower() for option in configuration ])
            command = f"taskset -c 1,3 bash -c \"for i in \\$(seq 1 {RUNS_NUMBER}); do time ./{binary}; done\""
            print(f"[{i+1}/{len(self.plausible_configurations)}] {command}")
            result = subprocess.run(command, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            stdout = result.stdout.decode()
            stderr = result.stderr.decode()
            print(stdout)
            print(stderr)
            if stdout.count("->") != 4 * RUNS_NUMBER: raise Exception(f"Not exactly {RUNS_NUMBER} results")
            result = { "real":[], "user":[], "sys":[], "parse":[], "solve":[] }
            for line in stderr.splitlines():
                for key in result:
                    if line.find(key) == -1: continue
                    result[key].append(decode_time(line.replace(key, "")))
            for key in result:
                if len(result[key]) != RUNS_NUMBER: raise Exception(f"Not exactly {RUNS_NUMBER} runs")
            
            self.results[configuration] = result
            self.change = True

def main():
    if len(sys.argv) == 2 and sys.argv[1] == "--print":
        with Masterrace() as m:
            m.print_configurations()
    elif len(sys.argv) == 2 and sys.argv[1] == "--result":
        with Masterrace() as m:
            m.print_results()
    elif len(sys.argv) == 2 and sys.argv[1] == "--build":
        with Masterrace() as m:
            m.build()
    elif len(sys.argv) == 2 and sys.argv[1] == "--test":
        with Masterrace() as m:
            m.build()
            m.test()
    #elif len(sys.argv) == 4 and sys.argv[1] == "--compare": raise Exception("Compare not implemented")
    #elif len(sys.argv) == 2 and sys.argv[1] == "--visualize": raise Exception("Visualize not implemented")
    else: raise Exception("Invalid usage")

if __name__ == "__main__":
    main()
