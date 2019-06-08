/// ElanToggle.cpp
/// jeiea, for toggle of elan touchpad function

#include "ElanToggle.h"
#include "framework.h"
using namespace std;

struct Error {
  DWORD errorCode;
  LPCTSTR message;

  static Error fromLastError(LPCTSTR message) {
    return Error{GetLastError(), message};
  }
};

wstring GetErrorString(DWORD lastError) {
  LPWSTR messageBuffer = nullptr;
  size_t size = FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPWSTR)&messageBuffer, 0, NULL);

  wstring message(messageBuffer, size);
  LocalFree(messageBuffer);

  return message;
}

void ErrorMessageBox(LPCTSTR caption, DWORD errorCode = 0) {
  DWORD code = errorCode ? errorCode : GetLastError();
  wstring msg = GetErrorString(code);
  MessageBox(nullptr, msg.data(), caption, MB_OK);
}

void DispatchUpdateEvent() {
  LPCTSTR regPath = _T("Global\\ETDCombo_UpdateRegistry");
  HANDLE eventHandle = OpenEvent(EVENT_MODIFY_STATE, FALSE, regPath);
  if (eventHandle == nullptr || eventHandle == INVALID_HANDLE_VALUE) {
    throw Error::fromLastError(_T("OpenEvent failed"));
  }

  int setResult = SetEvent(eventHandle);
  if (setResult == 0) {
    throw Error::fromLastError(_T("SetEvent failed"));
  }

  CloseHandle(eventHandle);
}

void SetTouchpadActivation(bool enable) {
  LPCTSTR elanDeviceKey = _T("Software\\Elantech\\DeviceInformation");
  LPCTSTR touchpadVal = _T("Port0_MasterEnable");
  DWORD onOff = enable;
  LSTATUS setResult =
      RegSetKeyValue(HKEY_CURRENT_USER, elanDeviceKey, touchpadVal, REG_DWORD,
                     &onOff, sizeof(onOff));
  if (setResult != ERROR_SUCCESS) {
    throw Error::fromLastError(_T("RegOpenKeyEx failed"));
  }
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                       _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine,
                       _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  try {
    SetTouchpadActivation(false);
    DispatchUpdateEvent();
  }
  catch (Error e) {
    ErrorMessageBox(e.message, e.errorCode);
    return 1;
  }
  return 0;
}
