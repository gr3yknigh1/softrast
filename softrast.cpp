#include <cassert>

#include <windows.h>


typedef signed char    S8;
typedef signed short   S16;
typedef signed int     S32;

typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned int   U32;

#if defined(_WIN32)
    typedef signed   long long S64;
    typedef unsigned long long U64;
#else
    typedef signed   long S64;
    typedef unsigned long U64;
#endif


typedef char     C8;

#if !defined(__cplusplus)
    typedef S16  C16;

#else
    typedef wchar_t C16;
#endif

typedef S32    C32;

typedef float  F32;
typedef double F64;

typedef U8  Byte;
typedef U64 USZ;
typedef S64 SSZ;

#define MAX_U8  255
#define MAX_U16 65535
#define MAX_U32 4294967295
#define MAX_U64 18446744073709551615


#define global_var static
#define persist_var static


#define KEY_A 0x41
#define KEY_D 0x44
#define KEY_S 0x53
#define KEY_W 0x57


/*
 * Extracts width and height of Win32's `RECT` type.
 */
constexpr void
GetRectSize(_In_ const RECT *r,
            _Out_ S32        *w,
            _Out_ S32        *h) noexcept
{
    *w = r->right  - r->left;
    *h = r->bottom - r->top;
}


#define Win32_TextOutA_CStr(HDC, X, Y, MSG) TextOutA((HDC), (X), (Y), (MSG), CStr_GetLength((MSG)))



// TODO: Scope `T` type to numeric only.
// template <typename T>
struct Vec2i {
    S32 X;
    S32 Y;

    constexpr Vec2i(S32 x = 0, S32 y = 0) noexcept
        : X(x), Y(y)
    { }
};

struct Vec2u {
    U32 X;
    U32 Y;

    constexpr Vec2u(U32 x = 0, U32 y = 0) noexcept
        : X(x), Y(y)
    { }
};

struct Rect {
    U16 X;
    U16 Y;
    U16 Width;
    U16 Height;

    constexpr Rect(U16 x = 0, U16 y = 0, U16 width = 0, U16 height = 0) noexcept
        : X(x), Y(y), Width(width), Height(height)
    { }

    constexpr bool
    IsInside(U16 x, U16 y) const noexcept
    {
        return x >= X && x <= X + Width && y >= Y && y <= Y + Height;
    }


    // TODO(ilya.a): Fix bug when `r` is bigger than `this`.
    constexpr bool
    IsOverlapping(const Rect &r) const noexcept
    {
        return IsInside(r.X + 0      , r.Y + 0)
            || IsInside(r.X + r.Width, r.Y + r.Height)
            || IsInside(r.X + r.Width, r.Y + 0)
            || IsInside(r.X + 0      , r.Y + r.Height);
    }
};


constexpr U64
GetOffset(U64 width, U64 y, U64 x) noexcept
{
    return width * y + x;
}

struct Color4 {
    U8 B;
    U8 G;
    U8 R;
    U8 A;

    constexpr Color4(U8 r = 0, U8 g = 0, U8 b = 0, U8 a = 0) noexcept
        : R(r), G(g), B(b), A(a)
    { }

    constexpr Color4
    operator+(const Color4 &other) const noexcept
    {
        return Color4(R+other.R,
                      G+other.G,
                      B+other.B,
                      A+other.A);
    }
};


static_assert(sizeof(Color4) == sizeof(U32));

global_var constexpr Color4 COLOR_WHITE = Color4(MAX_U8, MAX_U8, MAX_U8, MAX_U8);
global_var constexpr Color4 COLOR_RED   = Color4(MAX_U8, 0, 0, 0);
global_var constexpr Color4 COLOR_GREEN = Color4(0, MAX_U8, 0, 0);
global_var constexpr Color4 COLOR_BLUE  = Color4(0, 0, MAX_U8, 0);
global_var constexpr Color4 COLOR_BLACK = Color4(0, 0, 0, 0);

global_var constexpr Color4 COLOR_YELLOW = COLOR_GREEN + COLOR_RED;

global_var bool shouldStop = false;

struct Basic_Renderer {
    Color4 clear_color;

    U8 bytes_per_pixel = 4;
    U64 x_offset = 0;
    U64 y_offset = 0;

    void *pixels_buffer = nullptr;
    U64 pixels_width = 0;
    U64 pixels_height = 0;

    BITMAPINFO info{};

    void blit(HDC dc, S32 x_offset, S32 y_offset, S32 width, S32 height);
    void resize(S32 w, S32 h);
};

static Basic_Renderer global_renderer{};

