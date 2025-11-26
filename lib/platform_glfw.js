var GLFW = require("@lfdoherty/node-glfw");
var WebGL = require("./webgl");

module.exports = function () {
	//if (process.platform !== 'win32')
	//  process.on('SIGINT', function () { process.exit(0); });
	//process.on('exit', function () { console.log('exiting app'); });
	//glewExperimental = GL_TRUE;

	var platform;
	var window;

	var events;
	Object.defineProperty(GLFW, "events", {
		get: function () {
			if (events) return events;
			events = new (require("events").EventEmitter)();

			var _emit = events.emit;
			events.emit = function () {
				var args = Array.prototype.slice.call(arguments);
				var evt = args[1]; // args[1] is the event, args[0] is the type of event
				//console.log("emitting event: "+require('util').inspect(args));
				if (args[0] != "quit") {
					evt.preventDefault = function () {};
					evt.stopPropagation = function () {};
				}
				//_emit.apply(this,args);
				events.listeners(args[0]).forEach(function (listener) {
					listener(args[1]);
				});
			};
			return events;
		},
		// enumerable: true,
		// configurable: true
	});

	GLFW.Init();
	GLFW.events.on("event", console.dir);
	GLFW.events.on("quit", function () {
		process.exit(0);
	});
	GLFW.events.on("keydown", function (evt) {
		if (evt.keyCode === 27) process.exit(0); // ESC TODO REMOVE?
	});

	platform = {
		type: "nodeGLFW",
		ratio: 1, // ratio between window and framebuffer, normally 1 except on screens like Mac Retina
		setTitle: function (title) {
			GLFW.SetWindowTitle(window, title);
		},
		setIcon: function () {},
		flip: function () {
			GLFW.SwapBuffers(window);
		},
		getElementById: function (name) {
			return this.createElement(name);
		},
		createElement: function (name, width, height) {
			if (name.toLowerCase().indexOf("canvas") >= 0) {
				this.createWindow(width || 800, height || 800);
				this.canvas = this;
				WebGL.canvas = this;
				return this;
			}
			return null;
		},
		createWindow: function (width, height, config = {}) {
			var attribs = GLFW.WINDOW;

			if (width == 0 || height == 0) {
				attribs = GLFW.FULLSCREEN;
				width = height = 0;
			}

			var resizeListeners = [],
				rl = GLFW.events.listeners("framebuffer_resize");
			for (var l = 0, ln = rl.length; l < ln; ++l) resizeListeners[l] = rl[l];
			GLFW.events.removeAllListeners("framebuffer_resize");

			GLFW.DefaultWindowHints();

			// we use OpenGL 2.1, GLSL 1.20. Comment this for now as this is for GLSL 1.50
			//GLFW.OpenWindowHint(GLFW.OPENGL_FORWARD_COMPAT, 1);
			//GLFW.WindowHint(GLFW.OPENGL_VERSION_MAJOR, 4);
			//GLFW.WindowHint(GLFW.OPENGL_VERSION_MINOR, 6);
			//GLFW.OpenWindowHint(GLFW.OPENGL_PROFILE, GLFW.OPENGL_CORE_PROFILE);

			GLFW.WindowHint(GLFW.RESIZABLE, config.resizable === false ? 0 : 1);
			GLFW.WindowHint(GLFW.VISIBLE, config.visible === false ? 0 : 1);
			GLFW.WindowHint(GLFW.DECORATED, config.decorated === false ? 0 : 1);
			GLFW.WindowHint(GLFW.RED_BITS, 8);
			GLFW.WindowHint(GLFW.GREEN_BITS, 8);
			GLFW.WindowHint(GLFW.BLUE_BITS, 8);
			GLFW.WindowHint(GLFW.DEPTH_BITS, 24);
			GLFW.WindowHint(GLFW.REFRESH_RATE, 0);
			//GLFW.WindowHint(GLFW.SAMPLES, 4);

			if (!(window = GLFW.CreateWindow(width, height))) {
				GLFW.Terminate();
				throw "Can't initialize GL surface";
			}

			GLFW.MakeContextCurrent(window);

			GLFW.SetWindowTitle(window, "WebGL");

			// make sure GLEW is initialized
			WebGL.Init();

			GLFW.SwapBuffers(window);
			//GLFW.SwapInterval(0); // Disable VSync (we want to get as high FPS as possible!)

			for (var l = 0, ln = resizeListeners.length; l < ln; ++l)
				GLFW.events.addListener("framebuffer_resize", resizeListeners[l]);

			var sizeWin = GLFW.GetWindowSize(window);
			var sizeFB = GLFW.GetFramebufferSize(window);
			this.ratio = sizeFB.width / sizeWin.width;
			this.width = this.drawingBufferWidth = sizeFB.width;
			this.height = this.drawingBufferHeight = sizeFB.height;

			console.log(
				"glfw " + JSON.stringify(GLFW.GetVersion()) + " window created",
			);
		},
		getContext: function (name) {
			return WebGL;
		},
		on: function (name, callback) {
			this.addEventListener(name, callback);
		},
		pollEventsFast() {
			//workaround for bug described here: https://github.com/glfw/glfw/issues/747
			//console.log('polling events')
			setInterval(function () {
				// console.log('polling fast')
				GLFW.PollEvents();
			}, 5);
		},
		pollEvents() {
			setInterval(function () {
				GLFW.PollEvents();
			}, 16);
		},
		pollEventsOnce() {
			GLFW.PollEvents();
		},
		addEventListener: function (name, callback) {
			if (callback && typeof callback === "function") {
				if (name == "resize") {
					name = "framebuffer_resize";
					var tmpcb = callback;
					var self = callback.this;
					callback = function (evt) {
						platform.width = evt.width;
						platform.height = evt.height;
						tmpcb.call(self, evt);
					};
				} else if (
					name == "mousemove" ||
					name == "mousedown" ||
					name == "mouseup"
				) {
					var tmpcb = callback;
					var self = callback.this;
					callback = function (evt) {
						evt.x *= platform.ratio;
						evt.y *= platform.ratio;
						tmpcb.call(self, evt);
					};
				}
				GLFW.events.on(name, callback);
			}
		},
		removeEventListener: function (name, callback) {
			if (callback && typeof callback === "function") {
				if (name == "resize") name = "framebuffer_resize";
				GLFW.events.removeListener(name, callback);
			}
		},
		requestAnimationFrame: function (callback, delay = 16) {
			//console.log('swapping buffers, polling events')
			// console.log('swapped buffers')
			//console.log('began requestAnimationFrame, polling events')
			//GLFW.PollEvents();
			//platform.pollEventsFast()
			// console.log('scheduling timer')

			var timer = setImmediate; //process.nextTick;
			// var d = 16;
			if (delay > 0) {
				timer = setTimeout;
				//d = delay;
			}
			//console.log('delay: ' + delay)
			timer(function () {
				callback(GLFW.GetTime() * 1000.0);
				GLFW.SwapBuffers(window);
			}, delay);
		},
		destroy() {
			GLFW.DestroyWindow(window);
			GLFW.Terminate();
		},
	};

	Object.defineProperty(platform, "onkeydown", {
		set: function (cb) {
			this.on("keydown", cb);
		},
	});

	Object.defineProperty(platform, "onkeyup", {
		set: function (cb) {
			this.on("keyup", cb);
		},
	});

	return platform;
};
