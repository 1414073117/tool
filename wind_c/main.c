#include <stdio.h>
// #include <Windows.h>

// LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
// {
//     switch (message)
//     {
//     case WM_PAINT:
//         {
//             PAINTSTRUCT ps;
//             HDC hdc = BeginPaint(hWnd, &ps);
//             RECT rc = { 50, 50, 200, 200 };
//             HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
//             FillRect(hdc, &rc, hBrush);
//             DeleteObject(hBrush);
//             EndPaint(hWnd, &ps);
//         }
//         break;
//     case WM_KEYDOWN:
//         if (wParam == VK_ESCAPE)
//         {
//             PostQuitMessage(0);
//         }
//         break;
//     case WM_DESTROY:
//         PostQuitMessage(0);
//         break;
//     default:
//         return DefWindowProc(hWnd, message, wParam, lParam);
//     }
//     return 0;
// }

// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
// {
//     WNDCLASS wc = { 0 };
//     wc.lpfnWndProc = WndProc;
//     wc.hInstance = hInstance;
//     wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//     wc.lpszClassName = "MyWindowClass";
//     RegisterClass(&wc);

//     HWND hWnd = CreateWindow("MyWindowClass", "Hello, World!", WS_OVERLAPPEDWINDOW,
//         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

//     ShowWindow(hWnd, nCmdShow);
//     UpdateWindow(hWnd);

//     MSG msg = { 0 };
//     while (GetMessage(&msg, NULL, 0, 0))
//     {
//         TranslateMessage(&msg);
//         DispatchMessage(&msg);
//     }

//     return (int)msg.wParam;
// }


int main() {
    int x = 10;
    x =  x * 5;
    printf("Hello, World!\n");
    printf("x %d %s\n", x, NULL);
    return 0;
}
