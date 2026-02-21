/*
 * Author: ItzCooode
 * GitHub: https://github.com/Itz-Cooode
 */

#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <random>
#include <thread>
#include <atomic>
#include <chrono>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#pragma comment(lib, "d3d11.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

enum class TargetColor {
    YELLOW,
    RED,
    PURPLE,
    CUSTOM
};

struct Config {
    int boxSize = 2;
    int colorTolerance = 80;
    TargetColor targetColor = TargetColor::YELLOW;
    COLORREF customColor = 0xFFFF00;
    int minDelay = 2;
    int maxDelay = 7;
    int sleepDelay = 160;
    int holdButton = VK_XBUTTON1;
    int holdButton2 = VK_MENU;
    
    COLORREF GetColor() const {
        switch (targetColor) {
            case TargetColor::YELLOW: return 0xFFFF00;
            case TargetColor::RED: return 0xFF0000;
            case TargetColor::PURPLE: return 0xFA64FA;
            case TargetColor::CUSTOM: return customColor;
            default: return 0xFFFF00;
        }
    }
    
    const char* GetColorName() const {
        switch (targetColor) {
            case TargetColor::YELLOW: return "Yellow";
            case TargetColor::RED: return "Red";
            case TargetColor::PURPLE: return "Purple";
            case TargetColor::CUSTOM: return "Custom";
            default: return "Unknown";
        }
    }
};

Config config;

std::atomic<bool> isRunning(true);
std::atomic<bool> holdMode(false);
std::atomic<bool> autoMode(false);
std::atomic<bool> standardMode(true);
std::atomic<bool> letsCrouch(false);
std::atomic<bool> stopMove(false);

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

int searchLeft, searchRight, searchTop, searchBottom;

void UpdateSearchArea() {
    searchLeft = screenWidth / 2 - config.boxSize;
    searchRight = screenWidth / 2 + config.boxSize;
    searchTop = screenHeight / 3 - config.boxSize;
    searchBottom = screenHeight / 2 + config.boxSize;
}

std::random_device rd;
std::mt19937 gen(rd());

int GetRandomDelay() {
    std::uniform_int_distribution<> delayDist(config.minDelay, config.maxDelay);
    return delayDist(gen);
}

void LoadConfig();
void SaveConfig();
void Crouch();
void CrouchRelease();
void ReleaseMove();
bool PixelSearch(int& foundX, int& foundY);
void StandardEngine();
void VandalEngine();
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

HHOOK keyboardHook = NULL;

