
#include <cassert>
#include <cmath>

#include <algorithm>
#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <fstream>
#include <sstream>
#include <functional>
#include <random>

#if !defined(NOMINMAX)
#define NOMINMAX
#endif

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


S64 perf_get_counter_frequency(void);
S64 perf_get_counter(void);

struct Clock {
    S64 ticks_begin;
    S64 ticks_end;
    S64 frequency;

    Clock(void) : ticks_begin(perf_get_counter()), ticks_end(0), frequency(perf_get_counter_frequency()) {}

    inline F64
    tick(void) noexcept
    {
        this->ticks_end = perf_get_counter();
        F64 elapsed = static_cast<double>(this->ticks_end - this->ticks_begin) / static_cast<double>(this->frequency);
        this->ticks_begin = perf_get_counter();
        return elapsed;
    }
};


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

struct V2 {

    F32 x = 0;
    F32 y = 0;

    constexpr V2(F32 x_ = 0, F32 y_ = 0) noexcept : x(x_), y(y_) {}

    F32 dot(V2 other) const;
    F32 magnitude(void) const;

    V2 normal(void) const;

    V2 perpendicular_ccw(void) const;
    V2 perpendicular_cw(void) const;

    constexpr V2
    operator+ (V2 other) const noexcept
    {
        return { this->x + other.x, this->y + other.y };
    }

    constexpr V2
    operator- (V2 other) const noexcept
    {
        return { this->x - other.x, this->y - other.y };
    }

    constexpr V2
    operator* (F32 value) const noexcept
    {
        return { this->x * value, this->y * value };
    }

    constexpr V2
    operator/ (F32 value) const noexcept
    {
        return { this->x / value, this->y / value };
    }
};

struct V3 {

    F32 x = 0;
    F32 y = 0;
    F32 z = 0;

    constexpr V3
    operator+ (V3 other) const noexcept
    {
        return { this->x + other.x, this->y + other.y, this->z + other.z };
    }

    constexpr V3
    operator+ (V2 other) const noexcept
    {
        return { this->x + other.x, this->y + other.y, this->z };
    }

    constexpr V3
    operator- (V3 other) const noexcept
    {
        return { this->x - other.x, this->y - other.y, this->z - other.z };
    }

    constexpr V3
    operator- (V2 other) const noexcept
    {
        return { this->x - other.x, this->y - other.y, this->z };
    }

    constexpr V3
    operator* (F32 value) const noexcept
    {
        return { this->x * value, this->y * value, this->z * value };
    }

    template<typename Ty> constexpr Ty
    to() const noexcept
    {
        static_assert("Convertion not implemented!");
    }

    template<> constexpr V2
    to() const noexcept
    {
        return { this->x, this->y };
    }
};

// Matrix 3x3
struct M3x3 {
    V3 r0{}, r1{}, r2{};
};


/*
    constexpr V3
    transform(V3 ihat, V3 jhat, V3 khat, V3 p) const
    {
        // ihat - x axis, jhat - y axis, khat - z axis.
        return ihat * p.x + jhat * p.y + khat * p.z;
    }
*/


V3
operator* (M3x3 m, V3 v)
{
    V3 ret = m.r0 * v.x + m.r1 * v.y + m.r2 * v.z;
    return ret;
}


//
// source: https://danceswithcode.net/engineeringnotes/rotations_in_3d/rotations_in_3d_part1.html
//


M3x3
get_rotation_mat3x3_roll(F32 roll)
{
    // roll - u
    // as eurler angle symbol

    M3x3 ret{};

    ret.r0 = { 1, 0, 0 };
    ret.r1 = { 0, std::cos(roll), -std::sin(roll) };
    ret.r2 = { 0, std::sin(roll), std::cos(roll) };

    return ret;
}


M3x3
get_rotation_mat3x3_pitch(F32 pitch)
{
    // pitch - v
    // as eurler angle symbol

    M3x3 ret{};

    ret.r0 = { std::cos(pitch), 0, std::sin(pitch) };
    ret.r1 = { 0, 1, 0 };
    ret.r2 = { -std::sin(pitch), 0, std::cos(pitch) };

    return ret;
}


