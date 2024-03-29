#define VIVA_IMPL
#define VI_VALIDATE
#define VI_VULKAN_H "c:/VulkanSDK/1.1.108.0/Include/vulkan/vulkan.h"
#define VI_VULKAN_LIB "C:/VulkanSDK/1.1.108.0/Lib/vulkan-1.lib"
#include "viva.h"

namespace examples
{
    template<typename T, uint sz> 
    struct list
    {
        T data[sz];
        uint capacity;
        uint len;

        list() 
        { 
            this->capacity = sz; 
            this->len = 0; 
        }

        T* find(std::function<bool(T*)> pred)
        {
            for (uint i = 0; i < this->len; i++)
            {
                if (pred(this->data + i)) return this->data + i;
            }
            return nullptr;
        }

        T* push(T* src)
        {
#ifdef VI_VALIDATE
            if (this->len == this->capacity)
            {
                fprintf(stderr, "list is full");
                return nullptr;
            }
#endif

            memcpy(this->data + this->len, src, sizeof(T));
            this->len++;
            return this->data + len - 1;
        }

        void remove(uint i)
        {
#ifdef VI_VALIDATE
            if (i >= this->len)
            {
                fprintf(stderr, "out of bounds");
                return;
            }
#endif
            if (i < this->len - 1) memcpy(this->data + i, this->data + this->len - 1, sizeof(T));
            this->len--;
        }

        T* at(uint i)
        {
#ifdef VI_VALIDATE
            if (i >= this->len)
            {
                fprintf(stderr, "out of bounds");
                return nullptr;
            }
#endif
            return this->data + i;
        }
    };

#define SPRITE_COUNT 1000

    struct gameData
    {
        vi::graphics::texture tex[10];
        vi::graphics::text t[10];
        vi::graphics::dynamic dyn[50];
        vi::graphics::sprite sprites[SPRITE_COUNT];
        vi::graphics::animation ani[50];
        vi::viva v;

        gameData()
        {
            vi::util::zero<gameData>(this);
        }
    };

    struct user
    {
        char name[20];
        uint id;
        vi::net::endpoint ep;
    };

    void chatServer()
    {
        list<user, 10> users;
        const uint bufferLen = 100;
        byte buffer[bufferLen];
        vi::util::stream<100> stream;
        vi::net::endpoint currentEp;
        vi::net::initNetwork();
        vi::net::server server;
        server.init(10000);
        vi::time::timer timer;
        timer.init();
        uint id = 1;

        while (true)
        {
            Sleep(1);
            timer.update();
            float gameTime = timer.getGameTimeSec();

            // disconnect older than 5 s
            for (int i = users.len - 1; i >= 0; i--)
            {
                user* u = users.at(i);
                if (gameTime - u->ep.lastMessage > 5) users.remove(i);
            }

            memset(buffer, 0, bufferLen);
            vi::util::zero(&currentEp);
            bool msgReceived = server.receive(buffer, bufferLen, &currentEp);

            if (msgReceived)
            {
                user* currentUser = users.find([&currentEp](user* e) { return vi::net::compareEndpoints(&currentEp, &e->ep); });
                if (currentUser == nullptr && users.len < users.capacity && memcmp(buffer, "login", 5) == 0)
                {
                    user newUser;
                    newUser.id = id++;
                    memcpy(&newUser.ep, &currentEp, sizeof(vi::net::endpoint));
                    memcpy(&newUser.name, buffer, strlen((const char*)buffer + 6) + 1);
                    users.push(&newUser);
                    currentUser = users.at(users.len - 1);
                }
                else if (currentUser == nullptr)
                {

                }
                else if (memcmp(buffer, "shout", 5) == 0)
                {
                    stream.clear();
                    stream.push("shout", 6);
                    stream.push(currentUser->id);
                    stream.push(buffer + 6, strlen((const char*)buffer + 6 + 1));

                    for (uint i = 0; i < users.len; i++)
                    {
                        user* ui = users.at(i);
                        if (users.at(i) != currentUser) server.send(stream.data, stream.len, &ui->ep);
                    }
                }
            }
        }
    }

    void appendline(const char* src, char* dst, uint maxlen)
    {
        uint lensrc = strlen(src);
        uint lendst = strlen(dst);
        uint newlines = 0;
        for (uint i = 0; i < lendst; i++)
        {
            if (dst[i] == '\n') newlines++;
        }
        if (lendst + lensrc >= maxlen || newlines > 20)
        {
            const char* newline = strstr(dst, "\n");
            memcpy(dst, newline + 1, newline - dst);
            lendst = strlen(dst);
        }
        memcpy(dst + lendst, src, lensrc + 1);
    }