std::string GetKeyName(int vkCode) {
    static std::map<int, std::string> keyNames = {
        {VK_LBUTTON, "LButton"}, {VK_RBUTTON, "RButton"}, {VK_MBUTTON, "MButton"},
        {VK_XBUTTON1, "Mouse4"}, {VK_XBUTTON2, "Mouse5"},
        {VK_SHIFT, "Shift"}, {VK_CONTROL, "Ctrl"}, {VK_MENU, "Alt"},
        {VK_LSHIFT, "LShift"}, {VK_RSHIFT, "RShift"},
        {VK_LCONTROL, "LCtrl"}, {VK_RCONTROL, "RCtrl"},
        {VK_LMENU, "LAlt"}, {VK_RMENU, "RAlt"},
        {VK_BACK, "Backspace"}, {VK_TAB, "Tab"}, {VK_RETURN, "Enter"},
        {VK_PAUSE, "Pause"}, {VK_CAPITAL, "CapsLock"}, {VK_ESCAPE, "Esc"},
        {VK_SPACE, "Space"}, {VK_PRIOR, "PageUp"}, {VK_NEXT, "PageDown"},
        {VK_END, "End"}, {VK_HOME, "Home"},
        {VK_LEFT, "Left"}, {VK_UP, "Up"}, {VK_RIGHT, "Right"}, {VK_DOWN, "Down"},
        {VK_SNAPSHOT, "PrintScreen"}, {VK_INSERT, "Insert"}, {VK_DELETE, "Delete"},
        {VK_F1, "F1"}, {VK_F2, "F2"}, {VK_F3, "F3"}, {VK_F4, "F4"},
        {VK_F5, "F5"}, {VK_F6, "F6"}, {VK_F7, "F7"}, {VK_F8, "F8"},
        {VK_F9, "F9"}, {VK_F10, "F10"}, {VK_F11, "F11"}, {VK_F12, "F12"},
        {VK_NUMPAD0, "Num0"}, {VK_NUMPAD1, "Num1"}, {VK_NUMPAD2, "Num2"},
        {VK_NUMPAD3, "Num3"}, {VK_NUMPAD4, "Num4"}, {VK_NUMPAD5, "Num5"},
        {VK_NUMPAD6, "Num6"}, {VK_NUMPAD7, "Num7"}, {VK_NUMPAD8, "Num8"},
        {VK_NUMPAD9, "Num9"}, {VK_MULTIPLY, "Num*"}, {VK_ADD, "Num+"},
        {VK_SUBTRACT, "Num-"}, {VK_DECIMAL, "Num."}, {VK_DIVIDE, "Num/"},
        {'0', "0"}, {'1', "1"}, {'2', "2"}, {'3', "3"}, {'4', "4"},
        {'5', "5"}, {'6', "6"}, {'7', "7"}, {'8', "8"}, {'9', "9"},
        {VK_OEM_1, ";"}, {VK_OEM_PLUS, "="}, {VK_OEM_COMMA, ","},
        {VK_OEM_MINUS, "-"}, {VK_OEM_PERIOD, "."}, {VK_OEM_2, "/"},
        {VK_OEM_3, "`"}, {VK_OEM_4, "["}, {VK_OEM_5, "\\"},
        {VK_OEM_6, "]"}, {VK_OEM_7, "'"},
        {VK_NUMLOCK, "NumLock"}, {VK_SCROLL, "ScrollLock"}
    };
    
    if (keyNames.find(vkCode) != keyNames.end()) {
        return keyNames[vkCode];
    }
    
    if (vkCode >= 'A' && vkCode <= 'Z') {
        return std::string(1, (char)vkCode);
    }
    
    return "VK:" + std::to_string(vkCode);
}

void LoadConfig() {
    std::ifstream file("config.ini");
    if (!file.is_open()) {
        UpdateSearchArea();
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (key == "boxSize") config.boxSize = std::stoi(value);
        else if (key == "colorTolerance") config.colorTolerance = std::stoi(value);
        else if (key == "targetColor") {
            if (value == "yellow") config.targetColor = TargetColor::YELLOW;
            else if (value == "red") config.targetColor = TargetColor::RED;
            else if (value == "purple") config.targetColor = TargetColor::PURPLE;
            else if (value == "custom") config.targetColor = TargetColor::CUSTOM;
        }
        else if (key == "customColor") config.customColor = std::stoul(value, nullptr, 16);
        else if (key == "minDelay") config.minDelay = std::stoi(value);
        else if (key == "maxDelay") config.maxDelay = std::stoi(value);
        else if (key == "sleepDelay") config.sleepDelay = std::stoi(value);
        else if (key == "holdButton") config.holdButton = std::stoi(value);
        else if (key == "holdButton2") config.holdButton2 = std::stoi(value);
    }
    
    file.close();
    UpdateSearchArea();
}

