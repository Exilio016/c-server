#define BFUTILS_BUILD_IMPLEMENTATION
#define BFUTILS_BUILD_CFLAGS "-Wall -Werror -ggdb"
#include "bfutils_build.h"

void bfutils_build(int argc, char *argv[]) {
    BFUtilsBuildCfg server = {
        .name = "server",
        .files = (char*[]) { "server.c" },
        .files_len = 1,
    };
    bfutils_add_executable(server);
}