M3x3
get_rotation_mat3x3_yaw(F32 yaw)
{
    // yaw - w
    // as eurler angle symbol

    M3x3 ret{};

    ret.r0 = { std::cos(yaw), -std::sin(yaw), 0 };
    ret.r1 = { std::sin(yaw), std::cos(yaw), 0 };
    ret.r2 = { 0, 0, 1 };

    return ret;
}


struct Transform {

    F32 roll = 0, pitch = 0, yaw = 0;

    // void
    // basis_vectors(V3 *ihat, V3 *jhat, V3 *khat) const
    // {
    //     if (ihat) {
    //         *ihat = { std::cos(this->pitch), 0, std::sin(this->pitch) };
    //     }

    //     if (jhat) {
    //         *jhat = { 0, 1, 0 };
    //     }

    //     if (khat) {
    //         *khat = { -std::sin(this->pitch), 0, std::cos(this->pitch) };
    //     }
    // }

    inline V3
    to_world(V3 p) const noexcept
    {
        V3 result = p;

        result = get_rotation_mat3x3_roll(this->roll) * result;
        result = get_rotation_mat3x3_pitch(this->pitch) * result;
        result = get_rotation_mat3x3_yaw(this->yaw) * result;

        return result;
    }

    // constexpr V3
    // transform(V3 ihat, V3 jhat, V3 khat, V3 p) const
    // {
    //     // ihat - x axis, jhat - y axis, khat - z axis.
    //     return ihat * p.x + jhat * p.y + khat * p.z;
    // }

};


struct V3S32 {
    S32 x = 0, y = 0, z = 0;

    constexpr V3S32
    operator- (S32 value) const noexcept
    {
        return { this->x - value, this->y - value, this->z - value };
    }
};


struct R32 {
    S32 x = 0, y = 0, w = 0, h = 0;
};



bool point_inside_triangle(V2 p, V2 a, V2 b, V2 c);

std::pair<std::vector<V3>, std::vector<S32>> load_obj(std::string_view file_name);

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


constexpr S32
get_offset(S32 width, S32 y, S32 x) noexcept
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

bool get_window_dim(HWND window, S32 *x, S32 *y, S32 *w, S32 *h);
R32 calculate_bounding_box(V2 window_size, V2 triangle[3]);
V2 world_to_screen(V3 v, Transform transform, V2 screen_size);

struct Basic_Renderer {
    Color4 clear_color;

    U8 bytes_per_pixel = 4;
    U64 x_offset = 0;
    U64 y_offset = 0;

    void *pixels_buffer = nullptr;
    U32 pixels_width = 0;
    U32 pixels_height = 0;

    BITMAPINFO info{};

    void blit(HDC dc, S32 x_offset, S32 y_offset, S32 width, S32 height);
    void resize(S32 w, S32 h);
};

static Basic_Renderer global_renderer{};