    void chatClient()
    {
        gameData data;
        char text[1000];
        char buffer[100];
        memset(text, 0, 1000);
        memset(buffer, 0, 100);

        auto loop = [&](uint i)
        {
            vi::graphics::drawScene(&data.v.graphics, data.sprites, 1000, &data.v.camera);
        };

        vi::vivaInfo info;
        info.width = 960;
        info.height = 540;
        info.title = "Chat";
        data.v.init(&info);

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/font1.png", data.tex);
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

        const char* initMsg = "Type ip and port and then press enter to connect to chat \nserver. Ex. 1.1.1.1:10000";
        memcpy(text, initMsg, strlen(initMsg) + 1);
        vi::graphics::font font1(data.tex);
        vi::graphics::uvSplitInfo usi = { 256,36,0,0,8,12,32,96 };
        vi::graphics::uvSplit(&usi, font1.uv);

        data.t[0] = { &font1, data.sprites, initMsg, 0, 0 };
        vi::graphics::setScreenPos(&data.v.graphics, &data.v.camera, 20, 20, 
            &data.sprites[0].s1.x, &data.sprites[0].s1.y);
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 16, 24,
            &data.sprites[0].s1.sx, &data.sprites[0].s1.sy);
        data.t[0].update();
        data.t[1] = { &font1, data.sprites + 600, buffer, 0, 0 };
        vi::graphics::setScreenPos(&data.v.graphics, &data.v.camera, 20, 940,
            &data.t[1].s[0].s1.x, &data.t[1].s[0].s1.y);
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 16, 24,
            &data.t[1].s[0].s1.sx, &data.t[1].s[0].s1.sy);
        for (uint i = 0; i < SPRITE_COUNT; i++) data.sprites[i].s2.col = { 0,0,0 };

        vi::system::loop<uint>(loop, 0);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        data.v.destroy();
    }

    void network()
    {
        vi::net::endpoint serverSide;
        vi::net::client client;

        // init network in windows
        vi::net::initNetwork();
        vi::net::server server;
        // start server at port 10000
        server.init(10000);
        // start client and point it at localhost:10000
        client.init("127.0.0.1", 10000);
        // client sends to server
        client.send((byte*)"Hello", 6);
        byte data[20];
        // server receives
        // since this is not blocking it might fail if send is not fast enough
        // but I guess it is in localhost
        server.receive(data, 20, &serverSide);
        char clientip[20];
        memset(clientip, 0, 20);
        // server determines ip of client
        serverSide.getAddress(clientip, 20);
        printf("server side, %s said: %s\n", clientip, data);
        // server sends to client
        server.send((const byte*)"Hi", 3, &serverSide);
        // client receives
        // since this is not blocking it might fail if send is not fast enough
        // but I guess it is in localhost
        client.receive(data, 20);
        printf("client side, server said: %s\n", data);
        // release resources
        client.destroyClient();
        server.destroyServer();
        // uninit network in windows
        vi::net::uninitNetwork();
    }

    // top chest will blink from start
    // middle chest will start blinking after 5s
    // bottom chest will stop blinking after 5s
    void queue()
    {
        gameData data;

        auto loop = [&](uint i)
        {
            data.v.timer.update();
            data.v.queue.update();
            vi::graphics::drawScene(&data.v.graphics, data.sprites, 10, &data.v.camera);
        };

        
        vi::vivaInfo info;
        info.width = 960;
        info.height = 540;
        info.title = "Queue";
        // set a queue capacity
        info.queueCapacity = 10;
        data.v.init(&info);

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/0x72_DungeonTilesetII_v1.png", data.tex);
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

#define MAKE_SPRITE(n,x,y)  vi::graphics::initSprite(data.sprites + n, data.tex[0].index); \
        vi::graphics::setUvFromPixels(240, 208, 16, 16, 512, 512, &data.sprites[n].s2.uv1); \
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 16 * 5, 16 * 5, \
            &data.sprites[n].s1.sx, &data.sprites[n].s1.sy); \
        data.sprites[n].s2.pos = {x,y,0};

        MAKE_SPRITE(0, 1, -0.4f)
        MAKE_SPRITE(1, 1, 0)
        MAKE_SPRITE(2, 1, 0.4f)

        data.v.queue.setInterval([&]() 
        {
            if (data.sprites[0].s1.x > 0)
                data.sprites[0].s1.x = -10;
            else
                data.sprites[0].s1.x = 1;

            return 1;
        }, 0.15f);

        data.v.queue.initRoutine([&]()
        {
            if (data.sprites[1].s1.x > 0)
                data.sprites[1].s1.x = -10;
            else
                data.sprites[1].s1.x = 1;

            return 1;
        }, 5, 0.12f, 0);

        data.v.queue.initRoutine([&]()
        {
            if (data.sprites[2].s1.x > 0)
                data.sprites[2].s1.x = -10;
            else
                data.sprites[2].s1.x = 1;

            return 1;
        }, 0, 0.1f, 5);

