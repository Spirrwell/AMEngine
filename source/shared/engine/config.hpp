#ifndef CONFIG_H
#define CONFIG_H

struct window_config
{
	int resolution_width;
	int resolution_height;
	bool fullscreen;
};

struct config
{
	config()
	{
		windowConfig.resolution_width = 800;
		windowConfig.resolution_height = 600;
		windowConfig.fullscreen = false;
	}

	window_config windowConfig;
};

#endif // CONFIG_H
