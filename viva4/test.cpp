#include "viva.h"

namespace examples
{
    struct gameData
    {
        vi::graphics::texture tex[10];
        vi::graphics::dynamic dyn[50];
        vi::graphics::sprite sprites[50];
        vi::graphics::animation ani[50];
        vi::viva v;
    };

    void keyboard()
    {

    }

    void timerMotionAnimation()
    {
        auto loop = [](gameData* _gameData)
        {
            // must update timer itself
            vi::time::updateTimer(&_gameData->v.timer);

            // have to update every dynamic object to take effect
            vi::graphics::updateDynamic(_gameData->sprites, _gameData->dyn, &_gameData->v.timer);

            // manually update some properties scaled by timer
            _gameData->sprites[1].sx = sinf(vi::time::getGameTimeSec(&_gameData->v.timer) * 10) / 2 + 1;
            _gameData->sprites[2].sy = sinf(vi::time::getGameTimeSec(&_gameData->v.timer) * 7) / 2 + 5;
            _gameData->sprites[3].r = sinf(vi::time::getGameTimeSec(&_gameData->v.timer)) / 4 + 0.75f;
            _gameData->sprites[3].g = sinf(vi::time::getGameTimeSec(&_gameData->v.timer) + vi::math::FORTH_PI) / 4 + +0.75f;
            _gameData->sprites[3].b = sinf(vi::time::getGameTimeSec(&_gameData->v.timer) + vi::math::FORTH_PI * 2) / 4 + +0.75f;

            // must update animations
            vi::graphics::updateAnimation(&_gameData->v.timer, _gameData->ani);

            vi::graphics::drawScene(&_gameData->v.graphics, _gameData->sprites, 100, &_gameData->v.camera);
        };

        gameData data = {};
        vi::vivaInfo info = {};
        info.width = 960;
        info.height = 540;
        info.title = "Timer, motion and animation";
        vi::initViva(&data.v, &info);
        data.v.camera.scale = 0.1f;

        vi::graphics::createTextureFromFile(&data.v.graphics, "0x72_DungeonTilesetII_v1.png", data.tex);
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

#define MAKE_SPRITE(__n,__x,__y,__rot,__sx,__sy,__r,__g,__b) data.sprites[__n] = {}; \
        vi::graphics::transform::setUvFromPixels(data.sprites + __n, 293.f, 18.f, 6.f, 13.f, 512.f, 512.f); \
        data.sprites[__n].r = __r; \
        data.sprites[__n].g = __g; \
        data.sprites[__n].b = __b; \
        data.sprites[__n].x = __x; \
        data.sprites[__n].y = __y; \
        data.sprites[__n].rot = __rot; \
        data.sprites[__n].textureIndex = data.tex[0].index; \
        vi::graphics::transform::setPixelScale(&data.v.graphics, &data.v.camera, 6 * __sx, 13 * __sy, data.sprites + __n);

        MAKE_SPRITE(0, -14, -0, 0, 10, 10, 1, 1, 1)
        MAKE_SPRITE(1, -10, -0, 0, 10, 10, 1, 1, 1)
        MAKE_SPRITE(2, -6, -0, 0, 10, 10, 1, 1, 1)
        MAKE_SPRITE(3, -2, -0, 0, 10, 10, 1, 1, 1)

#undef MAKE_SPRITE

        // make one object spin
        data.dyn[0] = {};
        data.dyn[0].velrot = 1.f;

        // init sprite for animation
        vi::graphics::initSprite(data.sprites + 4, data.tex[0].index);
        data.sprites[4].x = 4;
        data.sprites[4].y = 2;
        vi::graphics::transform::setPixelScale(&data.v.graphics, &data.v.camera, 16 * 4, 28 * 4, data.sprites + 4);
        // init uv for animation using convenience function
        vi::graphics::uv uvForAni[9];
        vi::graphics::uvSplitInfo usi = {};
        usi.frameCount = 4;
        usi.pixelFrameHeight = 28;
        usi.pixelFrameWidth = 16;
        usi.pixelOffsetx = 192;
        usi.pixelOffsety = 4;
        usi.pixelTexHeight = 512;
        usi.pixelTexWidth = 512;
        usi.rowLength = 4;
        vi::graphics::uvSplit(&usi, uvForAni);
        // init animation
        vi::graphics::initAnimation(data.ani, data.sprites + 4, uvForAni, 4, 0.1f, 0);
        vi::graphics::playAnimation(data.ani, &data.v.timer);

        vi::system::loop<gameData*>(loop, &data);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        vi::graphics::destroyGraphics(&data.v.graphics);
        vi::system::destroyWindow(&data.v.window);
    }

