#include <ctype.h>
#include <fcntl.h>

static int valid_proxy(const char *proxy) {
    const char *host;
    if (strncmp(proxy, "http://", 7) == 0) host = proxy + 7;
    else if (strncmp(proxy, "https://", 8) == 0) host = proxy + 8;
    else return 0;
    const char *port = strrchr(host, ':');
    if (!port || port == host || !port[1]) return 0;
    if (*host == '[') {
        if (port - host < 4 || port[-1] != ']') return 0;
        for (const unsigned char *p = (const unsigned char *)host + 1; p < (const unsigned char *)port - 1; p++)
            if (!isxdigit(*p) && *p != ':' && *p != '.') return 0;
    } else {
        for (const unsigned char *p = (const unsigned char *)host; p < (const unsigned char *)port; p++)
            if (!isalnum(*p) && *p != '.' && *p != '-') return 0;
    }
    size_t n = strlen(port + 1);
    if (n > 5 || strspn(port + 1, "0123456789") != n) return 0;
    long number = strtol(port + 1, NULL, 10);
    return number >= 1 && number <= 65535;
}

static const char *read_proxy(const char *directory, int argc, char **argv, int *first,
                              char *line, size_t capacity) {
    const char *value = getenv("HANDOFF_PROXY");
    if (argc > 1 && strcmp(argv[1], "--proxy") == 0) {
        if (argc < 3) return NULL;
        value = argv[2];
        *first = 3;
    } else if (argc > 1 && strncmp(argv[1], "--proxy=", 8) == 0) {
        value = argv[1] + 8;
        *first = 2;
    }
    if (value) return valid_proxy(value) ? value : NULL;
#ifdef DESKTOP
    const char *relative = "../../../config/proxy.txt";
#else
    const char *relative = "../config/proxy.txt";
#endif
    char path[PATH_MAX];
    if (snprintf(path, sizeof(path), "%s/%s", directory, relative) >= sizeof(path)) return NULL;
    FILE *file = fopen(path, "r");
    if (!file) return NULL;
    int ok = fgets(line, (int)capacity, file) != NULL && fgetc(file) == EOF && !ferror(file);
    fclose(file);
    if (!ok) return NULL;
    line[strcspn(line, "\r\n")] = '\0';
    return valid_proxy(line) ? line : NULL;
}

#ifdef DESKTOP
static int lock_profile(const char *data) {
    char path[PATH_MAX];
    if (snprintf(path, sizeof(path), "%s/.handoff-launch.lock", data) >= sizeof(path)) return 0;
    int fd = open(path, O_CREAT | O_RDWR | O_NOFOLLOW, 0600);
    struct flock lock = {.l_type = F_WRLCK, .l_whence = SEEK_SET};
    if (fd >= 0 && fcntl(fd, F_SETLK, &lock) == 0) return 1;
    if (fd >= 0) close(fd);
    fprintf(stderr, "Could not open this app profile. Quit the other copy of this handoff and try again.\n");
    return 0;
}
#endif
