ObjC.import('Foundation');

function run(args) {
    const file = args[0];
    const source = $.NSString.stringWithContentsOfFileEncodingError(file, $.NSUTF8StringEncoding, null);
    const config = JSON.parse(ObjC.unwrap(source));
    if (Array.isArray(config.chromiumFlags)) {
        const flags = config.chromiumFlags.filter(flag => (
            typeof flag !== 'string' || !/^(--)?(proxy-server|proxy-pac-url|proxy-bypass-list|no-proxy-server|proxy-auto-detect)(=|$)/.test(flag)
        ));
        if (flags.length !== config.chromiumFlags.length) {
            const serialized = $(JSON.stringify({...config, chromiumFlags: flags}, null, 2) + '\n');
            if (!serialized.writeToFileAtomicallyEncodingError(file, true, $.NSUTF8StringEncoding, null)) {
                throw Error('Could not update the Claude Desktop proxy flags');
            }
        }
    }
}