void SaveConfig() {
    std::ofstream file("config.ini");
    if (!file.is_open()) return;
    
    file << "# Valorant Aim Assist Configuration\n";
    file << "# Created by ItzCooode\n\n";
    file << "boxSize=" << config.boxSize << "\n";
    file << "colorTolerance=" << config.colorTolerance << "\n";
    
    std::string colorName;
    switch (config.targetColor) {
        case TargetColor::YELLOW: colorName = "yellow"; break;
        case TargetColor::RED: colorName = "red"; break;
        case TargetColor::PURPLE: colorName = "purple"; break;
        case TargetColor::CUSTOM: colorName = "custom"; break;
    }
    file << "targetColor=" << colorName << "\n";
    file << "customColor=0x" << std::hex << config.customColor << std::dec << "\n";
    file << "minDelay=" << config.minDelay << "\n";
    file << "maxDelay=" << config.maxDelay << "\n";
    file << "sleepDelay=" << config.sleepDelay << "\n";
    file << "holdButton=" << config.holdButton << "\n";
    file << "holdButton2=" << config.holdButton2 << "\n";
    
    file.close();
}

void Crouch() {
    keybd_event('K', 0, 0, 0);
}

void CrouchRelease() {
    keybd_event('K', 0, KEYEVENTF_KEYUP, 0);
}

void ReleaseMove() {
    keybd_event('Z', 0, KEYEVENTF_KEYUP, 0);
    keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
    keybd_event('S', 0, KEYEVENTF_KEYUP, 0);
    keybd_event('D', 0, KEYEVENTF_KEYUP, 0);
}

bool ColorMatch(COLORREF color1, COLORREF color2, int tolerance) {
    int r1 = GetRValue(color1);
    int g1 = GetGValue(color1);
    int b1 = GetBValue(color1);
    
    int r2 = GetRValue(color2);
    int g2 = GetGValue(color2);
    int b2 = GetBValue(color2);
    
    return (abs(r1 - r2) <= tolerance && 
            abs(g1 - g2) <= tolerance && 
            abs(b1 - b2) <= tolerance);
}

bool PixelSearch(int& foundX, int& foundY) {
    HDC hdc = GetDC(NULL);
    COLORREF targetColor = config.GetColor();
    
    for (int y = searchTop; y <= searchBottom; y++) {
        for (int x = searchLeft; x <= searchRight; x++) {
            COLORREF pixel = GetPixel(hdc, x, y);
            if (ColorMatch(pixel, targetColor, config.colorTolerance)) {
                foundX = x;
                foundY = y;
                ReleaseDC(NULL, hdc);
                return true;
            }
        }
    }
    
    ReleaseDC(NULL, hdc);
    return false;
}

void StandardEngine() {
    int foundX = 0, foundY = 0;
    
    if (PixelSearch(foundX, foundY)) {
        if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
            int randomDelay = GetRandomDelay();
            std::this_thread::sleep_for(std::chrono::milliseconds(randomDelay));
            
            if (stopMove) {
                ReleaseMove();
            }
            
            HWND valorantWindow = FindWindowA(NULL, "VALORANT");
            if (valorantWindow) {
                PostMessage(valorantWindow, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(foundX, foundY));
                PostMessage(valorantWindow, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(foundX, foundY));
            }
            
            if (letsCrouch) {
                Crouch();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            if (letsCrouch) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                CrouchRelease();
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(config.sleepDelay));
        }
    }
}

