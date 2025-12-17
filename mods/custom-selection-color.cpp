// ==WindhawkMod==
// @id              custom-selection-color
// @name            Custom Selection Color
// @description     Customize Windows selection colors. Best for Windows 11.
// @author          Vinarator
// @version         1.0.0
// @github          "https://github.com/Vinarat0r"
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lcomdlg32 -luser32 -lgdi32 -ladvapi32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Custom Selection Color
Customize the Windows selection rectangle (fill & outline).

## Features:
- **Visual Customization:** Set custom Fill and Outline colors independently.
- **Smart Balance:** One-click automatic color matching for the outline.


![Screenshot with labels](https://i.imgur.com/eJ2Aklx.png)
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <commdlg.h>
#include <string>
#include <cstdio>

struct AppState {
    COLORREF fillColor;
    COLORREF borderColor;
    BOOL isDarkMode;
    UINT hotkeyChar;
    HWND hMainWnd;
    HWND hSettingsWnd;
};

static AppState g_State = {
    RGB(0, 120, 215),
    RGB(255, 255, 255),
    TRUE,
    'C',
    NULL,
    NULL
};

static COLORREF g_CustomColors[16];

#define UI_WIDTH 380 
#define UI_HEIGHT 340
#define BOX_WIDTH 150
#define BOX_HEIGHT 110
#define BTN_SIZE 30 

int MinVal(int a, int b) { return (a < b) ? a : b; }


void WriteRegistryColor(const wchar_t* valueNameW, COLORREF color) {
    char colorStrA[64];
    sprintf_s(colorStrA, sizeof(colorStrA), "%d %d %d", GetRValue(color), GetGValue(color), GetBValue(color));

    char valueNameA[128];
    int conv = WideCharToMultiByte(CP_ACP, 0, valueNameW, -1, valueNameA, sizeof(valueNameA), NULL, NULL);
    if (conv == 0) {
        valueNameA[0] = '\0';
    }

    HKEY hKey = NULL;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Control Panel\\Colors", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, valueNameA, 0, REG_SZ, (const BYTE*)colorStrA, (DWORD)strlen(colorStrA) + 1);
        RegCloseKey(hKey);
    } else {
        if (RegCreateKeyExA(HKEY_CURRENT_USER, "Control Panel\\Colors", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
            RegSetValueExA(hKey, valueNameA, 0, REG_SZ, (const BYTE*)colorStrA, (DWORD)strlen(colorStrA) + 1);
            RegCloseKey(hKey);
        }
    }

    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Control Panel\\Desktop\\Colors", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, valueNameA, 0, REG_SZ, (const BYTE*)colorStrA, (DWORD)strlen(colorStrA) + 1);
        RegCloseKey(hKey);
    } else {
        if (RegCreateKeyExA(HKEY_CURRENT_USER, "Control Panel\\Desktop\\Colors", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
            RegSetValueExA(hKey, valueNameA, 0, REG_SZ, (const BYTE*)colorStrA, (DWORD)strlen(colorStrA) + 1);
            RegCloseKey(hKey);
        }
    }

    SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"Colors",
                        SMTO_ABORTIFHUNG, 5000, NULL);
}

void RestartExplorer() {
    ShellExecuteW(NULL, L"open", L"cmd.exe", L"/c taskkill /f /im explorer.exe & start explorer.exe", NULL, SW_HIDE);
}

void ApplyColors() {
    int nIndices[3] = { COLOR_HIGHLIGHT, COLOR_HIGHLIGHTTEXT, COLOR_HOTLIGHT };
    COLORREF crColors[3] = { g_State.fillColor, g_State.borderColor, g_State.fillColor };
    SetSysColors(3, nIndices, crColors);

    WriteRegistryColor(L"Hilight", g_State.fillColor);
    WriteRegistryColor(L"HilightText", g_State.borderColor);
    WriteRegistryColor(L"HotTrackingColor", g_State.fillColor);

}

COLORREF AdjustBrightness(COLORREF color, double factor) {
    int r = (int)(GetRValue(color) * factor);
    int g = (int)(GetGValue(color) * factor);
    int b = (int)(GetBValue(color) * factor);
    return RGB(MinVal(r, 255), MinVal(g, 255), MinVal(b, 255));
}

