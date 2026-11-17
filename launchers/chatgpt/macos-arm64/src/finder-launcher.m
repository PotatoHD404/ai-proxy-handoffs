#import <AppKit/AppKit.h>
#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

static void show_failure(NSString *message) {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
    [NSApp activateIgnoringOtherApps:YES];
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = @"ChatGPT Proxy could not start";
    alert.informativeText = message;
    [alert runModal];
}

int main(void) {
    @autoreleasepool {
        NSString *root = [[NSBundle mainBundle].bundlePath stringByDeletingLastPathComponent];
        NSString *launcher = [root stringByAppendingPathComponent:@"Open ChatGPT.command"];
        if (![[NSFileManager defaultManager] isExecutableFileAtPath:launcher]) {
            show_failure(@"Extract the full ZIP and keep ChatGPT.app beside Open ChatGPT.command and the config folder.");
            return 1;
        }
        NSString *logs = [NSHomeDirectory() stringByAppendingPathComponent:@"Library/Logs"];
        [[NSFileManager defaultManager] createDirectoryAtPath:logs withIntermediateDirectories:YES attributes:nil error:nil];
        NSString *log = [logs stringByAppendingPathComponent:@"ChatGPT Proxy Launch.log"];
        posix_spawn_file_actions_t actions;
        int error = posix_spawn_file_actions_init(&actions);
        if (error) {
            show_failure([NSString stringWithUTF8String:strerror(error)]);
            return 1;
        }
        error = posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, log.fileSystemRepresentation,
                                                 O_CREAT | O_WRONLY | O_APPEND | O_NOFOLLOW, 0600);
        if (!error) error = posix_spawn_file_actions_adddup2(&actions, STDOUT_FILENO, STDERR_FILENO);
        pid_t child = -1;
        char *arguments[] = {"/bin/bash", (char *)launcher.fileSystemRepresentation, NULL};
        if (!error) error = posix_spawn(&child, "/bin/bash", &actions, NULL, arguments, environ);
        posix_spawn_file_actions_destroy(&actions);
        if (error) {
            show_failure([NSString stringWithUTF8String:strerror(error)]);
            return 1;
        }
        for (int attempt = 0; attempt < 15; attempt++) {
            int status;
            if (waitpid(child, &status, WNOHANG) == child) {
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) return 0;
                if (WIFEXITED(status) && WEXITSTATUS(status) == 75) {
                    show_failure(@"Another proxy copy is using the same ChatGPT profile. Quit that copy first.");
                } else {
                    show_failure([@"See the launch log for details: " stringByAppendingString:log]);
                }
                return 1;
            }
            usleep(100000);
        }
    }
    return 0;
}