LRESULT CALLBACK win32_window_proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI
WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance, _In_ LPSTR commandLine, _In_ int showMode)
{

    persist_var LPCSTR CLASS_NAME = "Software Rasterizer";
    persist_var LPCSTR WINDOW_TITLE = "Software Rasterizer";

    WNDCLASS windowClass{};
    windowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    windowClass.lpfnWndProc = win32_window_proc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = CLASS_NAME;

    assert(RegisterClassA(&windowClass));

    HWND window = CreateWindowExA(
        0,
        windowClass.lpszClassName,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,  // int x
        CW_USEDEFAULT,  // int y
        CW_USEDEFAULT,  // int width
        CW_USEDEFAULT,  // int height
        nullptr,        // windowParent
        nullptr,        // menu
        instance,
        nullptr);
    assert(window);

    ShowWindow(window, showMode);

    global_renderer.clear_color = COLOR_GREEN;

    while (!shouldStop) {

        MSG message = {};
        while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                // NOTE(ilya.a): Make sure that we will quit the mainloop.
                shouldStop = true;
            }

            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        USZ pitch = global_renderer.pixels_width * global_renderer.bytes_per_pixel;
        U8 *row = static_cast<U8 *>(global_renderer.pixels_buffer);

        for (U64 y = 0; y < global_renderer.pixels_height; ++y) {
            Color4 *pixel = reinterpret_cast<Color4 *>(row);

            for (U64 x = 0; x < global_renderer.pixels_width; ++x) {
                *pixel = global_renderer.clear_color;
                ++pixel;
            }

            row += pitch;
        }

        HDC dc = GetDC(window);

        RECT windowRect;
        GetClientRect(window, &windowRect);
        S32 x = windowRect.left;
        S32 y = windowRect.top;
        S32 width = 0, height = 0;
        GetRectSize(&windowRect, &width, &height);

        global_renderer.blit(dc, x, y, width, height);

        ReleaseDC(window, dc);

    }

    return 0;
}


LRESULT CALLBACK
win32_window_proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (message) {
        case WM_ACTIVATEAPP: {
            OutputDebugString("WM_ACTIVATEAPP\n");
        } break;
        case WM_SIZE: {
            OutputDebugString("WM_SIZE\n");
            S32 width = LOWORD(lParam);
            S32 height = HIWORD(lParam);

            global_renderer.resize(width, height);
        } break;
        case WM_PAINT: {
            OutputDebugString("WM_PAINT\n");

            PAINTSTRUCT ps{};
            HDC dc = BeginPaint(window, &ps);
            assert(dc && dc != INVALID_HANDLE_VALUE);

            S32 x = ps.rcPaint.left;
            S32 y = ps.rcPaint.top;
            S32 width = 0, height = 0;
            GetRectSize(&ps.rcPaint, &width, &height);
            global_renderer.blit(dc, x, y, width, height);

            EndPaint(window, &ps);

        } break;
        case WM_CLOSE: {
            // TODO(ilya.a): Ask for closing?
            OutputDebugString("WM_CLOSE\n");
            shouldStop = true;
        } break;
        case WM_DESTROY: {
            // TODO(ilya.a): Casey says that we maybe should recreate
            // window later?
            OutputDebugString("WM_DESTROY\n");
            // PostQuitMessage(0);
        } break;

        default: {
            // Leave other events to default Window's handler.
            result = DefWindowProc(window, message, wParam, lParam);
        } break;
    }

    return result;
}

void
Basic_Renderer::blit(HDC dc, S32 x_offset, S32 y_offset, S32 width, S32 height)
{
    StretchDIBits(dc,
        this->x_offset, this->y_offset, this->pixels_width, this->pixels_height,
        x_offset, y_offset, width, height,
        this->pixels_buffer, &this->info,
        DIB_RGB_COLORS, SRCCOPY
    );
}

void
Basic_Renderer::resize(S32 w, S32 h)
{
    if (this->pixels_buffer != nullptr && VirtualFree(this->pixels_buffer, 0, MEM_RELEASE) == 0) {
        //                                                                    ^^^^^^^^^^^
        // NOTE(ilya.a): Might be more reasonable to use MEM_DECOMMIT instead for
        // MEM_RELEASE. Because in that case it's will be keep buffer around, until
        // we use it again.
        // P.S. Also will be good to try protect buffer after deallocating or other
        // stuff.
        //
        // TODO(ilya.a):
        //     - [ ] Checkout how it works.
        //     - [ ] Handle allocation error.
        assert(false && "Failed to deallocate!");
    }
    this->pixels_width = w;
    this->pixels_height = h;

    this->info.bmiHeader.biSize          = sizeof(this->info.bmiHeader);
    this->info.bmiHeader.biWidth         = w;
    this->info.bmiHeader.biHeight        = h;
    this->info.bmiHeader.biPlanes        = 1;
    this->info.bmiHeader.biBitCount      = 32;      // NOTE: Align to WORD
    this->info.bmiHeader.biCompression   = BI_RGB;
    this->info.bmiHeader.biSizeImage     = 0;
    this->info.bmiHeader.biXPelsPerMeter = 0;
    this->info.bmiHeader.biYPelsPerMeter = 0;
    this->info.bmiHeader.biClrUsed       = 0;
    this->info.bmiHeader.biClrImportant  = 0;

    USZ bufferSize = w * h * this->bytes_per_pixel;
    this->pixels_buffer = VirtualAlloc(nullptr, bufferSize, MEM_COMMIT, PAGE_READWRITE);
    assert(this->pixels_buffer && "Failed to allocate memory!");
}