#undef MAKE_SPRITE

        vi::system::loop<uint>(loop, 0);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        data.v.destroy();
    }

    // layer sprites based on z coordinate
    // if objects have the same z coord then they layered based
    // on order in which they are drawn which is not good
    // if you want to put something at the top
    // Here, upper sprites should be all above lower sprites
    // so they are layered independently from order.
    void zindex()
    {
        auto loop = [](gameData* _gameData)
        {
            vi::graphics::drawScene(&_gameData->v.graphics, _gameData->sprites, 10, &_gameData->v.camera);
        };

        gameData data;
        vi::vivaInfo info;
        info.width = 960;
        info.height = 540;
        info.title = "Z Index";
        data.v.init(&info);

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/0x72_DungeonTilesetII_v1.png", data.tex);
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

#define MAKE_SPRITE(n,x,y,z)  vi::graphics::initSprite(data.sprites + n, data.tex[0].index); \
        vi::graphics::setUvFromPixels(240, 208, 16, 16, 512, 512, &data.sprites[n].s2.uv1); \
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 16 * 5, 16 * 5, \
            &data.sprites[n].s1.sx, &data.sprites[n].s1.sy); \
        data.sprites[n].s2.pos = {x,y,z};

        for (int i = 0; i < 10; i++)
        {
            MAKE_SPRITE(i, -0.8f + i * 0.2f, i % 2 ? 0.2f : 0.1f, i % 2 ? 0.2f : 0.1f)
        }

