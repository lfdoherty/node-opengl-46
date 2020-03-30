
process.env.MESA_GL_VERSION_OVERRIDE=4.6

module.exports = {
    webgl: require('./lib/webgl'),
    Image: require('./lib/image'),
    document: require('./lib/platform_glfw')
    //document: require('./lib/platform_sdl')
    //document: require('./lib/platform_sfml')
};