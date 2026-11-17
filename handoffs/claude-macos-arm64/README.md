# Claude Desktop and Claude Code for Apple Silicon

This ZIP contains Claude Desktop 5.11492.5 and Claude Code 2.1.325. Both are ad hoc signed. The launchers target macOS 14.0.

Extract the full directory and run **Open Claude.command**. For the CLI, run `bin/setup` once and use `bin/claude`. Both use the proxy selected by the launcher. The launchers and bundled app settings set the proxy. Desktop state is stored in `~/Library/Application Support/Claude Direct Proxy`.

The optional `config/Claude Direct Proxy.mobileconfig` profile sets the same upstream in the Claude Desktop and Claude Code managed preferences. Installing it in System Settings → General → Device Management can affect other copies using those domains. Remove an older localhost profile before using this profile. `config/managed-settings.json` is the equivalent Claude Code file for administrator deployment.


## Local tools and MCP

The launcher uses the normal macOS filesystem and retains Claude's built-in permission settings. Claude Code reads your existing user and project configuration. Local shell commands and stdio MCP processes use a Claude Code shell prefix that removes the launcher's proxy variables. Remote HTTP MCP clients running within Claude may share the app proxy.

To reuse Desktop MCP definitions from another installation, copy the `mcpServers` section from that copy's `claude_desktop_config.json` into the same file in the handoff's Desktop state directory. The Desktop launch script removes old proxy flags from the handoff profile while retaining its MCP entries.

## Routing scope

Proxy routing is configured in the client. This revision removes the outer OS sandbox so child tools can access the host network and filesystem. It does not enforce proxy-only app egress at the OS level. The managed profiles are optional. Remove any older localhost proxy profile before applying an updated one. Signed-in runtime and Tahoe runtime were not exercised for this revision.

## Choose the proxy

Edit `config/proxy.txt` to set the default. Use one URL with an explicit port, such as `http://172.20.10.1:1082`. Both launchers read that file. Set `HANDOFF_PROXY` to override it for one invocation, or pass `--proxy` as the first argument:

```sh
./Open\ Claude.command --proxy http://proxy.example:3128
./bin/claude --proxy http://proxy.example:3128
```

HTTP and HTTPS proxy URLs are supported. The launchers accept host and port without embedded credentials. Quit the previous copy before changing a running Desktop app's proxy. An installed managed policy can take precedence. The shipped policies and sample configuration use the original address. Update the administrator-managed policy too if you use a different upstream.
