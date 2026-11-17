# Direct proxy launchers for ChatGPT and Claude

Three installable bundles for routing the desktop app and CLI through an HTTP or HTTPS proxy. Local shell commands and stdio MCP servers can use the host network and filesystem under the apps' normal permissions.

## Downloads

Use the ZIP assets on the [release page](https://github.com/PotatoHD404/ai-proxy-handoffs/releases/tag/v1.0.0).

| Platform | Release asset | Start here |
| --- | --- | --- |
| Apple Silicon Mac | [ChatGPT + Codex](https://github.com/PotatoHD404/ai-proxy-handoffs/releases/download/v1.0.0/ChatGPT-Direct-Proxy-macOS-arm64.zip) | Double-click the top-level `ChatGPT.app` or run `Open ChatGPT.command`. |
| Apple Silicon Mac | [Claude Desktop + Claude Code](https://github.com/PotatoHD404/ai-proxy-handoffs/releases/download/v1.0.0/Claude-Direct-Proxy-macOS-arm64.zip) | Run `Open Claude.command`. |
| Ubuntu amd64 | [Claude Desktop + Claude Code](https://github.com/PotatoHD404/ai-proxy-handoffs/releases/download/v1.0.0/Claude-Direct-Proxy-Ubuntu-amd64.zip) | Run `./setup.sh`, then `./Open\ Claude.sh`. |

Each ZIP includes its own README. The `launchers/` directory contains launcher source, scripts, and sample configuration organized by app and platform. The runnable app bundles and vendor binaries are in the ZIP assets.

## Configure your proxy

1. Extract the full ZIP into a fresh folder and keep its contents together.
2. Edit `config/proxy.txt` if you use a different proxy. The included default is already configured as the HTTP proxy `http://172.20.10.1:1082`. Use one URL with an explicit port.
3. Open the app using the entry in the table. Restart it after changing the proxy.

For a one-time CLI override, run this from the extracted directory:

```sh
./bin/codex --proxy http://proxy.example:3128
./bin/claude --proxy http://proxy.example:3128
```

Use the command for your bundle. The desktop command scripts also accept `--proxy` as their first argument. If you installed a previous managed proxy profile, check whether its settings take precedence over the launcher.

## Local tools and MCP

- **Codex:** Run `Import Codex Config.command` to copy your existing `~/.codex/config.toml` into the launcher’s profiles. It backs up the previous launcher config. Review any imported network or sandbox settings.
- **Claude Code:** The launcher reads the standard Claude configuration. Its local tool wrapper removes the launcher proxy variables for supported shell commands and stdio MCP processes.
- **Claude Desktop on Mac:** Copy existing `mcpServers` definitions to the launcher’s Desktop configuration if you want to reuse them. See the [Mac instructions](launchers/claude/macos-arm64/README.md).

Proxy routing is set at the application level. HTTP MCP clients running inside the apps may share the app proxy. These launchers do not install an OS firewall.

## Integrity

Download `SHA256SUMS` from the release alongside the ZIP you use. Run `shasum -a 256 <zip-file>` on macOS or `sha256sum <zip-file>` on Ubuntu and compare the result with its entry.
