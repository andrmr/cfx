#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "cfx/tmx/map.h"
#include "cfx/util/polygon.h"

struct SdlTextureClip
{
    SDL_Texture* texture = nullptr;
    SDL_Rect text_rect;
    SDL_Rect clip_rect;
};

int main(int argc, char* args[])
{
#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_OFF
    spdlog::set_default_logger(spdlog::stdout_logger_st("console"));
    spdlog::set_level(static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));
    spdlog::set_pattern("[%H:%M:%S][%L] %v");
#endif

    cfx::tmx::Map map;
    map.load(fmt::format("{}/{}", RES_DIR, "level01/level01.tmx"));

    const auto map_px_width  = map.width() * map.tilewidth();
    const auto map_px_height = map.height() * map.tileheight();

    const auto scale         = 1;
    const auto screen_width  = scale * map_px_width;
    const auto screen_height = scale * map_px_height;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SPDLOG_ERROR("[SDL2] Failed to initialize: {}", SDL_GetError());
        return 1;
    }

    auto window = std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>>(
        SDL_CreateWindow("demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_RESIZABLE),
        SDL_DestroyWindow);

    auto renderer = std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer*)>>(
        SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED),
        SDL_DestroyRenderer);

    SDL_RenderSetLogicalSize(renderer.get(), screen_width, screen_height);
    SDL_RenderSetIntegerScale(renderer.get(), SDL_TRUE);

    std::unordered_map<std::string_view, SDL_Texture*> textures;
    std::vector<SdlTextureClip> tex_clips;

    for (const auto& lr: map.layers())
    {
        for (const auto& t: lr.tiles)
        {
            SDL_Texture* img = nullptr;
            if (auto it = textures.find(t.tex->src); it == textures.end())
            {
                img = IMG_LoadTexture(renderer.get(), t.tex->src.data());
                textures.insert({t.tex->src, img});
            }
            else
            {
                img = it->second;
            }

            if (img)
            {
                SDL_Rect texr;
                texr.w = t.tex->w;
                texr.h = t.tex->h;
                texr.x = t.col * texr.w;
                texr.y = t.row * texr.h;

                SDL_Rect clipr;
                clipr.x = t.tex->x;
                clipr.y = t.tex->y;
                clipr.w = t.tex->w;
                clipr.h = t.tex->h;

                tex_clips.emplace_back(img, texr, clipr);
            }
        }
    }

    // pre-render tiles to buffer texture
    SDL_Texture* buffer = SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, map_px_width, map_px_height);
    SDL_SetRenderTarget(renderer.get(), buffer);
    for (const auto& c: tex_clips)
    {
        SDL_RenderCopy(renderer.get(), c.texture, &c.clip_rect, &c.text_rect);
    }
    SDL_SetRenderTarget(renderer.get(), NULL);

    bool keep_running = true;
    while (keep_running)
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev) > 0)
        {
            switch (ev.type)
            {
                case SDL_QUIT:
                    keep_running = false;
            }
        }

        SDL_RenderClear(renderer.get());
        SDL_RenderCopy(renderer.get(), buffer, NULL, NULL);
        SDL_RenderPresent(renderer.get());
    }

    SDL_Quit();

    return 0;
}