void DrawRect(HDC hdc, RECT rc, COLORREF color) {
    HBRUSH br = CreateSolidBrush(color);
    FillRect(hdc, &rc, br);
    DeleteObject(br);
}
void DrawBorder(HDC hdc, RECT rc, COLORREF color) {
    HBRUSH br = CreateSolidBrush(color);
    FrameRect(hdc, &rc, br);
    DeleteObject(br);
}
void DrawButton(HDC hdc, RECT rc, const wchar_t* text, bool isPressed, bool isPrimary, bool isDark) {
    COLORREF bg;
    if (isPrimary) {
        bg = isPressed ? RGB(0, 90, 180) : RGB(0, 120, 215); 
    } else {
        if (isDark) bg = isPressed ? RGB(70, 70, 70) : RGB(50, 50, 50);
        else        bg = isPressed ? RGB(200, 200, 200) : RGB(230, 230, 230);
    }

    DrawRect(hdc, rc, bg);
    
    COLORREF border = isDark ? RGB(100, 100, 100) : RGB(180, 180, 180);
    DrawBorder(hdc, rc, border);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, (isPrimary || isDark) ? RGB(255, 255, 255) : RGB(0, 0, 0));
    
    RECT rcText = rc;
    if (isPressed) { rcText.top += 2; rcText.left += 2; }
    
    DrawTextW(hdc, text, -1, &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}
