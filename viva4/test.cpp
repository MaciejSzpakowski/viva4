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

    void mouse()
    {
    }

    void text()
    {
    }

    void camera()
    {
        auto loop = [](gameData* _gameData)
        {
            vi::time::updateTimer(&_gameData->v.timer);
            vi::input::updateKeyboardState(&_gameData->v.keyboard);
            float frameTime = vi::time::getTickTimeSec(&_gameData->v.timer);

            if (vi::input::isKeyDown(&_gameData->v.keyboard, 'A'))
            {
                _gameData->v.camera.x -= frameTime;
            }
            else if (vi::input::isKeyDown(&_gameData->v.keyboard, 'D'))
            {
                _gameData->v.camera.x += frameTime;
            }

            if (vi::input::isKeyDown(&_gameData->v.keyboard, 'W'))
            {
                _gameData->v.camera.y -= frameTime;
            }
            else if (vi::input::isKeyDown(&_gameData->v.keyboard, 'S'))
            {
                _gameData->v.camera.y += frameTime;
            }

            if (vi::input::isKeyDown(&_gameData->v.keyboard, 'Q'))
            {
                _gameData->v.camera.scale *= 1 - frameTime * .3f;
            }
            else if (vi::input::isKeyDown(&_gameData->v.keyboard, 'E'))
            {
                _gameData->v.camera.scale *= 1 + frameTime * .3f;
            }

            vi::graphics::drawScene(&_gameData->v.graphics, _gameData->sprites, 10, &_gameData->v.camera);
        };

        gameData data = {};
        vi::vivaInfo info = {};
        info.width = 960;
        info.height = 540;
        info.title = "Camera";
        vi::initViva(&data.v, &info);

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/0x72_DungeonTilesetII_v1.png", data.tex);
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

        vi::graphics::initSprite(data.sprites, data.tex[0].index);
        data.sprites[0].x = -1;
        data.sprites[0].y = -1;
        vi::graphics::transform::setUvFromPixels(data.sprites, 293.f, 18.f, 6.f, 13.f, 512.f, 512.f);
        vi::graphics::transform::setPixelScale(&data.v.graphics, &data.v.camera, 6 * 10, 13 * 10, data.sprites);

        vi::graphics::initSprite(data.sprites + 1, data.tex[0].index);
        data.sprites[1].x = 1;
        data.sprites[1].y = -1;
        vi::graphics::transform::setUvFromPixels(data.sprites + 1, 293.f, 18.f, 6.f, 13.f, 512.f, 512.f);
        vi::graphics::transform::setPixelScale(&data.v.graphics, &data.v.camera, 6 * 10, 13 * 10, data.sprites + 1);

        vi::graphics::initSprite(data.sprites + 2, data.tex[0].index);
        data.sprites[2].x = -1;
        data.sprites[2].y = 1;
        vi::graphics::transform::setUvFromPixels(data.sprites + 2, 293.f, 18.f, 6.f, 13.f, 512.f, 512.f);
        vi::graphics::transform::setPixelScale(&data.v.graphics, &data.v.camera, 6 * 10, 13 * 10, data.sprites + 2);

        vi::graphics::initSprite(data.sprites + 3, data.tex[0].index);
        data.sprites[3].x = 1;
        data.sprites[3].y = 1;
        vi::graphics::transform::setUvFromPixels(data.sprites + 3, 293.f, 18.f, 6.f, 13.f, 512.f, 512.f);
        vi::graphics::transform::setPixelScale(&data.v.graphics, &data.v.camera, 6 * 10, 13 * 10, data.sprites + 3);

        vi::graphics::initSprite(data.sprites + 4, data.tex[0].index);
        vi::graphics::transform::setUvFromPixels(data.sprites + 4, 293.f, 18.f, 6.f, 13.f, 512.f, 512.f);
        vi::graphics::transform::setPixelScale(&data.v.graphics, &data.v.camera, 6 * 10, 13 * 10, data.sprites + 4);

        vi::system::loop<gameData*>(loop, &data);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        vi::graphics::destroyGraphics(&data.v.graphics);
        vi::system::destroyWindow(&data.v.window);
    }

    // move camera with WSAD zoom Q/E
    // just to make sure they still work
    void multipleTextures()
    {
        auto loop = [](gameData* _gameData)
        {
            vi::graphics::drawScene(&_gameData->v.graphics, _gameData->sprites, 3, &_gameData->v.camera);
        };

        gameData data = {};
        vi::vivaInfo info = {};
        info.width = 960;
        info.height = 540;
        info.title = "Multiple textures";
        vi::initViva(&data.v, &info);
        data.v.camera.scale = 0.5f;

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/bk.png", data.tex);
        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/elf.png", data.tex + 1);
        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/sm.png", data.tex + 2);

        vi::graphics::pushTextures(&data.v.graphics, data.tex, 3);

        vi::graphics::initSprite(data.sprites, data.tex[0].index);
        data.sprites[0].x = -1;
        data.sprites[0].y = 1;
        vi::graphics::initSprite(data.sprites + 1, data.tex[1].index);
        data.sprites[1].x = -1;
        data.sprites[1].y = -1;
        vi::graphics::initSprite(data.sprites + 2, data.tex[2].index);
        data.sprites[2].x = 1;
        data.sprites[2].y = -1;

        vi::system::loop<gameData*>(loop, &data);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        vi::graphics::destroyTexture(&data.v.graphics, data.tex + 1);
        vi::graphics::destroyTexture(&data.v.graphics, data.tex + 2);
        vi::graphics::destroyGraphics(&data.v.graphics);
        vi::system::destroyWindow(&data.v.window);
    }

    // move with WSAD
    // there a lot of noise to make it more interesting
    // state management is done in crapy way just to have quick and dirty example
    // relevant parts are: updateKeyboardState, isKeyDown, 
    // animationFlipHorizontally, switchAnimation, moveTo
    void keyboardMultipleAnimationsMath()
    {
        // this variable is kept to keep direction for elf
        // so when it changes, animation is flipped
        float elfDirection = 1;
        float monsterDirection = 1;

        auto loop = [&elfDirection,&monsterDirection](gameData* _gameData)
        {
            // update timer first
            vi::time::updateTimer(&_gameData->v.timer);
            // this is not the best way to move objects around but that's not the point of this example
            float frameTime = vi::time::getTickTimeSec(&_gameData->v.timer);
            // updating keyboard state once a frame is a good idea
            vi::input::updateKeyboardState(&_gameData->v.keyboard);
            // animation management is non trivial
            // this is simple state variable to indicate two states: walk and idle
            // it's used to switch animation between walk and idle
            bool elfIsMoving = false;

            // check for key presses
            // if any is down then elf will move and is moving state will be true
            if (vi::input::isKeyDown(&_gameData->v.keyboard, 'A'))
            {
                // direction changed, flip elf animations
                if (elfDirection > 0)
                {
                    elfDirection = -1;
                    vi::graphics::animationFlipHorizontally(_gameData->ani);
                    vi::graphics::animationFlipHorizontally(_gameData->ani + 1);
                }

                elfIsMoving = true;
                _gameData->sprites[0].x -= frameTime;
            }
            else if (vi::input::isKeyDown(&_gameData->v.keyboard, 'D'))
            {
                // direction changed, flip elf animations
                if (elfDirection < 0)
                {
                    elfDirection = 1;
                    vi::graphics::animationFlipHorizontally(_gameData->ani);
                    vi::graphics::animationFlipHorizontally(_gameData->ani + 1);
                }

                elfIsMoving = true;
                _gameData->sprites[0].x += frameTime;
            }

            if (vi::input::isKeyDown(&_gameData->v.keyboard, 'W'))
            {
                elfIsMoving = true;
                _gameData->sprites[0].y -= frameTime;
            }
            else if (vi::input::isKeyDown(&_gameData->v.keyboard, 'S'))
            {
                elfIsMoving = true;
                _gameData->sprites[0].y += frameTime;
            }

            float distance = vi::graphics::transform::distance2Dsq(
                _gameData->sprites[0].x, _gameData->sprites[0].y,
                _gameData->sprites[1].x, _gameData->sprites[1].y);

            // if monster is far enough then start moving towards elf
            if (distance > 0.31f * 0.31f)
            {
                vi::graphics::transform::moveTo(_gameData->sprites[1].x, _gameData->sprites[1].y,
                    _gameData->sprites[0].x, _gameData->sprites[0].y, 0.9f, 
                    &_gameData->dyn[0].velx, &_gameData->dyn[0].vely);
                // switch from idle to walk
                vi::graphics::switchAnimation(_gameData->ani + 3, _gameData->ani + 2, &_gameData->v.timer);

                if (monsterDirection > 0 && _gameData->dyn[0].velx < 0)
                {
                    monsterDirection = -1;
                    vi::graphics::animationFlipHorizontally(_gameData->ani + 2);
                    vi::graphics::animationFlipHorizontally(_gameData->ani + 3);
                }
                else if(monsterDirection < 0 && _gameData->dyn[0].velx > 0)
                {
                    monsterDirection = 1;
                    vi::graphics::animationFlipHorizontally(_gameData->ani + 2);
                    vi::graphics::animationFlipHorizontally(_gameData->ani + 3);
                }
            }

            // if monster is close enough the stop moving
            // this number is smaller than to start moving
            // because weird things might happen at the border
            if(distance < 0.29f * 0.29f)
            {
                // stop moving
                _gameData->dyn[0].velx = 0;
                _gameData->dyn[0].vely = 0;
                // switch from walk to idle
                vi::graphics::switchAnimation(_gameData->ani + 2, _gameData->ani + 3, &_gameData->v.timer);
            }

            vi::graphics::updateDynamicSprite(_gameData->sprites + 1, _gameData->dyn, &_gameData->v.timer);

            // 'switchAnimation' is the most convenient way (as of writing this) to switch animation
            // 'switchAnimation' does nothing if correct animation is already playing
            // i.e. you dont have to check if state actually changed
            if (elfIsMoving)
                vi::graphics::switchAnimation(_gameData->ani + 1, _gameData->ani, &_gameData->v.timer);
            else
                vi::graphics::switchAnimation(_gameData->ani, _gameData->ani + 1, &_gameData->v.timer);

            // dont forget to update all animation, wheter they playing or not, doesnt matter
            vi::graphics::updateAnimation(&_gameData->v.timer, _gameData->ani);
            vi::graphics::updateAnimation(&_gameData->v.timer, _gameData->ani + 1);
            vi::graphics::updateAnimation(&_gameData->v.timer, _gameData->ani + 2);
            vi::graphics::updateAnimation(&_gameData->v.timer, _gameData->ani + 3);

            vi::graphics::drawScene(&_gameData->v.graphics, _gameData->sprites, 100, &_gameData->v.camera);
        };

        gameData data = {};
        vi::vivaInfo info = {};
        info.width = 960;
        info.height = 540;
        info.title = "Keyboard";
        vi::initViva(&data.v, &info);

        // init textures
        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/0x72_DungeonTilesetII_v1.png", data.tex);
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

        // init sprites
        vi::graphics::initSprite(data.sprites, data.tex[0].index);
        vi::graphics::transform::setPixelScale(&data.v.graphics, &data.v.camera, 16 * 4, 28 * 4, data.sprites);
        vi::graphics::initSprite(data.sprites + 1, data.tex[0].index);
        data.sprites[1].x = 1;
        vi::graphics::transform::setPixelScale(&data.v.graphics, &data.v.camera, 16 * 4, 20 * 4, data.sprites + 1);

        // init animations
        vi::graphics::uv elfWalkAni[4];
        vi::graphics::uvSplitInfo usi1 = { 512,512,192,4,16,28,4,4 };
        vi::graphics::uvSplit(&usi1, elfWalkAni);
        vi::graphics::initAnimation(data.ani, data.sprites, elfWalkAni, 4, 0.09f, 0);

        vi::graphics::uv elfIdleAni[4];
        vi::graphics::uvSplitInfo usi2 = { 512,512,128,4,16,28,4,4 };
        vi::graphics::uvSplit(&usi2, elfIdleAni);
        vi::graphics::initAnimation(data.ani + 1, data.sprites, elfIdleAni, 4, 0.1f, 0);

        vi::graphics::uv monsterWalkAni[4];
        vi::graphics::uvSplitInfo usi3 = { 512,512,432,204,16,20,4,4 };
        vi::graphics::uvSplit(&usi3, monsterWalkAni);
        vi::graphics::initAnimation(data.ani + 2, data.sprites + 1, monsterWalkAni, 4, 0.09f, 0);

        vi::graphics::uv monsterIdleAni[4];
        vi::graphics::uvSplitInfo usi4 = { 512,512,368,204,16,20,4,4 };
        vi::graphics::uvSplit(&usi4, monsterIdleAni);
        vi::graphics::initAnimation(data.ani + 3, data.sprites + 1, monsterIdleAni, 4, 0.1f, 0);

        // dynamic for monster
        data.dyn[0] = {};

        // start playing idle animation
        vi::graphics::playAnimation(data.ani + 1, &data.v.timer);
        vi::graphics::playAnimation(data.ani + 3, &data.v.timer);

        vi::system::loop<gameData*>(loop, &data);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        vi::graphics::destroyGraphics(&data.v.graphics);
        vi::system::destroyWindow(&data.v.window);
    }

    void timerMotionAnimation()
    {
        auto loop = [](gameData* _gameData)
        {
            // must update timer itself
            vi::time::updateTimer(&_gameData->v.timer);

            // have to update every dynamic object to take effect
            vi::graphics::updateDynamicSprite(_gameData->sprites, _gameData->dyn, &_gameData->v.timer);

            // manually update some properties scaled by timer
            _gameData->sprites[1].sx = sinf(vi::time::getGameTimeSec(&_gameData->v.timer) * 10) / 2 + 1;
            _gameData->sprites[2].sy = sinf(vi::time::getGameTimeSec(&_gameData->v.timer) * 7) / 2 + 5;
            _gameData->sprites[3].r = sinf(vi::time::getGameTimeSec(&_gameData->v.timer)) / 4 + 0.75f;
            _gameData->sprites[3].g = sinf(vi::time::getGameTimeSec(&_gameData->v.timer) + vi::math::FORTH_PI) / 4 + 0.75f;
            _gameData->sprites[3].b = sinf(vi::time::getGameTimeSec(&_gameData->v.timer) + vi::math::FORTH_PI * 2) / 4 + 0.75f;

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

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/0x72_DungeonTilesetII_v1.png", data.tex);
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

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/0x72_DungeonTilesetII_v1.png", data.tex);

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
        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/0x72_DungeonTilesetII_v1.png", data.tex);

        // once you are done creating texures they have to be pushed
        // it is relatively expensive step so that's why it has to be done explicitly by programmer
        // don't do it too often
        // for example do it once per level/game
        // texture that are not pushed cannot be used in draw step
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

        // initialize one sprite for drawing
        // CRUCIAL FIELDS
        // "left,right,top,bottom" is for UV
        //         if uninitialized then they gonna be 0,0,0,0 so segment of nothing will be used
        //         init function will initialize to 0,0,1,1 which means use the whole texture
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
        basicSprite();
        moreSprites();
        timerMotionAnimation();
        keyboardMultipleAnimationsMath();
        multipleTextures();
        camera();
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