LRESULT CALLBACK win32_window_proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI
WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance, _In_ LPSTR commandLine, _In_ int showMode)
{
    assert(AllocConsole());
    freopen("CONOUT$", "w+", stdout); // redirect stdout to console
    freopen("CONOUT$", "w+", stderr); // redirect stderr to console
    freopen("CONIN$", "r+", stdin);   // redirect stdin to console

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
        600,  // int width
        600 + 23,  // int height
        nullptr,        // windowParent
        nullptr,        // menu
        instance,
        nullptr);
    assert(window);

    ShowWindow(window, showMode);

    global_renderer.clear_color = COLOR_WHITE;

    auto [ vertexes, indexes ] = load_obj(R"(P:\softrast\assets\cube.obj)");

    using result_type = decltype(std::default_random_engine())::result_type;
    auto engine = std::default_random_engine();
    std::uniform_int_distribution<result_type> dist(0, MAX_U8);

    auto get_random_color = [](decltype(dist) *dist, decltype(engine) *engine) -> Color4 {
        return {
            static_cast<U8>((*dist)(*engine)),
            static_cast<U8>((*dist)(*engine)),
            static_cast<U8>((*dist)(*engine)),
            MAX_U8
        };
    };

    Clock clock{};
    F32 rotation = 1.0f;
    F32 rotation_speed = 0.8f;

    std::vector<Color4> colors(indexes.size());

    /// XXX
    for (USZ i = 0; i < indexes.size(); i += 3) {
        Color4 color = get_random_color(&dist, &engine);
        colors[i] = color;
        colors[i + 1] = color;
        colors[i + 2] = color;
    }


    while (!shouldStop) {

        F32 dt = static_cast<F32>(clock.tick());

        MSG message = {};
        while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                // NOTE(ilya.a): Make sure that we will quit the mainloop.
                shouldStop = true;
            }

            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        rotation += rotation_speed * dt;

        S32 window_x = 0, window_y = 0, window_w = 0, window_h = 0;
        assert(get_window_dim(window, &window_x, &window_y, &window_w, &window_h));

        V2 window_size { static_cast<F32>(window_w), static_cast<F32>(window_h) };

        // printf(" window.w = %f  window.h = %f\n", window_size.x, window_size.y);

        Basic_Renderer *r = &global_renderer;

        // Setting screen to be gray!
        memset(r->pixels_buffer, 69, r->pixels_width * r->pixels_height *  r->bytes_per_pixel);

        for (USZ i = 0; i < indexes.size(); i += 3) {
            V3 vertexes_[3]{vertexes[indexes[i]], vertexes[indexes[i + 1]], vertexes[indexes[i + 2]]};

            V2 triangle[3]{};
            Transform transform{rotation, rotation * 0.1f, rotation * 0.3f};

            triangle[0] = world_to_screen(vertexes_[0], transform, window_size);
            triangle[1] = world_to_screen(vertexes_[1], transform, window_size);
            triangle[2] = world_to_screen(vertexes_[2], transform, window_size);

            R32 bb = calculate_bounding_box(window_size, triangle);

            for (S32 y = bb.y; y < bb.h; ++y) {
                for (S32 x = bb.x; x < bb.w; ++x) {
                    Color4 *pixel = static_cast<Color4 *>(r->pixels_buffer) + get_offset(r->pixels_height, x, y);

                    V2 p{static_cast<F32>(x), static_cast<F32>(y)};

                    if (point_inside_triangle(p, triangle[0], triangle[1], triangle[2])) {
                        *pixel = colors[i];
                    }
                }
            }


        }

        #if 0
        USZ pitch = global_renderer.pixels_width * global_renderer.bytes_per_pixel /* sizeof(Color4) */;
        U8 *row = static_cast<U8 *>(global_renderer.pixels_buffer);

        for (U32 y = 0; y < global_renderer.pixels_height; ++y) {
            Color4 *pixel = reinterpret_cast<Color4 *>(row);

            for (U32 x = 0; x < global_renderer.pixels_width; ++x) {
                //*pixel = COLOR_BLACK;

                V2 p{static_cast<F32>(x), static_cast<F32>(y)};


                if constexpr (0) {
                    V2 a{ window_size.x * 0.2f, window_size.y * 0.2f };
                    V2 b{ window_size.x * 0.6f, window_size.y * 0.4f };
                    V2 c{ window_size.x * 0.4f, window_size.y * 0.5f };

                    if (point_inside_triangle(p, a, b, c)) {
                        *pixel = COLOR_WHITE;
                    }
                }

                if constexpr (0) {
                    pixel->R = MAX_U8 * p.x / window_size.x;
                    pixel->G = MAX_U8 * p.y / window_size.y;
                }

                ++pixel;
            }

            row += pitch;
        }
        #endif // #if 0

        HDC dc = GetDC(window);
        global_renderer.blit(dc, window_x, window_y, window_w, window_h);
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

bool
get_window_dim(HWND window, S32 *x, S32 *y, S32 *w, S32 *h)
{
    RECT window_rect{};
    if (!GetClientRect(window, &window_rect)) {
        return false;
    }

    if (x) {
        *x = window_rect.left;
    }

    if (y) {
        *y = window_rect.top;
    }

    if (w) {
        *w = window_rect.right - window_rect.left;
    }

    if (h) {
        *h = window_rect.bottom - window_rect.top;
    }

    return true;
}