    // more drawing options
    void moreSprites()
    {
        auto loop = [](gameData* _gameData)
        {
            vi::graphics::drawScene(&_gameData->v.graphics, _gameData->sprites, 100, &_gameData->v.camera);
        };

        gameData data = {};
        vi::vivaInfo info = {};
        info.width = 960;
        info.height = 540;
        info.title = "More sprites";
        vi::initViva(&data.v, &info);
        data.v.camera.scale = 0.1f;

        vi::graphics::createTextureFromFile(&data.v.graphics, "0x72_DungeonTilesetII_v1.png", data.tex);

        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

#define MAKE_SPRITE(__n,__x,__y,__rot,__sx,__sy,__r,__g,__b) data.sprites[__n] = {}; \
        vi::graphics::transform::setUvFromPixels(data.sprites + __n, 293.f, 18.f, 6.f, 13.f, 512.f, 512.f); \
        data.sprites[__n].r = __r; \
        data.sprites[__n].g = __g; \
        data.sprites[__n].b = __b; \
        data.sprites[__n].x = __x; \
        data.sprites[__n].y = __y; \
        data.sprites[__n].rot = __rot; \
        data.sprites[__n].textureIndex = data.tex[0].index; \
        vi::graphics::transform::setPixelScale(&data.v.graphics, &data.v.camera, 6 * __sx, 13 * __sy, data.sprites + __n);

        // different scales
        MAKE_SPRITE(0, -14, -5, 0, 10, 10, 1, 1, 1)
        MAKE_SPRITE(1, -11, -5, 0, 8, 8, 1, 1, 1)
        MAKE_SPRITE(2, -9, -5, 0, 6, 6, 1, 1, 1)
        MAKE_SPRITE(3, -7, -5, 0, 4, 4, 1, 1, 1)
        MAKE_SPRITE(4, -5, -5, 0, 2, 2, 1, 1, 1)
        MAKE_SPRITE(5, -3, -5, 0, 1, 1, 1, 1, 1)

        // different colors
        MAKE_SPRITE(6, -14, 5, 0, 10, 10, 1, 0, 0)
        MAKE_SPRITE(7, -11, 5, 0, 10, 10, 0, 1, 0)
        MAKE_SPRITE(8, -8, 5, 0, 10, 10, 0, 0, 1)
        MAKE_SPRITE(9, -5, 5, 0, 10, 10, 0, 1, 1)
        MAKE_SPRITE(10, -2, 5, 0, 10, 10, 1, 0, 1)
        MAKE_SPRITE(11, 1, 5, 0, 10, 10, 1, 1, 0)

        // different rotations
        MAKE_SPRITE(12, 0, -5, 0, 10, 10, 1, 1, 1)
        MAKE_SPRITE(13, 3, -5, vi::math::THIRD_PI, 10, 10, 1, 1, 1)
        MAKE_SPRITE(14, 6, -5, vi::math::THIRD_PI * 2, 10, 10, 1, 1, 1)
        MAKE_SPRITE(15, 9, -5, vi::math::THIRD_PI * 3, 10, 10, 1, 1, 1)
        MAKE_SPRITE(16, 12, -5, vi::math::THIRD_PI * 4, 10, 10, 1, 1, 1)
        MAKE_SPRITE(17, 15, -5, vi::math::THIRD_PI * 5, 10, 10, 1, 1, 1)

        // uv swap to flip vertically and horizontally
        MAKE_SPRITE(18, 7, 5, 0, 10, 10, 1, 1, 1)
        MAKE_SPRITE(19, 11, 5, 0, 10, 10, 1, 1, 1)
        vi::util::swap(data.sprites[19].left, data.sprites[19].right);
        MAKE_SPRITE(20, 7, 0, 0, 10, 10, 1, 1, 1)
        vi::util::swap(data.sprites[20].top, data.sprites[20].bottom);
        MAKE_SPRITE(21, 11, 0, 0, 10, 10, 1, 1, 1)
        vi::util::swap(data.sprites[21].left, data.sprites[21].right);
        vi::util::swap(data.sprites[21].top, data.sprites[21].bottom);

#undef MAKE_SPRITE

        vi::system::loop<gameData*>(loop, &data);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        vi::graphics::destroyGraphics(&data.v.graphics);
        vi::system::destroyWindow(&data.v.window);
    }

