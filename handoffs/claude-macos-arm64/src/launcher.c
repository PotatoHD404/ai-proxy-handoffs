#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "proxy-config.h"
static void set_value(const char *name, const char *value) {
    if (setenv(name, value, 1) != 0) {
        perror(name);
        exit(126);
    }
}

static void clear_value(const char *name) {
    if (unsetenv(name) != 0) {
        perror(name);
        exit(126);
    }
}

int main(int argc, char **argv) {
    char executable[PATH_MAX];
    char resolved[PATH_MAX];
    uint32_t length = sizeof(executable);
    if (_NSGetExecutablePath(executable, &length) || !realpath(executable, resolved)) {
        fputs("Could not resolve Claude launcher path.\n", stderr);
        return 126;
    }

    const struct passwd *account = getpwuid(getuid());
    if (!account) {
        fputs("Could not resolve the current macOS account.\n", stderr);
        return 126;
    }
    char data[PATH_MAX];
#ifdef DESKTOP
    const char *runtimeName = "Claude-runtime";
    const char *dataName = "Desktop";
#else
    const char *runtimeName = "claude-runtime";
    const char *dataName = "Code";
#endif
    if (snprintf(data, sizeof(data), "%s/Library/Application Support/Claude Direct Proxy/%s",
                 account->pw_dir, dataName) >= sizeof(data)) {
        return 126;
    }
    char runtime[PATH_MAX];
    const char *executableDirectory = dirname(resolved);
    if (snprintf(runtime, sizeof(runtime), "%s/%s", executableDirectory, runtimeName) >= sizeof(runtime)) {
        return 126;
    }
    int first = 1;
    char proxyLine[512];
    const char *proxy = read_proxy(executableDirectory, argc, argv, &first, proxyLine, sizeof(proxyLine));
    if (!proxy || strlen(proxy) >= sizeof(proxyLine)) {
        fputs("Set config/proxy.txt, HANDOFF_PROXY, or --proxy to an HTTP(S) URL with an explicit port.\n", stderr);
        return 64;
    }
    const char *proxyVars[] = {"HTTP_PROXY", "HTTPS_PROXY", "ALL_PROXY", "http_proxy", "https_proxy", "all_proxy"};
    for (size_t i = 0; i < sizeof(proxyVars) / sizeof(proxyVars[0]); i++) {
        set_value(proxyVars[i], proxy);
    }
    set_value("NO_PROXY", "");
    set_value("no_proxy", "");
    set_value("DISABLE_AUTOUPDATER", "1");
    char toolWrapper[PATH_MAX];
    char toolWrapperPath[PATH_MAX];
#ifdef DESKTOP
    const char *wrapperRelative = "../../../bin/host-tool";
#else
    const char *wrapperRelative = "host-tool";
#endif
    if (snprintf(toolWrapper, sizeof(toolWrapper), "%s/%s", executableDirectory, wrapperRelative) >= sizeof(toolWrapper) ||
        !realpath(toolWrapper, toolWrapperPath) || access(toolWrapperPath, X_OK) != 0) {
        fputs("Could not locate the local Claude tool launcher.\n", stderr);
        return 126;
    }
    set_value("CLAUDE_CODE_SHELL_PREFIX", toolWrapperPath);
    clear_value("ELECTRON_RUN_AS_NODE");
#ifdef DESKTOP
    set_value("CLAUDE_USER_DATA_DIR", data);
    char app[PATH_MAX];
    if (snprintf(app, sizeof(app), "%s/../..", executableDirectory) >= sizeof(app) ||
        !realpath(app, executable)) {
        return 126;
    }
    char temporary[PATH_MAX];
    if (snprintf(temporary, sizeof(temporary), "%s/tmp", data) >= sizeof(temporary)) return 126;
    set_value("TMPDIR", temporary);
    char browserData[PATH_MAX + 32];
    snprintf(browserData, sizeof(browserData), "--user-data-dir=%s", data);
    char proxyFlag[sizeof(proxyLine) + 32];
    snprintf(proxyFlag, sizeof(proxyFlag), "--proxy-server=%s", proxy);
    if (!lock_profile(data)) return 75;
    char *arguments[] = { runtime, browserData, proxyFlag,
                         "--proxy-bypass-list=<-loopback>", "--disable-quic",
                         "--disable-features=UseDnsHttpsSvcb,AsyncDns,NetworkTimeServiceQuerying",
                         NULL };
#else

    char settings[sizeof(proxyLine) * 7 + 256];
    snprintf(settings, sizeof(settings), "{\"env\":{\"HTTP_PROXY\":\"%s\",\"HTTPS_PROXY\":\"%s\","
             "\"ALL_PROXY\":\"%s\",\"http_proxy\":\"%s\",\"https_proxy\":\"%s\","
             "\"all_proxy\":\"%s\",\"NO_PROXY\":\"\",\"no_proxy\":\"\"}}",
             proxy, proxy, proxy, proxy, proxy, proxy);
    char **arguments = calloc((size_t)(argc - first) + 4, sizeof(*arguments));
    if (!arguments) return 126;
    arguments[0] = runtime;
    arguments[1] = "--settings";
    arguments[2] = settings;
    for (int i = first; i < argc; i++) arguments[i - first + 3] = argv[i];
#endif
    execv(runtime, arguments);
    perror("Could not launch Claude");
    return 126;
}
