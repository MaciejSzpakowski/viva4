#include "viva.h"

struct gameData
{
    vi::graphics::animation a[10];
    vi::graphics::drawInfo t[10];
    vi::viva v;
};

void loop(gameData* data)
{
    vi::graphics::drawInfo* t = data->t;
    vi::input::updateKeyboardState(&data->v.keyboard);
    vi::time::updateTimer(&data->v.timer);

    float gameTime = vi::time::getGameTimeSec(&data->v.timer);
    //t[0].x = sinf(gameTime) * 3;
    //t[0].y = cosf(gameTime) * 3;
    //t[0].rot = gameTime;

    //t[1].x = sinf(gameTime + vi::math::PI * 2 / 3) * 3;
    //t[1].y = cosf(gameTime + vi::math::PI * 2 / 3) * 3;
    //t[1].rot = gameTime + vi::math::TWO_PI / 3;

    //t[2].x = sinf(gameTime + vi::math::PI * 4 / 3) * 3;
    //t[2].y = cosf(gameTime + vi::math::PI * 4 / 3) * 3;
    //t[2].rot = gameTime + vi::math::TWO_PI / 3 * 2;

    if (vi::input::isKeyPressed(&data->v.keyboard, 'A'))
        t[0].x--;

    if (vi::input::isKeyPressed(&data->v.keyboard, 'D'))
        t[0].x++;

    if (vi::input::isKeyPressed(&data->v.keyboard, 'W'))
        t[0].y--;

    if (vi::input::isKeyPressed(&data->v.keyboard, 'S'))
        t[0].y++;

    if (vi::input::isKeyPressed(&data->v.keyboard, 'Q'))
        t[0].rot -= 3.141592f / 6;

    if (vi::input::isKeyPressed(&data->v.keyboard, 'E'))
        t[0].rot += 3.141592f / 6;

    if (vi::input::isKeyPressed(&data->v.keyboard, (int)vi::input::key::PAGEDOWN))
        t[0].sx -= 0.5f;

    if (vi::input::isKeyPressed(&data->v.keyboard, (int)vi::input::key::PAGEUP))
        t[0].sx += 0.5f;

    if (vi::input::isKeyPressed(&data->v.keyboard, (int)vi::input::key::HOME))
        t[0].sy -= 0.5f;

    if (vi::input::isKeyPressed(&data->v.keyboard, (int)vi::input::key::END))
        t[0].sy += 0.5f;

    vi::graphics::updateAnimation(&data->v.timer, data->a);
    vi::graphics::drawScene(&data->v.graphics, data->t + 4, 1, &data->v.camera);
}

int main()
{
    vi::graphics::texture tex[4];
    gameData data = {};
    vi::vivaInfo info = {};
	info.width = 1920;
	info.height = 1080;
	info.title = "Viva4!";
    vi::initViva(&data.v, &info);
    data.v.camera.scale = 0.1f;

    vi::graphics::createTextureFromFile(&data.v.graphics, "bk.png", tex);
    vi::graphics::createTextureFromFile(&data.v.graphics, "sm.png", tex + 1);
    vi::graphics::createTextureFromFile(&data.v.graphics, "elf.png", tex + 2);
    vi::graphics::createTextureFromFile(&data.v.graphics, "ani.bmp", tex + 3);

    vi::graphics::pushTextures(&data.v.graphics, tex, 4);

    vi::graphics::uv ani1uv[10] = { {0,0,0.1f,1},{0.1f,0,0.2f,1},{0.2f,0,0.3f,1},{0.3f,0,0.4f,1},{0.4f,0,0.5f,1},{0.5f,0,0.6f,1},{0.6f,0,0.7f,1},{0.7f,0,0.8f,1},{0.8f,0,0.9f,1},{0.9f,0,1,1} };

    data.a[0] = {};
    data.t[4] = {};
    data.a[0].info = data.t + 4;
    data.a[0].frameCount = 10;
    data.a[0].speed = 0.3f;
    data.a[0].uv = ani1uv;
    data.a[0].info->sx = 3;
    data.a[0].info->sy = 3;
    data.a[0].info->textureIndex = 3;
    data.a[0].info->r = 1;
    data.a[0].info->g = 1;
    data.a[0].info->b = 1;

    vi::graphics::startAnimation(&data.v.timer, data.a, 3);
        
    data.t[0] = {};
    data.t[0].sx = 3;
    data.t[0].sy = 3;
    data.t[0].textureIndex = 0;
    data.t[0].left = 0;
    data.t[0].top = 0;
    data.t[0].right = 1;
    data.t[0].bottom = 1;
    data.t[0].r = 1;
    data.t[0].g = 1;
    data.t[0].b = 1;
    data.t[0].ox = -0.5f;
    data.t[0].oy = -0.5f;

    data.t[1] = {};
    data.t[1].sx = 3;
    data.t[1].sy = 3;
    data.t[1].textureIndex = 1;
    data.t[1].left = 0;
    data.t[1].top = 0;
    data.t[1].right = 1;
    data.t[1].bottom = 1;
    data.t[1].r = 1;
    data.t[1].g = 1;
    data.t[1].b = 1;
    data.t[1].ox = -0.5f;
    data.t[1].oy = -0.5f;

    data.t[2] = {};
    data.t[2].sx = 3;
    data.t[2].sy = 3;
    data.t[2].textureIndex = 2;
    data.t[2].left = 1;
    data.t[2].top = 1;
    data.t[2].right = 0;
    data.t[2].bottom = 0;
    data.t[2].r = 1;
    data.t[2].g = 1;
    data.t[2].b = 1;
    data.t[2].ox = -0.5f;
    data.t[2].oy = -0.5f;

    data.t[3] = {};
    data.t[3].sx = 3;
    data.t[3].sy = 3;
    data.t[3].x = -10;
    data.t[3].textureIndex = -1;
    data.t[3].r = 1.0f;
    data.t[3].g = 0.2f;
    data.t[3].b = 0;

    vi::graphics::transform::setScreenPos(&data.v.graphics, &data.v.camera, 0, 0, data.t);
    vi::graphics::transform::setScalePixels(&data.v.graphics, &data.v.camera, tex[0].width, tex[0].height, data.t);
    vi::graphics::transform::setScreenPos(&data.v.graphics, &data.v.camera, 100, 100, data.t + 1);
    vi::graphics::transform::setScreenPos(&data.v.graphics, &data.v.camera, 1000, 800, data.t + 2);
    
    vi::system::loop<gameData*>(loop, &data);

    vi::graphics::destroyTexture(&data.v.graphics, tex);
    vi::graphics::destroyTexture(&data.v.graphics, tex + 1);
    vi::graphics::destroyTexture(&data.v.graphics, tex + 2);
    vi::graphics::destroyGraphics(&data.v.graphics);

	return 0;
}