F32
V2::dot(V2 other) const
{
    return (this->x * other.x) + (this->y * other.y);
}

F32
V2::magnitude(void) const
{
    return std::sqrt(this->x * this->x + this->y * this->y);
}

V2
V2::normal(void) const
{
    F32 magnitude = this->magnitude();
    return { this->x / magnitude, this->y / magnitude};
}

V2
V2::perpendicular_cw(void) const
{
    return { -this->y, this->x };
}

V2
V2::perpendicular_ccw(void) const
{
    return { this->y, -this->x };
}

bool
point_inside_triangle(V2 p, V2 a, V2 b, V2 c)
{

    V2 a_side = (b - a).perpendicular_ccw();
    if (a_side.dot(p - a) > 0) {
        return false;
    }

    V2 b_side = (c - b).perpendicular_ccw();
    if (b_side.dot(p - b) > 0) {
        return false;
    }

    V2 c_side = (a - c).perpendicular_ccw();
    if (c_side.dot(p - c) > 0) {
        return false;
    }

    return true;
}

std::pair<std::vector<V3>, std::vector<S32>>
load_obj(std::string_view file_name)
{

    std::ifstream file(file_name.data());

    std::vector<V3> vertexes{};
    std::vector<S32> indexes{};

    std::string line{};
    while (std::getline(file, line)) {
        std::istringstream iss(line);

        if (line.starts_with("v ")) {

            C8 v = 0;
            F32 x = 0, y = 0, z = 0;

            iss >> v >> x >> y >> z;

            vertexes.emplace_back(x, y, z);
        }

        if (line.starts_with("f ")) {

            C8 prefix = 0, sep = 0;
            iss >> prefix;

            S32 count = 0;
            while (!iss.eof()) {
                S32 index = 0, stub = 0;
                iss >> index >> sep >> stub >> sep >> stub;

                if (count >= 3) {
                    S32 a = indexes[indexes.size() - 3];
                    S32 b = indexes[indexes.size() - 1];

                    indexes.push_back(a);
                    indexes.push_back(b);
                }

                indexes.push_back(index - 1); // In .obj index values starts from 1.

                ++count;
            }
        }
    }



    return {vertexes, indexes};
}

V2
world_to_screen(V3 v, Transform transform, V2 screen_size)
{
    V3 v_world = transform.to_world(v);

    F32 world_units_in_screen_height = 5;
    F32 pixels_per_unit = screen_size.y / world_units_in_screen_height;

    V2 offset = v_world.to<V2>() * pixels_per_unit;
    return (screen_size / 2) + offset;

}

R32
calculate_bounding_box(V2 window_size, V2 triangle[3])
{
    R32 result{};

    V2 a = triangle[0], b = triangle[1], c = triangle[2];

    F32 min_x = std::min(std::min(a.x, b.x), c.x);
    F32 min_y = std::min(std::min(a.y, b.y), c.y);
    F32 max_x = std::max(std::max(a.x, b.x), c.x);
    F32 max_y = std::max(std::max(a.y, b.y), c.y);

    result.x = std::clamp(static_cast<S32>(min_x), 0, static_cast<S32>(window_size.x) - 1);
    result.y = std::clamp(static_cast<S32>(min_y), 0, static_cast<S32>(window_size.y) - 1);

    result.w = std::clamp(static_cast<S32>(std::ceil(max_x)), 0, static_cast<S32>(window_size.x) - 1);
    result.h = std::clamp(static_cast<S32>(std::ceil(max_y)), 0, static_cast<S32>(window_size.y) - 1);

    return result;
}

S64
perf_get_counter_frequency(void)
{
    LARGE_INTEGER perf_frequency_result;
    QueryPerformanceFrequency(&perf_frequency_result);
    S64 perf_frequency = perf_frequency_result.QuadPart;
    return perf_frequency;
}

S64
perf_get_counter(void)
{
    LARGE_INTEGER perf_counter_result;
    QueryPerformanceCounter(&perf_counter_result);
    S64 perf_counter = perf_counter_result.QuadPart;
    return perf_counter;
}
