# Claude Desktop and Claude Code for Ubuntu amd64

Includes Anthropic's verified amd64 Desktop 5.11492.5 and CLI 2.1.325. Ubuntu uses the vendor package and checksums. Extract the archive, run `./setup.sh`, then run `./Open\ Claude.sh` for the desktop app or `bin/claude` for Claude Code. Quit a running Desktop instance before using the launcher.

The launcher configures the upstream directly. No local relay or added network namespace is included. Claude keeps its normal home directory, MCP configuration, and built-in approval and sandbox settings. The shell prefix removes the handoff's proxy variables from supported shell commands and stdio MCP servers. HTTP MCP clients running inside Claude may share its proxy. Existing Claude Code settings or managed policy can override proxy variables. Review any old localhost proxy settings before launch.

The optional `config/desktop-managed.json` supplies the same proxy for `/etc/claude-desktop/managed-settings.json`. `config/managed-settings.json` contains the corresponding Claude Code proxy settings. Merge them through your normal administrator workflow if needed. Starting the Desktop app from its stock menu entry requires the managed Desktop policy for consistent routing.

The configured clients use application-level proxy routing, which does not provide a per-process OS firewall. Local tools can use the host's network and filesystem under the standard Claude permission settings. The wrapped amd64 configuration and vendor checksums were checked on macOS. Runtime testing of this revision on Ubuntu is pending.

## Choose the proxy

Edit `config/proxy.txt` to set the default. Use one URL with an explicit port, such as `http://172.20.10.1:1082`. Both launchers read that file. Set `HANDOFF_PROXY` to override it for one invocation, or pass `--proxy` as the first argument:

```sh
./Open\ Claude.sh --proxy http://proxy.example:3128
./bin/claude --proxy http://proxy.example:3128
```

HTTP and HTTPS proxy URLs are supported. The launchers accept host and port without embedded credentials. Quit the previous copy before changing a running Desktop app's proxy. An installed managed policy can take precedence. The shipped policies and sample configuration use the original address. Update the administrator-managed policy too if you use a different upstream.