void VandalEngine() {
    int foundX = 0, foundY = 0;
    
    if (PixelSearch(foundX, foundY)) {
        if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
            int randomDelay = GetRandomDelay();
            std::this_thread::sleep_for(std::chrono::milliseconds(randomDelay));
            
            if (stopMove) {
                ReleaseMove();
            }
            
            HWND valorantWindow = FindWindowA(NULL, "VALORANT");
            if (valorantWindow) {
                PostMessage(valorantWindow, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(foundX, foundY));
            }
            
            if (letsCrouch) {
                Crouch();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
            if (letsCrouch) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                CrouchRelease();
            }
            
            if (valorantWindow) {
                PostMessage(valorantWindow, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(foundX, foundY));
            }
        }
    }
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        
        if (wParam == WM_KEYDOWN) {
            switch (pKeyboard->vkCode) {
                case VK_F1:
                    holdMode = true;
                    autoMode = false;
                    Beep(400, 200);
                    break;
                    
                case VK_F2:
                    holdMode = false;
                    autoMode = true;
                    Beep(300, 400);
                    break;
                    
                case VK_F3:
                    standardMode = true;
                    Beep(300, 200);
                    break;
                    
                case VK_F4:
                    standardMode = false;
                    Beep(300, 200);
                    break;
                    
                case VK_F5:
                    letsCrouch = !letsCrouch;
                    Beep(letsCrouch ? 400 : 200, letsCrouch ? 400 : 200);
                    break;
                    
                case VK_F6:
                    stopMove = !stopMove;
                    Beep(stopMove ? 400 : 200, stopMove ? 400 : 200);
                    break;
                    
                case VK_DOWN:
                    holdMode = false;
                    autoMode = false;
                    Beep(400, 300);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    Beep(300, 400);
                    break;
            }
        }
    }
    
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

