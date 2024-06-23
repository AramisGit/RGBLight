#ifndef UNICODE
#define UNICODE
#endif

#include <string>
#include <fstream>
#include <sstream>

#include <windows.h>
#include <WinUser.h>

#include "colour32.h"
#include "keycodes.h"

double h, s, v;
const char *pConfigPath = "start_colour.ini";

// trim from end of string (right)
std::string RightTrim(std::string const &str, const char *t)
{
  std::string newStr(str);
  newStr.erase(newStr.find_last_not_of(t) + 1);
  return newStr;
}

// trim from beginning of string (left)
std::string LeftTrim(std::string const &str, const char *t)
{
  std::string newStr(str);
  newStr.erase(0, newStr.find_first_not_of(t));
  return newStr;
}

// trim from both ends of string (right then left)
std::string Trim(std::string const &str)
{
  static const char *ws = " \t\n\r\f\v";
  return LeftTrim(RightTrim(str, ws), ws);
}

/// <summary>
/// Reads the colour specified in the colour config file.
/// </summary>
/// <param name="pPath"></param>
/// <returns></returns>
void ReadColourFromFile(const char *pPath)
{
  std::ifstream ifs(pPath);

  const size_t bufSize = 1024;
  char buffer[bufSize] = {};

  if (!ifs.good())
    return;

  do
  {
    ifs.getline(buffer, bufSize);

    std::string line(buffer);
    line = Trim(line);

    if (line.size() == 0)
      continue;

    // Comments
    if (line.front() == ';')
      continue;

    size_t pos = line.find_first_of('=');

    if (pos == line.size())
      continue;
    std::string key = Trim(line.substr(0, pos));
    std::string value = Trim(line.substr(pos + 1));

    std::stringstream code(value);

    double x = 0;
    code >> x;

    if (key == "H")
      h = x;
    if (key == "S")
      s = x;
    if (key == "V")
      v = x;
      
  } while (ifs.good());

  ifs.close();
}

/// <summary>
/// Writes the last colour to the specified colour confid file.
/// </summary>
/// <param name="pPath"></param>
/// <returns></returns>
void WriteColourToFile(const char *pPath)
{
  std::ofstream ofs(pPath);

  if (!ofs.good())
    return;

  ofs << "; saves the colour from last successful app closing.\n";
  ofs << "H = " << (int)h % 360 << "\n";
  ofs << "S = " << s << "\n";
  ofs << "V = " << v << "\n";

  ofs.close();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

/// <summary>
/// The MAIN function for a windows program.
/// </summary>
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"Window Class"; // Register the window class name.

    WNDCLASS wc = { }; // Create empty Window Class.

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.style = CS_DBLCLKS; //enables double click messages to be sent
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    h = 0; // Initial hue value of 0 degrees. (Red)
    s = 100; // Initial saturation value of 100. (Full Saturation)
    v = 100; // Initial vibrance of 100. (Full Vibrance)

    ReadColourFromFile(pConfigPath);

    HWND hWnd = CreateWindowEx(
        WS_EX_COMPOSITED,        // Optional window styles
        CLASS_NAME,                    // Window class
        L"RGBLight",                   // Window text
        WS_OVERLAPPEDWINDOW,           // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, // Size and position
        NULL,       // Parent window
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    SetWindowLong(
        hWnd, 
        GWL_EXSTYLE, // Disable default titlebar icon
        GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_DLGMODALFRAME); 

    if (hWnd == NULL)
        return 0;

    ShowWindow(hWnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

/// <summary>
/// Updates the global HSV values with any key data.
/// </summary>
void UpdateColourHSVKeys()
{
    bool upPressed = GetKeyState(VK_UP) & 0x8000;
    bool downPressed = GetKeyState(VK_DOWN) & 0x8000;
    bool leftPressed = GetKeyState(VK_LEFT) & 0x8000;
    bool rightPressed = GetKeyState(VK_RIGHT) & 0x8000;
    bool backPressed = GetKeyState(VK_BACK) & 0x8000;
    bool enterPressed = GetKeyState(VK_RETURN) & 0x8000;
    bool spacePressed = GetKeyState(VK_SPACE) & 0x8000;

    bool vIsBelowMax = v < 100;
    bool vIsAboveMin = v > 0;
    bool sIsBelowMax = s < 100;
    bool sIsAboveMin = s > 0;

    uint8_t rate = (1 + !spacePressed * 9);

    h += rightPressed * rate;
    h -= leftPressed * rate;
    s += enterPressed * sIsBelowMax;
    s -= backPressed * sIsAboveMin;
    v += upPressed * vIsBelowMax;
    v -= downPressed * vIsAboveMin;
}

/// <summary>
/// Updates the global HSV values with any mouse data.
/// </summary>
void UpdateColourHSVMouse(int wheelDelta) 
{
    bool spacePressed = GetKeyState(VK_SPACE) & 0x8000;
    bool leftMousePressed = GetKeyState(VK_LBUTTON) & 0x8000;
    bool rightMousePressed = GetKeyState(VK_RBUTTON) & 0x8000;
    bool middleMousePressed = GetKeyState(VK_MBUTTON) & 0x8000;

    bool sPosAndBelow = (s < 100) * (wheelDelta > 0);
    bool sNegAndAbove = (s > 0) * (wheelDelta < 0);
    bool vPosAndBelow = (v < 100) * (wheelDelta > 0);
    bool vNegAndAbove = (v > 0) * (wheelDelta < 0);

    uint8_t rate = (1 + !spacePressed * 9);
    int delta = wheelDelta / 120;

    h += delta * (!rightMousePressed) * (!leftMousePressed) * rate;
    s += delta * (rightMousePressed) * (sPosAndBelow + sNegAndAbove);
    v += delta * (leftMousePressed) * (vPosAndBelow + vNegAndAbove);
}

/// <summary>
/// Fills the rect with the given colour.
/// </summary>
void ColourRect(HWND hwnd, colour32 colour) 
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps); 
    HBRUSH brush = CreateSolidBrush(RGB(colour.r(), colour.g(), colour.b()));
    FillRect(hdc, &ps.rcPaint, brush);
    EndPaint(hwnd, &ps);
    DeleteObject(brush);
}

/// <summary>
/// Toggles the window to and from fullscreen, preserving its window style.
/// </summary>
void ToggleFullscreen(HWND hwnd)
{
    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE); //gets the window style of the window
    DWORD dwExStyle = GetWindowLong(hwnd, GWL_EXSTYLE); //gets the edge style of the window
    dwExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE); //appends these flags to edge style

    if (dwStyle & WS_OVERLAPPEDWINDOW) //if not in fullscreen (windowed)
    {
        MONITORINFO mi = { sizeof(mi) };

        if (GetWindowPlacement(hwnd, &g_wpPrev) 
            && GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) //if we're in primary monitor
        {
            SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW); //set window style to same as windowed style without overlapped window flag
            SetWindowLong(hwnd, GWL_EXSTYLE, dwExStyle); //set edge style to no border.

            SetWindowPos(
                hwnd,
                HWND_TOP,
                mi.rcMonitor.left, //set pos to cover all of screen 
                mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED
            );

            ShowCursor(false);
        }
    }
    else 
    {
        SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hwnd, &g_wpPrev);
        SetWindowPos(
            hwnd,
            NULL, //insertAfter
            0, 0, 0, 0, // x, y, cx, cy
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED
        );

        ShowCursor(true);
    }
}

