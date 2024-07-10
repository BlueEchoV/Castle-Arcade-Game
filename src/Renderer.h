#pragma once
#include <stdint.h>
#include <Windows.h>
#include <SDL.h>

struct SDL_Renderer;
SDL_Renderer* MP_CreateRenderer(SDL_Window* window);
int MP_GetRendererOutputSize(SDL_Renderer* sdl_renderer, int* w, int* h);
void MP_DestroyRenderer(SDL_Renderer * renderer);

int MP_SetRenderDrawColor(SDL_Renderer* sdl_renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
int MP_GetRenderDrawColor(SDL_Renderer* sdl_renderer, Uint8* r, Uint8* g, Uint8* b, Uint8* a);
int MP_RenderClear(SDL_Renderer* sdl_renderer);
int MP_RenderFillRect(SDL_Renderer* sdl_renderer, const SDL_Rect* rect);
int MP_RenderFillRects(SDL_Renderer* sdl_renderer, const SDL_Rect* rects, int count);
int MP_RenderDrawLine(SDL_Renderer* sdl_renderer, int x1, int y1, int x2, int y2);
int MP_RenderDrawLines(SDL_Renderer* sdl_renderer, const SDL_Point* points, int count);
int MP_RenderDrawPoint(SDL_Renderer* sdl_renderer, int x, int y);
int MP_RenderDrawPoints(SDL_Renderer* sdl_renderer, const SDL_Point* points, int count);
int MP_RenderDrawRect(SDL_Renderer* sdl_renderer, const SDL_Rect* rect);
int MP_RenderDrawRects(SDL_Renderer* sdl_renderer, const SDL_Rect* rects, int count);

int MP_SetTextureBlendMode(SDL_Texture* texture, SDL_BlendMode blend_mode);
int MP_GetTextureBlendMode(SDL_Texture* texture, SDL_BlendMode* blendMode);
int MP_SetRenderDrawBlendMode(SDL_Renderer* sdl_renderer, SDL_BlendMode blendMode);
int MP_GetRenderDrawBlendMode(SDL_Renderer* sdl_renderer, SDL_BlendMode *blendMode);

int MP_SetTextureColorMod(SDL_Texture* texture, Uint8 r, Uint8 g, Uint8 b);
int MP_GetTextureColorMod(SDL_Texture* texture, Uint8* r, Uint8* g, Uint8* b);
int MP_SetTextureAlphaMod(SDL_Texture* texture, Uint8 alpha);
int MP_GetTextureAlphaMod(SDL_Texture* texture, Uint8* alpha);

void MP_UnlockTexture(SDL_Texture* texture);
int MP_LockTexture(SDL_Texture* texture, const SDL_Rect* rect, void **pixels, int *pitch);
SDL_Texture* MP_CreateTexture(SDL_Renderer* sdl_renderer, uint32_t format, int access, int w, int h);
int MP_UpdateTexture(SDL_Texture* texture, const SDL_Rect* rect, const void *pixels, int pitch);
void MP_DestroyTexture(SDL_Texture* texture);
int MP_RenderCopy(SDL_Renderer* sdl_renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect);
int MP_RenderCopyEx(SDL_Renderer* sdl_renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect, const float angle, const SDL_Point* center, const SDL_RendererFlip flip);
void MP_RenderPresent(SDL_Renderer* sdl_renderer);

int MP_RenderSetClipRect(SDL_Renderer* sdl_renderer, const SDL_Rect* rect);
void MP_RenderGetClipRect(SDL_Renderer* sdl_renderer, SDL_Rect* rect);
SDL_bool MP_RenderIsClipEnabled(SDL_Renderer* sdl_renderer);

int MP_RenderSetViewport(SDL_Renderer* sdl_renderer, const SDL_Rect* rect);
void MP_RenderGetViewport(SDL_Renderer* sdl_renderer, SDL_Rect* rect);

void draw_debug_images(SDL_Renderer* sdl_renderer);

#if 0
SDL_RenderGetWindow
SDL_GetTextureUserData
SDL_QueryTexture
SDL_RenderCopyEx
SDL_RenderSetVSync
SDL_SetTextureScaleMode
SDL_SetTextureUserData
#endif
