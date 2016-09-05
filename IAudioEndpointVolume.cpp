#include <Windows.h>
#include <MMDeviceAPI.h>
#include <EndpointVolume.h>
#include <sstream>

#define MAX_VOL  100

HRESULT hr = S_OK;
IMMDevice *defaultDevice = NULL;
IMMDeviceEnumerator *deviceEnumerator = NULL;;
IMMDeviceCollection *deviceCollection = NULL;
IAudioEndpointVolume *endpointVolume;

LONG   screenSize_X = 640;
LONG   screenSize_Y = 480;

int iPos;
float fMasterVolume;

void writeText(HDC screen)
{
	TextOut(screen, 10, 10, TEXT("GREETING"), 8); 
}

///////////////////////////////
///////////////////////////////
// A callback to handle Windows messages as they happen
LRESULT CALLBACK wndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
	switch (msg)
	{
	case WM_SYSKEYDOWN:
	{
		int virtual_code = (int)w;

		if (GetKeyState(VK_MENU) & 0x8000) {

			switch (virtual_code)
			{
			case VK_UP: 
				
				float fVolume;
					
				//nVolume = SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME, TBM_GETPOS, 0, 0);
				if (iPos < 100)
					iPos ++;

				std::wstringstream wss;
				wss << iPos;
				std::wstring ws = wss.str();
				HDC hdc = GetDC(wnd);
				TextOut(hdc, 10, 10, ws.c_str(), 8);

				fVolume = iPos / (float)100;
				hr = endpointVolume->SetMute(FALSE, &GUID_NULL);
				hr = endpointVolume->SetMasterVolumeLevelScalar(fVolume, &GUID_NULL);
				
				//endpointVolume->VolumeStepUp(NULL);
				
				break;
			}
		}
	}break;

	case WM_KEYDOWN:
	{
		int virtual_code = (int)w;

		switch (virtual_code)
		{
		case VK_UP: {
			/*HDC hdc = GetDC(wnd);   // Get the screen DC
			TextOut(hdc, 10, 10, TEXT("Key down"), 8);*/
			//if (GetKeyState(VK_MENU) & 0x8000) {
				// If ALT is pressed down.
				// The GetKeyState message takes a virtual - key code as input and returns a set of bit flags(actually just two flags).
				// The value 0x8000 contains the bit flag that tests whether the key is currently pressed.
				

				hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
				hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
				deviceEnumerator->Release();
				deviceEnumerator = NULL;
				hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);

				if (FAILED(hr))
				{
					return (0);
				}
				//endpointVolume->VolumeStepUp(NULL);
			//}
		} break;
		default: break;
			return (0);
		}
	}
		// we are interested in WM_PAINT, as that is how we draw
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC screen = BeginPaint(wnd, &ps);   // Get the screen DC
		//drawImage(screen);                  // draw our image to our screen DC
		writeText(screen);
		EndPaint(wnd, &ps);                  // clean up
	}break;

	// we are also interested in the WM_DESTROY message, as that lets us know when to close the window
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	// for everything else, let the default window message handler do its thing
	return DefWindowProc(wnd, msg, w, l);
}


///////////////////////////////
///////////////////////////////
// A function to create the window and get it set up
HWND createWindow(HINSTANCE inst)
{
	WNDCLASSEX wc = { 0 };        // create a WNDCLASSEX struct and zero it
	wc.cbSize = sizeof(WNDCLASSEX);     // tell windows the size of this struct
	wc.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));        // tell it to use the normal arrow cursor for this window
	wc.hInstance = inst;                   // give it our program instance
	wc.lpfnWndProc = wndProc;                // tell it to use our wndProc function to handle messages
	wc.lpszClassName = TEXT("Image");   // give this window class a name.

	RegisterClassEx(&wc);           // register our window class with Windows

									// the style of the window we want... we want a normal window but do not want it resizable.
	int style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;    // normal overlapped window with a caption and a system menu (the X to close)

															// Figure out how big we need to make the window so that the CLIENT area (the part we will be drawing to) is
															//  the desired size
	RECT rc = { 0,0,screenSize_X,screenSize_Y };      // desired rect
	AdjustWindowRect(&rc, style, FALSE);              // adjust the rect with the given style, FALSE because there is no menu

	/*return CreateWindow(            // create the window
	TEXT("DisplayImage"),       // the name of the window class to use for this window (the one we just registered)
	NULL,   // the text to appear on the title of the window
	style | WS_VISIBLE,         // the style of this window (OR it with WS_VISIBLE so it actually becomes visible immediately)
	100, 100,                    // create it at position 100,100
	rc.right - rc.left,         // width of the window we want
	rc.bottom - rc.top,         // height of the window
	NULL, NULL,                  // no parent window, no menu
	inst,                       // our program instance
	NULL);                      // no extra parameter*/

	return CreateWindow(TEXT("Image"), TEXT("Volume Control"), style | WS_VISIBLE , 100, 100, screenSize_X, screenSize_Y, NULL, NULL, inst, NULL);

}


///////////////////////////////
///////////////////////////////
// The actual entry point for the program!
//  This is Windows' version of the 'main' function:
int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
	// create our window
	HWND wnd = createWindow(inst);
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
	deviceEnumerator->Release();
	deviceEnumerator = NULL;
	hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);

	if (FAILED(hr))
	{
		exit(0);
	}
	else if (SUCCEEDED(endpointVolume->GetMasterVolumeLevelScalar(&fMasterVolume))) {
		iPos = (int)(100 * fMasterVolume);
		
	}
	else {
		exit(0);
	}
	//SetWindowLong(wnd, GWL_STYLE, 0);
	ShowWindow(wnd, SW_SHOW); //display window

							  // Do the message pump!  keep polling for messages (and respond to them)
							  //  until the user closes the window.
	MSG msg;
	while (GetMessage(&msg, wnd, 0, 0)) // while we are getting non-WM_QUIT messages...
	{
		TranslateMessage(&msg);     // translate them
		DispatchMessage(&msg);      // and dispatch them (our wndProc will process them)
	}
}