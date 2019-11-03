#include "viva.h"

vi::graphics::drawInfo t[10000];

int main()
{
    vi::input::keyboard k;
    vi::input::initKeyboard(&k);
	vi::graphics::engineInfo info;
	vi::graphics::engine g;
    vi::graphics::camera cam;
    vi::memory::heap h;
    vi::memory::heapInit(&h);
	info.width = 800;
	info.height = 600;
	info.title = "Viva4!";
	vi::graphics::graphicsInit(&g, &info);
    vi::graphics::initCamera(&g, &cam);
    cam.scale = 0.1f;
    vi::graphics::texture tex[2];
    vi::graphics::createTexture(&g, "a.png", tex);
    vi::graphics::texture t2;
    vi::graphics::createTexture(&g, "b.png", tex + 1);

    vi::graphics::pushTextures(&g, tex, 2);

    vkQueueWaitIdle(g.queue);

    t[0] = {};
    t[0].x = 0;
    t[0].y = 0;
    t[0].z = 0.01f;
    t[0].rot = 0;
    t[0].sx = 5;
    t[0].sy = 5;
    t[0].textureIndex = tex[0].index;

    t[1] = {};
    t[1].x = 0.5f;
    t[1].y = 0.5f;
    t[1].z = 0.02f;
    t[1].sx = 5;
    t[1].sy = 5;
    t[1].textureIndex = tex[1].index;
    
    vi::system::loop([&]()
    {
        vi::input::updateKeyboardState(&k);

        if (vi::input::isKeyPressed(&k, 'A'))
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
        }

        vi::graphics::beginScene(&g);
        vi::graphics::draw(&g, t, 2, &cam);
        vi::graphics::endScene(&g);
    });

    vi::graphics::destroyTexture(&g, tex);
    vi::graphics::destroyTexture(&g, tex + 1);
    vi::graphics::graphicsDestroy(&g);

	return 0;
}