# Unified ChatGPT and Codex CLI for Apple Silicon

This ZIP contains unified ChatGPT 26.1111.21308 from the test Mac, including the Chat, Work and Codex views, and standalone Codex CLI 0.169.0. The bundle and CLI are ad hoc signed. The launchers target macOS 14.0.

Extract the full directory and open **ChatGPT.app** in Finder. **Open ChatGPT.command** is the Terminal entry point. For the CLI, use `bin/codex`. Both use the proxy selected by the launcher. The app uses Chromium's fixed HTTP proxy setting and the CLI uses standard proxy environment variables. `bin/setup` prepares the bundled shell proxy settings for Codex commands. Client state is stored in `~/Library/Application Support/OpenAI Direct Proxy`.

The optional `config/ChatGPT Unified Browser Proxy.mobileconfig` profile sets the same upstream in the `com.openai.codex` managed Chromium browser preferences. Installing the profile in System Settings → General → Device Management affects copies of ChatGPT that read that domain. Remove an older localhost profile before using this profile.


## Local tools and MCP

This revision uses the normal macOS filesystem and restores Chromium and Codex's built-in sandboxes. The bundled Codex configuration uses workspace-write, enables network for workspace commands, and removes the launcher proxy variables from their environment. Change the permission mode in Codex if a project requires wider access.

To reuse existing MCP definitions, quit the proxy app and run **Import Codex Config.command**, then relaunch. This copies your local `~/.codex/config.toml` into the two proxy profiles and backs up their previous configs. Review the imported permission and shell environment settings if they differ from the bundled defaults. Stdio MCP servers use the host network. An HTTP MCP client running within Codex may inherit Codex's proxy.

OAuth uses the local callback on 1455 or 1457. The external sign-in browser uses its own proxy settings.

## Routing scope

Proxy routing is configured in the client. This revision removes the outer OS sandbox so child tools can access the host network and filesystem. It does not enforce proxy-only app egress at the OS level. The managed profiles are optional. Remove any older localhost proxy profile before applying an updated one. Signed-in runtime and Tahoe runtime were not exercised for this revision.

## Choose the proxy

Edit `config/proxy.txt` to set the default. Use one URL with an explicit port, such as `http://172.20.10.1:1082`. Both launchers read that file. Set `HANDOFF_PROXY` to override it for one invocation, or pass `--proxy` as the first argument:

```sh
./Open\ ChatGPT.command --proxy http://proxy.example:3128
./bin/codex --proxy http://proxy.example:3128
```

HTTP and HTTPS proxy URLs are supported. The launchers accept host and port without embedded credentials. Quit the previous copy before changing a running Desktop app's proxy. An installed managed policy can take precedence. The shipped policies and sample configuration use the original address. Update the administrator-managed policy too if you use a different upstream.
