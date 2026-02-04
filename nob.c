#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build"

#ifdef _WIN32
# define EXE_EXTENSION ".exe"
#else
# define EXE_EXTENSION
#endif

typedef struct {
  char *executable;
  int line;
  char *inputs;
  char *cflags;
  char *defines;
  char *includes;
  char *libraries;
  char *lflags;
} Target;

typedef struct {
  Target *items;
  size_t count;
  size_t capacity;
} Targets;

Procs procs = { 0 };
bool debug;

// prefix each item of a space separated list
void append_each(Cmd *cmd, char *prefix, char *list) {
  char *head = list;
  char *tail = head;
  while (head && *head != 0) {
    while (*tail != 0 && !isspace(*tail)) tail++;
    String_View sv = sv_from_parts(head, tail - head);
    char *lib      = nob_temp_sprintf("%s" SV_Fmt, prefix, SV_Arg(sv));
    nob_cmd_append(cmd, lib);
    if (*tail != 0) tail++;
    head = tail;
  }
}

void build_target(Cmd *cmd, Target target, String_Builder *commands_json) {
  nob_cc(cmd);
  nob_cc_flags(cmd); // default commonly used flags
  nob_cmd_append(cmd, "-Wno-unused-function", "-Wno-unused-parameter", "-Wno-unused-variable");
  if (debug) nob_cmd_append(cmd, "-ggdb");
  if (target.cflags) nob_cmd_append(cmd, target.cflags);
  if (target.defines) append_each(cmd, "-D", target.defines);
  if (target.includes) nob_cmd_append(cmd, target.includes);

  char inputs_realpath[4096];
  char *inputs = temp_sprintf("%s", target.inputs);

  // just put the first input file into the compile_commands.json
  char *head = target.inputs;
  char *tail = head;
  while (tail && *tail != 0 && !isspace(*tail)) tail++;
  String_View sv    = nob_sv_from_parts(head, tail - head);
  char *first_input = temp_sprintf(SV_Fmt, SV_Arg(sv));

#ifdef _WIN32
  GetFullPathNameA(first_input, sizeof(inputs_realpath), inputs_realpath, NULL);
#else
  realpath(first_input, inputs_realpath);
#endif
  append_each(cmd, "", inputs);

  char outputs_realpath[4096];
  char *outputs = temp_sprintf("%s/%s", BUILD_FOLDER, target.executable);
#ifdef _WIN32
  GetFullPathNameA(outputs, sizeof(outputs_realpath), outputs_realpath, NULL);
#else
  realpath(outputs, outputs_realpath);
#endif
  nob_cc_output(cmd, outputs);

  Nob_String_Builder compile_command = { 0 };
  nob_cmd_render(*cmd, &compile_command);
  sb_append_null(&compile_command);

  if (target.libraries) append_each(cmd, "-l", target.libraries);
  if (target.lflags) append_each(cmd, "", target.lflags);

  char cwd[4096];
  getcwd(cwd, 4096);
  printf("%s:%d: info: exe %s of files %s\n", __FILE__, target.line, target.executable, target.inputs);

  sb_appendf(commands_json, "{\n"
      "\"directory\": \"%s\",\n"
      "\"command\": \"%s\",\n"
      "\"file\": \"%s\",\n"
      "\"output\": \"%s\"\n"
      "}",
      cwd, compile_command.items, inputs_realpath, outputs_realpath);

  if (!cmd_run(cmd, .async = &procs)) exit(1);
}

//~ ONE BUILD TO RULE THEM ALL
int main(int argc, char **argv) {
  GO_REBUILD_URSELF(argc, argv);
  UNUSED(shift(argv, argc)); // programname
  if (!mkdir_if_not_exists(BUILD_FOLDER)) return 1;

  for (int i = 0; i < argc; i++) {
    if (strcmp("-d", argv[i]) == 0) {
      debug = true;
      nob_shift(argv, argc);
    }
  }

  Cmd cmd                      = { 0 };
  Targets targets              = { 0 };
  String_Builder commands_json = { 0 };

  da_append(&targets, ((Target) { .executable = "show", .inputs = "src/main.c src/lib/csapp.c", .defines = "_GNU_SOURCE", .lflags = "-Wl,-rpath,'$ORIGIN/lib'", .libraries = "SDL2 SDL2_ttf pthread m dl X11 OpenGL", .line = __LINE__ }));
  da_append(&targets, ((Target) { .executable = "libslider.so", .inputs = "src/show.c src/lib/csapp.c", .defines = "_GNU_SOURCE", .lflags = "-fPIC -shared", .line = __LINE__ }));
  da_append(&targets, ((Target) { .executable = "image-mediainfo", .inputs = "scratch/image-mediainfo.c", .libraries = "SDL2 m", .line = __LINE__ }));
  da_append(&targets, ((Target) { .executable = "inotify", .inputs = "scratch/inotify-man-page-example.c", .line = __LINE__ }));
  da_append(&targets, ((Target) { .executable = "str-to-double", .inputs = "scratch/str-to-double.c src/lib/csapp.c", .defines = "_GNU_SOURCE", .line = __LINE__ }));
  da_append(&targets, ((Target) { .executable = "window", .inputs = "scratch/window.c", .libraries = "SDL2 SDL2_ttf", .line = __LINE__ }));

  if (argc <= 0) {

    sb_append_cstr(&commands_json, "[\n");
    da_foreach (Target, it, &targets) {
      size_t index = it - targets.items;
      // NOTE:(sloth) build all the programs named above
      build_target(&cmd, *it, &commands_json);
      if (index != targets.count - 1)
        sb_append_cstr(&commands_json, ",\n");
    };
    if (!nob_procs_flush(&procs)) return 1;
    sb_append_cstr(&commands_json, "\n]\n");
    nob_write_entire_file("compile_commands.json", commands_json.items, commands_json.count);

    mkdir_if_not_exists(BUILD_FOLDER "/lib");
    if (file_exists(BUILD_FOLDER "/libslider.so"))
      nob_rename(BUILD_FOLDER "/libslider.so", BUILD_FOLDER "/lib/libslider.so");

    // .cpp - these files were cpp files
    // TODO: da_append(&names, "lsmicrophones"); needs to link with -lasound
    // TODO: da_append(&names, "find-usb"); needs to link with -ludev
    // TODO: da_append(&names, "udev-search"); needs to link with -ludev
    // .c files below
    // TODO: da_append(&names, "pci"); needs to link with -lpci
  } else {
    do {
      //- each arg
      char *arg = shift(argv, argc);
      nob_log(NOB_INFO, "build single program %s", arg);

      // NOTE:(sloth) parse args as [programname] [args ...] and run it
      da_foreach (Target, target, &targets) {
        if (strcmp(arg, target->executable) == 0) {
          build_target(&cmd, *target, &(String_Builder) { 0 });
          // char *binfile = temp_sprintf("%s/%s" EXE_EXTENSION, BUILD_FOLDER, target->executable);
          // cmd_append(&cmd, binfile);
          // while (argc > 0) cmd_append(&cmd, shift(argv, argc));
          // if (!cmd_run(&cmd)) return 1;
        }
      }
    } while (argc > 0);
  }

  //- end
  nob_log(NOB_INFO, "Successful build exit :)");
  return 0;
}
