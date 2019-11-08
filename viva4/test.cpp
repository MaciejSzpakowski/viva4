#include "viva.h"

vi::graphics::drawInfo t[PRIMITIVE_MAX_COUNT];

int main()
{
	vi::vivaInfo info;
	info.width = 1920;
	info.height = 1080;
	info.title = "Viva4!";
    vi::viva v;
    vi::initViva(&v, &info);
    v.camera.scale = 0.1f;

    vi::graphics::texture tex[3];
    vi::graphics::createTexture(&v.graphics, "bk.png", tex);
    vi::graphics::createTexture(&v.graphics, "sm.png", tex + 1);
    vi::graphics::createTexture(&v.graphics, "elf.png", tex + 2);

    vi::graphics::pushTextures(&v.graphics, tex, 3);
        
    t[0] = {};
    t[0].sx = 3;
    t[0].sy = 3;
    t[0].textureIndex = 0;
    t[0].left = 0;
    t[0].top = 0;
    t[0].right = 1;
    t[0].bottom = 1;
    t[0].r = 1;
    t[0].g = 1;
    t[0].b = 0;

    t[1] = {};
    t[1].sx = 3;
    t[1].sy = 3;
    t[1].textureIndex = 1;
    t[1].left = 0;
    t[1].top = 0;
    t[1].right = 1;
    t[1].bottom = 1;
    t[1].r = 1;
    t[1].g = 1;
    t[1].b = 1;

    t[2] = {};
    t[2].sx = 3;
    t[2].sy = 3;
    t[2].textureIndex = 2;
    t[2].left = 1;
    t[2].top = 1;
    t[2].right = 0;
    t[2].bottom = 0;
    t[2].r = 1;
    t[2].g = 1;
    t[2].b = 1;

    t[3] = {};
    t[3].sx = 3;
    t[3].sy = 3;
    t[3].x = -10;
    t[3].textureIndex = -1;
    t[3].r = 1.0f;
    t[3].g = 0.2f;
    t[3].b = 0;
    
    vi::system::loop([&]()
    {
        vi::input::updateKeyboardState(&v.keyboard);
        vi::time::updateTimer(&v.timer);

        float gameTime = vi::time::getGameTimeSec(&v.timer);
        t[0].x = sinf(gameTime) * 3;
        t[0].y = cosf(gameTime) * 3;
        //t[0].rot = gameTime;

        t[1].x = sinf(gameTime + vi::math::PI * 2 / 3) * 3;
        t[1].y = cosf(gameTime + vi::math::PI * 2 / 3) * 3;
        //t[1].rot = gameTime + vi::math::TWO_PI / 3;

        t[2].x = sinf(gameTime + vi::math::PI * 4 / 3) * 3;
        t[2].y = cosf(gameTime + vi::math::PI * 4 / 3) * 3;
        //t[2].rot = gameTime + vi::math::TWO_PI / 3 * 2;

        /*if (vi::input::isKeyPressed(&k, 'A'))
            t[0].x--;

        if (vi::input::isKeyPressed(&k, 'D'))
            t[0].x++;

        if (vi::input::isKeyPressed(&k, 'W'))
            t[0].y--;

        if (vi::input::isKeyPressed(&k, 'S'))
            t[0].y++;

        if (vi::input::isKeyPressed(&k, 'Q'))
            t[0].rot -= 3.141592f / 6;

        if (vi::input::isKeyPressed(&k, 'E'))
            t[0].rot += 3.141592f / 6;

        if (vi::input::isKeyPressed(&k, VK_PRIOR))
        {
            t[0].sx -= 0.5f;
            t[0].sy -= 0.5f;
        }

        if (vi::input::isKeyPressed(&k, VK_NEXT))
        {
            t[0].sx += 0.5f;
            t[0].sy += 0.5f;
        }*/

        vi::graphics::beginScene(&v.graphics);
        vi::graphics::draw(&v.graphics, t, 4, &v.camera);
        vi::graphics::endScene(&v.graphics);
    });

    vi::graphics::destroyTexture(&v.graphics, tex);
    vi::graphics::destroyTexture(&v.graphics, tex + 1);
    vi::graphics::destroyTexture(&v.graphics, tex + 2);
    vi::graphics::graphicsDestroy(&v.graphics);

	return 0;
}