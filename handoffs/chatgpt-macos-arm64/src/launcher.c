#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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

#ifdef DESKTOP
static void clear_codex_environment(void) {
    extern char **environ;
    for (size_t index = 0; environ[index] != NULL;) {
        const char *assignment = environ[index];
        const char *equals = strchr(assignment, '=');
        if (strncmp(assignment, "CODEX_", 6) != 0 || !equals) {
            index++;
            continue;
        }
        char *name = strndup(assignment, equals - assignment);
        if (!name) exit(126);
        clear_value(name);
        free(name);
    }
}
#endif

int main(int argc, char **argv) {
    char executable[PATH_MAX];
    char resolved[PATH_MAX];
    uint32_t length = sizeof(executable);
    if (_NSGetExecutablePath(executable, &length) || !realpath(executable, resolved)) {
        fputs("Could not resolve OpenAI client launcher path.\n", stderr);
        return 126;
    }

    const struct passwd *account = getpwuid(getuid());
    if (!account) {
        fputs("Could not resolve the current macOS account.\n", stderr);
        return 126;
    }
    char data[PATH_MAX];
#ifdef DESKTOP
    const char *runtimeName = "ChatGPT-runtime";
    const char *dataName = "Unified Desktop";
#else
    const char *runtimeName = "../lib/codex/bin/codex";
    const char *dataName = "Code";
#endif
    if (snprintf(data, sizeof(data), "%s/Library/Application Support/OpenAI Direct Proxy/%s",
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
    set_value("NO_PROXY", "127.0.0.1:1455,127.0.0.1:1457");
    set_value("no_proxy", "127.0.0.1:1455,127.0.0.1:1457");
    clear_value("ELECTRON_RUN_AS_NODE");
#ifdef DESKTOP
    clear_codex_environment();
    char state[PATH_MAX];
    if (snprintf(state, sizeof(state), "%s/Library/Application Support/OpenAI Direct Proxy", account->pw_dir) >= sizeof(state)) {
        return 126;
    }
    if ((mkdir(state, 0700) != 0 && errno != EEXIST) ||
        (mkdir(data, 0700) != 0 && errno != EEXIST)) {
        perror("Could not prepare the private ChatGPT data directory");
        return 126;
    }
    char codexHome[PATH_MAX];
    if (snprintf(codexHome, sizeof(codexHome), "%s/codex", data) >= sizeof(codexHome)) return 126;
    if (mkdir(codexHome, 0700) != 0 && errno != EEXIST) {
        perror("Could not prepare the private Codex data directory");
        return 126;
    }
    set_value("CODEX_HOME", codexHome);
    set_value("CODEX_ELECTRON_USER_DATA_PATH", data);
    set_value("HOME", account->pw_dir);
    clear_value("CFFIXED_USER_HOME");
    char tempBase[PATH_MAX];
    char temporary[PATH_MAX];
    char resolvedTemp[PATH_MAX];
    size_t tempLength = confstr(_CS_DARWIN_USER_TEMP_DIR, tempBase, sizeof(tempBase));
    if (tempLength == 0 || tempLength > sizeof(tempBase) ||
        snprintf(temporary, sizeof(temporary), "%soaiu", tempBase) >= sizeof(temporary)) {
        return 126;
    }
    struct stat temporaryStatus;
    if ((mkdir(temporary, 0700) != 0 && errno != EEXIST) ||
        lstat(temporary, &temporaryStatus) != 0 ||
        !S_ISDIR(temporaryStatus.st_mode) ||
        temporaryStatus.st_uid != getuid() ||
        (temporaryStatus.st_mode & 0077) != 0 ||
        !realpath(temporary, resolvedTemp)) {
        fputs("Could not prepare a private temporary directory.\n", stderr);
        return 126;
    }
    set_value("TMPDIR", resolvedTemp);
    set_value("MAC_CHROMIUM_TMPDIR", resolvedTemp);
    if (chdir(account->pw_dir) != 0) return 126;
    set_value("PWD", account->pw_dir);
    char browserData[PATH_MAX + 32];
    char browserCache[PATH_MAX + 32];
    snprintf(browserCache, sizeof(browserCache), "--disk-cache-dir=%s/cache", data);
    snprintf(browserData, sizeof(browserData), "--user-data-dir=%s", data);
    char proxyFlag[sizeof(proxyLine) + 32];
    snprintf(proxyFlag, sizeof(proxyFlag), "--proxy-server=%s", proxy);
    if (!lock_profile(data)) return 75;
    char *arguments[] = { runtime, browserData, browserCache, proxyFlag,
                         "--proxy-bypass-list=<-loopback>;127.0.0.1:1455;127.0.0.1:1457", "--disable-quic",
                         "--disable-features=UseDnsHttpsSvcb,AsyncDns,NetworkTimeServiceQuerying",
                         "--disable-component-update",
                         NULL };
#else
    set_value("CODEX_HOME", data);
    char **arguments = argv + first - 1;
    arguments[0] = runtime;
#endif
    execv(runtime, arguments);
    perror("Could not launch the OpenAI client");
    return 126;
}