#undef MAKE_SPRITE

        vi::system::loop<gameData*>(loop, &data);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        data.v.destroy();
    }

    // get some cursor data
    // left click and mouse move to move square
    // right click and mouse move to move square using raw input
    // HOME to return square to 0,0
    // move camera with WSAD zoom Q/E
    // fixed sprite wont be affected by camera transform
    // it will behave like camera was at 0,0 and scale 1
    void mouseAndFixedSprite()
    {
        gameData data;

        auto loop = [&](uint i)
        {
            vi::graphics::sprite* sq = data.sprites;
            vi::input::mouse* m = &data.v.mouse;
            data.v.timer.update();
            data.v.mouse.update(&data.v.window, &data.v.camera);
            data.v.keyboard.update();

            int x, y, dx, dy, xraw, yraw;
            float wx, wy;

            data.v.mouse.getCursorClientPos(&x, &y);
            data.v.mouse.getCursorScreenDelta(&dx, &dy);
            data.v.mouse.getCursorDeltaRaw(&xraw, &yraw);
            data.v.mouse.getCursorWorldPos(&wx, &wy);
            short mouseWheel = data.v.mouse.getWheelDelta();
            char str[500];
            // number of sprites to render
            uint count = sprintf(str, "Cursor:\n    Client %d %d\n    Delta %d %d\n    World %f %f\nSquare: %f %f\nMouse Wheel: %d",
                x, y, dx, dy, wx, wy, sq->s1.x, sq->s1.y, mouseWheel);
            // subtract new lines since they dont have glyphs and add back the square
            count = count - 5 + 1;

            data.t[0].str = str;
            data.t[0].update();

            // move square
            if (data.v.keyboard.isKeyDown(vi::input::key::LMOUSE))
            {
                sq->s1.x += (float)dx * 0.003f;
                sq->s1.y += (float)dy * 0.003f;
            }

            if (data.v.keyboard.isKeyDown(vi::input::key::RMOUSE))
            {
                sq->s1.x += (float)xraw * 0.01f;
                sq->s1.y += (float)yraw * 0.01f;
            }

            // reset square
            if (data.v.keyboard.isKeyDown(vi::input::key::HOME))
            {
                sq->s2.pos = { 0,0 };
            }

            float frameTime = data.v.timer.getTickTimeSec();

            if (data.v.keyboard.isKeyDown('A'))
            {
                data.v.camera.x -= frameTime;
            }
            else if (data.v.keyboard.isKeyDown('D'))
            {
                data.v.camera.x += frameTime;
            }

            if (data.v.keyboard.isKeyDown('W'))
            {
                data.v.camera.y -= frameTime;
            }
            else if (data.v.keyboard.isKeyDown('S'))
            {
                data.v.camera.y += frameTime;
            }

            if (data.v.keyboard.isKeyDown('Q'))
            {
                data.v.camera.scale *= 1 - frameTime * .3f;
            }
            else if (data.v.keyboard.isKeyDown('E'))
            {
                data.v.camera.scale *= 1 + frameTime * .3f;
            }

            vi::graphics::drawScene(&data.v.graphics, data.sprites, count, &data.v.camera);
        };

        vi::vivaInfo info;
        info.width = 960;
        info.height = 540;
        info.title = "Mouse";
        data.v.init(&info);

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/font1.png", data.tex);
        byte dot[] = { 255,255,255,255 };
        vi::graphics::createTextureFromBytes(&data.v.graphics, dot, 1, 1, data.tex + 1);
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 2);

        // font is a simple object that combines texture and uv for glyphs together
        vi::graphics::font font1 = { data.tex };
        vi::graphics::uvSplitInfo usi = { 256,36,0,0,8,12,32,96 };
        vi::graphics::uvSplit(&usi, font1.uv);

        // square for moving
        vi::graphics::initSprite(data.sprites, 1);
        data.sprites[0].s2.scale = { 0.3f,0.3f };
        data.sprites[0].s2.col = { 0.1f,0.4f,0.9f };

        // text starts at 2nd sprite
        data.t[0] = { &font1, data.sprites + 1, nullptr, 0, 0 };
        data.sprites[1].s2.pos = { -1, -0.5f };
        data.sprites[1].s2.origin = { -1,-1 };
        vi::graphics::setScreenPos(&data.v.graphics, &data.v.camera, 5, 5, 
            &data.sprites[1].s1.x, &data.sprites[1].s1.y);
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 16, 24,
            &data.sprites[1].s1.sx, &data.sprites[1].s1.sy);

        // set text color black
        for (uint i = 1; i < 1000; i++)
        {
            data.sprites[i].s2.col = { 0,0,0 };
            data.sprites[i].s1.fixed = true;
        }

        vi::system::loop<uint>(loop, 0);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        data.v.destroy();
    }

    /// <summary>
    /// Typing test
    /// </summary>
    void typing()
    {
        gameData data;
        char str[1000];
        memset(str, 0, 1000);
        sprintf(str, "Type something_");
        uint len = strlen(str) - 1;

        auto loop = [&](uint i)
        {
            data.v.keyboard.update();

            if (data.v.keyboard.isKeyPressed(vi::input::BACKSPACE) && len > 0)
            {
                str[--len] = 0;
                strcat(str, "_");
                data.t[0].update();
            }
            else if (data.v.keyboard.isKeyPressed(vi::input::ENTER) && len < 900)
            {
                str[len++] = '\n';
                strcat(str, "_");
                data.t[0].update();
            }
            else if (data.v.keyboard.typedKey != 0 && len < 900 &&
                data.v.keyboard.typedKey != '\t' /* there is no glyph for tab */)
            {
                str[len++] = data.v.keyboard.typedKey;
                strcat(str, "_");
                data.t[0].update();
            }

            vi::graphics::drawScene(&data.v.graphics, data.sprites, 256, &data.v.camera);
        };

        vi::vivaInfo info;
        info.width = 1200;
        info.height = 540;
        info.title = "Typing";
        data.v.init(&info);

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/font1.png", data.tex);
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

        vi::graphics::font font1 = { data.tex };
        vi::graphics::uvSplitInfo usi = { 256,36,0,0,8,12,32,96 };
        vi::graphics::uvSplit(&usi, font1.uv);

        data.t[0] = { &font1, data.sprites, str, 0, 0 };
        data.sprites[0].s2.pos = { -1, -0.75f };
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 16, 24,
            &data.sprites[0].s1.sx, &data.sprites[0].s1.sy);
        vi::graphics::setScreenPos(&data.v.graphics, &data.v.camera, 20, 20,
            &data.sprites[0].s1.x, &data.sprites[0].s1.y);
        data.t[0].update();

        vi::system::loop<uint>(loop, 0);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        data.v.destroy();
    }

    // display virtual key codes of keys that are down
    // some keys cause more than one code to be true (like shift,ctrl and alt)
    void inputState()
    {
        gameData data;
        char str[1000];

        auto loop = [&](uint i)
        {
            data.v.keyboard.update();

            sprintf(str, "Press key(s) to show codes\n");
            for (int i = 0, c = 0; c < 256; c++)
            {
                if (data.v.keyboard.isKeyDown(c))
                    sprintf(str + strlen(str), "%d ", c);
            }

            data.t[0].update();

            vi::graphics::drawScene(&data.v.graphics, data.sprites, 256, &data.v.camera);
        };

        vi::vivaInfo info;
        info.width = 1200;
        info.height = 540;
        info.title = "Input state";
        data.v.init(&info);

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/font1.png", data.tex);
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);
        
        vi::graphics::font font1 = { data.tex };
        vi::graphics::uvSplitInfo usi = { 256,36,0,0,8,12,32,96 };
        vi::graphics::uvSplit(&usi, font1.uv);

        data.t[0] = { &font1, data.sprites, str, 0, 0 };
        data.sprites[0].s2.pos = { -1, -0.75f };
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 16, 24,
            &data.sprites[0].s1.sx, &data.sprites[0].s1.sy);
        vi::graphics::setScreenPos(&data.v.graphics, &data.v.camera, 20, 20, 
            &data.sprites[0].s1.x, &data.sprites[0].s1.y);
        data.t[0].update();

        vi::system::loop<uint>(loop, 0);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        data.v.destroy();
    }

    void text()
    {
        gameData data;
        bool flag = false;
        char str[300];
        const char* cstr = "Some text that\ncontains new line characters.\nEach glyph is just a sprite and\n"
            "can be manipulated individually.\nPress space to toggle";
        const char* extra = "\n more stuff";
        auto loop = [&](uint i)
        {
            vi::graphics::drawScene(&data.v.graphics, data.sprites, 1000, &data.v.camera);
            data.v.keyboard.update();
            if (data.v.keyboard.isKeyPressed(vi::input::SPACE) && !flag)
            {
                flag = true;
                uint len = strlen(str);
                memcpy(str + len, extra, strlen(extra));
                data.t[0].update();

            }
            else if(data.v.keyboard.isKeyPressed(vi::input::SPACE) && flag)
            {
                flag = false;
                str[strlen(cstr)] = 0;
                data.t[0].update();
            }
        };

        vi::vivaInfo info;
        info.width = 960;
        info.height = 540;
        info.title = "Text";
        data.v.init(&info);

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/font1.png", data.tex);
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);
                
        memcpy(str, cstr, strlen(cstr) + 1);
        // font is a simple object that combines texture and uv for glyphs together
        vi::graphics::font font1(data.tex);
        vi::graphics::uvSplitInfo usi = {256,36,0,0,8,12,32,96};
        vi::graphics::uvSplit(&usi, font1.uv);

        // INIT TEXT
        // set position, scale, origin of the first sprite and others will have
        // the same scale and origin and will be advanced starting from that first one's position
        data.t[0] = { &font1, data.sprites, str, 0, 0 };
        data.sprites[0].s2.pos = { -1, -0.5f };
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 16, 24, 
            &data.sprites[0].s1.sx, &data.sprites[0].s1.sy);

        // 'updateText(text*)' is really util function to set sprites to look like text
        data.t[0].update();

        // set all glyphs to be black
        // texture is white so col can be used directly to set text color
        uint len1 = strlen(str);
        for (uint i = 0; i < len1; i++)
            data.sprites[i].s2.col = { 0,0,0 };

        // find where 'individually' starts and color it red
        const char* ptr = strstr(str, "individually");
        // subtract 3 because new line characters dont have corresponding sprites
        uint index = ptr - str - 3;
        uint len2 = strlen("individually");
        for (uint i = index; i < index + len2; i++) data.sprites[i].s2.col = { 1,0,0 };
        
        vi::system::loop<uint>(loop, 0);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        data.v.destroy();
    }

    // move camera with WSAD zoom Q/E
    // zooming should be towards the center of the screen
    void camera()
    {
        gameData data;
        auto loop = [&](uint i)
        {
            data.v.timer.update();
            data.v.keyboard.update();
            float frameTime = data.v.timer.getTickTimeSec();

            if (data.v.keyboard.isKeyDown('A')) data.v.camera.x -= frameTime;
            else if (data.v.keyboard.isKeyDown('D')) data.v.camera.x += frameTime;

            if (data.v.keyboard.isKeyDown('W')) data.v.camera.y -= frameTime;
            else if (data.v.keyboard.isKeyDown('S')) data.v.camera.y += frameTime;

            if (data.v.keyboard.isKeyDown('Q')) data.v.camera.scale *= 1 - frameTime * .3f;
            else if (data.v.keyboard.isKeyDown('E')) data.v.camera.scale *= 1 + frameTime * .3f;

            vi::graphics::drawScene(&data.v.graphics, data.sprites, 10, &data.v.camera);
        };

        vi::vivaInfo info;
        info.width = 960;
        info.height = 540;
        info.title = "Camera";
        data.v.init(&info);

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/0x72_DungeonTilesetII_v1.png", data.tex);
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

        vi::graphics::initSprite(data.sprites, data.tex[0].index);
        data.sprites[0].s2.pos = { -1,-1 };
        vi::graphics::setUvFromPixels(293.f, 18.f, 6.f, 13.f, 512.f, 512.f, &data.sprites->s2.uv1);
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 6 * 10, 13 * 10, 
            &data.sprites[0].s1.sx, &data.sprites[0].s1.sy);

        vi::graphics::initSprite(data.sprites + 1, data.tex[0].index);
        data.sprites[1].s2.pos = { 1,-1 };
        vi::graphics::setUvFromPixels(293.f, 18.f, 6.f, 13.f, 512.f, 512.f, &data.sprites[1].s2.uv1);
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 6 * 10, 13 * 10, 
            &data.sprites[1].s1.sx, &data.sprites[1].s1.sy);

        vi::graphics::initSprite(data.sprites + 2, data.tex[0].index);
        data.sprites[2].s2.pos = { -1,1 };
        vi::graphics::setUvFromPixels(293.f, 18.f, 6.f, 13.f, 512.f, 512.f, &data.sprites[2].s2.uv1);
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 6 * 10, 13 * 10, 
            &data.sprites[2].s1.sx, &data.sprites[2].s1.sy);

        vi::graphics::initSprite(data.sprites + 3, data.tex[0].index);
        data.sprites[3].s2.pos = { 1,1 };
        vi::graphics::setUvFromPixels(293.f, 18.f, 6.f, 13.f, 512.f, 512.f, &data.sprites[3].s2.uv1);
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 6 * 10, 13 * 10, 
            &data.sprites[3].s1.sx, &data.sprites[3].s1.sy);

        vi::graphics::initSprite(data.sprites + 4, data.tex[0].index);
        vi::graphics::setUvFromPixels(293.f, 18.f, 6.f, 13.f, 512.f, 512.f, &data.sprites[4].s2.uv1);
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 6 * 10, 13 * 10, 
            &data.sprites[4].s1.sx, &data.sprites[4].s1.sy);

        vi::system::loop<uint>(loop, 0);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        data.v.destroy();
    }
        
    // just to make sure they still work
    void multipleTextures()
    {
        auto loop = [](gameData* _gameData)
        {
            vi::graphics::drawScene(&_gameData->v.graphics, _gameData->sprites, 3, &_gameData->v.camera);
        };

        gameData data;
        vi::vivaInfo info;
        info.width = 960;
        info.height = 540;
        info.title = "Multiple textures";
        data.v.init(&info);
        data.v.camera.scale = 0.5f;

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/bk.png", data.tex);
        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/elf.png", data.tex + 1);
        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/sm.png", data.tex + 2);

        vi::graphics::pushTextures(&data.v.graphics, data.tex, 3);

        vi::graphics::initSprite(data.sprites, data.tex[0].index);
        data.sprites[0].s2.pos = { -1,1 };
        vi::graphics::initSprite(data.sprites + 1, data.tex[1].index);
        data.sprites[1].s2.pos = { -1,-1 };
        vi::graphics::initSprite(data.sprites + 2, data.tex[2].index);
        data.sprites[2].s2.pos = { 1,-1 };

        vi::system::loop<gameData*>(loop, &data);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        vi::graphics::destroyTexture(&data.v.graphics, data.tex + 1);
        vi::graphics::destroyTexture(&data.v.graphics, data.tex + 2);
        data.v.destroy();
    }

    // move with WSAD, flip sword with SPACE (this is to test keyPressed)
    // there a lot of noise to make it more interesting
    // state management is done in crapy way just to have quick and dirty example
    // relevant parts are: updateKeyboard, isKeyDown, 
    // animationFlipHorizontally, switchAnimation, moveTo
    void keyboardMultipleAnimationsMath()
    {
        // this variable is kept to keep direction for elf
        // so when it changes, animation is flipped
        float elfDirection = 1;
        float monsterDirection = 1;
        gameData data;

        auto loop = [&](uint i)
        {
            // update timer first
            data.v.timer.update();
            // this is not the best way to move objects around but that's not the point of this example
            float frameTime = data.v.timer.getTickTimeSec();
            // updating keyboard state once a frame is a good idea
            data.v.keyboard.update();
            // animation management is non trivial
            // this is simple state variable to indicate two states: walk and idle
            // it's used to switch animation between walk and idle
            bool elfIsMoving = false;

            // check for key presses
            // if any is down then elf will move and is moving state will be true
            if (data.v.keyboard.isKeyDown('A'))
            {
                // direction changed, flip elf animations
                if (elfDirection > 0)
                {
                    elfDirection = -1;
                    vi::graphics::animationFlipHorizontally(data.ani);
                    vi::graphics::animationFlipHorizontally(data.ani + 1);
                }

                elfIsMoving = true;
                data.sprites[0].s1.x -= frameTime;
            }
            else if (data.v.keyboard.isKeyDown('D'))
            {
                // direction changed, flip elf animations
                if (elfDirection < 0)
                {
                    elfDirection = 1;
                    vi::graphics::animationFlipHorizontally(data.ani);
                    vi::graphics::animationFlipHorizontally(data.ani + 1);
                }

                elfIsMoving = true;
                data.sprites[0].s1.x += frameTime;
            }

            if (data.v.keyboard.isKeyDown('W'))
            {
                elfIsMoving = true;
                data.sprites[0].s1.y -= frameTime;
            }
            else if (data.v.keyboard.isKeyDown('S'))
            {
                elfIsMoving = true;
                data.sprites[0].s1.y += frameTime;
            }

            float distance = vi::math::distance2Dsq(
                data.sprites[0].s1.x, data.sprites[0].s1.y,
                data.sprites[1].s1.x, data.sprites[1].s1.y);

            // if monster is far enough then start moving towards elf
            if (distance > 0.31f * 0.31f)
            {
                vi::math::moveTo(data.sprites[1].s1.x, data.sprites[1].s1.y,
                    data.sprites[0].s1.x, data.sprites[0].s1.y, 0.9f,
                    &data.dyn[0].velx, &data.dyn[0].vely);
                // switch from idle to walk
                vi::graphics::switchAnimation(data.ani + 3, data.ani + 2, &data.v.timer);

                if (monsterDirection > 0 && data.dyn[0].velx < 0)
                {
                    monsterDirection = -1;
                    vi::graphics::animationFlipHorizontally(data.ani + 2);
                    vi::graphics::animationFlipHorizontally(data.ani + 3);
                }
                else if(monsterDirection < 0 && data.dyn[0].velx > 0)
                {
                    monsterDirection = 1;
                    vi::graphics::animationFlipHorizontally(data.ani + 2);
                    vi::graphics::animationFlipHorizontally(data.ani + 3);
                }
            }

            // if monster is close enough the stop moving
            // this number is smaller than to start moving
            // because weird things might happen at the border
            if(distance < 0.29f * 0.29f)
            {
                // stop moving
                data.dyn[0].velx = 0;
                data.dyn[0].vely = 0;
                // switch from walk to idle
                vi::graphics::switchAnimation(data.ani + 2, data.ani + 3, &data.v.timer);
            }

            vi::graphics::updateDynamicSprite(data.sprites + 1, data.dyn, &data.v.timer);

            // 'switchAnimation' is the most convenient way (as of writing this) to switch animation
            // 'switchAnimation' does nothing if correct animation is already playing
            // i.e. you dont have to check if state actually changed
            if (elfIsMoving)
                vi::graphics::switchAnimation(data.ani + 1, data.ani, &data.v.timer);
            else
                vi::graphics::switchAnimation(data.ani, data.ani + 1, &data.v.timer);

            // dont forget to update all animation, wheter they playing or not, doesnt matter
            vi::graphics::updateAnimation(&data.v.timer, data.ani);
            vi::graphics::updateAnimation(&data.v.timer, data.ani + 1);
            vi::graphics::updateAnimation(&data.v.timer, data.ani + 2);
            vi::graphics::updateAnimation(&data.v.timer, data.ani + 3);

            // swap cleaver
            if (data.v.keyboard.isKeyPressed(vi::input::key::SPACE))
            {
                vi::util::swap(data.sprites[2].s1.left, data.sprites[2].s1.right);
            }

            vi::graphics::drawScene(&data.v.graphics, data.sprites, 100, &data.v.camera);
        };

        vi::vivaInfo info;
        info.width = 960;
        info.height = 540;
        info.title = "Keyboard";
        data.v.init(&info);

        // init textures
        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/0x72_DungeonTilesetII_v1.png", data.tex);
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

        // init sprites
        vi::graphics::initSprite(data.sprites, data.tex[0].index);
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 16 * 4, 28 * 4, 
            &data.sprites[0].s1.sx, &data.sprites[0].s1.sy);
        vi::graphics::initSprite(data.sprites + 1, data.tex[0].index);
        data.sprites[1].s1.x = 1;
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 16 * 4, 20 * 4, 
            &data.sprites[1].s1.sx, &data.sprites[1].s1.sy);
        vi::graphics::initSprite(data.sprites + 2, data.tex[0].index);
        data.sprites[2].s1.x = -1;
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 8 * 4, 19 * 4,
            &data.sprites[2].s1.sx, &data.sprites[2].s1.sy);
        vi::graphics::setUvFromPixels(310, 124, 8, 19, 512, 512, &data.sprites[2].s2.uv1);

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

        vi::system::loop<uint>(loop, 0);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        data.v.destroy();
    }

    void timerMotionAnimation()
    {
        gameData data;

        auto loop = [&](uint i)
        {
            // must update timer itself
            data.v.timer.update();

            // have to update every dynamic object to take effect
            vi::graphics::updateDynamicSprite(data.sprites, data.dyn, &data.v.timer);

            // manually update some properties scaled by timer
            data.sprites[1].s1.sx = sinf(data.v.timer.getGameTimeSec() * 10) / 2 + 1;
            data.sprites[2].s1.sy = sinf(data.v.timer.getGameTimeSec() * 7) / 2 + 5;
            data.sprites[3].s1.r = sinf(data.v.timer.getGameTimeSec()) / 4 + 0.75f;
            data.sprites[3].s1.g = sinf(data.v.timer.getGameTimeSec() + vi::math::FORTH_PI) / 4 + 0.75f;
            data.sprites[3].s1.b = sinf(data.v.timer.getGameTimeSec() + vi::math::FORTH_PI * 2) / 4 + 0.75f;

            // must update animations
            vi::graphics::updateAnimation(&data.v.timer, data.ani);
            vi::graphics::drawScene(&data.v.graphics, data.sprites, 100, &data.v.camera);
        };

        vi::vivaInfo info;
        info.width = 960;
        info.height = 540;
        info.title = "Timer, motion and animation";
        data.v.init(&info);
        data.v.camera.scale = 0.1f;

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/0x72_DungeonTilesetII_v1.png", data.tex);
        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

#define MAKE_SPRITE(__n,__x,__y,__rot,__sx,__sy,__r,__g,__b) data.sprites[__n] = {}; \
        vi::graphics::setUvFromPixels(293.f, 18.f, 6.f, 13.f, 512.f, 512.f, &data.sprites[__n].s2.uv1); \
        data.sprites[__n].s2.col = {__r,__g,__b}; \
        data.sprites[__n].s2.pos = {__x,__y}; \
        data.sprites[__n].s2.rot = __rot; \
        data.sprites[__n].s2.textureIndex = data.tex[0].index; \
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 6 * __sx, 13 * __sy, \
            &data.sprites[__n].s1.sx, &data.sprites[__n].s1.sy);

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
        data.sprites[4].s2.pos = { 4,2 };
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 16 * 4, 28 * 4, 
            &data.sprites[4].s1.sx, &data.sprites[4].s1.sy);
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

        vi::system::loop<uint>(loop, 0);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        data.v.destroy();
    }

    // more drawing options
    void moreSprites()
    {
        auto loop = [](gameData* _gameData)
        {
            vi::graphics::drawScene(&_gameData->v.graphics, _gameData->sprites, 100, &_gameData->v.camera);
        };

        gameData data;
        vi::vivaInfo info;
        info.width = 960;
        info.height = 540;
        info.title = "More sprites";
        data.v.init(&info);
        data.v.camera.scale = 0.1f;

        vi::graphics::createTextureFromFile(&data.v.graphics, "textures/0x72_DungeonTilesetII_v1.png", data.tex);

        vi::graphics::pushTextures(&data.v.graphics, data.tex, 1);

#define MAKE_SPRITE(__n,__x,__y,__rot,__sx,__sy,__r,__g,__b) data.sprites[__n] = {}; \
        vi::graphics::setUvFromPixels(293.f, 18.f, 6.f, 13.f, 512.f, 512.f, &data.sprites[__n].s2.uv1); \
        data.sprites[__n].s2.col = {__r,__g,__b}; \
        data.sprites[__n].s2.pos = {__x,__y}; \
        data.sprites[__n].s2.rot = __rot; \
        data.sprites[__n].s2.textureIndex = data.tex[0].index; \
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 6 * __sx, 13 * __sy, \
            &data.sprites[__n].s1.sx, &data.sprites[__n].s1.sy);

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
        vi::util::swap(data.sprites[19].s1.left, data.sprites[19].s1.right);
        MAKE_SPRITE(20, 7, 0, 0, 10, 10, 1, 1, 1)
        vi::util::swap(data.sprites[20].s1.top, data.sprites[20].s1.bottom);
        MAKE_SPRITE(21, 11, 0, 0, 10, 10, 1, 1, 1)
        vi::util::swap(data.sprites[21].s1.left, data.sprites[21].s1.right);
        vi::util::swap(data.sprites[21].s1.top, data.sprites[21].s1.bottom);