/// <summary>
/// Sends a WM_PAINT message to the message loop.
/// </summary>
void CallRepaintOnRect(HWND hwnd)
{
    InvalidateRect(hwnd, NULL, FALSE);
}

/// <summary>
/// Sends a WM_CLOSE message to the message loop.
/// </summary>
void CallCloseOnRect(HWND hwnd)
{
    SendMessage(hwnd, WM_CLOSE, 0, 0); //sends a WM_CLOSE message
}

/// <summary>
/// Returns a wideChar string in the form of a hexcode: "#RRGGBB".
/// </summary>
std::wstring wRGBtoHex(int r, int g, int b)
{
    wchar_t hexcol[16];
    std::swprintf(hexcol, sizeof hexcol / 2, L"#%02X%02X%02X", r, g, b);
    return hexcol;
}

/// <summary>
/// Returns a wideChar string in the form of its components: "R, G, B".
/// </summary>
std::wstring RGBtoComp(int r, int g, int b)
{
    wchar_t hexcol[16];
    std::swprintf(hexcol, sizeof hexcol / 2, L"%d, %d, %d", r, g, b);
    return hexcol;
}

/// <summary>
/// Copies the current colour displayed to the clipboard as a HexCode string.
/// </summary>
/// <param name="hwnd"></param>
/// <param name="colour"></param>
void CopyCodeToClipBoard(HWND hwnd, colour32 colour) 
{
    if (GetKeyState(VK_CONTROL) & 0x8000)
    {
      HANDLE handl = GlobalAlloc(GMEM_MOVEABLE, 8 * sizeof(WCHAR));

      if (!handl)
        return;

      memcpy((wchar_t *)GlobalLock(handl), wRGBtoHex(colour.r(), colour.g(), colour.b()).c_str(), 8 * sizeof(WCHAR));

      GlobalUnlock(handl);

      if (OpenClipboard(NULL))
      {
        EmptyClipboard();
        if(SetClipboardData(CF_UNICODETEXT, handl))
          handl = NULL;
        CloseClipboard();
      }

      if (handl)
        GlobalFree(handl);
    }
}

/// <summary>
/// The callback function called in the message loop.
/// </summary>
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
    {
        WriteColourToFile(pConfigPath);
        PostQuitMessage(0);
        return 0;
    }
    case WM_PAINT:
    {
        colour32 colour = colour32(h, s, v);
        ColourRect(hwnd, colour);
        SetWindowText(hwnd, wRGBtoHex(colour.r(), colour.g(), colour.b()).c_str());
        break;
    }
    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)
            CallCloseOnRect(hwnd);

        if (wParam == F_KEY)
            ToggleFullscreen(hwnd);

        if (wParam == C_KEY)
            CopyCodeToClipBoard(hwnd, colour32(h, s, v));
            
        UpdateColourHSVKeys();
        CallRepaintOnRect(hwnd);
        break;
    }
    case WM_LBUTTONDBLCLK: 
    {
        ToggleFullscreen(hwnd);
        break;
    }
    case WM_MOUSEWHEEL: 
    {
        UpdateColourHSVMouse(GET_WHEEL_DELTA_WPARAM(wParam));
        CallRepaintOnRect(hwnd);
        break;
    }
    return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}