void MainLoop() {
    while (isRunning) {
        if (autoMode) {
            if (standardMode) {
                StandardEngine();
            } else {
                VandalEngine();
            }
        } else if (holdMode) {
            bool keyPressed = (GetAsyncKeyState(config.holdButton) & 0x8000) || 
                            (GetAsyncKeyState(config.holdButton2) & 0x8000);
            
            if (keyPressed) {
                if (standardMode) {
                    StandardEngine();
                } else {
                    VandalEngine();
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, 
                                                 featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, 
                                                 &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
    return true;
}

void CleanupDeviceD3D() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        if (g_pd3dDevice != nullptr) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    LoadConfig();
    
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), 
                       nullptr, nullptr, nullptr, nullptr, L"ValorantAimAssist", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Valorant Aim Assist - by ItzCooode", 
                                 WS_OVERLAPPEDWINDOW, 100, 100, 800, 650, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

    std::thread mainThread(MainLoop);

    bool showDisclaimer = true;
    int colorSelection = (int)config.targetColor;
    char customColorHex[16] = "";
    sprintf_s(customColorHex, "%06X", config.customColor);

    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
    
    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (showDisclaimer) {
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("DISCLAIMER", &showDisclaimer, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
            
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "WARNING: USE AT YOUR OWN RISK!");
            ImGui::Separator();
            ImGui::TextWrapped("This software is provided \"as is\" without warranty of any kind.");
            ImGui::TextWrapped("The author (ItzCooode) is NOT responsible for:");
            ImGui::BulletText("Game bans or account suspensions");
            ImGui::BulletText("Any penalties from anti-cheat systems");
            ImGui::BulletText("Any other consequences of using this software");
            ImGui::Separator();
            ImGui::TextWrapped("This project is for EDUCATIONAL PURPOSES ONLY.");
            ImGui::Separator();
            
            if (ImGui::Button("I Understand and Accept the Risks", ImVec2(300, 30))) {
                showDisclaimer = false;
            }
            
            ImGui::End();
        } else {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
            ImGui::Begin("Valorant Aim Assist", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            ImGui::Text("Valorant Aim Assist - Created by ItzCooode");
            ImGui::Separator();

            ImGui::Text("Status:");
            ImGui::SameLine(150);
            if (autoMode) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "AUTO MODE ACTIVE");
            } else if (holdMode) {
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "HOLD MODE ACTIVE");
            } else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "INACTIVE");
            }

            ImGui::Text("Engine:");
            ImGui::SameLine(150);
            ImGui::Text(standardMode ? "Standard Mode" : "Vandal Mode");

            ImGui::Text("Crouch:");
            ImGui::SameLine(150);
            ImGui::Text(letsCrouch ? "Enabled" : "Disabled");

            ImGui::Text("Stop Move:");
            ImGui::SameLine(150);
            ImGui::Text(stopMove ? "Enabled" : "Disabled");

            ImGui::Separator();

            ImGui::Text("Quick Controls:");
            if (ImGui::Button("Hold Mode (F1)", ImVec2(150, 30))) {
                holdMode = true;
                autoMode = false;
                Beep(400, 200);
            }
            ImGui::SameLine();
            if (ImGui::Button("Auto Mode (F2)", ImVec2(150, 30))) {
                holdMode = false;
                autoMode = true;
                Beep(300, 400);
            }
            ImGui::SameLine();
            if (ImGui::Button("Pause (Down)", ImVec2(150, 30))) {
                holdMode = false;
                autoMode = false;
                Beep(400, 300);
            }

            if (ImGui::Button("Standard Mode (F3)", ImVec2(150, 30))) {
                standardMode = true;
                Beep(300, 200);
            }
            ImGui::SameLine();
            if (ImGui::Button("Vandal Mode (F4)", ImVec2(150, 30))) {
                standardMode = false;
                Beep(300, 200);
            }

            if (ImGui::Button(letsCrouch ? "Disable Crouch (F5)" : "Enable Crouch (F5)", ImVec2(150, 30))) {
                letsCrouch = !letsCrouch;
                Beep(letsCrouch ? 400 : 200, 200);
            }
            ImGui::SameLine();
            if (ImGui::Button(stopMove ? "Disable Stop Move (F6)" : "Enable Stop Move (F6)", ImVec2(150, 30))) {
                stopMove = !stopMove;
                Beep(stopMove ? 400 : 200, 200);
            }

            ImGui::Separator();

            ImGui::Text("Settings:");
            
            if (ImGui::SliderInt("Box Size", &config.boxSize, 1, 50)) {
                UpdateSearchArea();
            }
            
            ImGui::SliderInt("Color Tolerance", &config.colorTolerance, 0, 255);
            
            const char* colors[] = { "Yellow", "Red", "Purple", "Custom" };
            if (ImGui::Combo("Target Color", &colorSelection, colors, IM_ARRAYSIZE(colors))) {
                config.targetColor = (TargetColor)colorSelection;
            }
            
            if (config.targetColor == TargetColor::CUSTOM) {
                ImGui::InputText("Custom Color (Hex)", customColorHex, IM_ARRAYSIZE(customColorHex));
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    config.customColor = std::stoul(customColorHex, nullptr, 16);
                }
            }
            
            ImGui::SliderInt("Min Delay (ms)", &config.minDelay, 0, 50);
            ImGui::SliderInt("Max Delay (ms)", &config.maxDelay, 0, 50);
            ImGui::SliderInt("Sleep Delay (ms)", &config.sleepDelay, 50, 500);
            
            ImGui::InputInt("Hold Button 1 (VK)", &config.holdButton);
            ImGui::SameLine();
            ImGui::Text("(%s)", GetKeyName(config.holdButton).c_str());
            
            ImGui::InputInt("Hold Button 2 (VK)", &config.holdButton2);
            ImGui::SameLine();
            ImGui::Text("(%s)", GetKeyName(config.holdButton2).c_str());

            ImGui::Separator();

            if (ImGui::Button("Save Config", ImVec2(150, 30))) {
                SaveConfig();
                Beep(600, 100);
            }
            ImGui::SameLine();
            if (ImGui::Button("Load Config", ImVec2(150, 30))) {
                LoadConfig();
                colorSelection = (int)config.targetColor;
                sprintf_s(customColorHex, "%06X", config.customColor);
                Beep(600, 100);
            }

            ImGui::Separator();
            ImGui::TextWrapped("Hotkeys: F1=Hold, F2=Auto, F3=Standard, F4=Vandal, F5=Crouch, F6=StopMove, Down=Pause");
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Created by ItzCooode | Use at your own risk!");

            ImGui::End();
        }

        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x, clear_color.y, clear_color.z, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    isRunning = false;
    mainThread.join();
    
    UnhookWindowsHookEx(keyboardHook);
    
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}