#undef MAKE_SPRITE

        vi::system::loop<gameData*>(loop, &data);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        data.v.destroy();
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

        // init viva
        // it's a wrapper function that initializes some viva objects
        gameData data;
        vi::vivaInfo info;
        // viewport size
        info.width = 960;
        info.height = 540;
        // window title
        info.title = "Basic sprites";
        data.v.init(&info);

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
        vi::graphics::setUvFromPixels(293.f, 18.f, 6.f, 13.f, 512.f, 512.f, &data.sprites[0].s2.uv1);
        // in this case, object on the texture is 6x13 pixels
        // i want to set it to 60x130 to make it biger but still proportional
        vi::graphics::setPixelScale(&data.v.graphics, &data.v.camera, 6 * 10, 13 * 10, 
            &data.sprites[0].s1.sx, &data.sprites[0].s1.sy);

        // ready to start looping
        // any object can be passed in if you want to avoid lambda capture
        vi::system::loop<gameData*>(loop, &data);

        vi::graphics::destroyTexture(&data.v.graphics, data.tex);
        data.v.destroy();
    }

    int main()
    {
        //chatClient();

        /*basicSprite();
        moreSprites();
        timerMotionAnimation();
        keyboardMultipleAnimationsMath();
        multipleTextures();
        camera();
        text();*/
        //inputState();
        typing();
        /*mouseAndFixedSprite();
        zindex();
        queue();
        network();*/

        return 0;
    }
}

int main()
{
    return examples::main();
}