#include "viva.h"

vi::graphics::transform t[10000];

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
    vi::graphics::texture t1;
    vi::graphics::createTexture(&g, "a.png", &h, &t1);
    vi::graphics::pushTextures(&g, &t1, 1);

    vkQueueWaitIdle(g.queue);

    t[0] = {};
    t[0].x = 0;
    t[0].y = 0;
    t[0].z = 0.01f;
    t[0].rot = 0;
    t[0].sx = 5;
    t[0].sy = 5;

    t[1] = {};
    t[1].x = 0.5f;
    t[1].y = 0.5f;
    t[1].z = 0.02f;
    t[1].sx = 5;
    t[1].sy = 5;

    vi::graphics::destroyTexture(&g, &t1);
    vi::graphics::graphicsDestroy(&g);

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
        vi::graphics::draw(&g, &t1, t, 2, &cam);
        vi::graphics::endScene(&g);
    });

	return 0;
}