    // most basic example if you want to see something on the screen
    void basicSprite()
    {
        auto loop = [](gameData* _gameData)
        {
            // draw one sprite
            // make a single draw call per frame
            vi::graphics::drawScene(&_gameData->v.graphics, _gameData->sprites, 1, &_gameData->v.camera);
        };

        gameData data = {};
        // init viva
        // it's a wrapper function that initializes some viva objects
        vi::vivaInfo info = {};
        // viewport size
        info.width = 960;
        info.height = 540;
        // window title
        info.title = "Basic sprites";
        vi::initViva(&data.v, &info);

        // create some texture
        // this should be done once per level/game because resource creation is expensive
        vi::graphics::createTextureFromFile(&data.v.graphics, "0x72_DungeonTilesetII_v1.png", data.tex);

        // once you are done creating texures they have to be pushed
        // it is relatively expensive step so that's why it has to be done explicitly by programmer
        // don't do it to often
        // for example do it once per level/game
        // texture that are not pushed cannot be used in draw step
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

        // initialize one object for drawing
        // CRUCIAL FIELDS
        // "left,right,top,bottom" is for UV
        //         if uninitialized then they gonna be 0,0,0,0 so segment of nothing will be used
        // "textureIndex" when textures are pushed, each texture gets and index, use that index
        // "sx,sy" is scale, use something other than 0,0 because it means that sprite is infinitely small
        // "r,g,b" are color coefficients, texel is multiplied by them so use 1,1,1 if you want to use texel as it is
        // use initDrawInfo to initialize crucial fields
        vi::graphics::initSprite(data.sprites, data.tex[0].index);
        vi::graphics::transform::setUvFromPixels(data.sprites, 293.f, 18.f, 6.f, 13.f, 512.f, 512.f);
        // in this case, object on the texture is 6x13 pixels
        // i want to set it to 60x130 to make it biger but still proportional
        vi::graphics::transform::setPixelScale(&data.v.graphics, &data.v.camera, 6 * 10, 13 * 10, data.sprites);

        // ready to start looping
        // any object can be passed in if you want to avoid lambda capture
        vi::system::loop<gameData*>(loop, &data);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        vi::graphics::destroyGraphics(&data.v.graphics);
        vi::system::destroyWindow(&data.v.window);
    }

    int main()
    {
        //basicSprite();
        //moreSprites();
        timerMotionAnimation();
        return 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////

struct gameData
{
    vi::graphics::animation a[10];
    vi::graphics::sprite t[100];
    vi::viva v;
};

const char* str = "Jedziemy poziomo\nnikc mobilem\no juz na miejscu bedziemy za chwile";

void loop(gameData* data)
{
    vi::graphics::drawScene(&data->v.graphics, data->t, 6, &data->v.camera);
}

int sandbox()
{
    vi::graphics::texture tex[10];
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
    vi::graphics::createTextureFromFile(&data.v.graphics, "font1.png", tex + 4);
    vi::graphics::createTextureFromFile(&data.v.graphics, "0x72_DungeonTilesetII_v1.png", tex + 5);

    vi::graphics::pushTextures(&data.v.graphics, tex, 6);
      
        
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

    for (int i = 1; i < 100; i++)
    {
        data.t[i] = {};
        data.t[i].sx = 3;
        data.t[i].sy = 3;
        data.t[i].textureIndex = 0;
        data.t[i].left = 0;
        data.t[i].top = 0;
        data.t[i].right = 1;
        data.t[i].bottom = 1;
        data.t[i].r = 1;
        data.t[i].g = 1;
        data.t[i].b = 1;
        data.t[i].x = rand() % 10 - 5;
    }

    vi::system::loop<gameData*>(loop, &data);

    vi::graphics::destroyTexture(&data.v.graphics, tex);
    vi::graphics::destroyTexture(&data.v.graphics, tex + 1);
    vi::graphics::destroyTexture(&data.v.graphics, tex + 2);
    vi::graphics::destroyTexture(&data.v.graphics, tex + 3);
    vi::graphics::destroyTexture(&data.v.graphics, tex + 4);
    vi::graphics::destroyGraphics(&data.v.graphics);

	return 0;
}

int main()
{
    //return sandbox();
    return examples::main();
}