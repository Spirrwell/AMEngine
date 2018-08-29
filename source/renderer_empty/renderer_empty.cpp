#include "renderer_empty.hpp"
#include "interface.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

static DLLInterface< IRenderer, RendererEmpty > s_Renderer( RENDERER_INTERFACE );