void DrawWindowControls(HDC hdc, int winW, bool isDark, bool showMinimize) {
    COLORREF txtColor = isDark ? RGB(200, 200, 200) : RGB(50, 50, 50);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, txtColor);
    RECT rcClose = { winW - BTN_SIZE, 0, winW, BTN_SIZE };
    DrawTextW(hdc, L"✕", -1, &rcClose, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if (showMinimize) {
        RECT rcMin = { winW - (BTN_SIZE * 2), 0, winW - BTN_SIZE, BTN_SIZE };
        DrawTextW(hdc, L"─", -1, &rcMin, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}


LRESULT CALLBACK SettingsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool isWaitingForKey = false;

    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);

        DrawRect(hdc, rc, g_State.isDarkMode ? RGB(40, 40, 40) : RGB(240, 240, 240));
        DrawBorder(hdc, rc, RGB(100, 100, 100));
        DrawWindowControls(hdc, rc.right, g_State.isDarkMode, false);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, g_State.isDarkMode ? RGB(255, 255, 255) : RGB(0, 0, 0));
        
        RECT rcText = rc;
        rcText.bottom -= 40;
        
        wchar_t buf[128];
        if (isWaitingForKey) {
            wsprintfW(buf, L"Press any key...\n(Ctrl + Alt + [KEY])");
        } else {
            wsprintfW(buf, L"Current Hotkey:\nCtrl + Alt + %C", (char)g_State.hotkeyChar);
        }
        DrawTextW(hdc, buf, -1, &rcText, DT_CENTER | DT_VCENTER);

        RECT rcBtn = { 40, 100, 260, 140 };
        DrawButton(hdc, rcBtn, isWaitingForKey ? L"Waiting..." : L"Change Key", false, true, g_State.isDarkMode);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        RECT rc; GetClientRect(hwnd, &rc);

        if (x > rc.right - BTN_SIZE && y < BTN_SIZE) {
            DestroyWindow(hwnd);
            return 0;
        }

        RECT rcBtn = { 40, 100, 260, 140 };
        POINT pt = { x, y };
        if (PtInRect(&rcBtn, pt)) {
            isWaitingForKey = true;
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else {
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
        return 0;
    }
    case WM_KEYDOWN: {
        if (isWaitingForKey) {
            if (wParam != VK_CONTROL && wParam != VK_MENU && wParam != VK_SHIFT) {
                g_State.hotkeyChar = (UINT)wParam;
                isWaitingForKey = false;
                MessageBoxW(hwnd, L"Hotkey Saved!", L"Settings", MB_OK);
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        return 0;
    }
    case WM_DESTROY:
        g_State.hSettingsWnd = NULL;
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool btnApplyPressed = false;
    static bool btnAutoPressed = false;

    COLORREF clrBg = g_State.isDarkMode ? RGB(32, 32, 32) : RGB(245, 245, 245);
    COLORREF clrText = g_State.isDarkMode ? RGB(240, 240, 240) : RGB(20, 20, 20);

    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rcClient; GetClientRect(hwnd, &rcClient);

        DrawRect(hdc, rcClient, clrBg);
        DrawBorder(hdc, rcClient, RGB(80, 80, 80));

        RECT rcTitle = { 20, 10, 240, 50 };
        HFONT hFontTitle = CreateFont(22, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFontTitle);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, clrText);
        DrawTextW(hdc, L"Custom Selection Color", -1, &rcTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hOldFont);
        DeleteObject(hFontTitle);

        DrawWindowControls(hdc, rcClient.right, g_State.isDarkMode, true);

        int controlsStart = rcClient.right - (BTN_SIZE * 2); 
        
        RECT rcTheme = { controlsStart - 40, 10, controlsStart, 40 };
        HFONT hFontEmoji = CreateFont(20, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI Symbol");
        SelectObject(hdc, hFontEmoji);
        SetTextColor(hdc, clrText);
        DrawTextW(hdc, g_State.isDarkMode ? L"☀" : L"☾", -1, &rcTheme, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        RECT rcSet = { controlsStart - 80, 10, controlsStart - 40, 40 };
        DrawTextW(hdc, L"⚙", -1, &rcSet, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        DeleteObject(hFontEmoji);

        RECT rcFill = { 20, 70, 20 + BOX_WIDTH, 70 + BOX_HEIGHT };
        DrawRect(hdc, rcFill, g_State.fillColor);
        DrawBorder(hdc, rcFill, RGB(100, 100, 100));
        
        RECT rcOutline = { 210, 70, 210 + BOX_WIDTH, 70 + BOX_HEIGHT };
        DrawRect(hdc, rcOutline, g_State.borderColor);
        DrawBorder(hdc, rcOutline, RGB(100, 100, 100));

        HFONT hFontUI = CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        SelectObject(hdc, hFontUI);
        
        RECT rcL1 = rcFill; rcL1.top += 5; rcL1.left += 5;
        SetTextColor(hdc, AdjustBrightness(g_State.fillColor, 0.5));
        DrawTextW(hdc, L"Fill Color", -1, &rcL1, DT_LEFT | DT_TOP | DT_SINGLELINE);

        RECT rcL2 = rcOutline; rcL2.top += 5; rcL2.left += 5;
        SetTextColor(hdc, RGB(128, 128, 128));
        DrawTextW(hdc, L"Outline Color", -1, &rcL2, DT_LEFT | DT_TOP | DT_SINGLELINE);

        RECT rcAuto = { 20, 200, 360, 235 };
        DrawButton(hdc, rcAuto, L"Auto Color Balance", btnAutoPressed, false, g_State.isDarkMode);

        RECT rcApply = { 20, 250, 360, 300 };
        DrawButton(hdc, rcApply, L"APPLY / SAVE", btnApplyPressed, true, g_State.isDarkMode);

        DeleteObject(hFontUI);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        RECT rc; GetClientRect(hwnd, &rc);
        int controlsStart = rc.right - (BTN_SIZE * 2);

        if (x > rc.right - BTN_SIZE && y < BTN_SIZE) {
            DestroyWindow(hwnd);
            return 0;
        }
        if (x > rc.right - (BTN_SIZE * 2) && x < rc.right - BTN_SIZE && y < BTN_SIZE) {
            ShowWindow(hwnd, SW_MINIMIZE);
            return 0;
        }

        if (y < 60 && x < controlsStart - 80) { 
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            return 0;
        }

        if (x >= controlsStart - 40 && x <= controlsStart && y <= 40) {
            g_State.isDarkMode = !g_State.isDarkMode;
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else if (x >= controlsStart - 80 && x <= controlsStart - 40 && y <= 40) {
            if (!g_State.hSettingsWnd) {
                WNDCLASSW wc = {0};
                wc.lpfnWndProc = SettingsProc;
                wc.hInstance = GetModuleHandle(NULL);
                wc.lpszClassName = L"WindhawkSettings";
                wc.hCursor = LoadCursor(NULL, IDC_ARROW);
                RegisterClassW(&wc);
                
                RECT rcMain; GetWindowRect(hwnd, &rcMain);
                g_State.hSettingsWnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"WindhawkSettings", L"Key Bind", 
                    WS_POPUP | WS_VISIBLE | WS_BORDER, rcMain.left + 30, rcMain.top + 30, 300, 160, NULL, NULL, GetModuleHandle(NULL), NULL);
            }
        }
        else if (x >= 20 && x <= 170 && y >= 70 && y <= 180) {
            CHOOSECOLOR cc = {0}; cc.lStructSize = sizeof(cc); cc.hwndOwner = hwnd; cc.lpCustColors = (LPDWORD)g_CustomColors;
            cc.rgbResult = g_State.fillColor; cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            if (ChooseColor(&cc)) { g_State.fillColor = cc.rgbResult; InvalidateRect(hwnd, NULL, TRUE); }
        }
        else if (x >= 210 && x <= 360 && y >= 70 && y <= 180) {
            CHOOSECOLOR cc = {0}; cc.lStructSize = sizeof(cc); cc.hwndOwner = hwnd; cc.lpCustColors = (LPDWORD)g_CustomColors;
            cc.rgbResult = g_State.borderColor; cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            if (ChooseColor(&cc)) { g_State.borderColor = cc.rgbResult; InvalidateRect(hwnd, NULL, TRUE); }
        }
        else if (x >= 20 && x <= 360 && y >= 200 && y <= 235) {
            btnAutoPressed = true; InvalidateRect(hwnd, NULL, TRUE);
        }
        else if (x >= 20 && x <= 360 && y >= 250 && y <= 300) {
            btnApplyPressed = true; InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }

    case WM_LBUTTONUP: {
        if (btnAutoPressed) {
            btnAutoPressed = false;
            g_State.borderColor = AdjustBrightness(g_State.fillColor, 1.8);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        if (btnApplyPressed) {
            btnApplyPressed = false;
            InvalidateRect(hwnd, NULL, TRUE);

            ApplyColors();

            int result = MessageBoxW(hwnd, 
                L"Settings applied to Registry.\n\nRestart Explorer to see the Fill Color change now?\n(Required for Windows 10/11)", 
                L"Apply Complete", 
                MB_YESNO | MB_ICONQUESTION);
            
            if (result == IDYES) {
                RestartExplorer();
            } else {
                DestroyWindow(hwnd);
            }
        }
        return 0;
    }

    case WM_DESTROY:
        g_State.hMainWnd = NULL;
        if (g_State.hSettingsWnd) DestroyWindow(g_State.hSettingsWnd);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI InterfaceThread(LPVOID lpParam) {
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"WindhawkColorUI_FinalV7";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    g_State.fillColor = GetSysColor(COLOR_HIGHLIGHT);
    g_State.borderColor = GetSysColor(COLOR_HIGHLIGHTTEXT);

    int scrW = GetSystemMetrics(SM_CXSCREEN);
    int scrH = GetSystemMetrics(SM_CYSCREEN);

    g_State.hMainWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_APPWINDOW,
        L"WindhawkColorUI_FinalV7", 
        L"Custom Selection Color", 
        WS_POPUP | WS_VISIBLE | WS_BORDER,
        (scrW - UI_WIDTH) / 2, (scrH - UI_HEIGHT) / 2, 
        UI_WIDTH, UI_HEIGHT, 
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnregisterClassW(L"WindhawkColorUI_FinalV7", GetModuleHandle(NULL));
    g_State.hMainWnd = NULL;
    return 0;
}

DWORD WINAPI HotkeyThread(LPVOID lpParam) {
    while (true) {
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
            (GetAsyncKeyState(VK_MENU) & 0x8000) &&
            (GetAsyncKeyState(g_State.hotkeyChar) & 0x8000)) { 
            if (g_State.hMainWnd == NULL) {
                CreateThread(NULL, 0, InterfaceThread, NULL, 0, NULL);
                Sleep(1000); 
            }
        }
        Sleep(50);
    }
    return 0;
}

BOOL Wh_ModInit() {
    CreateThread(NULL, 0, HotkeyThread, NULL, 0, NULL);
    Wh_Log(L"Custom Selection Color V7 Loaded");
    return TRUE;
}

void Wh_ModUninit() {
    if (g_State.hMainWnd) SendMessage(g_State.hMainWnd, WM_CLOSE, 0, 0);
} 
